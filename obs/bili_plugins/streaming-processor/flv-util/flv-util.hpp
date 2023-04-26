#pragma once

#include <cstdint>
#include <memory>

namespace flv
{
    class IDataSource
    {
    public:
        virtual ~IDataSource() {}
        virtual std::streampos seek(std::streampos position) = 0;
        virtual std::streampos tell() = 0;
        virtual std::uint64_t read(void* ptr, std::uint64_t len) = 0;
        virtual bool eos() = 0;
    };
    using DataSourcePtr = std::shared_ptr<IDataSource>;

    class IDataSink
    {
    public:
        virtual ~IDataSink() {}
        virtual std::uint64_t write(const void* ptr, std::uint64_t len) = 0;
        virtual void set_eos() = 0;
    };
    using DataSinkPtr = std::shared_ptr<IDataSink>;

    class IDataBlock : public IDataSource, public IDataSink {};
    using DataBlockPtr = std::shared_ptr<IDataBlock>;


    class IDataSpan
    {
    public:
        virtual ~IDataSpan() {}
        virtual std::uint64_t offset() { return 0; }
        virtual int length() { return 0; }
        virtual bool getdata(void* ptr) { return false; }
        virtual bool getdata(IDataSource& src, void* ptr) { return getdata(ptr); }
    };
    using DataSpanPtr = std::shared_ptr<IDataSpan>;

    DataSourcePtr CreateFileDataProvider(const char* filename);
    DataSourcePtr CreateFilePtrDataProvider(FILE* source);

    DataSinkPtr CreateFileDataWriter(const char* filename);
    DataSinkPtr CreateFilePtrDataWriter(FILE* source);
    DataSinkPtr CreateMemDataWriter(std::vector<char> *data);

    DataSpanPtr CreateDataSpan(std::uint64_t offset, std::uint64_t len);
    DataSpanPtr CreateMemDataSpan(const void* mem, std::uint64_t len);
};
