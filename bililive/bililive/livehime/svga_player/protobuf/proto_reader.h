#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTOBUF_PROTO_READER_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTOBUF_PROTO_READER_H_

#include <fstream>
#include <map>
#include <stack>


/**
 * 通用 Protocol Buffers 解析器。
 * https://developers.google.com/protocol-buffers/docs/encoding
 *
 * 该解析器不会解析已弃用的 group 语法，若待解析的 protobuf 中包含有
 * group，请使用 skipValue() 跳过。
 */

namespace protobuf {

    class ProtoReader {
    public:
        enum WireType : uint8_t {
            WT_Varint = 0,
            WT_64Bit = 1,
            WT_LengthDelimited = 2,
            WT_StartGroup = 3,
            WT_EndGroup = 4,
            WT_32Bit = 5,
        };

        enum VarintType {
            VT_INT32,
            VT_INT64,
            VT_UINT32,
            VT_UINT64,
            VT_SINT32,
            VT_SINT64,
            VT_BOOL,
            VT_ENUM,
        };

        explicit ProtoReader(std::ifstream* file);
        explicit ProtoReader(const std::string& payload);
        ~ProtoReader();

        int nextTag(WireType* type = nullptr);
        bool skipValue();

        bool beginEmbedded();
        void endEmbedded();

        bool getVarint(VarintType type, uint64_t* val);
        bool get64Bit(bool zigzag, uint64_t* val);
        bool getBytes(std::string* val);
        bool get32Bit(bool zigzag, uint32_t* val);

        /**
         * 读取 int32_t 的简便方法。
         */
        bool getInt32(int32_t* val);

        /**
         * 读取 float 的简便方法。
         */
        bool getFloat(float* val);

        /**
         * 读取 double 的简便方法。
         */
        bool getDouble(double* val);

        /**
         * 读取 map 的简便方法。
         */
        bool getMap(std::map<std::string, std::string>* val);

        /**
         * 读取 enum 的简便方法。
         */
        template <typename T>
        bool getEnum(T* val) {
            uint64_t tmp;
            if (getVarint(VT_ENUM, &tmp)) {
                *val = T(tmp);
                return true;
            }
            return false;
        }

    private:
        bool read(char* buf, uint32_t length);
        uint32_t getCurIndex() const;

        bool use_file_;
        std::ifstream* file_;

        uint32_t index_;
        std::string payload_;

        bool is_type_available_ = false;
        WireType cur_type_ = WT_Varint;

        std::stack<uint32_t> embeddeds_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTOBUF_PROTO_READER_H_