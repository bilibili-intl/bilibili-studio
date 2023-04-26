#pragma once

#include <vector>

std::vector<std::tuple<int, std::vector<uint8_t>::const_iterator, std::vector<uint8_t>::const_iterator>>
split_nalus(const std::vector<uint8_t>& buf) {
    std::vector<std::tuple<int, std::vector<uint8_t>::const_iterator, std::vector<uint8_t>::const_iterator>> nalus;
    int p = 0;
    while(p + 4 < buf.size())
    {
        int size = buf[p] * 0x1000000 + buf[p + 1] * 0x10000 + buf[p + 2] * 0x100 + buf[p + 3];
        if (p + 4 + size <= buf.size())
        {
            int unit_type = buf[p + 4] & 0x1f;
            nalus.emplace_back(
                unit_type,
                buf.begin() + p,
                buf.begin() + p + 4 + size
            );
        }
        p += 4 + size;
    }
    return nalus;
}