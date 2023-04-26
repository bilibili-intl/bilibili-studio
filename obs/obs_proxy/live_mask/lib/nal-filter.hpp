#pragma once

#include <stdint.h>
#include <boost/algorithm/searching/boyer_moore.hpp>
#include <vector>
#include <utility>

class NalFilter
{
    const uint8_t* data_;
    size_t len_;
    
    using DataRange = std::pair<const uint8_t*, const uint8_t*>;
    std::vector<DataRange> nals_;

    std::list<std::vector<uint8_t>> appended_data_;

    // ref_idc, nal_type
    static std::tuple<int, int> ParseNalHeader(DataRange nal)
    {
        auto error_ret = std::make_tuple(-1, -1);

        if (nal.second - nal.first < 4)
            return error_ret;
        if (nal.first[0] != 0 || nal.first[1] != 0 || nal.first[2] != 1)
            return error_ret;

        auto nal_header = nal.first[3];

        //forbidden bit
        if ((nal_header & 0b10000000) != 0)
            return error_ret;

        // nal_ref_idc & nal_unit_type
        return std::make_tuple(
            (nal_header & 0b01100000) >> 5,
            (nal_header & 0b00011111)
        );
    }

    static std::vector<DataRange> SplitNals(const uint8_t* data, size_t len)
    {
        std::vector<DataRange> nals;
        uint8_t startcode[] = { 0, 0, 1 };
        auto bm_search = boost::algorithm::boyer_moore<uint8_t*>(startcode, startcode + sizeof(startcode) / sizeof(*startcode));

        auto curpos = data;
        auto endpos = data + len;
        while (curpos < endpos)
        {
            auto p = bm_search(curpos, endpos);
            if (p.first == endpos)
                break;
            nals.push_back(std::make_pair(p.first, nullptr));
            curpos = p.second;
        }

        if (!nals.empty())
            nals.back().second = data + len;

        for (size_t i = 1; i < nals.size(); ++i)
            nals[i - 1].second = nals[i].first;

        return nals;
    }

public:
    NalFilter(const uint8_t* data, size_t len)
        : data_(data)
        , len_(len)
    {
        nals_ = SplitNals(data, len);

        // drop bad nal
        auto it = std::remove_if(nals_.begin(), nals_.end(), [](const DataRange& x) {
            return std::get<0>(ParseNalHeader(x)) < 0;
        });
        nals_.erase(it, nals_.end());
    }

    void DropAUD()
    {
        auto it = std::remove_if(nals_.begin(), nals_.end(), [](const DataRange& x) {
            return std::get<1>(ParseNalHeader(x)) == 9;
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
            total_size += 1 + x.second - x.first; //start code 3byte -> 4byte

        std::vector<uint8_t> result;
        result.reserve(total_size);
        for (auto& x : nals_)
        {
            result.push_back(0);
            result.insert(result.end(), x.first, x.second);
        }

        return result;
    }
};
