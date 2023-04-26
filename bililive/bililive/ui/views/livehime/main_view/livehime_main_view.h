#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_MAIN_VIEW_LIVEHIME_MAIN_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_MAIN_VIEW_LIVEHIME_MAIN_VIEW_H_

#include "base/memory/scoped_ptr.h"
#include "base/notification/notification_observer.h"
#include "base/notification/notification_registrar.h"

#include "ui/views/controls/menu/menu_delegate.h"

#include "bililive/bililive/livehime/live_model/live_model_controller.h"
#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"
#include "bililive/bililive/livehime/pluggable/pluggable_controller.h"
#include "bililive/bililive/status_icons/status_icon.h"
#include "bililive/bililive/status_icons/status_tray.h"
#include "bililive/bililive/status_icons/status_icon_observer.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"
#include "stretch_control_view.h"

namespace obs_proxy{
    class Scene;
    class SceneItem;
}

class TitleBarView;
class LivehimePreviewView;
class TabAreaView;
class LivehimeStatusBarView;
class SceneItemMenu;
class MeleePkStatusView;
class FinalTaskView;
class FauxAudientEffectView;
class DanmakuMergedMainView;
class DmkhimeWindowBridge;
class LivehimeTopToolbarView;
class MainNoviceGuideView;

class BililiveOBSView
    : public BililiveWidgetDelegate
    , public base::NotificationObserver
    , public StatusIconObserver
    , public views::MenuDelegate
    , public BililiveBroadcastObserver
    , BililiveNonTitleBarFrameViewDelegate
    , LiveModelControllerObserver
    , public PresetMaterialBridge
{
public:
    static const char kViewClassName[];
    static BililiveOBSView *CreateBililiveObsView();

public:
    BililiveOBSView();
    virtual ~BililiveOBSView();

    virtual TabAreaView *tabarea_view()
    {
        return nullptr;
    }

    virtual gfx::Rect get_preview_rect();

    virtual bool IsWinddowMinimized() {
        return GetWidget()->IsMinimized();
    }

    virtual bool GetInstalledResult()const { return is_install_ios_drive_; }

    //virtual void ShowGuide(bililive::NoviceGuideAction action);
    //virtual bool IsGuideShow() const;

    virtual void DisplaySysBalloon(const base::string16& title, const base::string16& msg);

    virtual void AttachMergedDanmakuMainView(views::View* danmaku_merged_view);

    virtual void ShowDanmakuMergedView(bool show);
    virtual void RequestCoverImage();
    virtual void AddPluggableInfo(const PluggableInfo& info);
    virtual void SetRoomTitleEditFocus();

    virtual void OpenChatRoomView();
    virtual void CloseChatRoomView();

protected:
    // WidgetDelegate
    views::View *GetContentsView() override{ return this; }
    bool CanMaximize() const override { return true; }
    bool CanResize() const override { return true; };
    string16 GetWindowTitle() const override;
    views::NonClientFrameView* CreateNonClientFrameView(views::Widget *widget) override;
    bool ExecuteWindowsCommand(int command_id) override;
    void OnWidgetMove() override;

    // View
    const char *GetClassName() const override{ return kViewClassName; }
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) override;
    void OnBoundsChanged(const gfx::Rect& previous_bounds) override;
    //gfx::Size GetPreferredSize() override;
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
    void ProcessingPresetMaterialMsg(PresetMaterial::PresetMaterialMsgInfo msgInfo) override;

private:
    void InitViews();
    void UninitViews();
    void ShowTray();
    void UpdateMenu();

    LivehimePreviewView *preview_view()
    {
        return nullptr;
    }

private:
    scoped_ptr<StatusTray> status_tray_;
    StatusIcon *status_icon_ = nullptr;

    scoped_refptr<BililiveOBS> bililive_obs_;

    base::WeakPtrFactory<BililiveOBSView> weakptr_factory_;

    bool first_start_ = true;
    bool module_entrance_finish_ = false;
    bool is_install_ios_drive_ = false;
    DISALLOW_COPY_AND_ASSIGN(BililiveOBSView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_MAIN_VIEW_LIVEHIME_MAIN_VIEW_H_