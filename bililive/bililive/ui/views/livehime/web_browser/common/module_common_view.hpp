#pragma once

#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"


template<typename TView>
class ModuleCommonNonClientFrameView : public views::NonClientFrameView
{
public:
    explicit ModuleCommonNonClientFrameView(TView* client_content_view)
        : client_content_view_(client_content_view) {}

protected:
    // View
    void Layout() override
    {
        client_view_bounds_ = bounds();
    }

    // NonClientFrameView
    gfx::Rect GetBoundsForClientView() const override {
        return client_view_bounds_;
    }

    gfx::Rect GetWindowBoundsForClientBounds(const gfx::Rect& client_bounds) const override
    {
        return client_bounds;
    }

    int NonClientHitTest(const gfx::Point& point) override
    {
        gfx::Point pt = point;
        views::View::ConvertPointToTarget(this, client_content_view_, &pt);
        return client_content_view_->NonClientHitTest(pt);
    }

    void GetWindowMask(const gfx::Size& size, gfx::Path* window_mask) override {};
    void ResetWindowControls() override {};
    void UpdateWindowIcon() override {};
    void UpdateWindowTitle() override {};

    gfx::Size GetMinimumSize() override
    {
        return client_content_view_->GetMinimumSize();
    }

    gfx::Size GetMaximumSize() override
    {
        return client_content_view_->GetMaximumSize();
    }

private:
    gfx::Rect client_view_bounds_;
    TView* client_content_view_;

    DISALLOW_COPY_AND_ASSIGN(ModuleCommonNonClientFrameView);
};

template<typename TView>
class ModuleCommonView : public BililiveWidgetDelegate
{
public:
    static const int kFrameEdgeThickness;
public:
    ModuleCommonView()
    {

    }

    virtual ~ModuleCommonView()
    {

    }

    static bool Pop(const gfx::Rect rect, views::Widget*parent = nullptr)//返回是否刚创建
    {
        auto widget_view = ModuleCommonView::GetView();

        if (!view_widget_)
        {
            view_widget_ = new views::Widget();
            BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(view_widget_);
            native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_NONE);

            views::Widget::InitParams params(views::Widget::InitParams::TYPE_PANEL);
            params.bounds = rect;
            params.native_widget = native_widget;
            params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;
            params.remove_taskbar_icon = true;
            params.remove_standard_frame = true;
            if (parent)
            {
                params.parent = parent->GetNativeWindow();
            }
            view_widget_->set_frame_type(views::Widget::FRAME_TYPE_FORCE_NATIVE);

            if (widget_view->parent())
            {
                widget_view->RemoveChildView(widget_view);
            }
            BililiveWidgetDelegate::ShowWidget(widget_view, view_widget_, params);
            view_widget_->SetAlwaysOnTop(widget_view->IsPinned());

            DCHECK(view_widget_->non_client_view()->client_view() == ModuleCommonView::GetView()->parent());
            return true;
        }
        else
        {
            if (widget_view->parent())
            {
                widget_view->parent()->RemoveChildView(widget_view);
            }

            view_widget_->non_client_view()->client_view()->AddChildView(widget_view);
            auto size = view_widget_->GetRootView()->size();
            view_widget_->non_client_view()->Layout();
            view_widget_->Show();
            widget_view->Pin(widget_view->IsPinned());

            return false;
        }
    }

    static void Hide()
    {
        if (view_widget_)
        {
            view_widget_->Hide();
        }
    }

    bool IsVisible()
    {
        if (view_widget_)
        {
            return view_widget_->IsVisible();
        }
        return false;
    }

    bool IsPinned()
    {
        return is_pinned_;
    }

    virtual void Pin(bool pin)
    {
        if (GetWidget() && pin != is_pinned_)
        {
            GetWidget()->SetAlwaysOnTop(pin);
        }
        is_pinned_ = pin;
    }

    static TView* GetView()
    {
        if (!view_instance_)
        {
            view_instance_ = new TView();
        }

        return view_instance_;
    }

    static TView* GetViewInstance()
    {
        return view_instance_;
    }

    static views::Widget* GetModuleWidget()
    {
        return view_widget_;
    }

    void SetMiniSize(const gfx::Size& size)
    {
        mini_size_ = size;
    }

    void SetMaxSize(const gfx::Size& size)
    {
        max_size_ = size;
    }

    int NonClientHitTest(const gfx::Point& pt)
    {
        int hit_code = HTNOWHERE;
        gfx::Rect right_bottom(bounds().width() - kFrameEdgeThickness, bounds().height() - kFrameEdgeThickness, kFrameEdgeThickness, kFrameEdgeThickness);
        if (right_bottom.Contains(pt))
        {
            hit_code =  HTBOTTOMRIGHT;
        }

        gfx::Rect left_bottom(0, bounds().height() - kFrameEdgeThickness, kFrameEdgeThickness, kFrameEdgeThickness);
        if (left_bottom.Contains(pt))
        {
            hit_code =  HTBOTTOMLEFT;
        }

        gfx::Rect right_top(bounds().width() - kFrameEdgeThickness, 0, kFrameEdgeThickness, kFrameEdgeThickness);
        if (right_top.Contains(pt))
        {
            hit_code = HTTOPRIGHT;
        }

        gfx::Rect left_top(0, 0, kFrameEdgeThickness, kFrameEdgeThickness);
        if (left_top.Contains(pt))
        {
            hit_code = HTTOPLEFT;
        }

        gfx::Rect bottom(kFrameEdgeThickness, bounds().height() - kFrameEdgeThickness, bounds().width() - 2 * kFrameEdgeThickness, kFrameEdgeThickness);
        if (bottom.Contains(pt))
        {
            hit_code = HTBOTTOM;
        }

        gfx::Rect right(bounds().width() - kFrameEdgeThickness, kFrameEdgeThickness, kFrameEdgeThickness, bounds().height() - 2 * kFrameEdgeThickness);
        if (right.Contains(pt))
        {
            hit_code = HTRIGHT;
        }

        gfx::Rect left(0, kFrameEdgeThickness, kFrameEdgeThickness, bounds().height() - 2 * kFrameEdgeThickness);
        if (left.Contains(pt))
        {
            hit_code = HTLEFT;
        }

        if (!is_can_resize_height_ &&
                !(hit_code == HTLEFT||
                    hit_code == HTRIGHT)) {
            hit_code = HTNOWHERE;
        }
        return hit_code;
    }

    void ApplyOpacityToUI(int opacity)
    {
        view_bk_opacity_ = opacity;
        OnOpacityChanged();
    }

    void SetIsCanResizeHeight(bool is_can_resize) {
        is_can_resize_height_ = is_can_resize;
    }

    void SetBackGroundColor(SkColor color) {
        view_bk_color_ = color;
        view_bk_opacity_ = 255;
        OnOpacityChanged();
        SchedulePaint();
    }

protected:
    bool OnMousePressed(const ui::MouseEvent& event) override
    {
        POINT point = { 0 };
        ::GetCursorPos(&point);

        RECT rect = { 0 };
        ::GetWindowRect(GetWidget()->GetNativeView(), &rect);

        down_pt_ = gfx::Point(point);
        down_rect_ = gfx::Rect(rect);

        hit_code_ = NonClientHitTest(event.location());

        return true;
    }

    bool OnMouseDragged(const ui::MouseEvent& event) override
    {
        POINT point = { 0 };
        ::GetCursorPos(&point);
        auto cur_pt = gfx::Point(point);

        if (hit_code_ == HTNOWHERE)
        {
            gfx::Vector2d v2 = cur_pt - down_pt_;
            int x = down_rect_.x() + v2.x();
            int y = down_rect_.y() + v2.y();
            ::SetWindowPos(GetWidget()->GetNativeView(), nullptr, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        }
        else
        {
            if (!is_can_resize_height_ &&
                (hit_code_ == HTBOTTOM||
                 hit_code_ == HTBOTTOMRIGHT||
                 hit_code_== HTBOTTOMLEFT)) {
                return true;
            }
            ResizeWindow(hit_code_, down_pt_, cur_pt);
        }

        return true;
    }

    void OnWidgetDestroying(views::Widget* widget) override
    {
        __super::OnWidgetDestroying(widget);

        if (view_widget_ == widget)
        {
            mouse_timer_.Stop();
        }
    }

    void OnWidgetDestroyed(views::Widget* widget) override
    {
        if (view_widget_ == widget)
        {
            view_widget_->RemoveObserver(view_instance_);
            view_widget_ = nullptr;
            view_instance_ = nullptr;
        }
    }

    View* GetEventHandlerForPoint(const gfx::Point& point) override
    {
        int result = NonClientHitTest(point);
        if (result != HTNOWHERE)
        {
            return this;
        }

        return __super::GetEventHandlerForPoint(point);
    }

    views::NonClientFrameView* CreateNonClientFrameView(views::Widget* widget) override
    {
        return new ModuleCommonNonClientFrameView<ModuleCommonView>(this);
    }

    void AcceptMouseEvent(bool accept)
    {
        if (is_hwnd_transparent_ != accept)
        {
            return;
        }
        HWND hWnd = GetWidget()->GetNativeView();
        if (hWnd) {
            DWORD dwStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
            if (accept) {
                is_hwnd_transparent_ = false;
                ::SetWindowLong(hWnd, GWL_EXSTYLE, dwStyle & ~WS_EX_TRANSPARENT);
            }
            else {
                is_hwnd_transparent_ = true;
                ::SetWindowLong(hWnd, GWL_EXSTYLE, dwStyle | WS_EX_TRANSPARENT);
            }
        }
    }

    virtual void OnLockMouseDect()
    {

    }

    virtual void OnOpacityChanged()
    {

    }

    virtual void OnLockedChanged()
    {

    }

    // views::View
    void OnPaintBackground(gfx::Canvas* canvas) override
    {
        //__super::OnPaintBackground(canvas);
        auto bounds = GetContentsBounds();
        SkColor bk_color = SkColorSetA(view_bk_color_, view_bk_opacity_);
        canvas->FillRect(bounds, bk_color);
    }

    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override
    {
        if (guide_view_ && details.child == guide_view_)
        {
            if (!details.is_add)
            {
                guide_view_ = nullptr;
            }
        }
    }

private:
    void ResizeWindow(int hit_code, const gfx::Point& origin, const gfx::Point& end)
    {
        gfx::Vector2d v2 = end - origin;
        auto rect = GetWidget()->GetWindowBoundsInScreen();

        gfx::Size size;
        switch (hit_code)
        {
            case HTBOTTOMRIGHT:
            {
                size = ClampWindowSize(end.x() - rect.origin().x(), end.y() - rect.origin().y());
                rect.set_width(size.width());
                rect.set_height(size.height());
                GetWidget()->SetBounds(rect);
            }
            break;

            case HTBOTTOMLEFT:
            {
                size = ClampWindowSize(rect.bottom_right().x() - end.x(), end.y() - rect.origin().y());
                rect.set_x(end.x());
                rect.set_width(size.width());
                rect.set_height(size.height());
                GetWidget()->SetBounds(rect);
            }
            break;

            case HTBOTTOM:
            {
                size = ClampWindowSize(rect.width(), end.y() - rect.origin().y());
                rect.set_height(size.height());
                GetWidget()->SetBounds(rect);
            }
            break;

            case HTRIGHT:
            {
                size = ClampWindowSize(end.x() - rect.origin().x(), rect.height());
                rect.set_width(size.width());
                GetWidget()->SetBounds(rect);
            }
            break;

            case HTLEFT:
            {
                size = ClampWindowSize(rect.bottom_right().x() - end.x(), rect.height());
                rect.set_x(end.x());
                rect.set_width(size.width());
                GetWidget()->SetBounds(rect);
            }
            break;

            default:
            break;
        }
    }

    gfx::Size ClampWindowSize(int width, int height)
    {
        int n_w = std::min(std::max(width, mini_size_.width()), max_size_.width());
        int n_h = std::min(std::max(height, mini_size_.height()), max_size_.height());
        return { n_w, n_h };
    }

    void OnMouseDetectTimer()
    {
        OnLockMouseDect();
    }

protected:
    SkColor view_bk_color_;
    int     view_bk_opacity_ = 2;
    bool    is_split_ = false;
    views::View* guide_view_ = nullptr;
    bool is_hwnd_transparent_ = false;

private:
    static TView*                           view_instance_;
    static views::Widget*                   view_widget_;

    gfx::Point                              down_pt_;
    gfx::Rect                               down_rect_;
    int                                     hit_code_ = 0;

    gfx::Size                               mini_size_ = { GetLengthByDPIScale(270), GetLengthByDPIScale(170) };
    gfx::Size                               max_size_ = { GetLengthByDPIScale(500), INT_MAX };

    bool                                    is_pinned_ = false;
    bool                                    is_can_resize_height_ = false;

    base::RepeatingTimer<ModuleCommonView>       mouse_timer_;
};

template<typename TView>
const int ModuleCommonView<TView>::kFrameEdgeThickness = 4;

template<typename TView>
TView* ModuleCommonView<TView>::view_instance_{nullptr};

template<typename TView>
views::Widget* ModuleCommonView<TView>::view_widget_{nullptr};

