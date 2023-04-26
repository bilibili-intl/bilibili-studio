#pragma once

#include <numeric>
#include <vector>

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"
#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "base/threading/thread.h"
#include "base/timer/timer.h"

#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/livehime/obs/obs_status.h"
#include "bililive/bililive/livehime/streaming_report/streaming_qmcs_types.h"

#include "obs/obs-studio/libobs/obs.h"


namespace streaming_performance
{
    class PerformanceReport;
}

class ApmStatisticAnalysisService;
class KGpuMon;

namespace
{
    class CpuMonitor;
    class GpuMonitor;
}

// 这个枚举只允许增，不允许删，改的话可以改变名称但不能改变其表示的含义，
// 因为质量埋点报的本地流质量用的是这里的值，这里删改了会导致数据平台的查询需要跟着改
enum StreamingQuality : uint32_t
{
    FINE = 0,
    CPU_HIGH = 1 << 0,
    NET_LOSS = 1 << 1,
    RENDER_LOSS = 1 << 2,
    ENC_LOSS = 1 << 3,
    GPU_HIGH = 1 << 4,
};

const int StreamWarningNum = 8;

enum StreamingWarning
{
    WARNING_USE_HARD_ENCODE = 0,//提醒用户用硬件编码
    WARNING_NET_LOSS,//提醒用户网络丢帧
    WARNING_CHANGE_GAME_SETTING,//提醒用户修改游戏设置
    WARNING_CHANGE_LIVEHIME_SETTING,//提醒用户修改编码器设置
    WARNING_NV_MEMORY_OCCUPY,  //提醒用户硬件编码内存占用
    WARNING_MEMORY_OCCUPY,  //提醒用户编码内存占用
    WARNING_NV_MEMORY_OCCUPY_MAX,  //提醒用户硬件编码内存占用-更高警告
    WARNING_MEMORY_OCCUPY_MAX,  //提醒用户编码内存占用
};

struct GpuInfo
{
    std::string gpu_name;
    std::string driver_version;
};

struct EncodeInfo
{
    int bitrate = 0;
    int fps = 0;
    int width = 0;
    int height = 0;
    int encoder_quality = 0; //高画质=3，平衡=2，高性能=1
    std::string encoder_name; //x264,nvenc,nv_turing_enc,qsv,amd
    std::string rate_type;
};

struct LiveQualityReportInfo
{
    //1:视频直播 2:录屏直播 3:语音直播 4:虚拟直播
    enum class StreamType
    {
        Video = 1,
        Game = 2,
        Voice = 3,
        Vtuber = 4
    };
    bool is_living = false;//是否直播中，直播中才上报
    bool is_report = true;//是否上报
    unsigned int time_interval_second = 60 * 10;//上报的时间间隔(s)，默认10分钟
    unsigned int time_count = 0;//时间计数
    int index_report = 0;//上报index，每次上报后+1
    int memory_total_size = 0;//系统内存大小，单位GB，2的整数倍，小于1算1GB
    int resolution_height = 1080;//分辨率高度

    int gpu_lag_count = 0;//GPU卡顿（FPS）- 周期内每秒发生GPU占用率>90%的总次数
    int cpu_lag_count = 0;//主线程卡顿 - 周期内主线程每秒CPU占用率>90%的总次数
    int render_lag_count = 0;//主播画面卡顿 - 周期内画面丢帧率（每秒）大于40%的总次数
    int push_lag_count = 0;//推流卡顿 - 周期内网络丢帧率（每秒）大于40%的总次数
    int send_rate_zero_count = 0; //周期内码率变成0,计数+1，一直0不变不统计
    std::string session_id;//每次推流session_id
    std::string stream_guid;//每次推流guid
};


class StreamingReportObserver
{
public:
    virtual void OnStreamingReportSampling(const StreamingReportInfo& info)
    {
    }

    virtual void OnStreamingWarning(StreamingWarning warning)
    {
    }

protected:
    virtual ~StreamingReportObserver() = default;
};

class AverageQueue
{
    //队列长度10
    const size_t kMaxQueueLen = 10;

public:
    AverageQueue() = default;
    ~AverageQueue() = default;

    void PushData(float val)
    {
        std::lock_guard<std::mutex> lg(mutex_);
        queue_.push_back(val);
        while (queue_.size() > kMaxQueueLen)
        {
            queue_.erase(queue_.begin());
        }
    }

    float GetAverageQuality()
    {
        std::lock_guard<std::mutex> lg(mutex_);
        if (queue_.size() < kMaxQueueLen)
            return 0.0f;
        return std::accumulate(queue_.begin(), queue_.end(), 0) / (float)queue_.size();
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lg(mutex_);
        queue_.clear();
    }

private:
    std::deque<float> queue_;
    std::mutex mutex_;
};

class StreamingReportService : public base::RefCountedThreadSafe<StreamingReportService>,
    public LivehimeLiveRoomObserver,
    public AppFunctionCtrlObserver
{
public:
    static StreamingReportService* GetInstance();
    static int GetSampleRate();

    void AddObserver(StreamingReportObserver* observer) { observer_list_.AddObserver(observer); }
    void RemoveObserver(StreamingReportObserver* observer) { observer_list_.RemoveObserver(observer); }

    bool IsRecording();

    void SetOutput(obs_output_t* stream_output);
    void ReleaseOutput();

    bool IsGpuValid() const;

    void SetAnalysisLiveQuality(bool enable);

    std::list<StreamingReportInfo> LastCacheSamples();

    uint64 GetTotalRenderLoss();
    uint64 GetTotalEncodeLoss();
    uint64 GetTotalNetLoss();
    void ReportLiveAppUseTime();

protected:
    // LivehimeLiveRoomObserver
    // 要开播成功并已经开始推流的时候才开启记录，记录推流的地址信息
    void OnLiveRoomStreamingStarted(const bililive::StartStreamingDetails& details) override;
    // 关播就停止记录，不在乎是什么原因引起的关播
    void OnLiveRoomClosed(bool is_restreaming, const secret::LiveStreamingService::StartLiveInfo& start_live_info) override;

    // AppFunctionCtrlObserver
    void OnUpdateAppCtrlInfo() override;
    void OnAllAppKVCtrlInfoCompleted()override;

private:
    StreamingReportService();
    ~StreamingReportService();

    void Initialize();
    void Uninitialize();
    void StartRecord();
    void StopRecord();

    void StartOnWorkerThread();
    void OnSamplingTimerOnWorkerThread();
    void StopOnWorkerThread();
    void OnCpuSampling(StreamingReportSampleData& sampledata);
    void OnGpuSampling(StreamingReportSampleData& sampledata);
    void OnMemorySampling(StreamingReportSampleData& sampledata);
    void OnLivehimeCpuSampling(StreamingReportSampleData& sampledata);
    void OnLivehimeMemorySampling(StreamingReportSampleData& sampledata);
    void OnNetworkDropFrameSampling(StreamingReportSampleData& sampledata,const streaming_info& info);
    void OnRenderDropFrameSampling(StreamingReportSampleData& sampledata, const streaming_info& info);
    void OnEncodeDropFrameSampling(StreamingReportSampleData& sampledata, const streaming_info& info);

    void InformSampling(const StreamingReportInfo& info);

    void Warning(StreamingWarning warning);
    base::FilePath SaveStreamingReport(const base::Time& start_time, bool full, std::string* pbstr = nullptr);

    void AnalysisLiveQuality();

    // 每五分钟请求一次视频云的流质量信息
    void RequestStreamQualityFromServer();

    void ReportLocalStreamQualityToServer();
    void FillQualityReportCommonFields(std::vector<std::string>& vec_outcome);
    //开播性能质量体验建设上报
    void ReportLiveQualityInfo(const StreamingReportInfo& info,bool force = false);

    //App时长统计
    void ReportLiveAppUseTime(int64 seconds);
private:
    friend class BililiveOBS;
    friend class base::RefCountedThreadSafe<StreamingReportService>;

    base::Thread worker_thread_;
    std::unique_ptr<base::RepeatingTimer<StreamingReportService>> sampling_timer_;

    ObserverList<StreamingReportObserver> observer_list_;

    std::string stream_guid_;
    base::Time start_stream_time_;
    bililive::StartStreamingDetails startlive_details_;
    base::Lock pb_locker_;
    std::unique_ptr<ApmStatisticAnalysisService> apm_statistic_;
    std::unique_ptr<streaming_performance::PerformanceReport> pb_writer_;
    std::unique_ptr<base::DictionaryValue> quality_ext_info_;
    std::vector<std::string> quality_report_common_fields_;
    base::Time start_record_time_;
    std::unique_ptr<CpuMonitor> cpu_monitor_;
    std::mutex gpu_mutex_;
    std::unique_ptr<GpuMonitor> gpu_monitor_;
    obs_output_t* stream_output_ = nullptr;
    std::mutex output_mutex_;
    uint64 total_render_loss_ = 0;
    uint64 total_encode_loss_ = 0;
    uint64 total_net_loss_ = 0;
    uint64 total_net_bytes_loss_ = 0;
    uint64 total_net_need_send_ = 0;
    float srt_send_rate_mbps_last_ = 0.0;

    std::list<StreamingReportInfo> last_cache_samples_;

    //网络丢帧队列
    AverageQueue net_queue_;
    //编码丢帧队列
    AverageQueue enc_queue_;
    //渲染丢帧队列
    AverageQueue rdr_queue_;
    //cpu队列
    AverageQueue cpu_queue_;
    //gpu队列
    AverageQueue gpu_queue_;
    //编码器信息
    EncodeInfo encode_info_;
    //提醒过了以后不再提醒
    bool is_warned_[StreamWarningNum] = { false };
    double warn_memory_ = 0.0f;
    //是否分析推流质量
    bool is_analysis_live_quality_ = false;
    std::atomic<uint32_t> last_quality_result_ = 0;
    //开播性能质量体验建设上报
    LiveQualityReportInfo live_quality_report_info_;
    int64 report_time_ = 60;
    int64 apptime_count_ = 0;
    DISALLOW_COPY_AND_ASSIGN(StreamingReportService);
};