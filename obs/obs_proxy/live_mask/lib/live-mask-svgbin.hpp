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
     * SVG ���� (version 1)
     *
     * 1. ����
     *
     *    SVGʹ��XML����ͼ��, XML�������ı�����, �ڴ洢�����紫��ʱ, ��Ҫռ�÷ǳ���Ŀռ�, Ϊ�˽��
     *    ��һ����, ���ǽ�SVG��XML�����Ϊ��һϵ��������<key, value>�ԵĶ���������, ��ͼ1.1
     *
     *    +-----+-------+-----+-------+-----+------+---------+-----+------+
     *    | key | value | key | value | key | value| ... ... | key | value|
     *    +-----+-------+-----+-------+-----+------+---------+-----+------+
     *                      �� 1.1 �����ĵ�svg���ݸ�ʽ
     *
     *    Ϊ����߱����ѹ��Ч��, ����ʹ����google protobuf�е�Zig-Zag encoding��varint���뼼��. ÿ��
     *    <key, value>���е�key��������varint����, ����������value����һ�����ݹ���, �������ݵĸ���
     *    �ͱ���Ҫ����key��ȷ��, ����, ��keyΪSVG_ELEM_KEY_VERSION��ʱ��, ��������value����һ��varint
     *    , ��������汾��.
     *
     *    �汾����Ӧ���ر��ע�ĵط�, �汾��ȷ���˱�������ϸ�淶, �����ĵ����������κ�ϸ��
     *
     *    ���ĵ������ı��ǰ汾��Ϊ 0 �ı������
     *    ���ĵ��������� varint ��google protobuf����ʹ�õ�varint
     *
     * 2. SVG��XMLģ��
     *    
     *    ʹ�ö����Ƹ�ʽ��ʾXMLʱ, ����ʹ�������µ�XMLģ��, ģ�����й̶����ֶ�, Ҳ���ڶ����Ƹ�ʽ�д����
     *    �ֶ�, ���� ${name} ��Ϊ�ڶ����������д�����ֶ�, ����ʱ��Ҫ������õ�XMLģ����Ӧ��λ��
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
     *    �����ģ���а�������������:
     *
     *    a. Ԫ��Ϣ: Ԫ��Ϣ������·����Ϣ֮�������, �ڱ��汾������, ����ָ:
     *       ${width}, ${height}, ${trans_x}, ${trans_y}, ${scale_x}, ${scale_y}
     *
     *    b. ·����Ϣ: ��SVG�е�path, ·��������һ���������е�${path_key}, ${path_value}���ж���
     *       ${path_value}�Ľ�������path_keyȷ��
     *
     *    ���������XMLģ��, ����ֻ�轫�����������ݱ���������Ƹ�ʽ����, ��Ϊ���ڶ����������������Ǻ�����Ϣ
     *    , ����Ҫ��<key, value>�Ե���ʽ��������Щ����, ��������ƶ��������<key, value>�Եĸ�ʽ
     *
     * 3. key�����Ӧvalue�ĸ�ʽ
     *    
     *    +----------------------+------------------------------------+-----------------------------+
     *    | key                  | value����   | value��������        | ˵��                        |
     *    +----------------------+-------------+----------------------+-----------------------------+
     *    | SVG_KEY_VERSION      | varint[1]   | int64                | �汾��, ��1��varint���     |
     *    +------------------------------------+----------------------+-----------------------------+
     *    | SVG_KEY_META         | xxxxxx      |                      | ��ϸ������                  |
     *    +------------------------------------+----------------------+-----------------------------+
     *    | SVG_KEY_PATH_CLOSE   | (nil)       |                      | �ر�·��, valueΪ��         |
     *    +------------------------------------+----------------------+-----------------------------+
     *    | SVG_KEY_PATH_MOVETO  | varint[2]   | int64[2]             | ��������, ��2��varint���   |
     *    +------------------------------------+----------------------+-----------------------------+
     *    | SVG_KEY_PATH_RMOVETO | varint[2]   | int64[2]             | ��������, ��2��varint���   |
     *    +------------------------------------+----------------------+-----------------------------+
     *    | SVG_KEY_PATH_LINETO  | varint[2]   | int64[2]             | ��������, ��2��varint���   |
     *    +------------------------------------+----------------------+-----------------------------+
     *    | SVG_KEY_PATH_RLINETO | varint[2]   | int64[2]             | ��������, ��2��varint���   |
     *    +------------------------------------+----------------------+-----------------------------+
     *    | SVG_KEY_PATH_CURVETO | varint[6]   | int64[6]             | 3����������, ��6��varint���|
     *    +------------------------------------+----------------------+-----------------------------+
     *    | SVG_KEY_PATH_RCURVETO| varint[6]   | int64[6]             | 3����������, ��6��varint���|
     *    +------------------------------------+----------------------+-----------------------------+
     *                                �� 3.1
     *
     *    �������������͵�����: ����������ָ�Ӷ����������а��ձ����������������Ӧ���洢��������������
     *    ���������ָ�������ڶ��������д��ڵĸ�ʽ
     *
     * 4. key = SVG_KEY_META ʱ��value
     *
     *    +--------+---------+----------+----------+----------+----------+
     *    |${width}|${height}|${trans_x}|${trans_y}|${scale_x}|${scale_y}|
     *    +--------+---------+----------+----------+----------+----------+
     *    | float  | float   | float    |float     |float     |float     |
     *    +--------+---------+----------+----------+----------+----------+
     *                                 ��4.1
     *
     *    ÿ��float�ڴ���ʱ������ ������(�����)
     *
     */

    static int32_t const SVG_BIN_VERSION = 1; // Э��汾��

    enum svg_key_t {
        SVG_KEY_VERSION = 1, // Э��汾
        SVG_KEY_META = 2, // Ԫ��Ϣ
        SVG_KEY_PATH_CLOSE = 3, // closepath
        SVG_KEY_PATH_MOVETO = 4, // move to, absolute
        SVG_KEY_PATH_RMOVETO = 5, // move to, relative
        SVG_KEY_PATH_LINETO = 6, // line to, absolute
        SVG_KEY_PATH_RLINETO = 7, // line to, relative
        SVG_KEY_PATH_CURVETO = 8, // cuve to, absolute
        SVG_KEY_PATH_RCURVETO = 9, // curve to, relative
        /* ��ʱδ�õ�������SVGԪ�� */
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
