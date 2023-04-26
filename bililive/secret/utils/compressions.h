#ifndef BILILIVE_SECRET_UTILS_DECOMPRESSION_H_
#define BILILIVE_SECRET_UTILS_DECOMPRESSION_H_

#include <vector>

#include "brotli/decode.h"
#include "brotli/encode.h"

#include "third_party/zlib/zlib.h"


namespace secret {

    //��ѹʹ�õĿ�
    enum DecompressLibCode
    {
        ZLIB = 2,
        BROTLI = 3
    };

    //����Brotli��û�еĴ�����
    enum BrotliErrorCode
    {
        BROTLI_CREATE_INSTANCE_FAILED = -32,
        BROTLI_UNKNOWN_ERROR = -33
    };

    int CompressDataZlib(const char* in_str, size_t in_len, std::vector<char>& out_data);
    int CompressDataBrotli(const char* in_str, size_t in_len, std::vector<char>& out_data);

    int DecompressDataZlib(const char* in_str, size_t in_len, std::vector<char>& out_data);
    int DecompressDataBrotli(const char* in_str, size_t in_len, std::vector<char>& out_data);

}

#endif  // BILILIVE_SECRET_UTILS_DECOMPRESSION_H_