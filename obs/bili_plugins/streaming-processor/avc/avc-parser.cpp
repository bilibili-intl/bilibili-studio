#include "avc-parser.h"
#include <algorithm>
#include <deque>

#include "avc-nalu.h"
#include "avc-sei-util.h"

extern "C" {
#define class Class
#include "lsmash.h"
#include "common/internal.h"
#include "codecs/h264.h"
#include "codecs/nalu.h"
#undef class
};

static void AssignAVCSPS(AVCSPS& lsps, h264_sps_t& rsps)
{
    lsps.w = rsps.cropped_width;
    lsps.h = rsps.cropped_height;
    lsps.sar_w = rsps.vui.sar_width;
    lsps.sar_h = rsps.vui.sar_height;
    lsps.max_frame_num = rsps.MaxFrameNum;
    lsps.max_poc_lsb = rsps.MaxPicOrderCntLsb;
    lsps.bits = rsps.bit_depth_luma_minus8 + 8;
}

class AVCParser : public IAVCParser
{
    h264_info_t h_;

    std::vector<uint8_t> avccSPS_;

public:
    AVCParser()
    {
        h264_setup_parser(&h_, 1);
    }

    ~AVCParser()
    {
        h264_cleanup_parser(&h_);
    }

    std::vector<AVCNalu> ParseAVCC(char* buf, int len) override
    {
        std::vector<AVCNalu> retval;

        auto bs = lsmash_bs_create();
        lsmash_bs_import_data(bs, buf, len);
        auto b = lsmash_bits_create(bs);

        [&]() {
            // ISO IEC 14496-15-2019 P21
            // AVCDecoderConfigurationRecord
            uint8_t configurationVersion = lsmash_bits_get(b, 8);
            assert(configurationVersion == 1);
            uint8_t AVCProfileIndication = lsmash_bits_get(b, 8);
            uint8_t profile_compatibility = lsmash_bits_get(b, 8);
            uint8_t AVCLevelIndication = lsmash_bits_get(b, 8);
            uint8_t reserved = lsmash_bits_get(b, 6);
            assert(reserved == 0b111111);
            uint8_t lengthSizeMinusOne = lsmash_bits_get(b, 2);
            assert(lengthSizeMinusOne + 1 == 4);
            reserved = lsmash_bits_get(b, 3);
            assert(reserved == 0b111);
            uint8_t numOfSPS = lsmash_bits_get(b, 5);
            assert(numOfSPS == 1);
            for (int i = 0; i < numOfSPS; ++i)
            {
                uint16_t spsLen = lsmash_bits_get(b, 16);
                std::vector<uint8_t> ebsp(spsLen);
                std::vector<uint8_t> buf(spsLen);
                if (spsLen != lsmash_bs_get_bytes_ex(bs, spsLen, ebsp.data()))
                    return;
                while(!ebsp.empty() && ebsp.back() == 0) ebsp.pop_back();
                int ret = h264_parse_sps(&h_, buf.data(), ebsp.data() + 1, ebsp.size() - 1); // nal idc & unit type = 1 byte
                assert(ret == 0);
                if (ret != 0)
                    return;

                avccSPS_.resize(ebsp.size());
                std::copy(ebsp.begin(), ebsp.end(), avccSPS_.begin());

                AVCNalu result;
                result.sps = AVCSPS();
                AssignAVCSPS(result.sps.value(), h_.sps);
                retval.push_back(result);
            }
            uint8_t numOfPPS = lsmash_bits_get(b, 8);
            for (int i = 0; i < numOfPPS; ++i)
            {
                uint16_t ppsLen = lsmash_bits_get(b, 16);
                std::vector<uint8_t> ebsp(ppsLen);
                std::vector<uint8_t> buf(ppsLen);
                if (ppsLen != lsmash_bs_get_bytes_ex(bs, ppsLen, ebsp.data()))
                    return;
                while(!ebsp.empty() && ebsp.back() == 0) ebsp.pop_back();
                int ret = h264_parse_pps(&h_, buf.data(), ebsp.data() + 1, ebsp.size() - 1); // nal idc & unit type = 1 byte
                assert(ret == 0);
                if (ret != 0)
                    return;
            }
            if (AVCProfileIndication != 66
                && AVCProfileIndication != 77
                && AVCProfileIndication != 88)
            {
                // sps ext, not implemented
            }
        }();

        lsmash_bits_cleanup(b);
        lsmash_bs_cleanup(bs);
        return std::move(retval);
    }

    std::vector<AVCNalu> ParseNalu(char* buf, int len) override
    {
        std::vector<AVCNalu> retval;

        auto bs = lsmash_bs_create();
        lsmash_bs_import_data(bs, buf, len);

        for (;;)
        {
            auto nalu_size = lsmash_bs_get_be32(bs);
            if (bs->eob)
                break;
            // to do: check size too large
            std::vector<uint8_t> nalu(nalu_size);
            std::vector<uint8_t> buf(nalu_size);
            if (lsmash_bs_get_bytes_ex(bs, nalu_size, nalu.data()) != nalu_size)
                break;

            h264_nalu_header_t header;
            header.forbidden_zero_bit = nalu[0] >> 7;
            header.nal_ref_idc = (nalu[0] >> 5) & 0b11;
            header.nal_unit_type = nalu[0] & 0b11111;
            header.length = 1;
            if (header.nal_unit_type == 1 || header.nal_unit_type == 5) // non-idr or idr
            {
                if (h264_parse_slice(&h_, &header, buf.data(), nalu.data() + header.length, nalu.size() - header.length) == 0)
                {
                    AVCNalu result;
                    result.slice = AVCSlice();
                    auto& slice = result.slice.value();
                    slice.ref_idc = header.nal_ref_idc;
                    slice.slice_type = h_.slice.type;
                    slice.field_pic_flag = h_.slice.field_pic_flag;
                    if (h_.slice.IdrPicFlag)
                        slice.idr_pic_id = h_.slice.idr_pic_id;
                    else
                        slice.idr_pic_id = -1;
                    slice.frame_num = h_.slice.frame_num;
                    slice.pic_order_cnt_lsb = h_.slice.pic_order_cnt_lsb;
                    retval.push_back(result);
                }
            }
            else if (header.nal_unit_type == 7 || header.nal_unit_type == 8)
            {
                auto tmpnal = nalu;
                int ret;
                if (header.nal_unit_type == 7)
                {
                    while(!tmpnal.empty() && tmpnal.back() == 0) tmpnal.pop_back();
                    if (!std::equal(avccSPS_.begin(), avccSPS_.end(), tmpnal.begin(), tmpnal.end()))
                    {
                        ret = h264_parse_sps(&h_, buf.data(), tmpnal.data() + header.length, tmpnal.size() - header.length);
                        if (ret == 0)
                        {
                            AVCNalu result;
                            result.sps = AVCSPS();
                            AssignAVCSPS(result.sps.value(), h_.sps);
                            retval.push_back(result);
                        }
                    }
                }
                else if (header.nal_unit_type == 8)
                {
                    while(!tmpnal.empty() && tmpnal.back() == 0) tmpnal.pop_back();
                    ret = h264_parse_pps(&h_, buf.data(), tmpnal.data() + header.length, tmpnal.size() - header.length);
                    if (ret == 0)
                    {
                        AVCNalu result;
                        result.pps = AVCPPS();
                        retval.push_back(result);
                    }
                }
            }
            else if (header.nal_unit_type == 6) // sei
            {
                AVCNalu result;
                result.sei = AVCSEI();

                std::vector<uint8_t> buf;
                if (ebsp_to_rbsp(nalu.data() + 1, nalu.data() + nalu.size(), std::back_inserter(buf)))
                {
                    auto it = buf.begin();
                    result.sei.value().sei_msgs = extract_seis(it, buf.end());
                }
                retval.push_back(result);
            }
            else
            {
                AVCNalu result;
                result.others = AVCOther();
                result.others.value().nal_unit_type = header.nal_unit_type;
                retval.push_back(result);
            }
            
        };
        lsmash_bs_cleanup(bs);

        return retval;
    }
};

IAVCParser* CreateAVCParser()
{
    return new AVCParser();
}
