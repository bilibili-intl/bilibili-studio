#if 1

#include "live-mask.h"

#include "avc/avc-nalu.h"
#include "avc/avc-sei-util.h"
#include "lib/live-mask-potrace.hpp"


// 因为弹幕蒙板是绘制能显示的部分
// 所以一开始先顺时针整个画面圈起来，然后逆时针抠掉

class PotraceAlphaToSvgBin : public AlphaToSvgBin
{
    std::vector<uint8_t> raw_svgbin_;

public:
    ~PotraceAlphaToSvgBin()
    {
    }

    bool AppendHeader(int width, int height)
    {
        using namespace potrace_svgbin;

        encode_ctx ctx;
        auto it = std::back_inserter(raw_svgbin_);
        encode_header(ctx, it, width, height, 0.0f, 0.0f, 1.0f, 1.0f);
        potrace_dpoint_t pt;
        pt.x = 0; pt.y = 0;
        encode_moveto(ctx, it, pt);
        pt.x = width; pt.y = 0;
        encode_lineto(ctx, it, pt);
        pt.x = width; pt.y = height;
        encode_lineto(ctx, it, pt);
        pt.x = 0; pt.y = height;
        encode_lineto(ctx, it, pt);
        pt.x = 0; pt.y = 0;
        encode_lineto(ctx, it, pt);
        encode_close(ctx, it);
        return true;
    }

    bool AppendMask(uint8_t* data, int word_bytes, int width, int height, int stride, float(*transform)[4]) override
    {
        using namespace potrace_svgbin;

        encode_ctx ctx;
        if (transform)
        {
            auto p_src = &transform[0][0];
            auto p_dst = ctx.transform.data();
            for (int i = 0; i < 16; ++i)
                p_dst[i] = p_src[i];
        }

        bool should_flip = false;

        // 检测变换矩阵是否会改变路径的正反面
        bool direction_changed = false;
        {
            using namespace boost::numeric::ublas;
            vector3 trans_x_axis = project(prod(make_vector4(1.0, 0, 0, 0.0), ctx.transform), range(0, 3));
            vector3 trans_y_axis = project(prod(make_vector4(0, 1.0, 0, 0.0), ctx.transform), range(0, 3));
            if (inner_prod(
                    cross(trans_x_axis, trans_y_axis),
                    make_vector3(0, 0, 1.0)
                ) < 0
            )
            {
                direction_changed = true;
            }
        }

        // 因为要求的结果是“弹幕能显示的地方”而不是“弹幕不能显示的地方”
        // 所以现行做法是先全部标记为可以显示，然后把需要防挡的地方抠掉
        // potrace出来是正向的
        // 参数中的变换矩阵如果不会改变方向，那么要手动改一下方向
        if (!direction_changed)
        {
            matrix4 pre_trans = ident_matrix4(4);
            pre_trans(1, 1) = -1.0;
            pre_trans(3, 1) = height;
            ctx.transform = prod(pre_trans, ctx.transform);

            should_flip = true;
        }

        // todo: median blur
        // byte to bit convert
        int pt_word_bits = sizeof(potrace_word) * 8;
        int input_stride = (width + pt_word_bits - 1) / pt_word_bits;
        std::vector<potrace_word> input_buf(input_stride * height);

        for (int y = 0; y < height; ++y)
        {
            uint8_t* input_byte_line;
            if (should_flip)
                input_byte_line = data + stride * (height - 1 - y);
            else
                input_byte_line = data + stride * y;
            potrace_word* input_bit_line = input_buf.data() + input_stride * y;
            for (int x = 0; x < width; ++x)
            {
                if (!(input_byte_line[word_bytes * x] & 0xf0)) // alpha < 0x80
                    input_bit_line[x / pt_word_bits] |= 1 << ((pt_word_bits - 1) - (x % pt_word_bits));
            }
        }

        // trace
        potrace_bitmap_t bm;
        bm.w = width;
        bm.h = height;
        bm.dy = input_stride;
        bm.map = input_buf.data();

        potrace_param_t param = *potrace_param_default();
        param.turdsize = 100;

        potrace_state_t* result = potrace_trace(&param, &bm);
        auto it = std::back_inserter(raw_svgbin_);
        bool retval = potrace_svgbin::encode_potracestate(ctx, result, it);
        potrace_state_free(result);
        return retval;
    }

    std::vector<uint8_t> GetSEI(bool isAnnexB) override
    {
        // mux into sei
        const char c[] = "BILIMASK__SVGBIN";
        sei_msg_t msg;
        msg.sei_type = 5;
        msg.sei_data.insert(msg.sei_data.end(), c, c + 16);
        msg.sei_data.insert(msg.sei_data.end(), raw_svgbin_.begin(), raw_svgbin_.end());
        auto sei = make_avc_sei(&msg, &msg + 1);

        // mux into nalu
        std::vector<uint8_t> ebsp(4);
        rbsp_to_ebsp(sei.begin(), sei.end(), std::back_inserter(ebsp));

        if (isAnnexB)
        {
            ebsp[0] = 0;
            ebsp[1] = 0;
            ebsp[2] = 0;
            ebsp[3] = 1;
        }
        else
        {
            size_t seisize = ebsp.size() - 4;
            ebsp[0] = (seisize >> 24) & 0xff;
            ebsp[1] = (seisize >> 16) & 0xff;
            ebsp[2] = (seisize >> 8) & 0xff;
            ebsp[3] = seisize & 0xff;
        }

        return ebsp;
    }
};


AlphaToSvgBin* CreatePotraceAlphaToSvgBin(int sceneWidth, int sceneHeight)
{
    auto r = new PotraceAlphaToSvgBin();
    r->AppendHeader(sceneWidth, sceneHeight);
    return r;
}

#endif
