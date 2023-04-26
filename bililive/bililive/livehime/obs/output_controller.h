/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_BILILIVE_LIVEHIME_OBS_OUTPUT_CONTROLLER_H_
#define BILILIVE_BILILIVE_LIVEHIME_OBS_OUTPUT_CONTROLLER_H_

#include <bitset>

#include "base/basictypes.h"
#include "base/ext/count_down_latch.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/time/time.h"
#include "base/timer/timer.h"

#include "bililive/bililive/livehime/live_room/live_room_contract.h"
#include "bililive/bililive/livehime/obs/obs_status_details.h"
#include "bililive/secret/public/live_streaming_service.h"

#include "obs/obs_proxy/public/proxy/obs_proxy.h"
#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"
//#include "bililive/bililive/livehime/colive/colive_utils.h"

class LivehimeLiveRoomController;

namespace bililive {

class OutputControllerDelegate
{
public:
    struct StreamingErrorInfos
    {
        obs_proxy::StreamingErrorCode streaming_errno;  // 断流错误码
        obs_proxy::StreamingErrorDetails streaming_error_details;// 断流时使用的推流信息
        bool should_close_live_room = true;   // 断流时是否需要关播
        bool cause_for_restreaming = false;   // 是否是重推流引发的断流
        bool restreaming_after_speedtest = false;   // 重推流是否需要在测速之后，当cause_for_restreaming=true时才有效
    };

    // 正在发起推流流程
    virtual void OnPreStartStreaming(bool is_restart) = 0;

    // 推流成功，正在推流
    virtual void OnStreamingStarted(const StartStreamingDetails& details,
        const obs_proxy::StreamingErrorDetails& streaming_error_details) = 0;

    // 流断了，但正准备尝试以另一个地址进行推流
    virtual void OnRetryStreaming(int index, int total) = 0;

    // 正在发起断流流程，一次开播中可能会被调多次，比如重推流
    virtual void OnPreStopStreaming(bool is_restart) = 0;

    // 没有连上远端或者流断了，且已无备用地址进行重连了
    // 1、CDN连接失败了；
    // 2、CDN推流失败了；
    // 3、主动断流；
    virtual void OnStreamingStopped(OutputControllerDelegate::StreamingErrorInfos& info) = 0;

    // 需要新的推流地址信息
    virtual void NeedNewStreamingAddrInfos() = 0;

    // 需要测速
    virtual bool NeedSpeedTest() = 0;

protected:
    virtual ~OutputControllerDelegate() = default;
};

// This class provides specific obs services to live-hime layer, and it is also responsible to
// maintain internal state of the underlying obs-core.
class OutputController {
public:
    enum class RecordingStatus {
        Ready,
        Starting,
        Recording,
        Stopping,
    };

    enum class LiveStreamingStatus {
        Ready,
        WaitForSpeedTestResult,
        OpeningLiveRoom,
        IgnitingStream,
        IgnitingStreamAfterColive,
        Streaming,
        ExtinguishingStream,
        ClosingLiveRoom
    };

    enum class ColiveStreamingStatus {
        Ready,
        Waiting,    // 等待普通推流停止
        Joining,    // 发起加入频道的请求，等待加入成功回调
        Joined,     // 加入频道成功
        Starting,   // 发起连麦推流
        Streaming,  // 连麦推流成功
        Stopping,   // 停止推流，不等推流停止回调，接着离开频道，等待离开成功回调
    };

    struct StreamingAddresses {
        std::string cdn_address;
        std::string cdn_key;
        std::string server_address;
        std::string server_key;
        std::string new_link;
        std::string protocol;
        std::string live_key;

        std::string stream_name;
        std::string stream_key;
    };

public:
    OutputController();
    virtual ~OutputController();

    static OutputController* GetInstance();

    static bool IsStreamProtocolSupported(const std::string& prtc);

    // Bind the controller with the obs-core.
    // The call to this function is required, before using any obs services.
    virtual void Bind(obs_proxy::OBSCoreProxy* core);

    // Unbind with the obs-core.
    virtual void UnBind();

    virtual void set_delegate(OutputControllerDelegate* delegate) { delegate_ = delegate; }

    // 给定地址列表进行推流
    virtual void StartStreaming(const secret::LiveStreamingService::StartLiveInfo& start_live_info);
    virtual void StopStreaming();

    virtual void StartRecording();
    virtual void StopRecording();

    virtual bool IsRecording() const
    {
        return recording_status() == RecordingStatus::Recording;
    }

    virtual bool IsRecordingReady() const
    {
        return recording_status() == RecordingStatus::Ready;
    }

    virtual bool IsStreaming() const
    {
        return streaming_status() == LiveStreamingStatus::Streaming;
    }

    virtual bool IsStreamingReady() const
    {
        return streaming_status() == LiveStreamingStatus::Ready;
    }

    virtual bool IsCloseApp() const
    {
        return is_close_apping;
    }

    virtual void SetCloseApp(bool is_close)
    {
        is_close_apping = is_close;
    }

    // 编码器是否闲置，与当前OutputController的具体状态无关，纯粹表示推流SDK是否处于闲置状态
    virtual bool IsCodecReady() const;

    virtual void Restart(LiveRestartMode mode);

    virtual const StreamingAddresses& GetStreamingAddr() const;

    // During this safe period, one should not stop live streaming.
    virtual bool IsRestartLiveStreamingSafePeriodExpires() const;

    virtual int GetOutputTotalFrames() const;

    virtual int GetOutputDroppedFrames() const;

#pragma region(invoke by LivehimeLiveRoomController)

    //************************************
    // Method:    InformLiveRoomClosed
    // FullName:  bililive::OutputController::InformLiveRoomClosed
    // Access:    private
    // Returns:   void
    // Parameter: const OutputControllerDelegate::StreamingErrorInfos* infos
    //            infos是由本层在断流时传给LivehimeLiveRoomController的，在LivehimeLiveRoomController
    //            切实关闭直播间后会原封不动回传到本层，以方便本层针对断流时的错误信息进行处理
    //************************************
    virtual void InformLiveRoomClosed(const OutputControllerDelegate::StreamingErrorInfos* infos);


    virtual void InformSpeedTestCompleted();

#pragma endregion

private:
    obs_proxy::OBSCoreProxy* obs_core() const
    {
        return obs_core_;
    }

    void bind_obs_core(obs_proxy::OBSCoreProxy* core)
    {
        DCHECK(core);
        DCHECK(!obs_core_);

        obs_core_ = core;
    }

    void unbind_obs_core()
    {
        DCHECK(obs_core_);

        obs_core_ = nullptr;
    }

    // obs calllback ---------------------------
    // record
    void OnStartRecording();
    void OnStopRecording(const base::FilePath& video_path);
    void OnRecordingError(obs_proxy::RecordingErrorCode error_code, const base::FilePath& video_path);

    // streaming
    void OnStartStreaming();
    void OnStopStreaming(obs_proxy::StreamingStopDetails stop_details);
    void OnStreamingError(obs_proxy::StreamingErrorCode error_code,
        obs_proxy::StreamingErrorDetails error_details);

    //CDN当前地址发包三次失败上报回调
    void OnPostSendFailing();

    static void OnStreamingFeedback(const obs_proxy::StreamingFeedbackMetrics& metrics);
    static void OnStreamEncodedQPFeedback(int qp);
    // ------------------------------------------

    void RetryStreaming();
    void OnRetryStreamingTimer();

    void set_recording_status(RecordingStatus s) { recording_status_ = s; }
    RecordingStatus recording_status() const { return recording_status_; }

    void set_streaming_status(LiveStreamingStatus s) { streaming_status_ = s; }
    LiveStreamingStatus streaming_status() const { return streaming_status_; }

    void set_colive_streaming_status(ColiveStreamingStatus s) { colive_streaming_status_ = s; }

    // 以较为完整的流程开启推流操作（从确定具体的推流地址这个点开始流程）
    void IgniteStreamUntilAddrDetermined(bool restart);

    // 以已经确定好了的实际地址开始推流）
    void IgniteStream(bool after_colive);
    
    // 以已经确定好了的实际地址开始推流）
    void IgniteStreamBySpecialAddr(const std::string& server_addr,
        const std::string& server_key, const std::string& protocol);
    
    // 要求先测速，测速结束之后再开始推流
    void IgniteStreamUntilSpeedTest();
    
    // 直接调SDK推流
    void StartStreamingInternal(const std::string& server_addr,
        const std::string& server_key, const std::string& protocol);
    void StopStreamingInternal(bool restart);
    
    // 根据当前重试偏移及其newlink决定实际的推流地址
    void DetermineStreamingAddr();
    void OnDetermineStreamingAddr(
        const std::string& cdn_server_addr, const std::string& cdn_server_key, bool new_link_mode,
        const secret::LiveStreamingService::ProtocolInfo& live_info);
    
    // 流已然终止，且不再自动重试推流了，上层应该关播了
    void StreamingAnnihilated(obs_proxy::StreamingErrorCode streaming_errno,
        const obs_proxy::StreamingErrorDetails& streaming_error_details);

    void DoRestartIfNeeded();
    bool DoRestartAfterStreamingStoppedIfNeeded();
    void DoRestart();
    void DoRestartForColive();

private:
    friend class LivehimeLiveRoomController;

    obs_proxy::OBSCoreProxy* obs_core_;
    RecordingStatus recording_status_;
    OutputControllerDelegate* delegate_ = nullptr;
    LiveStreamingStatus streaming_status_;
    ColiveStreamingStatus colive_streaming_status_;

    bool recording_while_streaming_;
    LiveRestartMode restart_mode_;
    int restart_count_down_;    // 记录重试的标识位个数，由于录制停止和推流停止的通知会无序到来，只有确定录制和推流都终止了才切实执行重推流逻辑
    base::Time last_restart_live_streaming_time_;
    bool new_link_mode_ = false;
    int retry_count_ = 0;
    bool studio_streaming_ = false;

    // quality_abnormal_streaming_，当前在用的推流地址是不是推流异常时接口重新指定的，重置为false的时机是：
    // 1、推流成功了；
    // 2、推流失败了，并且已经发起了以旧的被判定为异常的地址进行重推的操作之后；
    bool quality_abnormal_streaming_ = false;
    StreamingAddresses last_normal_address_;

    int64_t self_colive_id_;
    std::vector<int64_t> remote_colive_id_;
    std::string pk_id_;

    // 开播接口返回的全部开播信息
    secret::LiveStreamingService::StartLiveInfo start_live_info_;

    // 发起实际推流流程时使用的地址信息
    StreamingAddresses saved_addresses_;

    // 连麦结束的重试
    int retry_count_after_colive_ = 0;
    base::OneShotTimer<OutputController> retry_timer_;

    // 正往上推流时调 TransToStopLive，会等到推流成功之后再停
    bool trans_to_stop_live_ = false;

    // 自推流且竖屏时的连麦结束后标志
    bool trans_from_spp_colive_ = false;
    bool is_landscape_for_colive_ = true;
    bool is_self_publish_for_colive_ = true;
    bool is_bitrate_reduced_when_colive_ = false;
    bool is_spp_continued_ = false;

    bool is_close_apping = false;
    bool is_call_leave_channel_ = false;//是否主动调用离开channel
    bool is_lose_heart_beat_ = false;//是否心跳中断

    obs_proxy::StreamingErrorCode error_code_;
    obs_proxy::StreamingErrorDetails error_details_;
    
    base::RepeatingTimer<OutputController> rtc_realtime_stats_timer_;

    friend struct DefaultSingletonTraits<OutputController>;
    DISALLOW_COPY_AND_ASSIGN(OutputController);
};

}   // namespace bililive

#endif  // BILILIVE_BILILIVE_LIVEHIME_OBS_OUTPUT_CONTROLLER_H_
