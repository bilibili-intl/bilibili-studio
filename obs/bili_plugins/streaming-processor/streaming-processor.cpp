// streaming-processor.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "streaming-processor.h"

#include <algorithm>
#include <vector>
#include <util/base.h>
#include "avc/nal-filter.hpp"
#include "flv-util/flv-dumper.hpp"
#include "flv-util/flv-writer.hpp"

#define FOR_EACH_CALLBACK_LOCK(list_mutex, observer_list, func)         \
    std::unique_lock<std::mutex> lock(list_mutex);     \
    for (auto& iter : observer_list)                   \
    {                                                  \
        iter->func;                                    \
    }

#define FOR_EACH_CALLBACK(observer_list, func)         \
    {                                                  \
        std::unique_lock<std::mutex> lock(observer_list_mutex_);     \
        for (auto& iter : observer_list)                   \
        {                                                  \
            iter->func;                                    \
        }                                                  \
    }


namespace streaming_processor
{
    //此SEI插入类答题专用，关键帧和插入时机不一样
    class FlvSeiInserter
    {
    public:
        FlvSeiInserter()
            : parser_(true)
            , bufferSink_(flv::CreateMemDataWriter(&outputBuffer_))
        {
            flv::write_header(true, true, bufferSink_);
        }

        virtual ~FlvSeiInserter()
        {
        }

        bool Feed(const uint8_t* data, int size)
        {
            return parser_.send_data((const char*)data, size);
        }

        int Receive(uint8_t* data)
        {
            for (;;)
            {
                FLVTag tag;
                auto ret = parser_.receive_packet(tag);
                if (ret == FLVParser::Data)
                {
                    std::lock_guard<std::mutex> lg(mutex_);
                    if (!pendingSeis_.empty())
                    {
                        if (tag.tagType == FLVTag::Video && tag.videoHeader.avcPacketType == FLVTag::AVCType::AVCData)
                        {
                            //关键帧必插非关键帧超过500毫秒插
                            long cur_time = clock();
                            if (tag.videoHeader.frameType == FLVTag::FrameType::KeyFrame ||
                                cur_time - last_sei_time_ > 500)
                            {
                                last_sei_time_ = cur_time;

                                auto sei = pendingSeis_.front();
                                auto payload = tag.get_payload();
                                std::vector<uint8_t> pkt(payload->length());
                                payload->getdata(pkt.data());

                                NalFilter flt(pkt.data(), pkt.size(), false);
                                flt.DropAUD();
                                flt.DropTimingSEI();
                                flt.InsertBeforeVCLNal(sei.data(), sei.size());
                                auto newpkt = flt.GetResult();
                                tag.set_payload(flv::CreateMemDataSpan(newpkt.data(), newpkt.size()));
                            }
                        }
                    }
                    flv::write_full_tag(tag, bufferSink_, nullptr);
                }
                else if (ret == FLVParser::NeedMoreData)
                    break;
                else if (ret == FLVParser::Error)
                    return -1;
            }

            if (!data)
                return outputBuffer_.size();
            else
            {
                auto retval = outputBuffer_.size();
                memcpy(data, outputBuffer_.data(), outputBuffer_.size());
                outputBuffer_.clear();
                return retval;
            }
        }

        void AddSei(const uint8_t* data, int size)
        {
            std::lock_guard<std::mutex> lg(mutex_);
            //直播答题信息只存一条
            pendingSeis_.clear();
            sei_msg_t msg;
            msg.sei_type = 5;
            msg.sei_data.insert(msg.sei_data.end(), data, data + size);
            auto rbsp = make_avc_sei(&msg, &msg + 1);
            std::vector<uint8_t> ebsp(4);
            rbsp_to_ebsp(rbsp.begin(), rbsp.end(), std::back_inserter(ebsp));
            auto ebspsize = ebsp.size() - 4;
            ebsp[0] = (ebspsize >> 24) & 0xff;
            ebsp[1] = (ebspsize >> 16) & 0xff;
            ebsp[2] = (ebspsize >> 8) & 0xff;
            ebsp[3] = ebspsize & 0xff;
            pendingSeis_.push_back(std::move(ebsp));
        }

        void CleanSei()
        {
            std::lock_guard<std::mutex> lg(mutex_);
            pendingSeis_.clear();
        }

    private:
        FLVParser parser_;
        std::vector<char> outputBuffer_;
        flv::DataSinkPtr bufferSink_;
        std::vector<std::vector<std::uint8_t> > pendingSeis_;
        std::mutex mutex_;
        long last_sei_time_ = 0;
    };

    class SeiInserter
    {
    public:
        SeiInserter()
            : parser_(true)
            , bufferSink_(flv::CreateMemDataWriter(&outputBuffer_))
        {
            flv::write_header(true, true, bufferSink_);
        }

        virtual ~SeiInserter()
        {
        }

        bool Feed(const uint8_t* data, int size)
        {
            return parser_.send_data((const char*)data, size);
        }

        int Receive(uint8_t* data)
        {
            for (;;)
            {
                FLVTag tag;
                auto ret = parser_.receive_packet(tag);
                if (ret == FLVParser::Data)
                {
                    std::lock_guard<std::mutex> lg(mutex_);
                    if (!sei_info_deque_.empty())
                    {
                        if (tag.tagType == FLVTag::Video && tag.videoHeader.avcPacketType == FLVTag::AVCType::AVCData)
                        {

                            auto sei = sei_info_deque_.front();
                            auto payload = tag.get_payload();
                            std::vector<uint8_t> pkt(payload->length());
                            payload->getdata(pkt.data());

                            NalFilter flt(pkt.data(), pkt.size(), false);
                            flt.DropAUD();
                            flt.DropTimingSEI();
                            flt.InsertBeforeVCLNal(sei.data(), sei.size());
                            auto newpkt = flt.GetResult();
                            tag.set_payload(flv::CreateMemDataSpan(newpkt.data(), newpkt.size()));
                            sei_info_deque_.pop_front();
                        }
                    }
                    flv::write_full_tag(tag, bufferSink_, nullptr);
                }
                else if (ret == FLVParser::NeedMoreData)
                    break;
                else if (ret == FLVParser::Error)
                    return -1;
            }

            if (!data)
                return outputBuffer_.size();
            else
            {
                auto retval = outputBuffer_.size();
                memcpy(data, outputBuffer_.data(), outputBuffer_.size());
                outputBuffer_.clear();
                return retval;
            }
        }

        void AddSei(const uint8_t* data, int size)
        {
            std::lock_guard<std::mutex> lg(mutex_);
            sei_msg_t msg;
            msg.sei_type = 5;
            msg.sei_data.insert(msg.sei_data.end(), data, data + size);
            auto rbsp = make_avc_sei(&msg, &msg + 1);
            std::vector<uint8_t> ebsp(4);
            rbsp_to_ebsp(rbsp.begin(), rbsp.end(), std::back_inserter(ebsp));
            auto ebspsize = ebsp.size() - 4;
            ebsp[0] = (ebspsize >> 24) & 0xff;
            ebsp[1] = (ebspsize >> 16) & 0xff;
            ebsp[2] = (ebspsize >> 8) & 0xff;
            ebsp[3] = ebspsize & 0xff;
            sei_info_deque_.push_back(std::move(ebsp));
        }

        void CleanSei()
        {
            std::lock_guard<std::mutex> lg(mutex_);
            sei_info_deque_.clear();
        }

    private:
        FLVParser parser_;
        std::vector<char> outputBuffer_;
        flv::DataSinkPtr bufferSink_;
        std::deque<std::vector<std::uint8_t>> sei_info_deque_;
        std::mutex mutex_;
    };

    StreamingProcessor::StreamingProcessor(bool custom_rtmp)
        :is_custom_rtmp_(custom_rtmp)
    {
        start_push_cdn_handle_ = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    }

    StreamingProcessor::~StreamingProcessor()
    {
        StopProcess();
        if (start_push_cdn_handle_)
        {
            CloseHandle(start_push_cdn_handle_);
            start_push_cdn_handle_ = nullptr;
        }
    }

    bool StreamingProcessor::InitRtmpAddr()
    {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET)
        {
            return false;
        }

        int reuse = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

        sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_port = htons(1935);
        sin.sin_addr.s_addr = INADDR_ANY;

        bool port_bind_is_ok = false;
        if (bind(sock, (LPSOCKADDR)&sin, sizeof(sin)) != 0 )
        {
            DWORD error_code = GetLastError();
            if (error_code == WSAEADDRINUSE)
            {
                sin.sin_port = htons(0);
                if (bind(sock, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
                {
                    blog(LOG_ERROR, "[streaming-processor] InitRtmpAddr bind2 failed!error code:%d", GetLastError());
                    closesocket(sock);
                    return false;
                }
                port_bind_is_ok = true;
            }
            else
            {
                blog(LOG_ERROR, "[streaming-processor] InitRtmpAddr bind failed!error code:%d", error_code);
            }
        }
        else
        {
            port_bind_is_ok = true;
        }

        if (!port_bind_is_ok)
        {
            if (bind(sock, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
            {
                blog(LOG_ERROR, "[streaming-processor] InitRtmpAddr bind3 failed!error code:%d", GetLastError());
                closesocket(sock);
                return false;
            }
        }

        sockaddr addr;
        int len = sizeof(addr);
        int ret = getsockname(sock, &addr, &len);

        if (addr.sa_family != AF_INET)
        {
            blog(LOG_ERROR, "[streaming-processor] InitRtmpAddr addr.sa_family != AF_INET");
            closesocket(sock);
            return false;
        }

        sockaddr_in add_in;
        memcpy(&add_in, &addr, len);
        port_ = ntohs(add_in.sin_port);

        rtmp_addr_ = "rtmp://0.0.0.0:";
        rtmp_addr_ += std::to_string(port_);
        rtmp_addr_ += "/livehime";
        return true;
    }

    int StreamingProcessor::InitProcess()
    {
        int port = 0;
        if (InitRtmpAddr())
        {
            port = port_;

            if (process_thread_.joinable())
            {
                process_thread_.detach();
            }

            process_thread_ = std::thread(InitProcessThread, shared_from_this());
        }

        return port;
    }

    void* StreamingProcessor::Process()
    {
        AVIOContext* ctx = nullptr;
        AVDictionary* opt = nullptr;

        AVIOContext* out_ctx = nullptr;
        AVDictionary* out_opt = nullptr;

        if (is_custom_rtmp_)
        {
            flv_sei_inserter_.reset(new FlvSeiInserter());
        }

        sei_inserter_.reset(new SeiInserter());

        auto ret = [&] {
            if (opt)
                av_dict_free(&opt);

            if (ctx)
                avio_closep(&ctx);

            if(out_opt)
                av_dict_free(&out_opt);

            if (out_ctx)
                avio_closep(&out_ctx);

            if (flv_sei_inserter_)
            {
                flv_sei_inserter_.reset();
            }

            if (sei_inserter_)
            {
                sei_inserter_.reset();
            }

            return nullptr;
        };

        FOR_EACH_CALLBACK(observer_list_, ProcessorStatus(streaming_processor::WAIT_FOR_TP_LINK));

        av_dict_set_int(&opt, "listen", 1, 0);
        if (avio_open2(&ctx, rtmp_addr_.c_str(), AVIO_FLAG_READ | AVIO_FLAG_DIRECT, &icb_, &opt) < 0)
        {
            if (icb_.callback(&icb_) != AVERROR_EXIT)
            {
                status_ = streaming_processor::OPEN_LOCAL_ERROR;
                return ret();
            }
            else
            {
                status_ = streaming_processor::STREAMING_STOPPED;
                return ret();
            }
        }

        FOR_EACH_CALLBACK(observer_list_, ProcessorStatus(streaming_processor::TP_STREAM_RUNNING));

        while(!is_stop_)
        {
            ::WaitForSingleObject(start_push_cdn_handle_, INFINITE);
            if (is_start_push_cdn_)
            {
                std::vector<uint8_t> buf(4096);
                int readsize = avio_read_partial(ctx, buf.data(), buf.size());
                if (readsize <= 0)
                {
                    status_ = streaming_processor::TP_STREAMING_STOPPED;
                    return ret();
                }

                if(is_first_buf_)
                {
                    is_first_buf_ = false;
                    std::string temp((char*)buf.data(), buf.size());
                    size_t index = temp.find("encoder");
                    if (index != std::string::npos)
                    {
                        if ((index + sizeof("encoder") + 2) < buf.size())
                        {
                            uint8_t encoder_info_len_high = buf[index + sizeof("encoder")];
                            uint8_t encoder_info_len_low = buf[index + sizeof("encoder") + 1];
                            uint16_t len = (uint16_t(encoder_info_len_high) << 8) | encoder_info_len_low;
                            if ((index + sizeof("encoder") + 2 + len) < buf.size())
                            {
                                encoder_info_ = std::string((char*)(buf.data() + index + sizeof("encoder") + 2), len);
                                encoder_info_ += '\0';
                            }
                        }
                    }
                }

                if (!out_ctx)
                {
                    FOR_EACH_CALLBACK(observer_list_, ProcessorStatus(streaming_processor::START_CDN_STREAMING));

                    if (avio_open2(&out_ctx, out_rtmp_addr_.c_str(), AVIO_FLAG_WRITE | AVIO_FLAG_DIRECT, &icb_, &out_opt) < 0)
                    {
                        if (icb_.callback(&icb_) != AVERROR_EXIT) {
                            status_ = streaming_processor::CDN_STREAMING_CONNNECT_FAILED;
                            return ret();
                        }
                        else
                            return ret();
                    }

                    FOR_EACH_CALLBACK(observer_list_, ProcessorStatus(streaming_processor::CDN_STREAMING_RUNNING));
                }

                //自定义推流模式下才会有这个sei_inserter
                if (flv_sei_inserter_)
                {
                    flv_sei_inserter_->Feed(buf.data(), readsize);

                    std::unique_ptr<uint8_t> data;
                    int read_len = flv_sei_inserter_->Receive(nullptr);
                    if (read_len > 0)
                    {
                        data.reset(new uint8_t[read_len]);
                        flv_sei_inserter_->Receive(data.get());
                        avio_write(out_ctx, data.get(), read_len);
                    }
                }
                else
                {
                    //新的sei插入接口和答题互斥
                    if (sei_inserter_)
                    {
                        sei_inserter_->Feed(buf.data(), readsize);

                        std::unique_ptr<uint8_t> data;
                        int read_len = 0;
                        read_len = sei_inserter_->Receive(nullptr);
                        while (read_len > 0)
                        {
                            data.reset(new uint8_t[read_len]);
                            sei_inserter_->Receive(data.get());
                            avio_write(out_ctx, data.get(), read_len);
                            read_len = sei_inserter_->Receive(nullptr);
                        }
                    }
                    else
                    {
                        avio_write(out_ctx, buf.data(), readsize);
                    }
                }


                if (out_ctx->error != 0)
                {
                    status_ = streaming_processor::CDN_STREAMING_STOPPED;
                    return ret();
                }
            }
        }

        status_ = streaming_processor::STREAMING_STOPPED;
        return ret();
    }

    void StreamingProcessor::StopProcess()
    {
        if (process_thread_.joinable())
        {
            FOR_EACH_CALLBACK(observer_list_, ProcessorStatus(streaming_processor::STOP_STREAMING));

            is_stop_ = true;
            SetEvent(start_push_cdn_handle_);
            icb_.SetInterrupt();
            process_thread_.join();
        }

        StopSpeedTest();
    }

    bool StreamingProcessor::IsRunning() const
    {
        return ::WaitForSingleObject(
            const_cast<StreamingProcessor*>(this)->process_thread_.native_handle(), 0) == WAIT_TIMEOUT;
    }

    void StreamingProcessor::ProcessClear()
    {
        is_stop_ = false;
        is_start_push_cdn_ = false;
        is_first_buf_ = true;
        encoder_info_ = "";
        if (start_push_cdn_handle_)
        {
            SetEvent(start_push_cdn_handle_);
            ResetEvent(start_push_cdn_handle_);
        }

        icb_.Reset();
    }

    void StreamingProcessor::InitProcessThread(std::shared_ptr<IStreamingProcessor> ptr)
    {
        StreamingProcessor* self = static_cast<StreamingProcessor*>(ptr.get());
        self->ProcessClear();
        self->Process();
        self->is_stop_ = true;

        auto& observer_list = self->observer_list_;
        auto status = self->status_;
        auto& list_mutex_ = self->observer_list_mutex_;

        // 线程退出了才发通知，不然多线程的异步问题可能导致上层重开监听时下层线程还没有退出
        FOR_EACH_CALLBACK_LOCK(list_mutex_, observer_list, ProcessorStatus(status));
    }

    const std::string& StreamingProcessor::GetEncoderInfo()
    {
        return encoder_info_;
    }

    bool StreamingProcessor::StartProcess(const std::string& rtmp)
    {
        if (process_thread_.joinable())
        {
            out_rtmp_addr_ = rtmp;
            is_start_push_cdn_ = true;
            if (start_push_cdn_handle_)
            {
                SetEvent(start_push_cdn_handle_);
            }
            return true;
        }
        else
        {
            FOR_EACH_CALLBACK(observer_list_, ProcessorStatus(streaming_processor::START_CDN_STREAMING_FAILED));
        }
        return false;
    }

    void StreamingProcessor::AddObserver(ProcessorCallback* callback)
    {
        std::unique_lock<std::mutex> lock(observer_list_mutex_);
        observer_list_.push_back(callback);
    }

    void StreamingProcessor::RemoveObserver(ProcessorCallback* callback)
    {
        std::unique_lock<std::mutex> lock(observer_list_mutex_);
        observer_list_.remove(callback);
    }

    void StreamingProcessor::SetSeiInfo(const uint8_t* data, int len)
    {
        if (is_stop_)
            return;
        if (!flv_sei_inserter_)
            return;
        if (len > 0)
        {
            flv_sei_inserter_->AddSei(data, len);
        }
        else
        {
            flv_sei_inserter_->CleanSei();
        }
    }

    bool StreamingProcessor::SpeedTest(const unsigned char* data, size_t size, const std::string& addr,
        int base_on_kbps, int duration_per_addr)
    {
        if (IsSpeedTesting())
        {
            assert(false);
            return false;
        }

        speed_test_thread_ = std::thread(SpeedTestThread, shared_from_this(),
            data, size, addr, base_on_kbps, duration_per_addr);
        return true;
    }

    void StreamingProcessor::StopSpeedTest()
    {
        if (speed_test_thread_.joinable())
        {
            speed_test_icb_.SetInterrupt();
            speed_test_thread_.join();
        }
    }

    bool StreamingProcessor::IsSpeedTesting() const
    {
        return !!speed_test_thread_.native_handle();
    }

    void StreamingProcessor::SpeedTestThread(std::shared_ptr<IStreamingProcessor> ptr,
        const unsigned char* data, size_t size, std::string addr,
        int base_on_kbps, int duration_per_addr)
    {
        StreamingProcessor* self = static_cast<StreamingProcessor*>(ptr.get());
        self->SpeedTestInternal(data, size, addr, base_on_kbps, duration_per_addr);
    }

    void StreamingProcessor::SpeedTestInternal(const unsigned char* data, size_t size, const std::string& addr,
        int base_on_kbps, int duration_per_addr)
    {
        // 不校验文件内容，至少校验一下文件长度是否是我们最开始进行测试demo的文件源长
        if (size != 263515u)
        {
            assert(false);
            FOR_EACH_CALLBACK(observer_list_, OnSpeedTestEnd(-1, 0, duration_per_addr));
            return;
        }

        AVIOContext* ctx = nullptr;
        AVDictionary* opt = nullptr;

        AVIOContext* out_ctx = nullptr;
        AVDictionary* out_opt = nullptr;

        auto ret = [&](int speed) {
            if (opt)
                av_dict_free(&opt);

            if (ctx)
                avio_closep(&ctx);

            if (out_opt)
                av_dict_free(&out_opt);

            if (out_ctx)
                avio_closep(&out_ctx);

            speed_test_icb_.SetInterrupt();

            return;
        };

        //这个文件第一个关键帧长度，从407开始
        const unsigned char* all_buf = data;
        const int all_buf_len = size;
        const unsigned char* iframe_buf = data + 407;
        const int iframe_buf_len = size - 407;

        speed_test_icb_.Reset();
        if (avio_open2(&out_ctx, addr.c_str(), AVIO_FLAG_WRITE | AVIO_FLAG_DIRECT, &speed_test_icb_, &out_opt) < 0)
        {
            char error[MAX_PATH]{ 0 };
            av_strerror(speed_test_icb_.callback(&speed_test_icb_), error, MAX_PATH);
            if (speed_test_icb_.callback(&speed_test_icb_) != AVERROR_EXIT)
            {
                assert(false);
            }
            FOR_EACH_CALLBACK(observer_list_, OnSpeedTestEnd(-1, 0, duration_per_addr));
            return ret(0);
        }

        FOR_EACH_CALLBACK(observer_list_, OnSpeedTestStart());

        //8000码率测试，每毫秒写1024字节
        base_on_kbps = std::ceil(base_on_kbps / 1000.0f * 1024); // 将日常交流使用的码率转为基于1024的实际数字信号千比特率
        const int byte_pms = base_on_kbps / 8 * 1024 / 1000;
        auto start_time = clock();
        long need_write_time = 0;
        uint64_t write_len = 0;
        bool head_is_written = false;
        int iframe_buf_offset = 0;

        // 连发n秒数据
        //static const int kTestTimeout = 5000;
        const int kTestTimeout = duration_per_addr;
        while (clock() < (start_time + kTestTimeout))
        {
            if (!head_is_written)
            {
                avio_write(out_ctx, all_buf, all_buf_len);
                write_len += all_buf_len;
                head_is_written = true;
            }
            else
            {
                need_write_time = clock() - start_time;
                if (write_len < need_write_time * byte_pms)
                {
                    int need_write_len = need_write_time * byte_pms - write_len;
                    if ((need_write_len + iframe_buf_offset) > iframe_buf_len)
                    {
                        need_write_len = iframe_buf_len - iframe_buf_offset;
                    }

                    avio_write(out_ctx, iframe_buf + iframe_buf_offset, need_write_len);
                    iframe_buf_offset += need_write_len;
                    write_len += need_write_len;
                    if (iframe_buf_offset == iframe_buf_len)
                    {
                        iframe_buf_offset = 0;
                    }
                }
            }

            static const unsigned long sp_timeout = 10;
            if (!observer_list_.empty())
            {
                auto test_time = clock() - start_time;
                test_time = std::max<long>(1, test_time);
                // 底层的进度通知不需要太频繁，大概500ms报告一下差不多了
                if (test_time % 500 < sp_timeout)
                {
                    double progress = test_time * 1.0f / kTestTimeout;
                    int speed = out_ctx->written / test_time; // KBps
                    FOR_EACH_CALLBACK(observer_list_, OnSpeedTestProgressed(std::min<double>(1.0f, progress), speed));
                }
            }

            Sleep(sp_timeout);
        }

        auto end_time = clock();

        auto test_time = end_time - start_time;
        test_time = std::max<long>(1, test_time);
        int speed = out_ctx->written / test_time; // KBps

        FOR_EACH_CALLBACK(observer_list_, OnSpeedTestEnd(0, speed, duration_per_addr));

        return ret(speed);
    }


    void StreamingProcessor::AddSEIInfo(const uint8_t* data, int size)
    {
        if (is_stop_)
            return;
        if (sei_inserter_)
        {
            sei_inserter_->AddSei(data, size);
        }
    }


    extern "C" _declspec(dllexport) IStreamingProcessor* CreateStreamingProcessor(bool custom_rtmp)
    {
        IStreamingProcessor* processor = new StreamingProcessor(custom_rtmp);
        return processor;
    }

}