#pragma once

#include <iterator>
#include <vector>
#include <utility>
#include <algorithm>

template<class Iter>
struct NaluInfo
{
    int type;
    int length;
    std::pair<Iter, Iter> dataview;
};


template<class Iter>
std::vector<NaluInfo<Iter> > SplitNalus(Iter& begin, Iter end)
{
    std::vector<NaluInfo<Iter> > retval;

    while(begin != end)
    {
        NaluInfo<Iter> nalu;
        nalu.dataview.first = begin;

        // get size
        nalu.length = 0;
        int i = 0;
        for(; i < 4; ++i)
        {
            if (begin != end)
                nalu.length = (nalu.length << 8) | (uint8_t)*begin;
            else
                break;
            
            ++begin;
        }

        if (i < 4)
            return retval;
        
        // read data
        if (begin == end)
            return retval;
        
        nalu.type = (*begin) & 0x1f;
        if (std::distance(begin, end) < nalu.length)
            return retval;
        std::advance(begin, nalu.length);
        nalu.dataview.second = begin;
        retval.emplace_back(nalu);
    }

    return retval;
}



template<class Iter1, class Iter2>
static void rbsp_to_ebsp(Iter1 beg, Iter1 end, Iter2 dst)
{
    int zero_cnt = 0;
    while(beg != end)
    {
        if (zero_cnt >= 2 && !(*beg & 0b11111100))
        {
            *(dst++) = 3;
            zero_cnt = 0;
        }
        if (*beg == 0)
            ++zero_cnt;
        else
            zero_cnt = 0;
        *(dst++) = *(beg++);
    }
}


template<class Iter1, class Iter2>
static bool ebsp_to_rbsp(Iter1 beg, Iter1 end, Iter2 dst)
{
    int zero_cnt = 0;
    while(beg != end)
    {
        if (zero_cnt >= 2 && *beg == 3)
        {
            zero_cnt = 0;
            ++beg;
            continue;
        }

        if (*beg == 0)
            ++zero_cnt;
        else
            zero_cnt = 0;
        *(dst++) = *(beg++);
    }

    return true;
}
