#ifndef BILILIVE_BILILIVE_UI_VIEWS_TABAREA_MATERIALS_WIDGET_H_
#define BILILIVE_BILILIVE_UI_VIEWS_TABAREA_MATERIALS_WIDGET_H_

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/controls/navigation_bar.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"


namespace
{
    class MaterialAreaTabStripView;
}

class TabAreaMaterialsView;

class TabAreaMaterialsWidget
    : public BililiveWidgetDelegate
    , LivehimeLiveRoomObserver
{
public:
    TabAreaMaterialsWidget();
    virtual ~TabAreaMaterialsWidget();

    static void ShowWindow();
    static void Close();
    static void Hide();
    static int GetMinimumHeight();

protected:
    // WidgetDelegate
    views::View *GetContentsView() override { return this; }
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }
    void WindowClosing() override;

    // views::View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) override;
    gfx::Size GetPreferredSize() override;

    // LivehimeLiveRoomObserver
    void OnEnterIntoThirdPartyStreamingMode() override;

    void OnWidgetActivationChanged(views::Widget* widget, bool active)override;

private:
    void InitViews();

    NavigationBar* tabbed_pane_ = nullptr;
    MaterialAreaTabStripView* material_strip_ = nullptr;
    MaterialAreaTabStripView* danmaku_strip_ = nullptr;
    TabAreaMaterialsView* materials_view_ = nullptr;
    bool is_install_ios_drive_ = false;


    base::WeakPtrFactory<TabAreaMaterialsWidget> weakptr_factory_;
    DISALLOW_COPY_AND_ASSIGN(TabAreaMaterialsWidget);
};
#endif

