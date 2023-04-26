#ifndef BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_MAIN_VIEW_LIVEHIME_MAIN_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_MAIN_VIEW_LIVEHIME_MAIN_VIEW_H_

#include "base/memory/scoped_ptr.h"
#include "base/notification/notification_observer.h"
#include "base/notification/notification_registrar.h"

#include "ui/views/controls/menu/menu_delegate.h"

#include "bililive/bililive/livehime/live_model/live_model_controller.h"
#include "bililive/bililive/livehime/pluggable/pluggable_controller.h"
#include "bililive/bililive/status_icons/status_icon.h"
#include "bililive/bililive/status_icons/status_tray.h"
#include "bililive/bililive/status_icons/status_icon_observer.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"

#include "bililive/bililive/ui/views/viddup/main_view/stretch_control_viddup_view.h"
#include "bililive/bililive/viddup/server_broadcast/broadcast_viddup_service.h"


namespace obs_proxy{
    class Scene;
    class SceneItem;
}

class TitleBarViddupView;
class LivehimePreviewView;
class ToolbarViddupView;
class TabAreaViddupView;
class LivehimeStatusBarView;
class SceneItemMenu;
class MeleePkStatusView;
class FinalTaskView;
class FauxAudientEffectView;
class DanmakuMergedMainView;
class DmkhimeWindowBridge;
class LivehimeTopToolbarViddupView;
class MainNoviceGuideView;

class BililiveOBSViddupView
    : public BililiveOBSView
{
public:
    static const char kViewClassName[];
    static BililiveOBSView *CreateBililiveObsView();

public:

    gfx::Rect get_preview_rect() override;

    bool IsWinddowMinimized() override {
        return GetWidget()->IsMinimized();
    }

    bool GetInstalledResult() const override { return is_install_ios_drive_; }
    void DisplaySysBalloon(const base::string16& title, const base::string16& msg) override;
    void AttachMergedDanmakuMainView(views::View* danmaku_merged_view) override;
    void ShowDanmakuMergedView(bool show) override;
    void RequestCoverImage() override;
    void AddPluggableInfo(const PluggableInfo& info) override;
    void SetRoomTitleEditFocus() override;

    void OpenChatRoomView() override;
    void CloseChatRoomView() override;

protected:
    // WidgetDelegate
    views::View *GetContentsView() override{ return this; }
    bool CanMaximize() const override { return true; }
    bool CanResize() const override { return true; };
    string16 GetWindowTitle() const override;
    //views::NonClientFrameView* CreateNonClientFrameView(views::Widget *widget) override;
    bool ExecuteWindowsCommand(int command_id) override;
    void OnWidgetMove() override;

    // View
    const char *GetClassName() const override{ return kViewClassName; }
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) override;
    void OnBoundsChanged(const gfx::Rect& previous_bounds) override;
    gfx::Size GetPreferredSize() override;
    View* GetEventHandlerForPoint(const gfx::Point& point) override;
    void OnThemeChanged() override;
    void OnPaintBackground(gfx::Canvas* canvas) override;
    void ChildPreferredSizeChanged(View* child) override;

    // drag
    bool GetDropFormats(
        int *formats,
        std::set<ui::OSExchangeData::CustomFormat> *custom_formats) override;
    bool AreDropTypesRequired() override;
    bool CanDrop(const ui::OSExchangeData &data) override;
    void OnDragEntered(const ui::DropTargetEvent &event) override;
    int OnDragUpdated(const ui::DropTargetEvent &event) override;
    void OnDragExited() override;
    int OnPerformDrop(const ui::DropTargetEvent &event) override;

    // MenuDelegate
    void ExecuteCommand(int command_id, int event_flags) override;

    // BililiveNonTitleBarFrameViewDelegate
    int NonClientHitTest(const gfx::Point &point) override;

    // StatusIconObserver
    void OnStatusIconClicked() override;

    // NotificationObserver
    void Observe(int type,
        const base::NotificationSource& source,
        const base::NotificationDetails& details) override;

    // WidgetObserver
    void OnWidgetVisibilityChanged(views::Widget* widget, bool visible) override;
    void OnWidgetDestroying(views::Widget* widget) override;

    // BililiveBroadcastObserver
    void NewDanmaku(const DanmakuInfo& danmaku) override;

    // LiveModelControllerObserver
    void OnLiveLayoutModelChanged(bool user_invoke) override;
    void OnLiveRoomPreviewStatusChanged(bool show, bool landscape) override;

    //PresetMaterialBridge
    void ProcessingPresetMaterialMsg(PresetMaterial::PresetMaterialMsgInfo msgInfo)override;

private:
    BililiveOBSViddupView();
    virtual ~BililiveOBSViddupView();

    void InitViews();
    void UninitViews();
    void ShowTray();
    void UpdateMenu();
    void RecordLastBounds();
    void PostUIInitialized();
    void NotifyRequiredComponentsMissing();
    void SchedulePluggable();
    void ShowVideoPkByThirdPartyTips(const std::string& msg);
    void ShowGoldPkMatchingView();

    void PostUiNoviceGuideFinish();
    void OnLiveVerifyNotice(const DanmakuInfo& danmaku);

    LivehimePreviewView *preview_view()
    {
        return preview_view_;
    }

    void HandlePresetMaterial();
    void OnRequestGetLiveRecordTotalTimeRes(bool success, int64_t total_time);
    void EnterClientView();
    void EnterRealNameAuthView();
    void EnterMainNoviceGuideView();
    void PreEnterClientView();
    void ProcPluggableCmd();
    void PostUiAddPluggable(const PluggableInfo& info);
    void PvEnventReport();

private:
    bool start_record_bounds_ = false;
    bool request_get_live_record_total_time_finished_ = false;
    TitleBarViddupView* titlebar_view_ = nullptr;
    views::View* main_content_view_ = nullptr;

    LivehimePreviewView* preview_view_ = nullptr;
    FauxAudientEffectView* faux_view_ = nullptr;

    ToolbarViddupView* toolbar_view_ = nullptr;
    views::View* tabarea_dummy_view_ = nullptr;
    TabAreaViddupView* tabarea_view_ = nullptr;
    StretchControlViddupView* tabarea_stretch_view_ = nullptr;
    LivehimeStatusBarView* status_bar_view_ = nullptr;
    LivehimeTopToolbarViddupView* top_toolbar_view_ = nullptr;

    views::View* acttually_client_view_ = nullptr;
    views::View* client_view_ = nullptr;
    views::View* danmaku_dummy_view_ = nullptr;
    views::View* main_novice_guide_view_ = nullptr;

    int melee_seq_id_ = -1;
    MeleePkStatusView* melee_pk_status_view_ = nullptr;

    int melee_end_win_task_id_ = -1;
    FinalTaskView* melee_final_task_view_ = nullptr;

    base::NotificationRegistrar notifation_register_;

    scoped_ptr<StatusTray> status_tray_;
    StatusIcon *status_icon_ = nullptr;

    scoped_refptr<BililiveOBS> bililive_obs_;

    base::WeakPtrFactory<BililiveOBSViddupView> weakptr_factory_;

    std::string run_source_;
    std::string activity_id_;
    std::string open_app_url_;
    std::string open_app_uuid_;
    std::string open_app_buvid_;
    std::string open_spmid_;
    std::string function_type_;
    std::string web_link_open_report_;

    std::queue<PluggableInfo> pluggable_info_queue_;
    bool novice_guide_finish_ = false;
    bool first_start_ = true;
    bool module_entrance_finish_ = false;
    bool is_install_ios_drive_ = false;

    DISALLOW_COPY_AND_ASSIGN(BililiveOBSViddupView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_MAIN_VIEW_LIVEHIME_MAIN_VIEW_H_