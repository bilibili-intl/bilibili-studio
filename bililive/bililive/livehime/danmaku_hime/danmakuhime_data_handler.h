#ifndef BILILIVE_BILILIVE_LIVEHIME_DANMAKU_HIME_DANMAKUHIME_DATA_HANDLER
#define BILILIVE_BILILIVE_LIVEHIME_DANMAKU_HIME_DANMAKUHIME_DATA_HANDLER

#include "base/notification/notification_observer.h"

#include "bililive/bililive/livehime/danmaku_hime/danmaku_hime_pref_service.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_structs.h"

#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_widget.h"

#include "ui/views/animation/bounds_animator.h"
#include "ui/views/widget/widget_observer.h"

class DanmakuSplitMainView;
class DanmakuMergedMainView;
class DanmakuInteractionView;
class BililiveOBSView;

enum DanmakuWindowsType
{
    Windows_Type_Gift = 0,
    Windows_Type_Interaction,
    Windows_Type_activity_task,
    Windows_Type_All
};

enum GuardAchievementType
{
    GA_TYPE_UPGRADE = 1,
    GA_TYPE_DOWNGRADE = 2,
};

enum AssistantPanel
{
    OPEN_MELEE = 1000,
    OPEN_SHARE = 2000,
    OPEN_NOTICE = 3000,
    OPEN_CAMERA = 4000,
};



class DanmakuhimeDataHandler :
    public base::NotificationObserver,
    public BililiveBroadcastObserver,
    public LivehimeLiveRoomObserver,
    public views::WidgetObserver,
    public LivehimeWebBrowserDelegate
{
public:

    static const int64_t COMBO_FILTER_VALUE[4];
    static const int64_t GIFT_FILTER_VALUE[4];
    static const int64_t NAVY_FILTER_VALUE[3];

    static DanmakuhimeDataHandler* GetInstance();

    static bool FilterPaintMessage(HWND hwnd);

    DanmakuhimeDataHandler();
    virtual ~DanmakuhimeDataHandler();

    //DanmakuSplitMainView* GetDanmakuSplitMainView();
    virtual DanmakuMergedMainView* GetDanmakuMergedMainView();
    virtual DanmakuInteractionView* GetDanmakuInteractionView();

    virtual void SetMainView(BililiveOBSView* main_view);
    virtual void Init(BililiveOBSView* main_view);
    virtual void Uninit();

    virtual void Paint();
    virtual void SetSuspendingScrollingEnabled(bool enabled);
    virtual void SetHardwareAccelerationEnabled(bool enabled);
    virtual void ShowFreeGifts(bool show);
    virtual void SetGiftEffectsEnabled(bool enabled);
    virtual void SetGiftIconEnabled(bool enabled);
    virtual void SetDanmakuHimeOpacity(DanmakuWindowsType view_type,int opacity);
    virtual void SetDanmakuFontSize(DanmakuWindowsType view_type, int size);
    virtual void SetScrollingFluencyIndex(int index);
    virtual void SwitchTheme(DanmakuWindowsType view_type, int theme, bool force = false);
    virtual void SwitchTheme(DanmakuWindowsType view_type, dmkhime::Theme theme, bool force = false);
    virtual void ShowUserEnterEffect(bool show);
    virtual void ShowFaceEffect(bool show);
    virtual bool ShowFaceUISwitch();
    virtual bool JurdgeFaceEffectSwitch();
    virtual void SetUserEnterFilter(int filter);
    virtual void SetUserEnterMinMedalLevel(int medal_level);
    virtual void SetBlockLotteryEnabled(bool enabled);
    virtual void ShowChargedGifts(bool show);
    virtual void SetChargedGiftFilterEnabled(bool enabled);
    virtual void SetChargedGiftCostFilter(int gold);
    virtual void AddTestDanmaku(DanmakuData& data);

    virtual void CommonNoticeDanmakuOpenWeb(const std::string& url);
    virtual int GetRenderViewCurHeight(DanmakuWindowsType view_type);

protected:

    // BililiveBroadcastObserver
    void NewDanmaku(const DanmakuInfo& danmaku) override;
    void NewNoticeDanmaku(const NoticeDanmakuInfo& danmaku) override;
    void NewCommonNoticeDanmaku(const CommonNoticeDanmakuInfo& danmaku) override;
    void DanmakuStatus(int status) override;

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
    void AddGiftEffect(const DanmakuData& data);
    void AddInteractionViewGiftEffect(const DanmakuData& data);
    void AddDanmakuToRenderViews(const DanmakuData& data);
    void AddSuperChatMessageDanmaku(const secret::LiveStreamingService::MarkednessMessageInfo& danmaku);
    void SendToTTS(const DanmakuData& data);
    bool ProcessComboRecords(std::map<std::string, ComboRecord>& records, const std::string& combo_id, int64_t combo_num);

    void SwitchToMergedGiftTheme();
    void SwitchToMergedInteractionTheme();
    void SwitchToMergedActivityTaskTheme();

    void SwitchToSplitGiftTheme();
    void SwitchToSplitInteractionTheme();
    void SwitchToSplitActivityTaskTheme();
    void SwitchToSplitMainViewShowModify();

    void ReportWindowsStatusChangedEvent();
    void RedrawRenderViewsBefore(DanmakuWindowsType view_type);
    gfx::Rect GetDanmukaViewPrefRect(DanmakuWindowsType view_type, int x, int y, int width, int height);
    gfx::Rect GetDefaultPopRect(DanmakuWindowsType view_type);

    void EnableAllEffect(bool enable);

    static DanmakuhimeDataHandler                                   danmaku_data_handler_instance_;

    DanmakuMergedMainView*                                          danmaku_merged_main_view_ = nullptr;
    DanmakuInteractionView*                                         interaction_view_ = nullptr;

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
    int64_t                                                         total_gold_coin_ = 0;   //礼物总价值
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

#endif
