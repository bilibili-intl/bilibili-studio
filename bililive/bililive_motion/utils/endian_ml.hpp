// Copyright (c) 2016 ucclkp <ucclkp@gmail.com>.
// This file is part of utils project.
//
// This program is licensed under GPLv3 license that can be
// found in the LICENSE file.

#ifndef UTILS_ENDIAN_ML_H_
#define UTILS_ENDIAN_ML_H_

#include <cstdint>
#include <type_traits>


namespace utl {

    // 用于防止编译器直接触发下面的 static_assert
    template <typename T>
    class CheckingStub : public std::false_type {};

    /**
     * MLBExLE 的通用版本。
     * 如果给定的类型不是下面几种特例指定的类型的话应直接报错。
     */
    template <int N, typename T>
    T MLBExLE(T val) {
        static_assert(CheckingStub<T>::value, "unavailable type!");
        return 0;
    }

    template <>
    inline int32_t MLBExLE<3>(int32_t val) {
        int32_t tmp = 0;
        auto src = reinterpret_cast<uint8_t*>(&val);
        auto dst = reinterpret_cast<uint8_t*>(&tmp);
        dst[0] = src[2];
        dst[1] = src[1];
        dst[2] = src[0];
        if ((dst[2] >> 7) & 1) {
            dst[3] = 0xFF;
        }
        return tmp;
    }

    template <>
    inline uint32_t MLBExLE<3>(uint32_t val) {
        uint32_t tmp = 0;
        auto src = reinterpret_cast<uint8_t*>(&val);
        auto dst = reinterpret_cast<uint8_t*>(&tmp);
        dst[0] = src[2];
        dst[1] = src[1];
        dst[2] = src[0];
        return tmp;
    }

    template <>
    inline int64_t MLBExLE<4>(int64_t val) {
        int64_t tmp = 0;
        auto src = reinterpret_cast<uint8_t*>(&val);
        auto dst = reinterpret_cast<uint8_t*>(&tmp);
        dst[0] = src[3];
        dst[1] = src[2];
        dst[2] = src[1];
        dst[3] = src[0];
        if ((dst[3] >> 7) & 1) {
            dst[4] = 0xFF;
            dst[5] = 0xFF;
            dst[6] = 0xFF;
            dst[7] = 0xFF;
        }
        return tmp;
    }

    template <>
    inline uint64_t MLBExLE<4>(uint64_t val) {
        uint64_t tmp = 0;
        auto src = reinterpret_cast<uint8_t*>(&val);
        auto dst = reinterpret_cast<uint8_t*>(&tmp);
        dst[0] = src[3];
        dst[1] = src[2];
        dst[2] = src[1];
        dst[3] = src[0];
        return tmp;
    }


    /**
     * MLLExBE 的通用版本。
     * 如果给定的类型不是下面几种特例指定的类型的话应直接报错。
     */
    template <int N, typename T>
    T MLLExBE(T val) {
        static_assert(CheckingStub<T>::value, "unavailable type!");
        return 0;
    }

    template <>
    inline int32_t MLLExBE<3>(int32_t val) {
        int32_t tmp = 0;
        auto src = reinterpret_cast<uint8_t*>(&val);
        auto dst = reinterpret_cast<uint8_t*>(&tmp);
        dst[1] = src[2];
        dst[2] = src[1];
        dst[3] = src[0];
        if ((dst[1] >> 7) & 1) {
            dst[0] = 0xFF;
        }
        return tmp;
    }

    template <>
    inline uint32_t MLLExBE<3>(uint32_t val) {
        uint32_t tmp = 0;
        auto src = reinterpret_cast<uint8_t*>(&val);
        auto dst = reinterpret_cast<uint8_t*>(&tmp);
        dst[1] = src[2];
        dst[2] = src[1];
        dst[3] = src[0];
        return tmp;
    }

    template <>
    inline int64_t MLLExBE<4>(int64_t val) {
        int64_t tmp = 0;
        auto src = reinterpret_cast<uint8_t*>(&val);
        auto dst = reinterpret_cast<uint8_t*>(&tmp);
        dst[4] = src[3];
        dst[5] = src[2];
        dst[6] = src[1];
        dst[7] = src[0];
        if ((dst[4] >> 7) & 1) {
            dst[0] = 0xFF;
            dst[1] = 0xFF;
            dst[2] = 0xFF;
            dst[3] = 0xFF;
        }
        return tmp;
    }

    template <>
    inline uint64_t MLLExBE<4>(uint64_t val) {
        uint64_t tmp = 0;
        auto src = reinterpret_cast<uint8_t*>(&val);
        auto dst = reinterpret_cast<uint8_t*>(&tmp);
        dst[4] = src[3];
        dst[5] = src[2];
        dst[6] = src[1];
        dst[7] = src[0];
        return tmp;
    }


    template <size_t N, typename T>
    T MLToLE(T val) {
#ifdef ENDIAN_LE
        static_assert(std::is_integral<T>::value, "T must be a integral type!");
        static_assert(sizeof(T) >= N, "The size of T must be greater or equal to N!");
        return val & ((T(1) << (N * 8)) - 1);
#else
        return MLBExLE<N>(val);
#endif
    }

    template <size_t N, typename T>
    T MLToBE(T val) {
#ifdef ENDIAN_LE
        return MLLExBE<N>(val);
#else
        static_assert(std::is_integral<T>::value, "T must be a integral type!");
        static_assert(sizeof(T) >= N, "The size of T must be greater or equal to N!");
        return val >> ((sizeof(T) - N) * 8);
#endif
    }

    template <size_t N, typename T>
    T fromMLLE(T val) {
#ifdef ENDIAN_LE
        static_assert(std::is_integral<T>::value, "T must be a integral type!");
        static_assert(sizeof(T) >= N, "The size of T must be greater or equal to N!");
        return val & ((T(1) << (N * 8)) - 1);
#else
        return MLLExBE<N>(val);
#endif
    }

    template <size_t N, typename T>
    T fromMLBE(T val) {
#ifdef ENDIAN_LE
        return MLBExLE<N>(val);
#else
        static_assert(std::is_integral<T>::value, "T must be a integral type!");
        static_assert(sizeof(T) >= N, "The size of T must be greater or equal to N!");
        return val >> ((sizeof(T) - N) * 8);
#endif
    }

    template <size_t N, typename T>
    T toMLLE(T val) {
#ifdef ENDIAN_LE
        return val;
#else
        return MLLExBE<N>(val);
#endif
    }

    template <size_t N, typename T>
    T toMLBE(T val) {
#ifdef ENDIAN_LE
        return MLBExLE<N>(val);
#else
        static_assert(std::is_integral<T>::value, "T must be a integral type!");
        static_assert(sizeof(T) >= N, "The size of T must be greater or equal to N!");
        return val << ((sizeof(T) - N) * 8);
#endif
    }

}

#endif  // UTILS_ENDIAN_ML_H_