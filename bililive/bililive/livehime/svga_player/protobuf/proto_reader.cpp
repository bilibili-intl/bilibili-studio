#include "bililive/bililive/livehime/svga_player/protobuf/proto_reader.h"

#include <algorithm>
#include <assert.h>

#define CHECK_TYPE(type)  \
    if (!is_type_available_ || cur_type_ != type) { assert(false); return false; }


namespace protobuf {

    ProtoReader::ProtoReader(std::ifstream* file)
        : use_file_(true),
          file_(file), index_(0) {
    }

    ProtoReader::ProtoReader(const std::string& payload)
        : use_file_(false),
          file_(nullptr), index_(0), payload_(payload) {
    }

    ProtoReader::~ProtoReader() {
        assert(embeddeds_.empty());
    }

    int ProtoReader::nextTag(WireType* type) {
        uint64_t key;
        if (!getVarint(VT_UINT64, &key)) {
            return -1;
        }

        uint8_t field_num = uint8_t(key >> 3);
        uint8_t wire_type = uint8_t(key & 0x7);
        if (wire_type > WT_32Bit) {
            return -1;
        }

        if (type) {
            *type = WireType(wire_type);
        }

        is_type_available_ = true;
        cur_type_ = WireType(wire_type);
        return field_num;
    }

    bool ProtoReader::skipValue() {
        uint64_t v_val;
        uint32_t f_val;
        std::string s_val;

        if (!is_type_available_) {
            assert(false);
            return false;
        }

        switch (cur_type_) {
        case WT_Varint:
            if (!getVarint(VT_UINT64, &v_val)) return false;
            break;
        case WT_64Bit:
            if (!get64Bit(false, &v_val)) return false;
            break;
        case WT_LengthDelimited:
            if (!getBytes(&s_val)) return false;
            break;
        case WT_StartGroup:
        {
            // group 已被弃用，如果检测到 group 就直接跳过
            uint64_t count = 1;
            for (;;) {
                WireType type;
                if (nextTag(&type) == -1) return false;
                if (type == WT_StartGroup) {
                    ++count;
                } else if (type == WT_EndGroup) {
                    --count;
                    if (count == 0) {
                        break;
                    }
                } else {
                    if (!skipValue()) {
                        return false;
                    }
                }
            }
            break;
        }
        case WT_EndGroup:
            assert(false);
            break;
        case WT_32Bit:
            if(!get32Bit(false, &f_val)) return false;
            break;
        default:
            assert(false);
            break;
        }
        return true;
    }

    bool ProtoReader::beginEmbedded() {
        if (!embeddeds_.empty()) {
            CHECK_TYPE(WT_LengthDelimited);
        }

        if (embeddeds_.empty()) {
            embeddeds_.push(0);
        } else {
            uint64_t length;
            if (!getVarint(VT_UINT64, &length)) {
                return false;
            }
            embeddeds_.push(length + getCurIndex());
        }

        return true;
    }

    void ProtoReader::endEmbedded() {
        assert(!embeddeds_.empty());
        embeddeds_.pop();
    }

    bool ProtoReader::getVarint(VarintType type, uint64_t* val) {
        char buf[1];
        uint64_t out = 0;
        for (uint8_t i = 0; i < 10; ++i) {
            // 读一个字节
            if (!read(buf, 1)) {
                return false;
            }

            uint8_t byte = uint8_t(buf[0]);

            // 虽然文档没有规定变长整数的长度，但当前整数最大只能用 64 位来表示，
            // 因此进行这个检查。从编码中读出的每个字节的有效位数是 7 位，因此
            // 64 位整数可以放得下 9 个再加 1 位。
            assert(i < 9 || (i == 9 && byte < 2));

            // 如果当前字节最高位为 1，说明后面还有
            if (((byte >> 7) & 0x1) == 0) {
                out |= uint64_t(byte) << i * 7;
                break;
            }

            byte &= 0x7F;
            out |= uint64_t(byte) << i * 7;
        }

        switch (type) {
        case VT_INT32:
            out = int64_t(int32_t(out));
            break;

        case VT_INT64:
        case VT_UINT32:
        case VT_UINT64:
        case VT_BOOL:
        case VT_ENUM:
            break;

        case VT_SINT32:
            // ZigZag 编码转换
            out = int64_t(int32_t((uint32_t(out) >> 1) ^ (-(int32_t(out) & 0x1))));
            break;

        case VT_SINT64:
            // ZigZag 编码转换
            out = (out >> 1) ^ (-(int64_t(out) & 0x1));
            break;
        }

        *val = out;
        return true;
    }

    bool ProtoReader::get64Bit(bool zigzag, uint64_t* val) {
        CHECK_TYPE(WT_64Bit);

        uint64_t out = 0;
        // protobuf 是小端存储的
        if (!read(reinterpret_cast<char*>(&out), 8)) {
            return false;
        }

        // ZigZag 编码转换
        if (zigzag) {
            out = (out >> 1) ^ (-(int64_t(out) & 0x1));
        }

        *val = out;
        return true;
    }

    bool ProtoReader::getBytes(std::string* val) {
        CHECK_TYPE(WT_LengthDelimited);

        uint64_t length;
        if (!getVarint(VT_UINT64, &length)) {
            return false;
        }

        std::unique_ptr<char[]> buf(new char[length]);
        if (!read(buf.get(), length)) {
            return false;
        }

        *val = std::string(buf.get(), length);
        return true;
    }

    bool ProtoReader::get32Bit(bool zigzag, uint32_t* val) {
        CHECK_TYPE(WT_32Bit);

        uint32_t out = 0;
        // protobuf 是小端存储的
        if (!read(reinterpret_cast<char*>(&out), 4)) {
            return false;
        }

        // ZigZag 编码转换
        if (zigzag) {
            out = (out >> 1) ^ (-(int32_t(out) & 0x1));
        }

        *val = out;
        return true;
    }

    bool ProtoReader::getInt32(int32_t* val) {
        uint64_t tmp;
        if (getVarint(VT_INT32, &tmp)) {
            *val = int32_t(tmp);
            return true;
        }
        return false;
    }

    bool ProtoReader::getFloat(float* val) {
        float out = 0;
        // protobuf 是小端存储的
        if (!read(reinterpret_cast<char*>(&out), 4)) {
            return false;
        }
        *val = out;
        return true;
    }

    bool ProtoReader::getDouble(double* val) {
        double out = 0;
        // protobuf 是小端存储的
        if (!read(reinterpret_cast<char*>(&out), 8)) {
            return false;
        }
        *val = out;
        return true;
    }

    bool ProtoReader::getMap(std::map<std::string, std::string>* val) {
        uint64_t length;
        if (!getVarint(VT_UINT64, &length)) {
            return false;
        }

        auto cur_idx = getCurIndex();
        for (uint64_t i = 0; i < length;) {
            std::string key, data;
            int tag = nextTag();
            if (tag == -1 || !getBytes(&key)) {
                return false;
            }

            tag = nextTag();
            if (tag == -1 || !getBytes(&data)) {
                return false;
            }

            (*val)[key] = std::move(data);
            i += getCurIndex() - cur_idx;
        }
        return true;
    }

    bool ProtoReader::read(char* buf, uint32_t length) {
        if (use_file_) {
            if (!embeddeds_.empty() &&
                embeddeds_.top() != 0 &&
                uint64_t(file_->tellg()) + length > embeddeds_.top())
            {
                return false;
            }

            file_->read(buf, length);
            if (!file_ || file_->gcount() != length) {
                return false;
            }
            return true;
        }

        if (!embeddeds_.empty() &&
            embeddeds_.top() != 0 &&
            index_ + length > embeddeds_.top())
        {
            return false;
        }
        if (payload_.size() < index_ + length) {
            return false;
        }
        payload_.copy(buf, length, index_);
        index_ += length;
        return true;
    }

    uint32_t ProtoReader::getCurIndex() const {
        if (use_file_) {
            return file_->tellg();
        }
        return index_;
    }

}