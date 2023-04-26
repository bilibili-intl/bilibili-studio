#ifndef BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TOOLBAR_TOOLBAR_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TOOLBAR_TOOLBAR_VIEW_H

#include "base/prefs/pref_change_registrar.h"

#include "net/url_request/url_request_context.h"

#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/livehime/obs/obs_status.h"
#include "bililive/bililive/ui/bililive_command_receiver.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_hover_tip_button.h"
#include "bililive/secret/public/live_streaming_service.h"

#include "ui/base/view_prop.h"
#include "ui/views/controls/menu/menu_runner.h"
#include "ui/views/controls/menu/menu_delegate.h"
#include "ui/views/controls/button/button.h"
#include "bililive/bililive/livehime/volume/sing_identify_property_presenter.h"


namespace
{
    class ToolBarRecordLiveButton;
    class ToolBarEntrancesManagerButton;
    class CpmBubbleController;
}

class BililiveHideAwareView;
class BililiveImageButton;
class ToolBarVolumeCtrl;
class LivehimeModuleEntranceView;
class LivehimeTopToolbarView;
class LivehimeFunctionLabelButton;

struct WebLinkInfo {
    int source_event = 1;
    int64_t resource_id = 0;
    int64_t game_id = 0;
    int64_t virtual_id = 0;
    std::string function_type;
};

class ToolbarViddupView
    : public views::View
    , public bililive::OBSStatusDelegate
    , views::ButtonListener
    , base::NotificationObserver
    , obs_proxy_ui::OBSUIProxyObserver
    , BililiveBroadcastObserver
    , LivehimeLiveRoomObserver
{
public:
    ToolbarViddupView();
    ~ToolbarViddupView();

    void SetSourceEvent(int source_event) { source_event_ = source_event; }
    void SetResourceId(int64_t resource_id) { resource_id_ = resource_id; }
    void SetGameId(int64_t game_id) { game_id_ = game_id; }
    void SetVirtualId(int64_t virtual_id) { virtual_id_ = virtual_id; }
    void SetFunctionType(std::string function_type) { function_type_ = function_type; }

protected:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) override;
    gfx::Size GetPreferredSize() override;
    void OnPaintBackground(gfx::Canvas* canvas) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) OVERRIDE;

    // NotificationObserver
    void Observe(int type, const base::NotificationSource& source,
        const base::NotificationDetails& details) override;

    // bililive::OBSStatusDelegate
    void OnRecordingStarting() override;
    void OnStartRecording() override;
    void OnRecordingStopping() override;
    void OnStopRecording(const base::FilePath& video_path) override;
    void OnRecordingError(const base::FilePath& video_path, obs_proxy::RecordingErrorCode error_code,
        const std::wstring& error_message) override;

    // LivehimeLiveRoomObserver
    void OnOpenLiveRoom() override;
    void OnOpenLiveRoomSuccessed(const secret::LiveStreamingService::StartLiveInfo& start_live_info) override;
    void OnOpenLiveRoomError(const std::wstring& error_msg, int error_code, bool need_face_auth, const std::string& qr) override;
    void OnPreLiveRoomStartStreaming(bool is_restreaming) override;
    void OnLiveRoomStreamingStarted(const bililive::StartStreamingDetails& details) override;
    void OnPreLiveRoomStopStreaming() override;
    void OnLiveRoomStreamingStopped() override;
    void OnLiveRoomClosed(bool is_restreaming, const secret::LiveStreamingService::StartLiveInfo& start_live_info) override;
    void OnLiveRoomStreamingError(obs_proxy::StreamingErrorCode streaming_errno, const std::wstring& error_message, int error_code) override;
    void OnLiveRoomStreamingRetryNextAddr(int index, int total) override;
    void OnCloseLiveRoom() override;
    void OnNoticeAuthNeedSupplement(const secret::LiveStreamingService::StartLiveNotice& notice) override;

    // LivehimeLiveRoomObserver
    void OnEnterIntoThirdPartyStreamingMode() override;
    void OnLeaveThirdPartyStreamingMode() override;

    // OBSUIProxyObserver
    void OnAudioSourceActivate(obs_proxy::VolumeController* audio_source) override;

    // BililiveBroadcastObserver
    void NewAnchorNormalNotifyDanmaku(const AnchorNormalNotifyInfo& info) override;

private:
    void InitViews();
    void InitData();

    bool GetRenderFailedProcess(base::string16& process, const std::vector<std::pair<int, base::string16>>& process_vector);

    void TpsStopLiveEndDialog(const base::string16& btn, void* data);

    void OnTimeNotify(bool book_hide);

    void ShowFauxAudientEffect(bool show, bool landscape);

    void LiveStreamButtonClick();

    void SetTheRightTextOfLiveBtn(const base::string16& text);
    void ShowWarningBubble(views::View* view, const base::string16& text);
    void OnRealNameAuthTimer();

    void OnRequestGetAnchorInnerOperatingIdRes(bool valid_response, int code, const std::string& unique_id);
    void OnRequestGetShoppingCartStatusRes(bool valid_response, int code, const int& status);

    void StartAnchorEcommerceStatusLoop();
    void EndAnchorEcommerceStatusLoop();
    void AnchorEcommerceStatusLoop();
    void GetAnchorEcommerceStatus();
    void GetShoppingCartStatus();

    void GetPreLiveConf(bool switch_area);

    void StopLiveStream();

    void GetLiveDelayMeasureTime();

    void OnUpdateSEITimer();
    void OnGetLiveDelayMeasureTimer();

private:
    PrefChangeRegistrar profile_pref_registrar_;
    base::NotificationRegistrar notifation_register_;

    LivehimeTopToolbarView* top_toolbar_view_ = nullptr;
    LivehimeFunctionLabelButton* preview_switch_btn_ = nullptr;
    ToolBarVolumeCtrl* system_volume_ctrl_ = nullptr;
    ToolBarVolumeCtrl* mic_volume_ctrl_ = nullptr;
    BililiveImageButton* sound_effect_button_ = nullptr;
    ToolBarRecordLiveButton *record_btn_ = nullptr;
    ToolBarRecordLiveButton* live_btn_ = nullptr;

    bool show_sys_warn_ = true;
    bool show_mic_warn_ = true;
    bool load_roominfo_tips_ = false;
    bool more_five_min_ = false;
    base::string16 sys_render_warn_text_;
    LivehimeHoverTipButton* sys_render_warn_tip_ = nullptr;

    std::unique_ptr<ui::ViewProp> appmng_btn_prop_;
    std::unique_ptr<bililive::OBSStatusMonitor> obsmonitor_;
    std::vector<std::pair<int, base::string16>> process_vector_;

    base::Closure cpm_last_effect_task_;

    base::RepeatingTimer<ToolbarViddupView> real_name_auth_timer_;
    base::RepeatingTimer<ToolbarViddupView> update_SEI_timer_;
    base::RepeatingTimer<ToolbarViddupView> live_dalay_retry_timer_;

    std::thread anchor_ecommerce_thread_;
    bool anchor_ecommerce_ = false;
    bool anchor_ecommerce_status_loop_ = false;
    bool switch_area_ = false;
    int source_event_ = 1;
    int64_t resource_id_ = 0;
    int64_t game_id_ = 0;
    int64_t virtual_id_ = 0;
    std::string function_type_;
    std::unique_ptr<SingIdentifyPropertyPresenter> sing_presenter_ = nullptr;
    int cur_live_delay_retry_ = 0;

    base::WeakPtrFactory<ToolbarViddupView> weakptr_factory_;
    DISALLOW_COPY_AND_ASSIGN(ToolbarViddupView);
};

#endif  //BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TOOLBAR_TOOLBAR_VIEW_H