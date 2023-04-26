#pragma once

#include <stdint.h>
#include <boost/algorithm/searching/boyer_moore.hpp>
#include <vector>
#include <list>
#include <utility>

#include "avc-sei-util.h"
#include "avc-nalu.h"

class NalFilter
{
    const uint8_t* data_;
    size_t len_;

    using DataRange = std::pair<const uint8_t*, const uint8_t*>;
    std::vector<DataRange> nals_;

    std::list<std::vector<uint8_t>> appended_data_;
    bool isAnnexB_;

    // ref_idc, nal_type
    static std::tuple<int, int> ParseNalHeader(DataRange nal)
    {
        auto error_ret = std::make_tuple(-1, -1);

        if (nal.second - nal.first < 1)
            return error_ret;

        auto nal_header = nal.first[0];

        //forbidden bit
        if ((nal_header & 0b10000000) != 0)
            return error_ret;

        // nal_ref_idc & nal_unit_type
        return std::make_tuple(
            (nal_header & 0b01100000) >> 5,
            (nal_header & 0b00011111)
        );
    }

    std::vector<DataRange> SplitNals(const uint8_t* data, size_t len)
    {
        std::vector<DataRange> nals;

        if (isAnnexB_)
        {
            uint8_t startcode[] = { 0, 0, 1 };
            auto bm_search = boost::algorithm::boyer_moore<const uint8_t*>(startcode, startcode + sizeof(startcode) / sizeof(*startcode));

            auto curpos = data;
            auto endpos = data + len;
            while (curpos < endpos)
            {
                auto p = bm_search(curpos, endpos);
                if (p.first == endpos)
                    break;
                nals.push_back(std::make_pair(p.first + 3, nullptr));
                curpos = p.second;
            }

            if (!nals.empty())
                nals.back().second = data + len;

            for (size_t i = 1; i < nals.size(); ++i)
                nals[i - 1].second = nals[i].first;
        }
        else
        {
            auto new_nals = SplitNalus(data, data + len);
            for (auto& x : new_nals)
                nals.emplace_back(x.dataview.first + 4, x.dataview.second);
        }

        return nals;
    }

public:
    NalFilter(const uint8_t* data, size_t len, bool is_annexB = true)
        : data_(data)
        , len_(len)
        , isAnnexB_(is_annexB)
    {
        nals_ = SplitNals(data, len);

        // drop bad nal
        auto it = std::remove_if(nals_.begin(), nals_.end(), [](const DataRange& x) {
            return std::get<0>(ParseNalHeader(x)) < 0;
        });
        nals_.erase(it, nals_.end());
    }

    std::vector<DataRange> GetNalus()
    {
        return nals_;
    }

    void DropAUD()
    {
        auto it = std::remove_if(nals_.begin(), nals_.end(), [](const DataRange& x) {
            return std::get<1>(ParseNalHeader(x)) == 9;
        });
        nals_.erase(it, nals_.end());
    }

    void DropTimingSEI()
    {
        auto it = std::remove_if(nals_.begin(), nals_.end(), [](const DataRange& x) {
            if (std::get<1>(ParseNalHeader(x)) == 6) //sei
            {
                bool succeed = false;
                int sei_type;
                auto it = x.first + 1; // nal header
                std::tie(sei_type, succeed) = read_sei_number(it, x.second);
                if (!succeed)
                    return false;
                if (sei_type == 0 || sei_type == 1) // buffer period or timing
                    return true;
            }

            return false;
        });
        nals_.erase(it, nals_.end());
    }

    void InsertBeforeVCLNal(const uint8_t* p, size_t len)
    {
        appended_data_.emplace_back(p, p + len);
        auto& vec = appended_data_.back();
        auto new_nals = SplitNals(vec.data(), vec.size());
        auto it = std::find_if(nals_.begin(), nals_.end(), [&](const DataRange& x) {
            int ref_idc, unit_type;
            std::tie(ref_idc, unit_type) = ParseNalHeader(x);
            if (unit_type >= 1 && unit_type <= 5) //vcl
                return true;
            return false;
        });
        nals_.insert(it, new_nals.begin(), new_nals.end());
    }

    std::vector<uint8_t> GetResult()
    {
        size_t total_size = 0;
        for (auto& x : nals_)
            total_size += 4 + x.second - x.first; //start code

        std::vector<uint8_t> result;
        result.reserve(total_size);
        for (auto& x : nals_)
        {
            if (isAnnexB_)
            {
                result.push_back(0);
                result.push_back(0);
                result.push_back(0);
                result.push_back(1);
            }
            else
            {
                auto size = x.second - x.first;
                result.push_back((size >> 24) & 0xff);
                result.push_back((size >> 16) & 0xff);
                result.push_back((size >> 8) & 0xff);
                result.push_back((size >> 0) & 0xff);
            }
            result.insert(result.end(), x.first, x.second);
        }

        return result;
    }
};
