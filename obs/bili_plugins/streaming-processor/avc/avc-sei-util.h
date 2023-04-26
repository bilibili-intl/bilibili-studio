#pragma once

#include <vector>
#include <cstdint>
#include <tuple>
#include <iterator>
#include <type_traits>

#include "avc-nalu.h"

template<class OutIter>
using container_value_type = 
    typename OutIter::container_type::value_type;

template<class Iter>
constexpr bool IsUint8 =
    std::is_integral<container_value_type<Iter>>::value
    && std::is_unsigned<container_value_type<Iter>>::value
    && sizeof(container_value_type<Iter>) == 1;

struct sei_msg_t
{
    size_t sei_type;
    std::vector<uint8_t> sei_data;
};

template<class Iter>
std::tuple<int, bool> read_sei_number(Iter& it, Iter end)
{
    int retval = 0;
    bool succeed = true;
    while(it != end)
    {
        auto x = (std::uint8_t) *(it++);
        retval += x;
        if (x != 0xff)
        {
            succeed = true;
            break;
        }
    }
    if (succeed)
        return { retval, true };
    else
        return { 0, false };
}


template<class Iter>
void encode_sei_number(Iter& it, int num)
{
    static_assert(IsUint8<Iter>, "must be uint8_t");
    static_assert(std::is_same<typename std::iterator_traits<Iter>::iterator_category, std::output_iterator_tag>::value, "must be output iter");

    int ff_cnt = num / 0xff;
    int ff_rem = num % 0xff;
    std::fill_n(it, ff_cnt, 0xff);
    *it = ff_rem;
    ++it;
}


template<class Iter>
void append_sei_header(Iter& it)
{
    *it = 0b00000110; // nal_ref_idc = 0, nal_unit_type = 6
    ++it;
}


template<class Iter>
void append_sei_message(Iter& it, const sei_msg_t& msg)
{
    encode_sei_number(it, msg.sei_type); // payload type
    encode_sei_number(it, msg.sei_data.size()); // payload length
    std::copy(msg.sei_data.begin(), msg.sei_data.end(), it);
}


// feed sei_msg_t iterator, return rbsp
template<class Iter>
std::vector<std::uint8_t> make_avc_sei(Iter beg, Iter end) 
{
    std::vector<std::uint8_t> rbsp;
    auto it = std::back_inserter(rbsp);
    append_sei_header(it);
    while(beg != end)
    {
        append_sei_message(it, *beg);
        ++beg;
    }
    *(it++) = (char)0x80; // bit_equal_to_one && bit_equal_to_zero

    return rbsp;
}


//feed rbsp, uint8_t
template<class Iter>
std::vector<sei_msg_t> extract_seis(Iter& begin, Iter end) 
{
    std::vector<sei_msg_t> retval;

    while(begin != end && *begin != 0x80)
    {
        int sei_type, sei_size;
        bool sei_type_succ, sei_size_succ;
        std::tie(sei_type, sei_type_succ) = read_sei_number(begin, end);
        std::tie(sei_size, sei_size_succ) = read_sei_number(begin, end);
        if (sei_type_succ && sei_size_succ && std::distance(begin, end) >= sei_size)
        {
            //auto& msg = retval.emplace_back();
            retval.emplace_back();
            auto& msg = retval.back();
            msg.sei_type = sei_type;
            msg.sei_data.resize(sei_size);
            std::copy_n(begin, sei_size, msg.sei_data.begin());
        }
    }
    if (begin != end && *begin == 0x80)
        ++begin;
    
    return retval;
}
