#ifndef BILILIVE_BILILIVE_VIDDUP_LIVE_ROOM_LIVE_VIDDUP_CONTROLLER_H_
#define BILILIVE_BILILIVE_VIDDUP_LIVE_ROOM_LIVE_VIDDUP_CONTROLLER_H_

#include "base/observer_list.h"
#include "base/notification/notification_observer.h"
#include "base/notification/notification_registrar.h"

#include "bililive/bililive/livehime/live_model/live_model_controller.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/livehime/live_room/live_room_contract.h"
#include "bililive/bililive/livehime/obs/output_controller.h"
#include "bililive/bililive/livehime/third_party_streaming/third_party_output_controller.h"
#include "bililive/secret/public/live_streaming_service.h"

#include "obs/obs_proxy/public/proxy/obs_output_service.h"


class ViddupLiveRoomController :
    public LivehimeLiveRoomController
{
    enum class PartitionUpdateStatus
    {
        Updating,
        Success,
        Failure,
    };

public:

    static LivehimeLiveRoomController* GetInstance();

    // Confirm the address information of the connection, and obtain the push address again according to new_link
    using GetNewLinkAddrHandler = std::function<
        void(const std::string& server_addr, const std::string& server_key, bool new_link_mode,
            const secret::LiveStreamingService::ProtocolInfo& live_info)>;
    static void GetNewLinkAddr(const secret::LiveStreamingService::ProtocolInfo& live_info, GetNewLinkAddrHandler handler);

    ViddupLiveRoomController();
    ~ViddupLiveRoomController();

    void Initialize() override;
    void Uninitialize() override;

    void AddObserver(LivehimeLiveRoomObserver *observer) override;
    void RemoveObserver(LivehimeLiveRoomObserver *observer) override;

    // Open the broadcast room with the specified broadcast type
    StartLiveFlowResult OpenLiveRoom(StartLiveType start_type, bool start_by_hotkey,
        bool ignore_cover_check = false) override;

    void CloseLiveRoomPre() override;
    void CloseLiveRoom() override;
    void RestartLive(LiveRestartMode mode) override;
    bool TryToChangeStreamAddr(const std::string& addr, const std::string &code, const std::string &prtc) override;
    using GetStreamAddrListHandler = std::function<
        void(const secret::LiveStreamingService::ProtocolInfos& addrs)>;
    void GetStreamAddrList(GetStreamAddrListHandler handler) override;
    bool IsLiveReady() const override { return live_status_ == LiveStatus::Ready; }
    bool IsStreaming() const override { return live_status_ == LiveStatus::Streaming; }
    LiveStatus live_status() const override { return live_status_; }
    secret::LiveStreamingService::StartLiveInfo start_live_info() const override { return start_live_info_; }
    base::Time start_live_time() const override { return start_live_time_; }

    void GetBookLiveTimeInfos() override;
    void HideBookLiveTime() override;

private:
    void ThirdPartyStreamingCloseLiveRoomTimeout();
    void OnThirdPartyStreamingBreakCountDownTimer();
    void UpdateStreamVideoBitrateAtLiveArea(int parent_id);

protected:
    // NotificationObserver
    void Observe(int type, const base::NotificationSource& source,
        const base::NotificationDetails& details) override;

    // OutputControllerDelegate
    void OnPreStartStreaming(bool is_restart) override;
    void OnStreamingStarted(const bililive::StartStreamingDetails& details,
        const obs_proxy::StreamingErrorDetails& streaming_error_details) override;
    void OnRetryStreaming(int index, int total) override;
    void OnPreStopStreaming(bool is_restart) override;
    void OnStreamingStopped(OutputControllerDelegate::StreamingErrorInfos& info) override;
    void NeedNewStreamingAddrInfos() override;
    bool NeedSpeedTest() override;

    // static callback
    static void LocalStreamingStoppedEndDialog(int code, void* data);

private:

    void OnPostNoviceGuideFinish();
    void SetLiveStatus(LiveStatus status);
    bool CheckIfInSelfStreamingStatus();

    void OpenLiveRoomInternal();
    void CloseLiveRoomInternal(bool stop_streaming, void* param);
    void StartLive();
    void StopLive(void* param);
    void OnChoosePartition(PartitionUpdateStatus status, void* param);
    void OnStartLive(bool valid, int code, const std::string& err_msg,
        const secret::LiveStreamingService::StartLiveInfo& start_live_info);
    void OnStartLiveError(obs_proxy::StreamingErrorCode error_code,
        const std::wstring& extra_error_msg,
        int error_code_from_server,
        const secret::LiveStreamingService::StartLiveInfo& start_live_info);
    void HandleStartLiveNotice(const secret::LiveStreamingService::StartLiveNotice& notice);
    void OnStopLive(bool success, int result_code, const std::string& err_msg,
        void* param);
    void TerminateStartLiveFlow();
    void OnBindLiveTopic(bool success, int result_code, const std::string& err_msg);
    static void ChooseAreaEndDialog(int code, void* data);

    static void GetLiveErrorMsg(obs_proxy::StreamingErrorCode streaming_error_code, base::string16& error_msg, int& code);
    static const int kNoneServerErrorCode = 0;

    void HandleLiveRoomOpened();
    void HandlePreStartStreaming(bool is_restart);
    void HandleStreamingStarted(const bililive::StartStreamingDetails& details,
        const obs_proxy::StreamingErrorDetails& streaming_error_details);
    void HandleLiveStreamingError(obs_proxy::StreamingErrorCode streaming_errno, const std::wstring& error_msg, int error_code);
    void HandleLiveStreamingStoped();
    void HandlePreStopStreaming(bool is_restart);
    void HandleLiveRoomClosed(void* param);

#pragma endregion

private:
    base::NotificationRegistrar notifation_register_;

    std::atomic<LiveStatus> live_status_ = LiveStatus::Ready;
    bool stop_live_now_ = false;

    StartLiveType start_type_ = StartLiveType::Normal;
    base::Time last_set_channel_time_;
    base::Time start_live_time_;
    bool start_by_hotkey_ = false;
    bool choose_area_showed_by_start_live_flow_ = false;
    bool updating_room_info_ = false;

    secret::LiveStreamingService::StartLiveInfo start_live_info_;
    bool is_stop_live_intercept_ = false;

    ObserverList<LivehimeLiveRoomObserver> observer_list_;
    bool leave_third_party_mode_ = false;
    base::RepeatingTimer<ViddupLiveRoomController> tps_break_count_down_timer_;
    int tps_break_count_down_ = 0;

    base::WeakPtrFactory<ViddupLiveRoomController> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(ViddupLiveRoomController);
};

#endif  // BILILIVE_BILILIVE_VIDDUP_LIVE_ROOM_LIVE_VIDDUP_CONTROLLER_H_