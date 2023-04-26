/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_BILILIVE_VIDDUP_OBS_VIDDUP_OUTPUT_CONTROLLER_H_
#define BILILIVE_BILILIVE_VIDDUP_OBS_VIDDUP_OUTPUT_CONTROLLER_H_

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
#include "bililive/bililive/livehime/obs/output_controller.h"
#include "bililive/secret/public/live_streaming_service.h"

#include "obs/obs_proxy/public/proxy/obs_proxy.h"
#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"

class ViddupLiveRoomController;

namespace bililive {

// maintain internal state of the underlying obs-core.
class ViddupOutputController : 
    public OutputController {

public:
    ~ViddupOutputController();

    static ViddupOutputController* GetInstance();

    static bool IsStreamProtocolSupported(const std::string& prtc);

    // Bind the controller with the obs-core.
    // The call to this function is required, before using any obs services.

    void Bind(obs_proxy::OBSCoreProxy* core) override;

    // Unbind with the obs-core.
    void UnBind() override;

    void set_delegate(OutputControllerDelegate* delegate) override { delegate_ = delegate; }
    void StartStreaming(const secret::LiveStreamingService::StartLiveInfo& start_live_info) override;
    void StopStreaming() override;
    void StartRecording() override;
    void StopRecording() override;

    bool IsRecording() const override
    {
        return recording_status() == RecordingStatus::Recording;
    }

    bool IsRecordingReady() const override
    {
        return recording_status() == RecordingStatus::Ready;
    }

    bool IsStreaming() const override
    {
        return streaming_status() == LiveStreamingStatus::Streaming;
    }

    bool IsStreamingReady() const override
    {
        return streaming_status() == LiveStreamingStatus::Ready;
    }

    bool IsCloseApp() const override
    {
        return is_close_apping;
    }

    void SetCloseApp(bool is_close) override
    {
        is_close_apping = is_close;
    }

    bool IsCodecReady() const override;

    void Restart(LiveRestartMode mode) override;

    const StreamingAddresses& GetStreamingAddr() const override;

    // During this safe period, one should not stop live streaming.
    bool IsRestartLiveStreamingSafePeriodExpires() const override;

    int GetOutputTotalFrames() const override;

    int GetOutputDroppedFrames() const override;

#pragma region(invoke by LivehimeLiveRoomController)
    void InformLiveRoomClosed(const OutputControllerDelegate::StreamingErrorInfos* infos) override;
    void InformSpeedTestCompleted() override;
#pragma endregion
  
private:
    ViddupOutputController();

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

    void IgniteStreamUntilAddrDetermined(bool restart);

    void IgniteStream(bool after_colive);

    void IgniteStreamBySpecialAddr(const std::string& server_addr,
        const std::string& server_key, const std::string& protocol);

    void IgniteStreamUntilSpeedTest();

    void StartStreamingInternal(const std::string& server_addr,
        const std::string& server_key, const std::string& protocol);
    void StopStreamingInternal(bool restart);

    void DetermineStreamingAddr();
    void OnDetermineStreamingAddr(
        const std::string& cdn_server_addr, const std::string& cdn_server_key, bool new_link_mode,
        const secret::LiveStreamingService::ProtocolInfo& live_info);

    void StreamingAnnihilated(obs_proxy::StreamingErrorCode streaming_errno,
        const obs_proxy::StreamingErrorDetails& streaming_error_details);

    void DoRestartIfNeeded();
    bool DoRestartAfterStreamingStoppedIfNeeded();
    void DoRestart();
    void DoRestartForColive();

private:
    friend class ViddupLiveRoomController;

    obs_proxy::OBSCoreProxy* obs_core_;
    RecordingStatus recording_status_;
    OutputControllerDelegate* delegate_ = nullptr;
    LiveStreamingStatus streaming_status_;
    ColiveStreamingStatus colive_streaming_status_;

    bool recording_while_streaming_;
    LiveRestartMode restart_mode_;
    int restart_count_down_;
    base::Time last_restart_live_streaming_time_;
    bool new_link_mode_ = false;
    int retry_count_ = 0;
    bool studio_streaming_ = false;
    bool quality_abnormal_streaming_ = false;
    StreamingAddresses last_normal_address_;

    int64_t self_colive_id_;
    std::vector<int64_t> remote_colive_id_;
    std::string pk_id_;

    secret::LiveStreamingService::StartLiveInfo start_live_info_;
    StreamingAddresses saved_addresses_;

    int retry_count_after_colive_ = 0;
    base::OneShotTimer<ViddupOutputController> retry_timer_;
    bool trans_to_stop_live_ = false;
    bool trans_from_spp_colive_ = false;
    bool is_landscape_for_colive_ = true;
    bool is_self_publish_for_colive_ = true;
    bool is_bitrate_reduced_when_colive_ = false;
    bool is_spp_continued_ = false;

    bool is_close_apping = false;
    bool is_call_leave_channel_ = false;
    bool is_lose_heart_beat_ = false;

    obs_proxy::StreamingErrorCode error_code_;
    obs_proxy::StreamingErrorDetails error_details_;
    
    base::RepeatingTimer<ViddupOutputController> rtc_realtime_stats_timer_;

    friend struct DefaultSingletonTraits<ViddupOutputController>;
    DISALLOW_COPY_AND_ASSIGN(ViddupOutputController);
};

}   // namespace bililive

#endif  // BILILIVE_BILILIVE_VIDDUP_OBS_VIDDUP_OUTPUT_CONTROLLER_H_
