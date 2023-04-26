#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <assert.h>

#include "flv-util.hpp"

namespace flv
{
    template<class stream_t>
    class DataStream : public IDataBlock
    {
        std::streampos offset_;
        stream_t stream_;

    public:
        template<class... Arg>
        DataStream(std::streampos offset, Arg... arg)
            : offset_(offset)
            , stream_(arg...)
        {
            if (!stream_)
                throw std::runtime_error("fail to open stream");
        }

        std::streampos seek(std::streampos position) override
        {
            stream_.seekg(offset_ + position);
            return tell();
        }

        std::streampos tell() override
        {
            return stream_.tellg() + offset_;
        }

        std::uint64_t read(void* ptr, std::uint64_t len) override
        {
            stream_.read((char*)ptr, len);
            auto ret = stream_.gcount();
            stream_.clear();
            return ret;
        }

        bool eos() override
        {
            return stream_.eof();
        }

        std::uint64_t write(const void* ptr, std::uint64_t len) override
        {
            stream_.write((const char*)ptr, len);
            return len;
        }

        void set_eos() override
        {
            return;
        }
    };

    class DataSpan : public IDataSpan
    {
    public:
        std::uint64_t offset_;
        std::uint64_t len_;

        std::uint64_t offset() override { return offset_; }
        int length() override { return len_; }

        bool getdata(IDataSource& src, void* ptr) override
        {
            std::uint64_t posTemp = src.tell();
            src.seek(offset_);
            assert(src.tell() == offset_);
            int readlen = src.read(ptr, len_);
            src.seek(posTemp);
            return readlen == len_;
        }
    };

    class MemDataSpan : public IDataSpan
    {
    public:
        std::vector<char> buf_;

        int length() override
        {
            return buf_.size();
        }

        bool getdata(void* ptr) override
        {
            std::copy_n(buf_.data(), buf_.size(), (char*)ptr);
            return true;
        }
    };

    class FilePtrDataSource : public IDataSource
    {
        FILE* fp_;
    public:
        FilePtrDataSource(FILE* fp)
        {
            fp_ = fp;
        }

        ~FilePtrDataSource() 
        {
        }

        std::streampos seek(std::streampos position) override
        {
#if defined(WIN32)
            _fseeki64(fp_, position, SEEK_SET);
#else
            fseeko(fp_, position, SEEK_SET);
#endif
            return tell();
        }

        std::streampos tell() override
        {
#if defined(WIN32)
            return _ftelli64(fp_);
#else
            return ftello(fp_);
#endif
        }

        std::uint64_t read(void* ptr, std::uint64_t len) override
        {
            return fread(ptr, 1, len, fp_);
        }

        bool eos() override
        {
            return feof(fp_);
        }
    };

    class FilePtrDataSink : public IDataSink
    {
        FILE* fp_;
    public:
        FilePtrDataSink(FILE* fp)
        {
            fp_ = fp;
        }

        ~FilePtrDataSink()
        {
        }

        std::uint64_t write(const void* ptr, std::uint64_t len) override
        {
            return fwrite(ptr, 1, len, fp_);
        }

        void set_eos() override
        {
        }
    };

    class MemDataSink : public IDataSink
    {
        std::vector<char>* data_;
    public:
        MemDataSink(std::vector<char>* data)
        {
            data_ = data;
        }

        ~MemDataSink()
        {
        }

        std::uint64_t write(const void* ptr, std::uint64_t len) override
        {
            for(int i = 0; i < len; i++)
                data_->push_back(((char*)ptr)[i]);
            return data_->size();
        }

        void set_eos() override
        {
        }
    };

    DataSourcePtr CreateFileDataProvider(const char* filename)
    {
        return std::make_shared<DataStream<std::fstream>>(0, filename, std::ios::in | std::ios::binary);
    }

    DataSourcePtr CreateFilePtrDataProvider(FILE* source)
    {
        return std::make_shared<FilePtrDataSource>(source);
    }

    DataSinkPtr CreateFileDataWriter(const char* filename)
    {
        return std::make_shared<DataStream<std::fstream>>(0, filename, std::ios::out | std::ios::binary);
    }

    DataSinkPtr CreateFilePtrDataWriter(FILE* source)
    {
        return std::make_shared<FilePtrDataSink>(source);
    }

    DataSinkPtr CreateMemDataWriter(std::vector<char>* data)
    {
        return std::make_shared<MemDataSink>(data);
    }

    DataSpanPtr CreateDataSpan(std::uint64_t offset, std::uint64_t len)
    {
        auto result = std::make_shared<DataSpan>();
        result->offset_ = offset;
        result->len_ = len;
        return result;
    }

    DataSpanPtr CreateMemDataSpan(const void* mem, std::uint64_t len)
    {
        auto result = std::make_shared<MemDataSpan>();
        result->buf_.resize(len);
        std::copy_n((char*)mem, len, result->buf_.data());
        return result;
    }
} // namespace flv
