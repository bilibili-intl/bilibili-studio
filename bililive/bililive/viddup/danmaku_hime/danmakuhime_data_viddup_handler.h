#ifndef BILILIVE_BILILIVE_LIVEHIME_VIDDUP_DANMAKU_HIME_DANMAKUHIME_DATA_HANDLER
#define BILILIVE_BILILIVE_LIVEHIME_VIDDUP_DANMAKU_HIME_DANMAKUHIME_DATA_HANDLER

#include "base/notification/notification_observer.h"

#include "bililive/bililive/livehime/danmaku_hime/danmaku_hime_pref_service.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/viddup/server_broadcast/broadcast_viddup_service.h"
#include "bililive/bililive/ui/views/viddup/danmaku_hime/danmaku_interaction_viddup_view.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_structs.h"

#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_widget.h"

#include "ui/views/animation/bounds_animator.h"
#include "ui/views/widget/widget_observer.h"

#include "bililive/bililive/livehime/danmaku_hime/danmakuhime_data_handler.h"

class DanmakuMergedMainViddupView;
class DanmakuInteractionViddupView;

class DanmakuhimeDataViddupHandler :
    public DanmakuhimeDataHandler,
    public BililiveBroadcastViddupObserver
{
public:
    //static const int64_t COMBO_FILTER_VALUE[4];
    //static const int64_t GIFT_FILTER_VALUE[4];
    //static const int64_t NAVY_FILTER_VALUE[3];

    static DanmakuhimeDataHandler* GetInstance();

    static bool FilterPaintMessage(HWND hwnd);

    DanmakuhimeDataViddupHandler();
    virtual ~DanmakuhimeDataViddupHandler();

    //DanmakuSplitMainView* GetDanmakuSplitMainView();
    DanmakuMergedMainViddupView* GetDanmakuMergedMainViddupView();
    DanmakuInteractionViddupView* GetDanmakuInteractionViddupView();

    void SetMainView(BililiveOBSView* main_view) override;
    void Init(BililiveOBSView* main_view) override;
    void Uninit() override;

    void Paint() override;
    void SetSuspendingScrollingEnabled(bool enabled) override;
    void SetHardwareAccelerationEnabled(bool enabled);
    void ShowFreeGifts(bool show);
    void SetGiftEffectsEnabled(bool enabled);
    void SetGiftIconEnabled(bool enabled);
    void SetDanmakuHimeOpacity(DanmakuWindowsType view_type,int opacity);
    void SetDanmakuFontSize(DanmakuWindowsType view_type, int size);
    void SetScrollingFluencyIndex(int index);
    void SwitchTheme(DanmakuWindowsType view_type, int theme, bool force = false);
    void SwitchTheme(DanmakuWindowsType view_type, dmkhime::Theme theme, bool force = false);
    void ShowUserEnterEffect(bool show);
    void ShowFaceEffect(bool show);
    bool ShowFaceUISwitch();
    bool JurdgeFaceEffectSwitch();
    void SetUserEnterFilter(int filter);
    void SetUserEnterMinMedalLevel(int medal_level);
    void SetBlockLotteryEnabled(bool enabled);
    void ShowChargedGifts(bool show);
    void SetChargedGiftFilterEnabled(bool enabled);
    void SetChargedGiftCostFilter(int gold);
    void SetDanmakuTTSEnabled(bool enabled);
    void SetDanmakuTTSSource(const string16& name);
    void SetDanmakuTTSSpeed(int speed);
    void SetDanmakuTTSVolume(int volume);
    void ClearDanmakuTTSQueue();
    void SetFilterEffect(const std::array<int64_t, 5>& filter_value);
    void AddGenericDanmaku(const std::string& img_url, const std::string& img_data, const string16& plain_text, bool use_default_color = true, SkColor color = 0,bool use_backgroundcolor = true);
    void AddLocalLiveAssistantDanmaku(const string16& msg);
    void AddTestDanmaku(DanmakuData& data);

    void CommonNoticeDanmakuOpenWeb(const std::string& url);
    void OnGetUserInfo();
    int GetRenderViewCurHeight(DanmakuWindowsType view_type);

protected:
    // BililiveBroadcastObserver
    void NewDanmaku(const DanmakuInfo& danmaku) override;
    void NewNoticeDanmaku(const NoticeDanmakuInfo& danmaku) override;
    void NewCommonNoticeDanmaku(const CommonNoticeDanmakuInfo& danmaku) override;
    void DanmakuStatus(int status) override;

    //BililiveBroadcastViddupObserver
    void NewDanmaku(const std::vector<DanmakuViddupInfo>& danmaku) override;

    // NotificationObserver
    void Observe(int type, const base::NotificationSource& source, const base::NotificationDetails& details) override;

    // LivehimeLiveRoomObserver
    void OnOpenLiveRoomSuccessed(const secret::LiveStreamingService::StartLiveInfo& start_live_info) override;
    void OnPreLiveRoomStartStreaming(bool is_restreaming) override;
    void OnLiveRoomStreamingStopped() override;
    void OnLiveRoomStreamingError(obs_proxy::StreamingErrorCode streaming_errno, const std::wstring& error_message, int error_code) override;

    // views::WidgetObserver
    void OnWidgetDestroying(views::Widget* widget) override;
    void OnWidgetVisibilityChanged(views::Widget* widget, bool visible) override;
    void OnWidgetBoundsChanged(views::Widget* widget, const gfx::Rect& new_bounds) override;

private:
    enum class AnimStatus
    {
        None,
        In,
        Out,
    };

    struct ComboRecord
    {
        int64_t ts;
        int64_t combo_num;
    };

    void LoadPref();
    void SaveDanmakuStatePref();
    void LoadDanmakuStatePref();
    void LoadEffectFilterSetting();
    void AddDanmakuToRenderViddupViews(DanmakuData data);
    bool ProcessComboRecords(std::map<std::string, ComboRecord>& records, const std::string& combo_id, int64_t combo_num);

    void SwitchToMergedGiftTheme();
    void SwitchToMergedInteractionTheme();
    void SwitchToMergedActivityTaskTheme();

    void SwitchToSplitGiftTheme();
    void SwitchToSplitInteractionTheme();
    void SwitchToSplitActivityTaskTheme();
    void SwitchToSplitMainViewShowModify();

    void RedrawRenderViewsBefore(DanmakuWindowsType view_type);
    gfx::Rect GetDanmukaViewPrefRect(DanmakuWindowsType view_type, int x, int y, int width, int height);
    gfx::Rect GetDefaultPopRect(DanmakuWindowsType view_type);

    void EnableAllEffect(bool enable);

    static DanmakuhimeDataViddupHandler                             danmaku_data_handler_instance_;

    DanmakuMergedMainViddupView*                                          danmaku_merged_main_view_ = nullptr;
    DanmakuInteractionViddupView*                                   interaction_view_ = nullptr;

    BililiveOBSView*                                                main_view_ = nullptr;

    dmkhime::Theme                                                  split_view_theme_ = dmkhime::Theme::SORCERESS;
    DanmakuRenderingParams                                          rendering_params_gift_ = {};
    DanmakuRenderingParams                                          rendering_params_interaction_ = {};
    DanmakuRenderingParams                                          rendering_params_activity_task_ = {};
    bool                                                            is_first_show_ = false;
    bool                                                            is_all_effect_block_ = false;

    bool                                                            is_gift_effects_enabled_ = true;
    bool                                                            is_show_free_gift_ = true;
    bool                                                            is_show_charged_gift_ = true;
    bool                                                            is_charged_gift_filter_enabled_ = false;
    int                                                             charged_gift_min_gold_ = 0;
    bool                                                            is_show_user_enter_effect_ = true;
    bool                                                            is_show_core_user_area_ = true;
    bool                                                            is_show_face_effect_ = true;
    bool                                                            is_lottery_blocking_enabled_ = true;
    bool                                                            is_danmaku_tts_enabled_ = false;
    int                                                             user_enter_filter_ = prefs::DHEF_SHOW_ALL;
    int                                                             min_enter_medal_level_ = 1;
    bool                                                            is_idle_interact_timer_ = false;
    int64_t                                                         send_msgtime_rnd_ = 0;
    AnimStatus                                                      free_anim_status_ = AnimStatus::None;
    AnimStatus                                                      interact_anim_status_ = AnimStatus::None;
    int64_t                                                         combo_ts_ = 0;
    int64_t                                                         total_gold_coin_ = 0;
    gfx::Rect                                                       gift_pop_rect_;
    gfx::Rect                                                       interaction_pop_rect_;
    gfx::Rect                                                       activity_task_pop_rect_;

    std::map<std::string, ComboRecord>                              strip_combo_records_;
    std::map<std::string, ComboRecord>                              danmaku_combo_records_;
    std::unique_ptr<views::BoundsAnimator>                          free_anim_;
    std::queue<DanmakuData>                                         free_queue_;
    std::unique_ptr<base::NotificationRegistrar>                    notifation_register_;
    std::unique_ptr<base::RepeatingTimer<DanmakuhimeDataHandler>>   mouse_timer_;
    base::OneShotTimer<DanmakuhimeDataHandler>                      interact_timer_;
    std::map<int64_t, std::vector<DanmakuData>>                     interact_queue_;
    base::WeakPtrFactory<DanmakuhimeDataHandler>                    weak_ptr_factory_;
    int                                                             room_id_ = -1;
    bool                                                            first_pop_ = false;
};

#endif  //BILILIVE_BILILIVE_LIVEHIME_VIDDUP_DANMAKU_HIME_DANMAKUHIME_DATA_HANDLER
