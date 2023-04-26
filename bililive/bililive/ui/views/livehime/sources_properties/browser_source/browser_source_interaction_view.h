#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_BROWSER_SOURCE_INTERACTION_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_BROWSER_SOURCE_INTERACTION_VIEW_H_

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"

#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"

class LivehimeOBSPreviewControl;

class LivehimeBrowserSourceInteractionView
    : public BililiveWidgetDelegate
{
public:
    static void ShowForm(gfx::NativeView par_hwnd, obs_proxy::SceneItem* scene_item);

protected:
    // View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) override;
    gfx::Size GetPreferredSize() override;
    void Layout() override;

    // WidgetDelegate
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }
    views::View *GetContentsView() override { return this; }
    void WindowClosing() override;
    bool CanMaximize() const override { return true; }
    bool CanResize() const override { return true; };

    // BililiveWidgetDelegate
    void OnCreateNonClientFrameView(views::NonClientFrameView *non_client_frame_view) override;

private:
    LivehimeBrowserSourceInteractionView(obs_proxy::SceneItem* scene_item);
    virtual ~LivehimeBrowserSourceInteractionView();

    void InitViews();

private:
    LivehimeOBSPreviewControl* preview_ctrl_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(LivehimeBrowserSourceInteractionView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_BROWSER_SOURCE_INTERACTION_VIEW_H_