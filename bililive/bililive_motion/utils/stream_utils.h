// Copyright (c) 2016 ucclkp <ucclkp@gmail.com>.
// This file is part of utils project.
//
// This program is licensed under GPLv3 license that can be
// found in the LICENSE file.

#ifndef UTILS_STREAM_UTILS_H_
#define UTILS_STREAM_UTILS_H_

#include <istream>

#include "utils/endian.hpp"
#include "utils/endian_ml.hpp"

#define RET_FALSE  \
    { return false; }

#define READ_STREAM(mem, size)  \
    s.read(reinterpret_cast<char*>(&mem), size);  \
    if (!s.good()) RET_FALSE

#define READ_STREAM_BE(mem, size)  \
    READ_STREAM(mem, size)  \
    static_assert(size == sizeof(mem), "The size of 'mem' must be equal to 'size'");  \
    mem = utl::fromToBE(mem);

#define READ_STREAM_LE(mem, size)  \
    READ_STREAM(mem, size)  \
    static_assert(size == sizeof(mem), "The size of 'mem' must be equal to 'size'");  \
    mem = utl::fromToLE(mem);

#define READ_STREAM_MLBE(mem, size)  \
    READ_STREAM(mem, size)  \
    static_assert(size < sizeof(mem), "The size of 'mem' must be greater than 'size'");  \
    mem = utl::fromMLBE<size>(mem);

#define READ_STREAM_MLLE(mem, size)  \
    READ_STREAM(mem, size)  \
    static_assert(size < sizeof(mem), "The size of 'mem' must be greater than 'size'");  \
    mem = utl::fromMLLE<size>(mem);

#define PEEK_STREAM(buf)  \
    buf = s.peek();       \
    if (!s) RET_FALSE

#define SKIP_BYTES(size)  \
    s.seekg(size, std::ios::cur);  \
    if (!s.good()) RET_FALSE

#define NEXT_EQUAL(str, size) {  \
    char l_buf[size];  \
    READ_STREAM(l_buf, size);  \
    if (std::strncmp(l_buf, str, size) != 0) RET_FALSE;  \
}


#define WRITE_STREAM(mem, size)  \
    s.write(reinterpret_cast<const char*>(&mem), size);  \
    if (!s.good()) RET_FALSE

#define WRITE_STREAM_STR(str)  \
    WRITE_STREAM(str.data()[0], str.length())

#define WRITE_STREAM_BE(mem, size) {  \
    static_assert(size == sizeof(mem), "The size of 'mem' must be equal to 'size'");  \
    auto tmp = utl::fromToBE(mem);  \
    WRITE_STREAM(tmp, size)  \
}

#define WRITE_STREAM_MLBE(mem, size) {  \
    static_assert(size < sizeof(mem), "The size of 'mem' must be greater than 'size'");  \
    auto tmp = utl::toMLBE<size>(mem);  \
    WRITE_STREAM(tmp, size)  \
}


#define PUT_STREAM(v)  \
    s.put(char(v));  \
    if (!s.good()) RET_FALSE

#define SEEKP_STREAM(pos)  \
    if(!s.seekp(pos).good()) RET_FALSE

#define SEEKG_STREAM(pos)  \
    if(!s.seekg(pos).good()) RET_FALSE


namespace utl {

    /**
     * 查看从当前位置开始的流数据是否等于 str[N]。
     * 如果流中的数据足够，且等于 str[N]，返回 1；
     * 如果中途读取到流末尾，或数据不等，则回退流指针，并返回 0；
     * 如果读取流出错，返回 -1。
     */
    template <size_t N>
    int startWith(std::istream& s, const char* str) {
        char buf[N];
        s.read(buf, N);
        if (!s) {
            if (s.bad()) return -1;
            if (!s.eof() && s.fail()) return -1;
        }
        if (s.eof() || std::strncmp(buf, str, N) != 0) {
            auto size = s.gcount();
            s.seekg(-size, std::ios::cur);
            return 0;
        }
        return 1;
    }

}

#endif  // UTILS_STREAM_UTILS_H_