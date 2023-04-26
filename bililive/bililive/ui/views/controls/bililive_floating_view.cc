#include "bililive_floating_view.h"

#include "ui/views/layout/fill_layout.h"


class BililiveFloatingWidget : public views::Widget
{
public:
    explicit BililiveFloatingWidget(BililiveFloatingViewDelegate *delegate, BililiveFloatingView *floating_view)
        : delegate_(delegate)
        , floating_view_(floating_view){}
    virtual ~BililiveFloatingWidget(){}

    void SetFloatingViewDelegate(BililiveFloatingViewDelegate *delegate)
    {
        delegate_ = delegate;
    };

protected:
    virtual void OnKeyEvent(ui::KeyEvent* event) OVERRIDE
    {
        if (delegate_)
        {
            delegate_->OnFloatingViewKeyEvent(floating_view_, event);
        }
        __super::OnKeyEvent(event);
    };

    virtual void OnMouseEvent(ui::MouseEvent* event) OVERRIDE
    {
        if (delegate_)
        {
            delegate_->OnFloatingViewMouseEvent(floating_view_, event);
        }
        __super::OnMouseEvent(event);
    };

    virtual void OnNativeFocus(gfx::NativeView old_focused_view) OVERRIDE
    {
        if (delegate_)
        {
            delegate_->OnFloatingViewFocus(floating_view_);
        }
        __super::OnNativeFocus(old_focused_view);
    };

    virtual void OnNativeBlur(gfx::NativeView new_focused_view) OVERRIDE
    {
        if (delegate_)
        {
            delegate_->OnFloatingViewBlur(floating_view_);
        }
        __super::OnNativeBlur(new_focused_view);
    };

    virtual void OnNativeWidgetDestroying() OVERRIDE
    {
        if (delegate_)
        {
            delegate_->OnFloatingViewDestroying(floating_view_);
        }
        __super::OnNativeWidgetDestroying();
    };

private:
    BililiveFloatingViewDelegate *delegate_;
    BililiveFloatingView *floating_view_;
};



class BililiveFloatingWidgetDelegate : public views::WidgetDelegateView
{
public:
    BililiveFloatingWidgetDelegate(BililiveFloatingView *floating_view)
        : floating_view_(floating_view){}
    virtual ~BililiveFloatingWidgetDelegate(){}

    void UpdateContentsView(bool delete_old_content_view)
    {
        RemoveAllChildViews(delete_old_content_view);
        if (floating_view_->content_view())
        {
            AddChildView(floating_view_->content_view());
            Layout();
        }
        SchedulePaint();
    }

protected:
    views::View* GetContentsView() override { return this; }

    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override
    {
        if (details.child == this && details.is_add)
        {
            SetLayoutManager(new views::FillLayout());
            AddChildView(floating_view_->content_view());
        }
    }

private:
    BililiveFloatingView *floating_view_;

    DISALLOW_COPY_AND_ASSIGN(BililiveFloatingWidgetDelegate);
};


ObserverList<BililiveFloatingObserver> BililiveFloatingView::observers_;

void BililiveFloatingView::NotifyMainWidgetInitCompleted()
{
    FOR_EACH_OBSERVER(BililiveFloatingObserver, observers_,
        OnMainWidgetInitCompleted());
}

// static
const char BililiveFloatingView::kViewClassName[] = "BililiveFloatingView";

BililiveFloatingView::BililiveFloatingView(views::View *related_view, views::View *floating_widget_content_view,
    bool accept_events/* = true*/, bool translucent/* = true*/)
    : related_view_(related_view)
    , floating_widget_content_view_(floating_widget_content_view)
    , translucent_(translucent)
    , accept_events_(accept_events)
    , floating_widget_(NULL)
    , floating_hwnd_(NULL)
    , delegate_(NULL)
    , last_visible_(true)
{
    if (!floating_widget_content_view_)
    {
        floating_widget_content_view_ = new views::View();
        floating_widget_content_view_->set_background(views::Background::CreateSolidBackground(SkColorSetARGB(0, 0, 0, 0)));
    }
}

BililiveFloatingView::~BililiveFloatingView()
{
}

void BililiveFloatingView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitView();

            DCHECK(GetWidget());
            views::Widget* widget = GetWidget();
            if (widget && strcmp(widget->widget_delegate()->GetWidgetDelegateClassName(),
                BililiveWidgetDelegate::kWidgetDelegateClassName) == 0)
            {
                ((BililiveWidgetDelegate*)widget->widget_delegate())->AddObserver(this);
            }

            observers_.AddObserver(this);
        }
        else
        {
            DCHECK(GetWidget());
            views::Widget* widget = GetWidget();
            if (widget && strcmp(widget->widget_delegate()->GetWidgetDelegateClassName(),
                BililiveWidgetDelegate::kWidgetDelegateClassName) == 0)
            {
                ((BililiveWidgetDelegate*)widget->widget_delegate())->RemoveObserver(this);
            }

            observers_.RemoveObserver(this);

            UninitView();
        }
    }
}

void BililiveFloatingView::InitView()
{
    views::FillLayout *layout = new views::FillLayout();
    SetLayoutManager(layout);
    if (related_view_)
    {
        AddChildView(related_view_);
    }

    views::Widget::InitParams params(views::Widget::InitParams::Type::TYPE_WINDOW_FRAMELESS);
    params.delegate = new BililiveFloatingWidgetDelegate(this);
    params.remove_standard_frame = true;
    params.remove_taskbar_icon = true;
    params.accept_events = accept_events_;
    if (translucent_)
    {
        params.opacity = views::Widget::InitParams::WindowOpacity::TRANSLUCENT_WINDOW;
    }
    if (parent())
    {
        if (0 == strcmp(parent()->GetClassName(), BililiveFloatingView::kViewClassName))
        {
            params.parent = ((BililiveFloatingView*)parent())->GetNativeWindow();
        }
    }
    if (!params.parent && GetWidget())
    {
        params.parent = GetWidget()->GetNativeWindow();
    }
    InitWidgetParam(params);
    floating_widget_ = new BililiveFloatingWidget(delegate_, this);
    floating_widget_->set_frame_type(views::Widget::FRAME_TYPE_FORCE_CUSTOM);
    floating_widget_->Init(params);
    floating_widget_->Hide();

    floating_hwnd_ = floating_widget_->GetNativeWindow();
    OnWidgetCreated(floating_hwnd_);

    if (GetWidget())
    {
        GetWidget()->AddObserver(this);
    }
}

void BililiveFloatingView::UninitView()
{
    if (GetWidget())
    {
        GetWidget()->RemoveObserver(this);
    }

    Close();
}

void BililiveFloatingView::Layout()
{
    __super::Layout();

    SetFloatingWindowPosition();
}

void BililiveFloatingView::OnBoundsChanged(const gfx::Rect& previous_bounds)
{
    SetFloatingWindowPosition();
}

void BililiveFloatingView::OnVisibleBoundsChanged()
{
    SetFloatingWindowPosition();
}

void BililiveFloatingView::VisibilityChanged(views::View* starting_from, bool is_visible)
{
    bool temp = last_visible_;
    last_visible_ = visible() && IsDrawn();
    if (temp != last_visible_)
    {
        if (last_visible_)
        {
            Show();

            InvalidateLayout();
            Layout();
        }
        else
        {
            Hide();
        }
    }
}

void BililiveFloatingView::OnWidgetVisibilityChanged(views::Widget* widget, bool is_visible)
{
    if (is_visible)
    {
        if (visible() && IsDrawn())
        {
            Show();

            InvalidateLayout();
            Layout();
        }
    }
    else
    {
        Hide();
    }
}

void BililiveFloatingView::OnWidgetDestroyed(views::Widget* widget)
{
    if ((GetWidget() == widget) && strcmp(widget->widget_delegate()->GetWidgetDelegateClassName(),
        BililiveWidgetDelegate::kWidgetDelegateClassName) == 0)
    {
        ((BililiveWidgetDelegate*)widget->widget_delegate())->RemoveObserver(this);
    }
}

void BililiveFloatingView::OnWidgetBoundsChanged(views::Widget* widget, const gfx::Rect& new_bounds)
{
    SetFloatingWindowPosition();
}

void BililiveFloatingView::SetFloatingWindowPosition()
{
    if (::IsWindow(floating_hwnd_) && IsDrawn())
    {
        OnMainWidgetInitCompleted();
    }
}

void BililiveFloatingView::Close()
{
    if (::IsWindow(floating_hwnd_))
    {
        if (floating_widget_)
        {
            floating_widget_->Close();
            floating_widget_ = NULL;
        }
        floating_hwnd_ = NULL;
    }
}

void BililiveFloatingView::Hide()
{
    if (::IsWindow(floating_hwnd_))
    {
        if (floating_widget_)
        {
            floating_widget_->Hide();
        }
    }
}

void BililiveFloatingView::Show()
{
    if (::IsWindow(floating_hwnd_))
    {
        if (floating_widget_)
        {
            floating_widget_->Show();
        }
    }
}

void BililiveFloatingView::SetFloatingViewDelegate(BililiveFloatingViewDelegate *delegate)
{
    delegate_ = delegate;
    if (floating_widget_)
    {
        ((BililiveFloatingWidget*)floating_widget_)->SetFloatingViewDelegate(delegate_);
    }
}

void BililiveFloatingView::SetContentsView(views::View* content_view, bool delete_old_content_view)
{
    floating_widget_content_view_ = content_view;
    static_cast<BililiveFloatingWidgetDelegate*>(floating_widget_->widget_delegate())->
        UpdateContentsView(delete_old_content_view);
}

void BililiveFloatingView::OnMainWidgetInitCompleted()
{
    if (floating_widget_)
    {
        gfx::Rect rect = GetBoundsInScreen();
        floating_widget_->SetBounds(rect);
    }
}

bool BililiveFloatingView::HitTestRect(const gfx::Rect& rect) const
{
    if (!accept_events_)
    {
        return false;
    }
    return __super::HitTestRect(rect);
}

void BililiveFloatingView::OnWidgetEnabledChanged(BililiveWidgetDelegate *widget_delegate, BOOL enabled)
{
    if (widget_delegate->GetWidget() == GetWidget())
    {
        ::EnableWindow(floating_hwnd_, enabled);
        if (delegate_)
        {
            delegate_->OnFloatingViewEnabledChanged(this, enabled);
        }
    }
}
