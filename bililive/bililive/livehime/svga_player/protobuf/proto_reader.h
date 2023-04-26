#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTOBUF_PROTO_READER_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_PROTOBUF_PROTO_READER_H_

#include <fstream>
#include <map>
#include <stack>


/**
 * ͨ�� Protocol Buffers ��������
 * https://developers.google.com/protocol-buffers/docs/encoding
 *
 * �ý�����������������õ� group �﷨������������ protobuf �а�����
 * group����ʹ�� skipValue() ������
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
         * ��ȡ int32_t �ļ�㷽����
         */
        bool getInt32(int32_t* val);

        /**
         * ��ȡ float �ļ�㷽����
         */
        bool getFloat(float* val);

        /**
         * ��ȡ double �ļ�㷽����
         */
        bool getDouble(double* val);

        /**
         * ��ȡ map �ļ�㷽����
         */
        bool getMap(std::map<std::string, std::string>* val);

        /**
         * ��ȡ enum �ļ�㷽����
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