#include "bililive_frame_view.h"
#include "bililive_frame_titlebar_view.h"

#include "base/bind.h"

#include "ui/base/view_prop.h"
#include "ui/base/win/dpi.h"
#include "ui/base/win/hwnd_util.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/path.h"
#include "ui/views/widget/widget.h"
#include "ui/views/painter.h"
#include "ui/views/bubble/bubble_border.h"
#include "ui/views/window/window_shape.h"
#include "ui/views/widget/widget_delegate.h"

#include "grit/ui_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "grit/theme_resources.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"


namespace
{
    const int kShadowBorderThickness = 9;
    const int kFrameBorderThickness = 1 * ui::GetDPIScale();

    const char* const kBililiveShadowWidgetDelegateWndProcKey = "__BILILIVE_SHADOW_WIDGETDELEGATE_WNDPROC__";

#define IMAGE_BORDER_ACRONYM(x) { x ## _TL, x ## _T, x ## _TR, \
                                  x ## _L,  0,       x ## _R, \
                                  x ## _BL, x ## _B, x ## _BR, }

    // 阴影窗口
    class ShadowFrameWidgetDelegate : public views::WidgetDelegateView
    {
    public:
        static views::Widget* ShowShadowFrameWnd(HWND relative_hwnd)
        {
            DCHECK(relative_hwnd);

            ShadowFrameWidgetDelegate *delegate = new ShadowFrameWidgetDelegate(relative_hwnd);
            views::Widget *widget = new views::Widget();
            views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
            params.parent = relative_hwnd;
            params.delegate = delegate;
            params.remove_standard_frame = true;
            params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;
            params.accept_events = false;
            params.can_activate = false;
            widget->set_frame_type(views::Widget::FRAME_TYPE_FORCE_CUSTOM);
            widget->Init(params);
            widget->Hide();

            delegate->BeginRelativeWork();

            return widget;
        }

    protected:
        explicit ShadowFrameWidgetDelegate(HWND relative_hwnd)
            : relative_hwnd_(relative_hwnd)
            , weakptr_factory_(this)
        {
        }

        virtual ~ShadowFrameWidgetDelegate()
        {
        }

        // WidgetDelegate
        views::View *GetContentsView() override { return this; }

        void WindowClosing() override
        {
            ui::SetWindowProc(relative_hwnd_, reinterpret_cast<WNDPROC>(original_handler_));
            prop_.reset();
        }

        // View
        void OnPaint(gfx::Canvas* canvas) override
        {
            gfx::Rect rect = GetLocalBounds();
            // Bubble border and arrow image resource ids.
            /*const int kShadowImages[] = IMAGE_BORDER_ACRONYM(IDR_BUBBLE_SHADOW);
            int border_thickness = rb.GetImageSkiaNamed(kShadowImages[0])->width();
            scoped_ptr<views::Painter> border_painter(views::Painter::CreateImageGridPainter(kShadowImages));*/
            static scoped_ptr<views::Painter> border_painter;
            if (!border_painter)
            {
                static int thickness = kShadowBorderThickness * ui::win::GetDeviceScaleFactor();

                ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
                gfx::ImageSkia* img = rb.GetImageSkiaNamed(IDR_LIVEHIME_SHADOW_BORDER);
                border_painter.reset(views::Painter::CreateImagePainter(*img,
                    gfx::Insets(thickness, thickness, thickness, thickness)));
            }
            views::Painter::PaintPainterAt(canvas, border_painter.get(), rect);
        }

    private:
        void BeginRelativeWork()
        {
            // We subclass the hwnd so we get the interested messages.
            original_handler_ = ui::SetWindowProc(
                relative_hwnd_, &ShadowFrameWidgetDelegate::BilibiliShadowFrameWndProc);
            prop_.reset(new ui::ViewProp(relative_hwnd_, kBililiveShadowWidgetDelegateWndProcKey, this));

            AdjustWindowPos();
        }

        void AdjustWindowPos()
        {
            if (!hostview_restore_completed_)
            {
                return;
            }
            static int thickness = kShadowBorderThickness * ui::win::GetDeviceScaleFactor();

            RECT rect;
            ::GetWindowRect(relative_hwnd_, &rect);
            gfx::Rect relative_rect(rect);
            int inset = (ui::GetDPIScale() > 1.5f ? thickness * 2 : thickness);
            relative_rect.Inset(-inset, -inset);

            ::SetWindowPos(GetWidget()->GetNativeView(), relative_hwnd_,
                relative_rect.x(), relative_rect.y(), relative_rect.width(), relative_rect.height(),
                SWP_NOACTIVATE | (::IsWindowVisible(relative_hwnd_) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
        }

        static LRESULT CALLBACK BilibiliShadowFrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            ShadowFrameWidgetDelegate* widget_delegate = static_cast<ShadowFrameWidgetDelegate*>(
                ui::ViewProp::GetValue(hwnd, kBililiveShadowWidgetDelegateWndProcKey));
            DCHECK(widget_delegate);
            WNDPROC original_handler = widget_delegate->original_handler_;
            DCHECK(original_handler);

            switch (msg)
            {
            case WM_WINDOWPOSCHANGED:
                widget_delegate->AdjustWindowPos();
                break;
            case WM_SHOWWINDOW:
                if (wParam)
                {
                    widget_delegate->AdjustWindowPos();
                }
                else
                {
                    widget_delegate->GetWidget()->Hide();
                }
                break;
            case WM_CLOSE:
                widget_delegate->GetWidget()->Hide();
                break;
            case WM_DESTROY:
                widget_delegate->GetWidget()->Close();
                break;
            case WM_SYSCOMMAND:
            {
                WPARAM sys_cmd_wp = (0xFFF0 & wParam);
                if (sys_cmd_wp == SC_RESTORE || sys_cmd_wp == SC_MINIMIZE || sys_cmd_wp == SC_MAXIMIZE)
                {
                    widget_delegate->hostview_restore_completed_ = false;
                    widget_delegate->GetWidget()->Hide();
                }
                else if (sys_cmd_wp == SC_MOVE)
                {
                    widget_delegate->AfterHostViewRestore();
                }
            }
                break;
            case WM_QUERYOPEN:
            {
                // 还原并激活后延迟显示边框
                // 有一些情况下的窗口还原是不会触发WM_SYSCOMMAND:SC_RESTORE消息的，
                // 但不管以何种方式还原窗口显示，是一定首先触发WM_QUERYOPEN消息的，
                // 所以一个最有效的决定窗口最小化还原后的显示状态的地方就是WM_QUERYOPEN，只是相对于
                // WM_SYSCOMMAND:SC_RESTORE阻塞执行一系列还原相关操作的消息而言，其不够实时而已
                // 已知的窗口还原时不会触发WM_SYSCOMMAND:SC_RESTORE消息的情况：
                // 1、窗口先最小化，然后以其为owner进行模态弹窗并且不先将其还原，然后点击任务栏窗口图标进行还原；
                //    针对这种情况最好的做法就是弹模态窗时就应该先还原窗口，而且如果不这么做的话那么在弹窗后再还原
                //    窗口也会触发框架自己的还原并激活后获得焦点的DCHECK(HWNDMessageHandler::PostProcessActivateMessage)
                /*base::MessageLoop::current()->PostTask(
                    FROM_HERE, base::Bind(&ShadowFrameWidgetDelegate::AfterHostViewRestore,
                                          widget_delegate->weakptr_factory_.GetWeakPtr()));*/
            }
                break;
            default:
                break;
            }

            LRESULT lRet = ::CallWindowProc(original_handler, hwnd, msg, wParam, lParam);

            if(msg == WM_SYSCOMMAND)
            {
                WPARAM sys_cmd_wp = (0xFFF0 & wParam);
                if (sys_cmd_wp == SC_RESTORE)
                {
                    widget_delegate->AfterHostViewRestore();
                }
            }

            return lRet;
        }

        void AfterHostViewRestore()
        {
            if (!hostview_restore_completed_)
            {
                hostview_restore_completed_ = true;
                AdjustWindowPos();
            }
        }

    private:
        HWND relative_hwnd_ = nullptr;
        scoped_ptr<ui::ViewProp> prop_;
        WNDPROC original_handler_ = nullptr;

        bool hostview_restore_completed_ = true;
        base::WeakPtrFactory<ShadowFrameWidgetDelegate> weakptr_factory_;

        DISALLOW_COPY_AND_ASSIGN(ShadowFrameWidgetDelegate);
    };
}


const char BililiveFrameView::kDevViewClassName[] = "BililiveFrameView";

BililiveFrameView::BililiveFrameView(gfx::ImageSkia *skia, base::string16 caption,
    int TitleBarButtonCombination/* = TB_CLOSE*/)
    : title_bar_(new BililiveFrameTitleBarView(skia, caption, TitleBarButtonCombination))
    , weakptr_factory_(this)
{
    InitView();
}

BililiveFrameView::BililiveFrameView(int TitleBarButtonCombination/* = TBB_CLOSE*/)
    : title_bar_(new BililiveFrameTitleBarView(&gfx::ImageSkia(), L"", TitleBarButtonCombination))
    , weakptr_factory_(this)
{
    InitView();
}

BililiveFrameView::BililiveFrameView(BililiveFrameTitleBar *title_bar)
    : title_bar_(title_bar)
    , weakptr_factory_(this)
{
    InitView();
}

void BililiveFrameView::InitView()
{
    enable_left_button_drag_size_ = false;
    enable_left_button_drag_move_ = true;
    cache_maxmin_size_ = true;
    has_shadow_ = true;
    shadow_widget_ = nullptr;
    flash_times_ = 0;

    DCHECK(title_bar_);
    if (title_bar_)
    {
        AddChildView(title_bar_);
    }
}

BililiveFrameView::~BililiveFrameView()
{
    shadow_widget_ = nullptr;
}

gfx::Rect BililiveFrameView::GetBoundsForClientView() const
{
    return client_view_bounds_;
}

gfx::Rect BililiveFrameView::GetWindowBoundsForClientBounds(
    const gfx::Rect &client_bounds) const
{
    gfx::Rect rect = client_bounds;
    rect.Inset(border() ? - border()->GetInsets() : gfx::Insets());
    rect.Inset(0, -((views::View*)title_bar_)->GetPreferredSize().height(), 0, 0);
    return rect;
}

int BililiveFrameView::NonClientHitTest(const gfx::Point &point)
{
    int ht = HTNOWHERE;
    bool is_maximized = GetWidget() && GetWidget()->IsMaximized();

    if (enable_left_button_drag_size_ && !is_maximized)
    {
        ht = GetHTComponentForFrame(point, kHitTestThickness, kHitTestThickness,
            kHitTestThickness, kHitTestThickness, true);
    }
    if ((ht == HTNOWHERE) && enable_left_button_drag_move_)
    {
        if (title_bar_->bounds().Contains(point))
        {
            gfx::Point ptTitle = point;
            ConvertPointToTarget(this, title_bar_, &ptTitle);
            return title_bar_->NonClientHitTest(ptTitle);
        }
    }
    return ht;
}

void BililiveFrameView::GetWindowMask(const gfx::Size &size,
        gfx::Path *window_mask)
{
    window_mask->moveTo(0, 0);
    window_mask->lineTo(SkIntToScalar(size.width()), 0);
    window_mask->lineTo(SkIntToScalar(size.width()), SkIntToScalar(size.height()));
    window_mask->lineTo(0, SkIntToScalar(size.height()));
    window_mask->lineTo(0, 0);
    window_mask->close();
}

void BililiveFrameView::ResetWindowControls()
{
}

void BililiveFrameView::UpdateWindowIcon()
{
}

void BililiveFrameView::UpdateWindowTitle()
{
    if (title_bar_)
    {
        title_bar_->SetTitle(GetWidget()->widget_delegate()->GetWindowTitle());
    }
}

void BililiveFrameView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.is_add && details.child == this && GetWidget())
    {
        if (has_shadow_ && !GetWidget()->ShouldUseNativeFrame())
        {
            shadow_widget_ = ShadowFrameWidgetDelegate::ShowShadowFrameWnd(GetWidget()->GetNativeView());
        }
    }
}

void BililiveFrameView::Layout()
{
    gfx::Insets insets = border() ? border()->GetInsets() : gfx::Insets();
    title_bar_->SetBounds(
        insets.left(),
        insets.top(),
        width() - insets.width(),
        title_bar_->GetPreferredSize().height());

    client_view_bounds_ = CalculateClientAreaBounds();
    views::View *client_view = GetWidget()->client_view();
    if (client_view)
    {
        client_view->SetBoundsRect(client_view_bounds_);
    }
}

const char* BililiveFrameView::GetClassName() const
{
    return kDevViewClassName;
}

gfx::Size BililiveFrameView::GetMaximumSize()
{
    gfx::Size max_size;
    /*if (cache_maxmin_size_)
    {
        if (!max_size_.IsEmpty())
        {
            max_size = max_size_;
        }
    }
    if (max_size.IsEmpty())*/
    {
        // 拿到的是设备物理尺寸，若是改了dpi又改回1.0x而不注销生效，则拿到的尺寸是（物理尺寸*当前dpi倍率）
        gfx::Rect rcWork(GetWidget()->GetWorkAreaBoundsInScreen());
        max_size = rcWork.size();
        max_size_ = cache_maxmin_size_ ? max_size : gfx::Size();
    }
    return max_size;
}

gfx::Size BililiveFrameView::GetMinimumSize()
{
    gfx::Size min_size;
    if (cache_maxmin_size_)
    {
        if (!min_size_.IsEmpty())
        {
            min_size = min_size_;
        }
    }
    if (min_size.IsEmpty())
    {
        min_size = GetWidget()->client_view()->GetMinimumSize();
        if (border())
        {
            gfx::Insets ist = border()->GetInsets();
            min_size.Enlarge(ist.width(), ist.height());
        }

        gfx::Size max_size = GetMaximumSize();
        min_size.set_height(std::min(min_size.height(), max_size.height()));

        min_size_ = cache_maxmin_size_ ? min_size : gfx::Size();
    }

    return min_size;
}

gfx::Rect BililiveFrameView::CalculateClientAreaBounds() const
{
    gfx::Rect rect = bounds();
    rect.Inset(border() ? border()->GetInsets() : gfx::Insets());
    rect.Inset(0, title_bar_->GetPreferredSize().height(), 0, 0);
    return rect;
}

void BililiveFrameView::SetTitlebar(BililiveFrameTitleBar *title_bar)
{
    if (title_bar_)
    {
        RemoveChildView(title_bar_);
        delete title_bar_;
    }
    title_bar_ = title_bar;
    AddChildView(title_bar_);
    InvalidateLayout();
}

void BililiveFrameView::SetTitlebarDelegate(BililiveFrameTitleBarDelegate *deleagte)
{
    if (title_bar_)
    {
        title_bar_->SetDelegate(deleagte);
    }
}

void BililiveFrameView::SetBackgroundColor(SkColor color)
{
    set_background(views::Background::CreateSolidBackground(color));
}

void BililiveFrameView::SetBorderColor(SkColor color)
{
    set_border(views::Border::CreateSolidBorder(kFrameBorderThickness, color));
}

void BililiveFrameView::RestoreMaxminSize()
{
    max_size_.SetSize(0, 0);
    min_size_.SetSize(0, 0);
}

void BililiveFrameView::SetCacheMaxminSize(bool cache)
{
    if (cache_maxmin_size_ != cache)
    {
        cache_maxmin_size_ = cache;
        RestoreMaxminSize();
    }
}

void BililiveFrameView::FlashFrame()
{
    if (GetWidget()->ShouldUseNativeFrame()) {
        GetWidget()->FlashFrame(true);
    } else {
        // 一调就先闪一次
        flash_times_ = (flash_times_ % 2) ? 0 : 1;
        DoFlashFrame();

        flash_timer_.Start(FROM_HERE, base::TimeDelta::FromMilliseconds(100),
            base::Bind(&BililiveFrameView::OnFlashTimer, weakptr_factory_.GetWeakPtr()));
    }
}

void BililiveFrameView::OnFlashTimer()
{
    ++flash_times_;
    if (flash_times_ >= 10)
    {
        flash_timer_.Stop();
    }

    DoFlashFrame();
}

void BililiveFrameView::DoFlashFrame()
{
    if (shadow_widget_)
    {
        (flash_times_ % 2) ? shadow_widget_->Hide() : shadow_widget_->ShowInactive();
    }
    if (title_bar_)
    {
        title_bar_->SetActive((flash_times_ % 2) == 0);
    }
}



// BililiveNonTitleBarFrameView

const char BililiveNonTitleBarFrameView::kDevViewClassName[] = "BililiveNonTitleBarFrameView";

BililiveNonTitleBarFrameView::BililiveNonTitleBarFrameView(BililiveNonTitleBarFrameViewDelegate* delegate)
    : BililiveFrameView(new BililiveFrameTitleBar())
    , delegate_(delegate)
{
}

BililiveNonTitleBarFrameView::~BililiveNonTitleBarFrameView()
{

}

const char* BililiveNonTitleBarFrameView::GetClassName() const
{
    return kDevViewClassName;
}

int BililiveNonTitleBarFrameView::NonClientHitTest(const gfx::Point &point)
{
    int ht = HTNOWHERE;
    bool is_maximized = GetWidget() && GetWidget()->IsMaximized();

    if (enable_left_button_drag_size_ && !is_maximized)
    {
        ht = GetHTComponentForFrame(point, kHitTestThickness, kHitTestThickness,
            kHitTestThickness, kHitTestThickness, true);
    }
    if ((ht == HTNOWHERE) && enable_left_button_drag_move_)
    {
        if (delegate_)
        {
            ht = delegate_->NonClientHitTest(point);
        }
    }
    return ht;
}
