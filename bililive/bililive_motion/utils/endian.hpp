// Copyright (c) 2016 ucclkp <ucclkp@gmail.com>.
// This file is part of utils project.
//
// This program is licensed under GPLv3 license that can be
// found in the LICENSE file.

#ifndef UTILS_ENDIAN_H_
#define UTILS_ENDIAN_H_

#include <cstdint>

#include "utils/platform_utils.h"


namespace utl {

    inline int16_t swapBytes(int16_t val) {
        return (val << 8) | ((val >> 8) & 0xFF);
    }

    inline uint16_t swapBytes(uint16_t val) {
        return (val << 8) | (val >> 8);
    }

    inline int32_t swapBytes(int32_t val) {
        return (val << 24) | ((val << 8) & 0x00FF0000) |
            ((val >> 24) & 0xFF) | ((val >> 8) & 0x0000FF00);
    }

    inline uint32_t swapBytes(uint32_t val) {
        return (val << 24) | ((val << 8) & 0x00FF0000) |
            (val >> 24) | ((val >> 8) & 0x0000FF00);
    }

    inline int64_t swapBytes(int64_t val) {
        return (val << 56) | ((val << 40) & 0xFF000000000000) |
            ((val << 24) & 0xFF0000000000) | ((val << 8) & 0xFF00000000) |
            (val >> 56 & 0xFF) | ((val >> 40) & 0xFF00) |
            ((val >> 24) & 0xFF0000) | ((val >> 8) & 0xFF000000);
    }

    inline uint64_t swapBytes(uint64_t val) {
        return (val << 56) | ((val << 40) & 0xFF000000000000) |
            ((val << 24) & 0xFF0000000000) | ((val << 8) & 0xFF00000000) |
            (val >> 56) | ((val >> 40) & 0xFF00) |
            ((val >> 24) & 0xFF0000) | ((val >> 8) & 0xFF000000);
    }


    template <typename T>
    T fromToBE(T val) {
#ifdef ENDIAN_LE
        return swapBytes(val);
#else
        return val;
#endif
    }

    template <typename T>
    T fromToLE(T val) {
#ifdef ENDIAN_LE
        return val;
#else
        return swapBytes(val);
#endif
    }

}

#endif  // UTILS_ENDIAN_H_