#pragma once

#include <chrono>

#include "base/message_loop/message_pump_observer.h"

#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/bililive_labelbutton.h"
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_view_theme.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/public/bililive/bililive_thread.h"


class DanmakuNewGuideView :
    public views::View,
    public views::ButtonListener
{
public:
    DanmakuNewGuideView(bool is_can_resize_height = true)
    {
        SkColor bk_color = SkColorSetARGB(0xD8, 0x1B, 0x1B, 0x1B);
		this->set_background(views::Background::CreateSolidBackground(bk_color));

        top_view_ = new views::View();
		SkColor view_color = SkColorSetARGB(0x19, 0xFF, 0xFF, 0xFF);
        top_view_->set_background(views::Background::CreateSolidBackground(view_color));

        label_text_ = new BililiveLabel();
        label_text_->SetFont(ftFourteen);
        label_text_->SetTextColor(SkColorSetRGB(0xD3, 0xE2, 0xEE));
        if (label_text_ && is_can_resize_height) {
            label_text_->SetText(L"浮窗模式可以拖动边缘调整宽高");
        }
        else if (label_text_ && !is_can_resize_height) {
            label_text_->SetText(L"浮窗模式可以拖动边缘调整宽度");
        }

        bt_i_know_ = new BililiveLabelButton(this,L"我知道了");
        bt_i_know_->SetFont(ftTwelve);
        BililiveLabelButton::ColorStruct color_struct;
        color_struct.border_clrs[views::Button::STATE_NORMAL] = SkColorSetRGB(0xD3, 0xE2, 0xEE);
        color_struct.border_clrs[views::Button::STATE_HOVERED] = SkColorSetRGB(0xD3, 0xE2, 0xEE);
        color_struct.border_clrs[views::Button::STATE_PRESSED] = SkColorSetRGB(0xD3, 0xE2, 0xEE);
        color_struct.bkg_clrs[views::Button::STATE_NORMAL] = SkColorSetARGB(0x99, 0x1B, 0x1B, 0x1B);
        color_struct.bkg_clrs[views::Button::STATE_HOVERED] = SkColorSetARGB(0x99, 0x1B, 0x1B, 0x1B);
        color_struct.bkg_clrs[views::Button::STATE_PRESSED] = SkColorSetARGB(0x99, 0x1B, 0x1B, 0x1B);
        color_struct.text_clrs[views::Button::STATE_NORMAL] = SkColorSetRGB(0xD3, 0xE2, 0xEE);
        color_struct.text_clrs[views::Button::STATE_HOVERED] = SkColorSetRGB(0xD3, 0xE2, 0xEE);
        color_struct.text_clrs[views::Button::STATE_PRESSED] = SkColorSetRGB(0xD3, 0xE2, 0xEE);
        bt_i_know_->SetStyle(color_struct);
        bt_i_know_->SetCursor(LoadCursor(nullptr, IDC_HAND));

		image_ = new views::ImageView();
		image_->set_interactive(false);
		gfx::ImageSkia* img = GetImageSkiaNamed(IDR_LIVEHIME_DANMAKU_GUIDE_ARROW);
		image_->SetImage(img);

        this->AddChildView(top_view_);
		this->AddChildView(label_text_);
        this->AddChildView(bt_i_know_);
        this->AddChildView(image_);
    }
    ~DanmakuNewGuideView()
    {

    }
    void SetNoSubpixelRendering(bool no_subpixel_rendering)
    {
        label_text_->set_no_subpixel_rendering(no_subpixel_rendering);
        bt_i_know_->set_no_subpixel_rendering(no_subpixel_rendering);
    }
protected:
	void Layout() override
	{
        gfx::Rect rect = GetContentsBounds();
        gfx::Size text_size = label_text_->GetPreferredSize();
        gfx::Size top_view_size(rect.width() - GetLengthByDPIScale(20), rect.height() - GetLengthByDPIScale(20));
        gfx::Size bt_size(GetLengthByDPIScale(68), GetLengthByDPIScale(25));
        gfx::Size image_size = image_->GetPreferredSize();
        int space = GetLengthByDPIScale(5);

        top_view_->SetBounds(0,0, top_view_size.width(), top_view_size.height());
        label_text_->SetBounds((rect.width()- text_size.width()) /2, rect.height()/2 - space - text_size.height(),text_size.width(), text_size.height());
        bt_i_know_->SetBounds((rect.width() - bt_size.width()) / 2, rect.height() / 2 + space, bt_size.width(), bt_size.height());
        image_->SetBounds(top_view_size.width() - image_size.width()/2 - 2, top_view_size.height() - image_size.height()/2 -2, image_size.width(), image_size.height());
    }
    void ButtonPressed(views::Button* sender, const ui::Event& event)override
    {
        this->SetVisible(false);
        if (parent())
        {
            parent()->RemoveChildView(this);
            delete this;
        }
    }
private:
    views::View* top_view_;
    BililiveLabelButton* bt_i_know_;
    BililiveLabel* label_text_;
    views::ImageView* image_;
};

class DanmakuWidget :
    public views::Widget
{

public:
    void LockPaint(bool lock)
    {
        lock_paint_ = lock;
    }

    void SchedulePaintInRect(const gfx::Rect& rect) override
    {
        if (lock_paint_)
        {
            return;
        }

        views::Widget::SchedulePaintInRect(rect);
    }

private:
    bool lock_paint_ = false;
};

template<typename TView>
class DanmakuViewNonClientFrameView : public views::NonClientFrameView
{
public:
    explicit DanmakuViewNonClientFrameView(TView* client_content_view)
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

    DISALLOW_COPY_AND_ASSIGN(DanmakuViewNonClientFrameView);
};

class MessageLoopWaitCounter
{
public:
    void AddWaitCounter()
    {
        DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));
        animation_counter_++;
        base::MessageLoopForUI::current()->SetWaitMsg(false);
    }

    void DecreaseWaitCounter()
    {
        DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));
        animation_counter_ = std::max(animation_counter_ - 1, 0);
        if (animation_counter_ == 0)
        {
            base::MessageLoopForUI::current()->SetWaitMsg(true);
        }
    }

private:
    static int32_t animation_counter_;
};

template<typename TView>
class DanmakuView :
    public BililiveWidgetDelegate,
    public DanmakuThemeInterface,
    public DanmakuShowTypeInterface,
    public base::MessagePumpObserver,
    public MessageLoopWaitCounter
{
public:
    static const int kFrameEdgeThickness;
public:
    DanmakuView()
    {
        base::MessageLoopForUI::current()->AddObserver(this);
        layered_window_contents_.reset(new gfx::Canvas({ 500, 500 }, ui::SCALE_FACTOR_100P, false));
    }

    virtual ~DanmakuView()
    {
        base::MessageLoopForUI::current()->RemoveObserver(this);
    }

    static bool Pop(const gfx::Rect rect, views::Widget*parent = nullptr)//返回是否刚创建
    {
        auto widget_view = DanmakuView::GetView();

        if (!view_widget_)
        {
            view_widget_ = new DanmakuWidget{};
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

            DCHECK(view_widget_->non_client_view()->client_view() == DanmakuView::GetView()->parent());
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

    bool IsLocked()
    {
        return is_locked_;
    }

    virtual void Lock(bool lock)
    {
        if (is_locked_ == lock)
        {
            return;
        }

        is_locked_ = lock;
        OnLockedChanged();
        if (is_locked_)
        {
            // 被锁定时启动定时器。
            OnMouseDetectTimer();
            mouse_timer_.Start(FROM_HERE,
                base::TimeDelta::FromMilliseconds(200), this,
                &DanmakuView::OnMouseDetectTimer);
        }
        else
        {
            // 解锁后停止定时器。
            OnMouseDetectTimer();
            mouse_timer_.Stop();
        }
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

    static DanmakuWidget* GetDanmakuWidget()
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

        // [活动与任务]只能改变宽，鼠标其他的位置默认走 HTNOWHERE样式
        if (!is_can_resize_height_ &&
                !(hit_code == HTLEFT||
                    hit_code == HTRIGHT))
        {
            hit_code = HTNOWHERE;
        }
        return hit_code;
    }

    void ApplyOpacityToUI(int opacity)
    {
        view_bk_opacity_ = opacity;
        OnOpacityChanged();
        if (guide_view_)
        {
            DanmakuNewGuideView* view = static_cast<DanmakuNewGuideView*>(guide_view_);
            if (view)
            {
                view->SetNoSubpixelRendering(opacity < 255);
            }
        }
    }
    //引导页
    void ShowGuideView()
    {
        if (!guide_view_)
        {
			guide_view_ = new DanmakuNewGuideView(is_can_resize_height_);
            this->AddChildView(guide_view_);
            guide_view_->SetVisible(true);
        }
        Layout();
    }

    void SetIsCanResizeHeight(bool is_can_resize)
    {
        is_can_resize_height_ = is_can_resize;
    }

    bool IsAnimating()
    {
        return is_animating_;
    }

    void AddAnimation()
    {
        DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

        animation_counter_++;
        SetAnimating(true);
        AddWaitCounter();
    }

    void DecreaseAnimation()
    {
        DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

        animation_counter_ = std::max(animation_counter_ - 1, 0);
        if (animation_counter_ == 0)
        {
            SetAnimating(false);
        }

        DecreaseWaitCounter();
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
        if (GetShowType() != DanmakuShowType::kSplited)
        {
            return true;
        }
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
        //__super::OnWidgetDestroyed(widget);

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
        return new DanmakuViewNonClientFrameView<DanmakuView>(this);
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

    void OnWidgetBoundsChanged(views::Widget* widget, const gfx::Rect& new_bounds)
    {
        auto hwnd = GetDanmakuWidget()->GetNativeView();

        RECT rt{};
        ::GetClientRect(hwnd, &rt);
        gfx::Size canvas_size{ rt.right - rt.left, rt.bottom - rt.top };

        if (layered_window_contents_)
        {
            auto sk_size = layered_window_contents_->sk_canvas()->getDevice();
            if (sk_size->width() != canvas_size.width() ||
                sk_size->height() != canvas_size.height())
            {
                layered_window_contents_.reset(new gfx::Canvas(canvas_size, ui::SCALE_FACTOR_100P, false));
            }
        }
        else
        {
            layered_window_contents_.reset(new gfx::Canvas(canvas_size, ui::SCALE_FACTOR_100P, false));
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

    base::EventStatus WillProcessEvent(const base::NativeEvent& event) override
    {
        return base::EVENT_CONTINUE;
    }

    void DidProcessEvent(const base::NativeEvent& event) override
    {
    }

    void EventSpared() override
    {
        if (GetDanmakuWidget() &&
            GetDanmakuWidget()->IsVisible() &&
            GetShowType() == DanmakuShowType::kSplited &&
            IsAnimating())
        {
            static std::chrono::steady_clock::time_point pre_tp;
            auto cur_tp = std::chrono::steady_clock::now();

            auto tick = std::chrono::duration_cast<std::chrono::milliseconds>(cur_tp - pre_tp);
            if (tick > std::chrono::milliseconds(30))
            {
                auto hwnd = GetWidget()->GetNativeView();
                layered_window_contents_->sk_canvas()->clear(0);
                DanmakuView::GetDanmakuWidget()->GetRootView()->Paint(layered_window_contents_.get());

                RECT wr;
                ::GetWindowRect(hwnd, &wr);

                SIZE size = { wr.right - wr.left, wr.bottom - wr.top };
                POINT position = { wr.left, wr.top };
                HDC dib_dc = skia::BeginPlatformPaint(layered_window_contents_->sk_canvas());
                POINT zero = { 0, 0 };
                BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
                ::UpdateLayeredWindow(hwnd, NULL, &position, &size, dib_dc, &zero,
                    RGB(0xFF, 0xFF, 0xFF), &blend, ULW_ALPHA);
                skia::EndPlatformPaint(layered_window_contents_->sk_canvas());

                pre_tp = cur_tp;

                //std::wstring wstr(L"interval: ");
                //wstr += std::to_wstring(tick.count());
                //wstr += L"\n";
                //OutputDebugString(wstr.c_str());
                //pre_tp = cur_tp;

            }
        }
    }

    void SetAnimating(bool v)
    {
        if (GetDanmakuWidget())
        {
            is_animating_ = v;
            GetDanmakuWidget()->LockPaint(v);
        }
    }

protected:
    SkColor                                 view_bk_color_;
    int                                     view_bk_opacity_ = 255;
    views::View*                            guide_view_ = nullptr;
    bool                                    is_hwnd_transparent_ = false;

private:
    static TView*                           view_instance_;
    static DanmakuWidget*                   view_widget_;

    gfx::Point                              down_pt_;
    gfx::Rect                               down_rect_;
    int                                     hit_code_ = 0;

    scoped_ptr<gfx::Canvas>                 layered_window_contents_;

    gfx::Size                               mini_size_ = { GetLengthByDPIScale(270), GetLengthByDPIScale(170) };
    gfx::Size                               max_size_ = { GetLengthByDPIScale(500), INT_MAX };

    bool                                    is_pinned_ = false;
    bool                                    is_locked_ = false;
    bool                                    is_can_resize_height_ = true;
    bool                                    is_animating_ = false;
    int32_t                                 animation_counter_ = 0;

    base::RepeatingTimer<DanmakuView>       mouse_timer_;
};

template<typename TView>
const int DanmakuView<TView>::kFrameEdgeThickness = 4;

template<typename TView>
TView* DanmakuView<TView>::view_instance_{nullptr};

template<typename TView>
DanmakuWidget* DanmakuView<TView>::view_widget_{nullptr};


