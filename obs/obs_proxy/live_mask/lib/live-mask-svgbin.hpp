#pragma once

#include <cstdint>

namespace svgbin
{
    using std::int32_t;
    using std::uint32_t;
    using std::int64_t;
    using std::uint64_t;

    template<class T>
    static T htobe(T x)
    {
        uint8_t buf[sizeof(x)];
        T retval;
        memcpy(buf, &x, sizeof(x));
        std::reverse(buf, buf + sizeof(x));
        memcpy(&retval, buf, sizeof(x));
        return retval;
    }

    static uint32_t htobe32(uint32_t x)
    {
        return htobe(x);
    }

    static uint64_t htobe64(uint64_t x)
    {
        return htobe(x);
    }

    static uint32_t be32toh(uint32_t x)
    {
        return htobe(x);
    }

    static uint64_t be64toh(uint64_t x)
    {
        return htobe(x);
    }

    /**
     * add by chenzhiwei@bilibili.com
     * page svg into specified buffer, rather than to FILE
     * Date: 2019/01/07
     *
     * page_svg_x will call print_data(ctx->priv_data, format, ...) to dump svg elems
     *
     * print_data callback should return the same as printf function family
     *
     */

    /**
     * ZigZag Encoding Copied from google protobuf
     *
     * ZigZag Transform:  Encodes signed integers so that they can be
     * effectively used with varint encoding.
     * 
     * varint operates on unsigned integers, encoding smaller numbers into
     * fewer bytes.  If you try to use it on a signed integer, it will treat
     * this number as a very large unsigned integer, which means that even
     * small signed numbers like -1 will take the maximum number of bytes
     * (10) to encode.  ZigZagEncode() maps signed integers to unsigned
     * in such a way that those with a small absolute value will have smaller
     * encoded values, making them appropriate for encoding using varint.
     * 
     *       int32 ->     uint32
     * -------------------------
     *           0 ->          0
     *          -1 ->          1
     *           1 ->          2
     *          -2 ->          3
     *         ... ->        ...
     *  2147483647 -> 4294967294
     * -2147483648 -> 4294967295
     * 
     *        >> encode >>
     *        << decode <<
     */

    static constexpr uint32_t VARINT32_MAX_BYTES = 5;
    static constexpr uint32_t VARINT64_MAX_BYTES = 10;

    static inline uint32_t zigzag_encode_32(int32_t n) {
        // Note:  the right-shift must be arithmetic
        // Note:  left shift must be unsigned because of overflow
        return ((uint32_t)n << 1) ^ (uint32_t)(n >> 31);
    }

    static inline int32_t zigzag_decode_32(uint32_t n) {
        // Note:  Using unsigned types prevent undefined behavior
        return (int32_t)((n >> 1) ^ (~(n & 1) + 1));
    }

    static inline uint64_t zigzag_encode_64(int64_t n) {
        // Note:  the right-shift must be arithmetic
        // Note:  left shift must be unsigned because of overflow
        return ((uint64_t)n << 1) ^ (uint64_t)(n >> 63);
    }

    static inline int64_t zigzag_decode_64(uint64_t n) {
        // Note:  Using unsigned types prevent undefined behavior
        return (int64_t)((n >> 1) ^ (~(n & 1) + 1));
    }

    // return NULL if failed, or next postion to write
    static uint8_t* write_varint_to_buf_32(uint32_t n, uint8_t* buf, uint32_t bufsize) {
        while (bufsize > 0 && n >= 0x80) {
            *buf = (uint8_t)(n | 0x80);
            n >>= 7;
            bufsize -= 1;
            ++buf;
        }
        if (bufsize > 0) {
            *buf = (uint8_t)n;
            ++buf;
            bufsize -= 1;
            return buf;
        }
        return NULL;
    }

    // return NULL if failed, or next postion to write
    static uint8_t* write_varint_to_buf_64(uint64_t n, uint8_t* buf, uint32_t bufsize) {
        while (bufsize > 0 && n >= 0x80) {
            *buf = (uint8_t)(n | 0x80);
            n >>= 7;
            bufsize -= 1;
            ++buf;
        }
        if (bufsize > 0) {
            *buf = (uint8_t)n;
            ++buf;
            bufsize -= 1;
            return buf;
        }
        return NULL;
    }

    // return NULL if failed, or next postion to read and n setted
    static uint8_t const* read_varint_from_buf_32(uint32_t* n, uint8_t const* buf, uint32_t bufsize) {
        uint32_t pos = 0;
        uint32_t value = 0;
        while (pos < bufsize && pos < VARINT32_MAX_BYTES) {
            uint32_t b = buf[pos++]; 
            value += (b & 0x7F) << (7 * (pos - 1));
            if (! (b & 0x80)) {
                *n = value;
                return &buf[pos];
            }
        }
        return NULL;
    }

    // return NULL if failed, or next postion to read and n setted
    static uint8_t const* read_varint_from_buf_64(uint64_t* n, uint8_t const* buf, uint32_t bufsize) {
        uint32_t pos = 0;
        uint64_t value = 0;
        while (pos < bufsize && pos < VARINT64_MAX_BYTES) {
            uint64_t b = buf[pos++]; 
            value += (b & 0x7F) << (7 * (pos - 1));
            if (! (b & 0x80)) {
                *n = value;
                return &buf[pos];
            }
        }
        return NULL;
    }

    static uint8_t* write_int_to_buf_32(int32_t n, uint8_t* buf, uint32_t bufsize) {
        return write_varint_to_buf_32(zigzag_encode_32(n), buf, bufsize);
    }

    static uint8_t* write_int_to_buf_64(int64_t n, uint8_t* buf, uint32_t bufsize) {
        return write_varint_to_buf_64(zigzag_encode_64(n), buf, bufsize);
    }

    static uint8_t const* read_int_from_buf_32(int32_t* n, uint8_t const* buf, uint32_t bufsize) {
        uint32_t t = 0;
        uint8_t const* ret = NULL;
        ret = read_varint_from_buf_32(&t, buf, bufsize);
        if (ret) {
            *n = zigzag_decode_32(t);
        }
        return ret;
    }

    static uint8_t const* read_int_from_buf_64(int64_t* n, uint8_t const* buf, uint32_t bufsize) {
        uint64_t t = 0;
        uint8_t const* ret = NULL;
        ret = read_varint_from_buf_64(&t, buf, bufsize);
        if (ret) {
            *n = zigzag_decode_64(t);
        }
        return ret;
    }

    // return NULL if failed, or next postion to write
    static uint8_t* write_float_to_buf(float n, uint8_t* buf, uint32_t bufsize) {
        union {
            float f;
            uint32_t i;
        } t;
        t.f = n;
        if (bufsize < sizeof(n)) {
            return NULL;
        }
        uint32_t ben = htobe32(t.i);
        *((uint32_t*)buf) = ben;
        buf += sizeof(n);
        return buf;
    }

    // return NULL if failed, or next postion to write
    static uint8_t* write_double_to_buf(double n, uint8_t* buf, uint32_t bufsize) {
        union {
            double f;
            uint64_t i;
        } t;
        t.f = n;
        if (bufsize < sizeof(n)) {
            return NULL;
        }
        uint64_t ben = htobe64(t.i);
        *((uint64_t*)buf) = ben;
        buf += sizeof(n);
        return buf;
    }

    // return NULL if failed, or next postion to read and n setted
    static uint8_t const* read_float_from_buf(float* n, uint8_t const* buf, uint32_t bufsize) {
        union {
            float f;
            uint32_t i;
        } t;
        if (bufsize < sizeof(*n)) {
            return NULL;
        }
        uint32_t const* len = (uint32_t const*)buf;
        t.i = be32toh(*len);
        *n = t.f;
        buf += sizeof(*n);
        return buf;
    }

    static uint8_t const* read_double_from_buf(double* n, uint8_t const* buf, uint32_t bufsize) {
        union {
            double f;
            uint64_t i;
        } t;
        if (bufsize < sizeof(*n)) {
            return NULL;
        }
        uint64_t const* len = (uint64_t const*)buf;
        t.i = be64toh(*len);
        *n = t.f;
        buf += sizeof(*n);
        return buf;
    }


    /**
     * SVG 编码 (version 1)
     *
     * 1. 综述
     *
     *    SVG使用XML定义图像, XML本身是文本语言, 在存储和网络传输时, 需要占用非常大的空间, 为了解决
     *    这一问题, 我们将SVG的XML编码成为由一系列连续的<key, value>对的二进制数据, 如图1.1
     *
     *    +-----+-------+-----+-------+-----+------+---------+-----+------+
     *    | key | value | key | value | key | value| ... ... | key | value|
     *    +-----+-------+-----+-------+-----+------+---------+-----+------+
     *                      表 1.1 编码后的的svg数据格式
     *
     *    为了提高编码的压缩效率, 我们使用了google protobuf中的Zig-Zag encoding及varint编码技术. 每个
     *    <key, value>对中的key都采用了varint编码, 而紧随其后的value则由一组数据构成, 组内数据的个数
     *    和编码要依据key来确定, 例如, 当key为SVG_ELEM_KEY_VERSION的时候, 紧随其后的value就是一个varint
     *    , 用来代表版本号.
     *
     *    版本号是应该特别关注的地方, 版本号确定了编解码的详细规范, 即本文档中描述的任何细节
     *
     *    本文档描述的便是版本号为 0 的编码规则
     *    本文当中描述的 varint 即google protobuf中所使用的varint
     *
     * 2. SVG的XML模板
     *    
     *    使用二进制格式表示XML时, 我们使用了以下的XML模板, 模板中有固定的字段, 也有在二进制格式中传输的
     *    字段, 其中 ${name} 即为在二进制数据中传输的字段, 解码时需要将其放置到XML模板相应的位置
     *
     *    <?xml version="1.0" standalone="no"?>
     *    <svg version="1.0" xmlns="http://www.w3.org/2000/svg"
     *        width="${width}px" height="${height}px" viewBox="0 0 ${width} ${height}"
     *        preserveAspectRatio="xMidYMid meet">
     *      <g transform="translate(${trans_x}, ${trans_y}) scale(${scale_x}, ${scale_y})"
     *        fill="#000000" stroke="none">
     *      <path d="${path_key}${path_value}${path_key}${path_value}...${path_key}${path_value}">
     *      </path>
     *      </g>
     *    </svg>
     *
     *    上面的模板中包含了两种数据:
     *
     *    a. 元信息: 元信息即除了路径信息之外的数据, 在本版本编码中, 具体指:
     *       ${width}, ${height}, ${trans_x}, ${trans_y}, ${scale_x}, ${scale_y}
     *
     *    b. 路径信息: 即SVG中的path, 路径数据由一组连续排列的${path_key}, ${path_value}排列而成
     *       ${path_value}的解释依据path_key确定
     *
     *    有了上面的XML模板, 我们只需将上面两种数据编码进二进制格式即可, 而为了在二进制内容中区分是何种信息
     *    , 就需要以<key, value>对的形式来排列这些数据, 因此我们制定了下面的<key, value>对的格式
     *
     * 3. key及其对应value的格式
     *    
     *    +----------------------+------------------------------------+-----------------------------+
     *    | key                  | value编码   | value内置类型        | 说明                        |
     *    +----------------------+-------------+----------------------+-----------------------------+
     *    | SVG_KEY_VERSION      | varint[1]   | int64                | 版本号, 由1个varint组成     |
     *    +------------------------------------+----------------------+-----------------------------+
     *    | SVG_KEY_META         | xxxxxx      |                      | 详细见下文                  |
     *    +------------------------------------+----------------------+-----------------------------+
     *    | SVG_KEY_PATH_CLOSE   | (nil)       |                      | 关闭路径, value为空         |
     *    +------------------------------------+----------------------+-----------------------------+
     *    | SVG_KEY_PATH_MOVETO  | varint[2]   | int64[2]             | 顶点坐标, 由2个varint组成   |
     *    +------------------------------------+----------------------+-----------------------------+
     *    | SVG_KEY_PATH_RMOVETO | varint[2]   | int64[2]             | 顶点坐标, 由2个varint组成   |
     *    +------------------------------------+----------------------+-----------------------------+
     *    | SVG_KEY_PATH_LINETO  | varint[2]   | int64[2]             | 顶点坐标, 由2个varint组成   |
     *    +------------------------------------+----------------------+-----------------------------+
     *    | SVG_KEY_PATH_RLINETO | varint[2]   | int64[2]             | 顶点坐标, 由2个varint组成   |
     *    +------------------------------------+----------------------+-----------------------------+
     *    | SVG_KEY_PATH_CURVETO | varint[6]   | int64[6]             | 3个顶点坐标, 由6个varint组成|
     *    +------------------------------------+----------------------+-----------------------------+
     *    | SVG_KEY_PATH_RCURVETO| varint[6]   | int64[6]             | 3个顶点坐标, 由6个varint组成|
     *    +------------------------------------+----------------------+-----------------------------+
     *                                表 3.1
     *
     *    编码与内置类型的区别: 内置类型是指从二进制数据中按照编码解析出来的数据应当存储的语言内置类型
     *    而编码就是指在数据在二进制流中存在的格式
     *
     * 4. key = SVG_KEY_META 时的value
     *
     *    +--------+---------+----------+----------+----------+----------+
     *    |${width}|${height}|${trans_x}|${trans_y}|${scale_x}|${scale_y}|
     *    +--------+---------+----------+----------+----------+----------+
     *    | float  | float   | float    |float     |float     |float     |
     *    +--------+---------+----------+----------+----------+----------+
     *                                 表4.1
     *
     *    每个float在传输时均采用 网络序(大端序)
     *
     */

    static int32_t const SVG_BIN_VERSION = 1; // 协议版本号

    enum svg_key_t {
        SVG_KEY_VERSION = 1, // 协议版本
        SVG_KEY_META = 2, // 元信息
        SVG_KEY_PATH_CLOSE = 3, // closepath
        SVG_KEY_PATH_MOVETO = 4, // move to, absolute
        SVG_KEY_PATH_RMOVETO = 5, // move to, relative
        SVG_KEY_PATH_LINETO = 6, // line to, absolute
        SVG_KEY_PATH_RLINETO = 7, // line to, relative
        SVG_KEY_PATH_CURVETO = 8, // cuve to, absolute
        SVG_KEY_PATH_RCURVETO = 9, // curve to, relative
        /* 暂时未用到其他的SVG元素 */
    };


    #define impl_write_wrapper(type, func) \
        static inline uint8_t* write_proc(type x, uint8_t* buf, uint32_t bufsize) { return func(x, buf, bufsize); }

        impl_write_wrapper(int32_t, write_int_to_buf_32)
            impl_write_wrapper(int64_t, write_int_to_buf_64)
            impl_write_wrapper(uint32_t, write_varint_to_buf_32)
            impl_write_wrapper(uint64_t, write_varint_to_buf_64)
            impl_write_wrapper(float, write_float_to_buf)

    #undef impl_write_wrapper

    #define impl_read_wrapper(type, func) \
        static inline type read_proc(type, const uint8_t*& buf, uint32_t bufsize) { type ret; buf = func(&ret, buf, bufsize); return ret; }

            impl_read_wrapper(int32_t, read_int_from_buf_32)
            impl_read_wrapper(int64_t, read_int_from_buf_64)
            impl_read_wrapper(uint32_t, read_varint_from_buf_32)
            impl_read_wrapper(uint64_t, read_varint_from_buf_64)
            impl_read_wrapper(float, read_float_from_buf)

    #undef impl_read_wrapper
};
