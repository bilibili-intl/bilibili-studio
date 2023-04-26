#include "compressions.h"

#include <array>
#include <cassert>
#include <memory>


namespace secret {

    int CompressDataZlib(const char* in_str, size_t in_len, std::vector<char>& out_data) {
        if (!in_str) {
            return Z_DATA_ERROR;
        }

        z_stream strm;
        std::memset(&strm, 0, sizeof(strm));
        int ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
        if (ret != Z_OK) {
            return ret;
        }

        std::shared_ptr<z_stream> sp_strm(&strm, [](z_stream* strm) {
            (void)deflateEnd(strm);
            });

        int flush;
        const int chunk = 512;
        unsigned char out[chunk];
        const char* end = in_str + in_len;

        do {
            size_t distance = end - in_str;
            strm.avail_in = (distance >= chunk) ? chunk : distance;
            strm.next_in = (Bytef*)in_str;

            in_str += strm.avail_in;
            flush = (in_str == end) ? Z_FINISH : Z_NO_FLUSH;

            do {
                strm.avail_out = chunk;
                strm.next_out = out;
                ret = deflate(&strm, flush);
                if (ret == Z_STREAM_ERROR)
                    break;
                switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    return ret;
                default:
                    break;
                }
                int have = chunk - strm.avail_out;
                out_data.insert(out_data.end(), out, out + have);
            } while (strm.avail_out == 0);
        } while (flush != Z_FINISH);

        return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
    }

    int CompressDataBrotli(const char* in_str, size_t in_len, std::vector<char>& out_data) {
        // bililive-x64-todo[
        /*
        BrotliEncoderState* encoder_state = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
        if (!encoder_state)
        {
            assert(0);
            return BROTLI_CREATE_INSTANCE_FAILED;
        }

        std::array<uint8_t, 1024> buffer;
        const uint8_t* next_in = reinterpret_cast<const uint8_t*>(in_str);
        size_t in_used = in_len;
        uint8_t* next_out = buffer.data();
        size_t out_used = buffer.size();
        size_t total_out = 0;

        bool is_finished = false;
        BrotliEncoderOperation operation = BROTLI_OPERATION_FLUSH;

        for (;;)
        {
            int result = BrotliEncoderCompressStream(
                encoder_state, operation, &in_used, &next_in, &out_used, &next_out, &total_out);
            if (result == BROTLI_FALSE)
            {
                BrotliEncoderDestroyInstance(encoder_state);
                return result;
            }

            next_out = buffer.data();
            size_t out_used_len = buffer.size() - out_used;
            out_used = buffer.size();

            if (out_used_len > 0)
            {
                out_data.insert(out_data.end(), buffer.begin(), buffer.begin() + out_used_len);
            }

            if (is_finished) {
                break;
            }

            if (!BrotliEncoderHasMoreOutput(encoder_state)) {
                is_finished = true;
                operation = BROTLI_OPERATION_FINISH;
            }
        }

        if (BrotliEncoderIsFinished(encoder_state))
        {
            BrotliEncoderDestroyInstance(encoder_state);
            return BROTLI_DECODER_RESULT_SUCCESS;
        } else
        {
            BrotliEncoderDestroyInstance(encoder_state);
            assert(0);
            return BROTLI_UNKNOWN_ERROR;
        }
        */
        assert(false);
        return BROTLI_UNKNOWN_ERROR;
        // ]bililive-x64-todo
    }

    int DecompressDataZlib(const char* in_str, size_t in_len, std::vector<char>& out_data)
    {
        if (!in_str) {
            return Z_DATA_ERROR;
        }

        z_stream strm;
        std::memset(&strm, 0, sizeof(strm));
        int ret = inflateInit(&strm);
        if (ret != Z_OK) {
            return ret;
        }

        std::shared_ptr<z_stream> sp_strm(&strm, [](z_stream* strm) {
            (void)inflateEnd(strm);
            });

        int flush;
        const int chunk = 512;
        unsigned char out[chunk];
        const char* end = in_str + in_len;

        do {
            size_t distance = end - in_str;
            strm.avail_in = (distance >= chunk) ? chunk : distance;
            strm.next_in = (Bytef*)in_str;

            in_str += strm.avail_in;
            flush = (in_str == end) ? Z_FINISH : Z_NO_FLUSH;

            do {
                strm.avail_out = chunk;
                strm.next_out = out;
                ret = inflate(&strm, Z_NO_FLUSH);
                if (ret == Z_STREAM_ERROR)
                    break;
                switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    return ret;
                default:
                    break;
                }
                int have = chunk - strm.avail_out;
                out_data.insert(out_data.end(), out, out + have);
            } while (strm.avail_out == 0);
        } while (flush != Z_FINISH);

        return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
    }

    int DecompressDataBrotli(const char* in_str, size_t in_len, std::vector<char>& out_data)
    {
        BrotliDecoderState* decoder_state = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
        if (!decoder_state)
        {
            assert(0);
            return BROTLI_CREATE_INSTANCE_FAILED;
        }

        std::array<uint8_t, 1024> buffer;
        const uint8_t* next_in = reinterpret_cast<const uint8_t*>(in_str);
        size_t in_used = in_len;
        uint8_t* next_out = buffer.data();
        size_t out_used = buffer.size();
        size_t total_out = 0;

        do
        {
            BrotliDecoderResult result = BrotliDecoderDecompressStream(
                decoder_state, &in_used, &next_in, &out_used, &next_out, &total_out);
            assert(result != BROTLI_DECODER_RESULT_ERROR);
            if (result == BROTLI_DECODER_RESULT_ERROR)
            {
                int error_code = BrotliDecoderGetErrorCode(decoder_state);
                BrotliDecoderDestroyInstance(decoder_state);
                return error_code;
            }

            next_out = buffer.data();
            size_t out_used_len = buffer.size() - out_used;
            out_used = buffer.size();

            if (out_used_len > 0)
            {
                out_data.insert(out_data.end(), buffer.begin(), buffer.begin() + out_used_len);
            }
        } while (BrotliDecoderHasMoreOutput(decoder_state));

        if (BrotliDecoderIsFinished(decoder_state))
        {
            BrotliDecoderDestroyInstance(decoder_state);
            return BROTLI_DECODER_RESULT_SUCCESS;
        } else
        {
            BrotliDecoderDestroyInstance(decoder_state);
            assert(0);
            return BROTLI_UNKNOWN_ERROR;
        }
    }

}
