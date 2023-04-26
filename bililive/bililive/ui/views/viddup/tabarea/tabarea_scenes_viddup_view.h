#ifndef BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TABAREA_TAB_CONTENT_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TABAREA_TAB_CONTENT_VIEW_H

#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/ui/views/controls/navigation_bar.h"
#include "bililive/bililive/ui/views/viddup/tabarea/tabarea_tab_strip_viddup_view.h"
#include "bililive/secret/public/live_streaming_service.h"
#include "bililive/bililive/livehime/function_control/app_function_controller.h"


#include "obs/obs_proxy/public/proxy/obs_proxy.h"

class TabAreaViddupView;
class TabAreaSceneContentViddupView;
class TabAreaMaterialsAreaView;
class LivehimeModuleView;
class TabAreaActivityCenterView;
struct BannerInfo;

namespace {
    class  ControlButtonView;
}

// The TAB control Scene content page contains n specific scene pages
class TabAreaScenesViddupView
    : public views::View
    , views::ButtonListener
    , obs_proxy_ui::OBSUIProxyObserver
    , LivehimeLiveRoomObserver
    , NavigationBarListener
    , LiveModelControllerObserver
    , AppFunctionCtrlObserver
{
    TabAreaScenesViddupView();
    virtual ~TabAreaScenesViddupView();

    gfx::Rect GetCameraItemEditBtnBounds(const std::string& name) const;
    void ShowOperateView(const std::string& name, bool show);

    void OpenChatRoomView();
    void CloseChatRoomView();

protected:
    // View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) override;
    void Layout() override;
    bool OnMousePressed(const ui::MouseEvent& event) override;
    bool OnMouseDragged(const ui::MouseEvent& event) override;
    gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override;
    void ChildPreferredSizeChanged(views::View* child) override;
    void OnPaintBackground(gfx::Canvas* canvas) override;

    // OBSUIProxyObserver
    void OnCurrentSceneChanged(int scene_index) override;

    // NavigationBarListener
    void NavigationBarSelectedAt(int strip_id) override;

    // LivehimeLiveRoomObserver
    void OnEnterIntoThirdPartyStreamingMode() override;
    void OnLeaveThirdPartyStreamingMode() override;

    // LiveModelControllerObserver
    void OnLiveLayoutModelChanged(bool user_invoke) override;

    //ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    //VtuberPKUIObserver
    //void OnVtuberSourceLockChanged(bool lock) override;

    // AppFunctionCtrlObserver
    void OnAllAppKVCtrlInfoCompleted() override;

private:
    gfx::Rect GetDragSplitBounds();
    int CheckMaterialAreaValidHeight(int check_height);
    int DetermineMaterialAreaMaxHeight();
    void DoPendingLayout();
    gfx::Rect GetMiddleAreaBounds() const;

private:
    friend class TabAreaViddupView;

    NavigationBar* tabbed_pane_ = nullptr;
    LivehimeModuleView* livehime_module_view_ = nullptr;
    ControlButtonView* control_view_ = nullptr;
    ControlButtonView* control_plugin_view_ = nullptr;

    bool is_initing_ = true;
    bool is_first_layout_ = true;
    bool need_schedule_tabbed_sel_change_ = true;
    int materials_view_height_ = 0;
    gfx::Point start_drag_mouse_down_point_;
    int start_drag_materials_view_height_ = 0;

    std::vector<TabAreaTabStripViddupView*> strip_views_;
    std::vector<TabAreaSceneContentViddupView*> content_views_;
    int current_show_scene_index_;

    base::WeakPtrFactory<TabAreaScenesViddupView> weak_ptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(TabAreaScenesViddupView);
};

#endif  //BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TABAREA_TAB_CONTENT_VIEW_H