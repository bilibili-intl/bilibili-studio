#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_FLOATING_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_FLOATING_VIEW_H

#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/widget/widget_observer.h"

#include "bililive_widget_delegate/bililive_widget_delegate.h"

namespace
{
    class BililiveFloatingObserver
    {
    public:
        virtual void OnMainWidgetInitCompleted() = 0;

    protected:
        virtual ~BililiveFloatingObserver() {}
    };
}

class BililiveFloatingWidgetDelegate;
class BililiveFloatingViewDelegate;
class BililiveFloatingView
    : public views::View
    , public views::WidgetObserver
    , public BililiveFloatingObserver
    , public BililiveWidgetDelegateObserver
{
public:
    // The view class name.
    static const char kViewClassName[];

    BililiveFloatingView(views::View *related_view, views::View *floating_widget_content_view,
        bool accept_events = true, bool translucent = true);
    virtual ~BililiveFloatingView();

    static void NotifyMainWidgetInitCompleted();

    void Hide();
    void Show();
    gfx::NativeWindow GetNativeWindow(){ return floating_hwnd_; }
    views::Widget* GetFloatingWidget(){ return floating_widget_; }
    HWND GetSafeHwnd(){ return floating_hwnd_; }
    void SetFloatingViewDelegate(BililiveFloatingViewDelegate *delegate);

    void SetContentsView(views::View* content_view, bool delete_old_content_view);

    void Layout() OVERRIDE;
protected:
    void InitWidgetParam(views::Widget::InitParams &params) {}
    void OnWidgetCreated(gfx::NativeWindow hwnd) {}

    // View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;
    void VisibilityChanged(views::View* starting_from, bool is_visible) override;
    void OnBoundsChanged(const gfx::Rect& previous_bounds) override;
    bool NeedsNotificationWhenVisibleBoundsChange() const override{ return true; }
    void OnVisibleBoundsChanged() override;
    const char* GetClassName() const override { return kViewClassName; }
    bool HitTestRect(const gfx::Rect& rect) const;

    // WidgetObserver
    void OnWidgetBoundsChanged(views::Widget* widget, const gfx::Rect& new_bounds) override;
    void OnWidgetVisibilityChanged(views::Widget* widget, bool is_visible) override;
    void OnWidgetDestroyed(views::Widget* widget) override;

    // BililiveFloatingViewObserver
    void OnMainWidgetInitCompleted() override;

    // BililiveWidgetDelegateObserver
    void OnWidgetEnabledChanged(BililiveWidgetDelegate*, BOOL enabled) override;

    views::View* content_view() const { return floating_widget_content_view_; }
    views::View* content_view() { return floating_widget_content_view_; }

private:
    void SetFloatingWindowPosition();
    void Close();
    void InitView();
    void UninitView();

private:
    friend class BililiveFloatingWidgetDelegate;
    static ObserverList<BililiveFloatingObserver> observers_;
    views::View *related_view_;
    views::View *floating_widget_content_view_;
    views::Widget *floating_widget_;
    HWND floating_hwnd_;
    BililiveFloatingViewDelegate *delegate_;
    bool last_visible_;
    bool translucent_;
    bool accept_events_;

    DISALLOW_COPY_AND_ASSIGN(BililiveFloatingView);
};

class BililiveFloatingViewDelegate
{
public:
    virtual void OnFloatingViewKeyEvent(BililiveFloatingView *floating_view, ui::KeyEvent* event){}
    virtual void OnFloatingViewMouseEvent(BililiveFloatingView *floating_view, ui::MouseEvent* event){}
    virtual void OnFloatingViewFocus(BililiveFloatingView *floating_view){}
    virtual void OnFloatingViewBlur(BililiveFloatingView *floating_view){}
    virtual void OnFloatingViewDestroying(BililiveFloatingView *floating_view) = 0;
    virtual void OnFloatingViewEnabledChanged(BililiveFloatingView *floating_view, BOOL enabled) {}
};

#endif
