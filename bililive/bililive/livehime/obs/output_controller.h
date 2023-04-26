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
        obs_proxy::StreamingErrorCode streaming_errno;  // ����������
        obs_proxy::StreamingErrorDetails streaming_error_details;// ����ʱʹ�õ�������Ϣ
        bool should_close_live_room = true;   // ����ʱ�Ƿ���Ҫ�ز�
        bool cause_for_restreaming = false;   // �Ƿ��������������Ķ���
        bool restreaming_after_speedtest = false;   // �������Ƿ���Ҫ�ڲ���֮�󣬵�cause_for_restreaming=trueʱ����Ч
    };

    // ���ڷ�����������
    virtual void OnPreStartStreaming(bool is_restart) = 0;

    // �����ɹ�����������
    virtual void OnStreamingStarted(const StartStreamingDetails& details,
        const obs_proxy::StreamingErrorDetails& streaming_error_details) = 0;

    // �����ˣ�����׼����������һ����ַ��������
    virtual void OnRetryStreaming(int index, int total) = 0;

    // ���ڷ���������̣�һ�ο����п��ܻᱻ����Σ�����������
    virtual void OnPreStopStreaming(bool is_restart) = 0;

    // û������Զ�˻��������ˣ������ޱ��õ�ַ����������
    // 1��CDN����ʧ���ˣ�
    // 2��CDN����ʧ���ˣ�
    // 3������������
    virtual void OnStreamingStopped(OutputControllerDelegate::StreamingErrorInfos& info) = 0;

    // ��Ҫ�µ�������ַ��Ϣ
    virtual void NeedNewStreamingAddrInfos() = 0;

    // ��Ҫ����
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
        Waiting,    // �ȴ���ͨ����ֹͣ
        Joining,    // �������Ƶ�������󣬵ȴ�����ɹ��ص�
        Joined,     // ����Ƶ���ɹ�
        Starting,   // ������������
        Streaming,  // ���������ɹ�
        Stopping,   // ֹͣ��������������ֹͣ�ص��������뿪Ƶ�����ȴ��뿪�ɹ��ص�
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

    // ������ַ�б��������
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

    // �������Ƿ����ã��뵱ǰOutputController�ľ���״̬�޹أ������ʾ����SDK�Ƿ�������״̬
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
    //            infos���ɱ����ڶ���ʱ����LivehimeLiveRoomController�ģ���LivehimeLiveRoomController
    //            ��ʵ�ر�ֱ������ԭ�ⲻ���ش������㣬�Է��㱾����Զ���ʱ�Ĵ�����Ϣ���д���
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

    //CDN��ǰ��ַ��������ʧ���ϱ��ص�
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

    // �Խ�Ϊ���������̿���������������ȷ�������������ַ����㿪ʼ���̣�
    void IgniteStreamUntilAddrDetermined(bool restart);

    // ���Ѿ�ȷ�����˵�ʵ�ʵ�ַ��ʼ������
    void IgniteStream(bool after_colive);
    
    // ���Ѿ�ȷ�����˵�ʵ�ʵ�ַ��ʼ������
    void IgniteStreamBySpecialAddr(const std::string& server_addr,
        const std::string& server_key, const std::string& protocol);
    
    // Ҫ���Ȳ��٣����ٽ���֮���ٿ�ʼ����
    void IgniteStreamUntilSpeedTest();
    
    // ֱ�ӵ�SDK����
    void StartStreamingInternal(const std::string& server_addr,
        const std::string& server_key, const std::string& protocol);
    void StopStreamingInternal(bool restart);
    
    // ���ݵ�ǰ����ƫ�Ƽ���newlink����ʵ�ʵ�������ַ
    void DetermineStreamingAddr();
    void OnDetermineStreamingAddr(
        const std::string& cdn_server_addr, const std::string& cdn_server_key, bool new_link_mode,
        const secret::LiveStreamingService::ProtocolInfo& live_info);
    
    // ����Ȼ��ֹ���Ҳ����Զ����������ˣ��ϲ�Ӧ�ùز���
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
    int restart_count_down_;    // ��¼���Եı�ʶλ����������¼��ֹͣ������ֹͣ��֪ͨ����������ֻ��ȷ��¼�ƺ���������ֹ�˲���ʵִ���������߼�
    base::Time last_restart_live_streaming_time_;
    bool new_link_mode_ = false;
    int retry_count_ = 0;
    bool studio_streaming_ = false;

    // quality_abnormal_streaming_����ǰ���õ�������ַ�ǲ��������쳣ʱ�ӿ�����ָ���ģ�����Ϊfalse��ʱ���ǣ�
    // 1�������ɹ��ˣ�
    // 2������ʧ���ˣ������Ѿ��������Ծɵı��ж�Ϊ�쳣�ĵ�ַ�������ƵĲ���֮��
    bool quality_abnormal_streaming_ = false;
    StreamingAddresses last_normal_address_;

    int64_t self_colive_id_;
    std::vector<int64_t> remote_colive_id_;
    std::string pk_id_;

    // �����ӿڷ��ص�ȫ��������Ϣ
    secret::LiveStreamingService::StartLiveInfo start_live_info_;

    // ����ʵ����������ʱʹ�õĵ�ַ��Ϣ
    StreamingAddresses saved_addresses_;

    // �������������
    int retry_count_after_colive_ = 0;
    base::OneShotTimer<OutputController> retry_timer_;

    // ����������ʱ�� TransToStopLive����ȵ������ɹ�֮����ͣ
    bool trans_to_stop_live_ = false;

    // ������������ʱ������������־
    bool trans_from_spp_colive_ = false;
    bool is_landscape_for_colive_ = true;
    bool is_self_publish_for_colive_ = true;
    bool is_bitrate_reduced_when_colive_ = false;
    bool is_spp_continued_ = false;

    bool is_close_apping = false;
    bool is_call_leave_channel_ = false;//�Ƿ����������뿪channel
    bool is_lose_heart_beat_ = false;//�Ƿ������ж�

    obs_proxy::StreamingErrorCode error_code_;
    obs_proxy::StreamingErrorDetails error_details_;
    
    base::RepeatingTimer<OutputController> rtc_realtime_stats_timer_;

    friend struct DefaultSingletonTraits<OutputController>;
    DISALLOW_COPY_AND_ASSIGN(OutputController);
};

}   // namespace bililive

#endif  // BILILIVE_BILILIVE_LIVEHIME_OBS_OUTPUT_CONTROLLER_H_
