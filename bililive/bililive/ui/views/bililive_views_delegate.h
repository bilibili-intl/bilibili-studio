#ifndef BILILIVE_BILILIVE_UI_VIEWS_BILILIVE_VIEWS_DELEGATE_H
#define BILILIVE_BILILIVE_UI_VIEWS_BILILIVE_VIEWS_DELEGATE_H


#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "build/build_config.h"
#include "ui/base/accessibility/accessibility_types.h"
#include "ui/views/views_delegate.h"


class BililiveViewsDelegate
    : public views::ViewsDelegate
{
public:
    BililiveViewsDelegate() {}
    virtual ~BililiveViewsDelegate() {}

    virtual void SaveWindowPlacement(const views::Widget *window,
                                     const std::string &window_name,
                                     const gfx::Rect &bounds,
                                     ui::WindowShowState show_state) OVERRIDE;
    virtual bool GetSavedWindowPlacement(
        const std::string &window_name,
        gfx::Rect *bounds,
        ui::WindowShowState *show_state) const OVERRIDE;
    virtual void NotifyAccessibilityEvent(
        views::View *view, ui::AccessibilityTypes::Event event_type) OVERRIDE;
    virtual void NotifyMenuItemFocused(const string16 &menu_name,
                                       const string16 &menu_item_name,
                                       int item_index,
                                       int item_count,
                                       bool has_submenu) OVERRIDE;

    virtual HICON GetDefaultWindowIcon() const OVERRIDE;
    virtual views::NonClientFrameView *CreateDefaultNonClientFrameView(
        views::Widget *widget) OVERRIDE;
    virtual bool UseTransparentWindows() const OVERRIDE;
    virtual void AddRef() OVERRIDE;
    virtual void ReleaseRef() OVERRIDE;
    virtual void OnBeforeWidgetInit(
        views::Widget::InitParams *params,
        views::internal::NativeWidgetDelegate *delegate) OVERRIDE;
    virtual base::TimeDelta GetDefaultTextfieldObscuredRevealDuration() OVERRIDE;

private:
    DISALLOW_COPY_AND_ASSIGN(BililiveViewsDelegate);
};

#endif