#ifndef OBS_BILI_PLUGINS_STREAMING_PROCESSOR_STREAMING_PROCESSOR_INTERFACE_H_
#define OBS_BILI_PLUGINS_STREAMING_PROCESSOR_STREAMING_PROCESSOR_INTERFACE_H_
#include <string>

namespace streaming_processor
{
    enum ProcessStatus
    {
        OPEN_LOCAL_ERROR = 0, //打开本地接流服务器失败

        // 正在监听/正在等待第三方连接（即此时还没有第三方流过来）
        WAIT_FOR_TP_LINK,
        // 已经收到第三方流了（即第三方已经连上并已经成功把流推过来了，一个流程应该只通知一次，上层收到这个通知才去开播，才拿得到CND地址）
        TP_STREAM_RUNNING,
        // 第三方流断了
        TP_STREAMING_STOPPED,

        // 发起推流失败，线程都没开
        START_CDN_STREAMING_FAILED,
        // 正在发起对CDN的连接
        START_CDN_STREAMING,
        // 连接CDN失败了（上层可能会重新给一个新地址让继续重试，没有后备地址了就会关播了）
        CDN_STREAMING_CONNNECT_FAILED,
        // 正在往CDN推流了
        CDN_STREAMING_RUNNING,
        // 向CDN推流终止了，可能是错误引发的可能是上层主动stop的（此时上层要关播了，向CDN推终止就没必要收第三方的流，也没必要继续播了）
        CDN_STREAMING_STOPPED,

        // 停止整个底层的工作中，主动停止才会触发
        STOP_STREAMING,

        // 整个处理线程退出了，主动停止才会触发
        STREAMING_STOPPED
    };

    class ProcessorCallback
    {
    public:
        virtual ~ProcessorCallback() {}

        virtual void ProcessorStatus(ProcessStatus status) = 0;

        virtual void OnSpeedTestStart() {}

        //************************************
        // Method:    OnSpeedTestProgressed
        // FullName:  streaming_processor::ProcessorCallback::OnSpeedTestProgressed
        // Access:    virtual public
        // Returns:   void
        // Qualifier:
        // Parameter: double progress: 测试进度
        // Parameter: int cur_speed: 速度值，单位KBps
        //************************************
        virtual void OnSpeedTestProgressed(double progress, int cur_speed) {}

        // speed: 速度值，单位KBps
        //************************************
        // Method:    OnSpeedTestEnd
        // FullName:  streaming_processor::ProcessorCallback::OnSpeedTestEnd
        // Access:    virtual public
        // Returns:   void
        // Qualifier:
        // Parameter: int err_code
        // Parameter: int speed: 速度值，单位KBps
        // Parameter: int duration: 测速时长（不是实际的时长，仅仅只是发起测速时指定的期望时长）
        //************************************
        virtual void OnSpeedTestEnd(int err_code, int speed, int duration) {}
    };

    class IStreamingProcessor : public std::enable_shared_from_this<IStreamingProcessor>
    {
    public:
        virtual ~IStreamingProcessor() {}

        virtual void AddObserver(ProcessorCallback* callback) = 0;
        virtual void RemoveObserver(ProcessorCallback* callback) = 0;

        // 生成第三方推流本地接收终端信息
        // ※并且开始在此地址上进行监听，等待对端连接推流
        virtual int InitProcess() = 0;

        // 传入CDN地址，收到的本地流开始转发到这个地址
        virtual bool StartProcess(const std::string& rtmp) = 0;

        // 终止整个流程，停收流，停推流
        virtual void StopProcess() = 0;

        // 是否正在工作中
        virtual bool IsRunning() const = 0;

        // 传入需要插入的SEI信息
        virtual void SetSeiInfo(const uint8_t* data, int len) = 0;

        // 测速
        virtual bool SpeedTest(const unsigned char* data, size_t size, const std::string& addr,
            int base_on_kbps, int duration_per_addr) = 0;
        virtual void StopSpeedTest() = 0;
        virtual bool IsSpeedTesting() const = 0;

        // 获取flv头里编码器的信息
        virtual const std::string& GetEncoderInfo() = 0;

        //第三方推流通用SEI接口
        virtual void AddSEIInfo(const uint8_t* data, int size) = 0;
    };

    extern "C" _declspec(dllexport) IStreamingProcessor* CreateStreamingProcessor(bool custom_rtmp = false);

}//namespace
#endif