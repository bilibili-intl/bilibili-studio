#ifndef OBS_BILI_PLUGINS_STREAMING_PROCESSOR_STREAMING_PROCESSOR_H_
#define OBS_BILI_PLUGINS_STREAMING_PROCESSOR_STREAMING_PROCESSOR_H_
#include <atomic>
#include <list>
#include <mutex>
#include <thread>
#include <string>
#include <winsock2.h>

extern "C" {
#include "libavformat/avformat.h"
}

#include "streaming-processor-interface.h"

namespace streaming_processor
{
    class FlvSeiInserter;
    class SeiInserter;
    class InterruptCallback : public AVIOInterruptCB
    {
        std::atomic_bool interrupt;

        static int cb(void* p)
        {
            if (static_cast<InterruptCallback*>(p)->interrupt)
                return AVERROR_EXIT;
            else
                return 0;
        }
    public:
        InterruptCallback()
        {
            interrupt = false;
            callback = &InterruptCallback::cb;
            opaque = this;
        }

        void SetInterrupt()
        {
            interrupt = true;
        }

        void Reset()
        {
            interrupt = false;
        }
    };

    class StreamingProcessor : public IStreamingProcessor
    {
    public:
        StreamingProcessor(bool custom_rtmp = false);
        ~StreamingProcessor();

        void AddObserver(ProcessorCallback* callback) override;
        void RemoveObserver(ProcessorCallback* callback) override;

        int InitProcess() override;
        bool StartProcess(const std::string& rtmp) override;
        void StopProcess() override;
        bool IsRunning() const override;
        void SetSeiInfo(const uint8_t* data, int len) override;

        const std::string& GetEncoderInfo() override;

        // return KBps
        bool SpeedTest(const unsigned char* data, size_t size, const std::string& addr,
            int base_on_kbps, int duration_per_addr) override;
        void StopSpeedTest() override;
        bool IsSpeedTesting() const override;

        void AddSEIInfo(const uint8_t* data, int size) override;
    private:
        bool InitRtmpAddr();
        void* Process();
        void ProcessClear();

        static void InitProcessThread(std::shared_ptr<IStreamingProcessor> ptr);

        static void SpeedTestThread(std::shared_ptr<IStreamingProcessor> ptr,
            const unsigned char* data, size_t size, std::string addr,
            int base_on_kbps, int duration_per_addr);
        void SpeedTestInternal(const unsigned char* data, size_t size, const std::string& addr,
            int base_on_kbps, int duration_per_addr);

    private:
        std::atomic_bool is_stop_ = false;
        std::atomic_bool is_start_push_cdn_ = false;
        bool is_first_buf_ = true;
        std::string encoder_info_;
        HANDLE start_push_cdn_handle_ = nullptr;
        SOCKET sock_ = SOCKET_ERROR;
        std::thread process_thread_;
        std::string rtmp_addr_;
        InterruptCallback icb_;
        InterruptCallback speed_test_icb_;
        std::string out_rtmp_addr_;
        int port_ = 0;
        ProcessStatus status_ = STREAMING_STOPPED;

        std::mutex observer_list_mutex_;
        std::list<ProcessorCallback*> observer_list_;

        mutable std::thread speed_test_thread_;

        bool is_custom_rtmp_ = false;
        std::unique_ptr<FlvSeiInserter> flv_sei_inserter_;
        bool flv_sei_inserter_inited_ = false;

        std::unique_ptr <SeiInserter> sei_inserter_;
    };

}
#endif