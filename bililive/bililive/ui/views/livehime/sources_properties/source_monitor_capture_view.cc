#include "bililive/bililive/ui/views/livehime/sources_properties/source_monitor_capture_view.h"

#include "bililive/bililive/livehime/obs/source_creator.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/tabarea/tabarea_materials_widget.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "base/strings/stringprintf.h"

#include "ui/gfx/screen.h"
#include "ui/views/layout/box_layout.h"

#include <WinUser.h>

namespace
{
    enum
    {
        Button_Cancel = 1,
        Button_OK,
    };

    // These values match the Windows defaults.
    const int kDoubleClickTimeMS = 500;

    const SkColor kClrBorder = SkColorSetRGB(0, 174, 255);
    const SkColor kClrBk = SkColorSetA(SK_ColorBLACK, 128);

    MonitorCaptureView* g_single_instance = nullptr;

    BOOL CALLBACK EnumAllMonitor(HMONITOR monitor, HDC hdc, LPRECT rc, LPARAM data)
    {
        gfx::Rect* rect = (gfx::Rect*)data;
        *rect = gfx::UnionRects(*rect, gfx::Rect(*rc));
        return true;
    }

    gfx::Rect GetAllMonitorUnionRect()
    {
        static gfx::Rect rect;
        if (rect.IsEmpty())
        {
            bool success = !!::EnumDisplayMonitors(NULL, NULL, EnumAllMonitor, (LPARAM)&rect);
            DCHECK(success);
        }
        return rect;
    }

    class CaptureWindowFrameView : public BililiveNonTitleBarFrameView
    {
    public:
        CaptureWindowFrameView()
            : BililiveNonTitleBarFrameView(nullptr)
        { }

    protected:
        gfx::Size GetMaximumSize() override
        {
            return GetAllMonitorUnionRect().size();
        }
    };

    class CaptureViewport : public views::View
    {
    public:
        CaptureViewport(MonitorCaptureView* mask_view)
            : mask_view_(mask_view)
        {
        }

        void SetLimitBounds(const gfx::Rect& limit_bounds)
        {
            limit_bounds_ = limit_bounds;
        }

    protected:
        // View
        void Paint(gfx::Canvas* canvas) override
        {
            if (!visible())
            {
                return;
            }

            // 边框+周边八个拖拽点
            static int kAnchorPointThickness = GetLengthByDPIScale(6);
            static int kLineThickness = GetLengthByDPIScale(2);
            SkPath path;
            path.addRect(SkRect::MakeXYWH(x() - kLineThickness / 2, y() - kLineThickness / 2,
                width() + kLineThickness, height() + kLineThickness));
            SkPath center_path;
            center_path.addRect(SkRect::MakeXYWH(x() + kLineThickness / 2, y() + kLineThickness / 2,
                width() - kLineThickness, height() - kLineThickness));
            path.reverseAddPath(center_path);
            // top
            path.addRect(SkRect::MakeXYWH(x() - kAnchorPointThickness / 2, y() - kAnchorPointThickness / 2,
                kAnchorPointThickness, kAnchorPointThickness));
            path.addRect(SkRect::MakeXYWH(x() + (width() - kAnchorPointThickness)/2, y() - kAnchorPointThickness/2,
                kAnchorPointThickness, kAnchorPointThickness));
            path.addRect(SkRect::MakeXYWH(bounds().right() - kAnchorPointThickness / 2, y() - kAnchorPointThickness / 2,
                kAnchorPointThickness, kAnchorPointThickness));
            // center
            path.addRect(SkRect::MakeXYWH(x() - kAnchorPointThickness / 2, y() + (height() - kAnchorPointThickness) / 2,
                kAnchorPointThickness, kAnchorPointThickness));
            path.addRect(SkRect::MakeXYWH(bounds().right() - kAnchorPointThickness / 2, y() + (height() - kAnchorPointThickness) / 2,
                kAnchorPointThickness, kAnchorPointThickness));
            // bottom
            path.addRect(SkRect::MakeXYWH(x() - kAnchorPointThickness / 2, bounds().bottom() - kAnchorPointThickness / 2,
                kAnchorPointThickness, kAnchorPointThickness));
            path.addRect(SkRect::MakeXYWH(x() + (width() - kAnchorPointThickness) / 2, bounds().bottom() - kAnchorPointThickness / 2,
                kAnchorPointThickness, kAnchorPointThickness));
            path.addRect(SkRect::MakeXYWH(bounds().right() - kAnchorPointThickness / 2, bounds().bottom() - kAnchorPointThickness / 2,
                kAnchorPointThickness, kAnchorPointThickness));

            SkPaint paint;
            paint.setColor(kClrBorder);
            canvas->DrawPath(path, paint);
        }

        bool OnMousePressed(const ui::MouseEvent& event) override
        {
            if (event.IsLeftMouseButton())
            {
                static base::Time pre_click_stamp;
                base::Time click_stamp = base::Time::Now();

                if (base::TimeDelta(click_stamp - pre_click_stamp).InMilliseconds() < kDoubleClickTimeMS)
                {
                    mask_view_->ApplyCapture();
                }
                else
                {
                    down_bounds_ = bounds();
                    down_pt_ = gfx::Screen::GetNativeScreen()->GetCursorScreenPoint();
                    views::View::ConvertPointFromScreen(mask_view_, &down_pt_);
                }

                pre_click_stamp = click_stamp;

                return true;
            }
            return false;
        }

        bool OnMouseDragged(const ui::MouseEvent& event) override
        {
            // 偏移量仅限于当前显示器尺寸范围内计算
            gfx::Point cur_pt = gfx::Screen::GetNativeScreen()->GetCursorScreenPoint();
            views::View::ConvertPointFromScreen(mask_view_, &cur_pt);

            cur_pt.set_x(std::max(cur_pt.x(), limit_bounds_.x()));
            cur_pt.set_x(std::min(cur_pt.x(), limit_bounds_.right()));
            cur_pt.set_y(std::max(cur_pt.y(), limit_bounds_.y()));
            cur_pt.set_y(std::min(cur_pt.y(), limit_bounds_.bottom()));
            int x_offset = cur_pt.x() - down_pt_.x();
            int y_offset = cur_pt.y() - down_pt_.y();

            switch (hittest_type_)
            {
            case HTSIZE:
            {
                int dst_x = down_bounds_.x() + x_offset;
                int dst_y = down_bounds_.y() + y_offset;
                dst_x = std::max(dst_x, limit_bounds_.x());
                dst_x = std::min(dst_x, limit_bounds_.right() - down_bounds_.width());
                dst_y = std::max(dst_y, limit_bounds_.y());
                dst_y = std::min(dst_y, limit_bounds_.bottom() - down_bounds_.height());
                gfx::Rect rect(dst_x, dst_y, down_bounds_.width(), down_bounds_.height());
                mask_view_->AdjustViewport(&rect);
            }
            	break;
            case HTTOPLEFT:
            {
                gfx::Point pt_origin = down_bounds_.bottom_right();
                gfx::Point pt_bounding(down_bounds_.x() + x_offset, down_bounds_.y() + y_offset);
                gfx::Rect rect = gfx::BoundingRect(pt_origin, pt_bounding);
                mask_view_->AdjustViewport(&rect);
            }
                break;
            case HTTOP:
            {
                gfx::Point pt_origin = down_bounds_.bottom_left();
                gfx::Point pt_bounding(down_bounds_.right(), down_bounds_.y() + y_offset);
                gfx::Rect rect = gfx::BoundingRect(pt_origin, pt_bounding);
                mask_view_->AdjustViewport(&rect);
            }
                break;
            case HTTOPRIGHT:
            {
                gfx::Point pt_origin = down_bounds_.bottom_left();
                gfx::Point pt_bounding(down_bounds_.right() + x_offset, down_bounds_.y() + y_offset);
                gfx::Rect rect = gfx::BoundingRect(pt_origin, pt_bounding);
                mask_view_->AdjustViewport(&rect);
            }
                break;
            case HTLEFT:
            {
                gfx::Point pt_origin = down_bounds_.top_right();
                gfx::Point pt_bounding(down_bounds_.x() + x_offset, down_bounds_.bottom());
                gfx::Rect rect = gfx::BoundingRect(pt_origin, pt_bounding);
                mask_view_->AdjustViewport(&rect);
            }
                break;
            case HTRIGHT:
            {
                gfx::Point pt_origin = down_bounds_.origin();
                gfx::Point pt_bounding(down_bounds_.right() + x_offset, down_bounds_.bottom());
                gfx::Rect rect = gfx::BoundingRect(pt_origin, pt_bounding);
                mask_view_->AdjustViewport(&rect);
            }
                break;
            case HTBOTTOMLEFT:
            {
                gfx::Point pt_origin = down_bounds_.top_right();
                gfx::Point pt_bounding(down_bounds_.x() + x_offset, down_bounds_.bottom() + y_offset);
                gfx::Rect rect = gfx::BoundingRect(pt_origin, pt_bounding);
                mask_view_->AdjustViewport(&rect);
            }
                break;
            case HTBOTTOM:
            {
                gfx::Point pt_origin = down_bounds_.origin();
                gfx::Point pt_bounding(down_bounds_.right(), down_bounds_.bottom() + y_offset);
                gfx::Rect rect = gfx::BoundingRect(pt_origin, pt_bounding);
                mask_view_->AdjustViewport(&rect);
            }
                break;
            case HTBOTTOMRIGHT:
            {
                gfx::Point pt_origin = down_bounds_.origin();
                gfx::Point pt_bounding(down_bounds_.right() + x_offset, down_bounds_.bottom() + y_offset);
                gfx::Rect rect = gfx::BoundingRect(pt_origin, pt_bounding);
                mask_view_->AdjustViewport(&rect);
            }
                break;
            default:
                break;
            }
            return true;
        }

        gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override
        {
            hittest_type_ = HitTest(event.location());
            gfx::NativeCursor cursor = gfx::kNullCursor;
            switch (hittest_type_)
            {
            case HTSIZE:
                cursor =::LoadCursor(nullptr, IDC_SIZEALL);
                break;
            case HTTOPLEFT:
            case HTBOTTOMRIGHT:
                cursor = ::LoadCursor(nullptr, IDC_SIZENWSE);
                break;
            case HTTOP:
            case HTBOTTOM:
                cursor = ::LoadCursor(nullptr, IDC_SIZENS);
                break;
            case HTTOPRIGHT:
            case HTBOTTOMLEFT:
                cursor = ::LoadCursor(nullptr, IDC_SIZENESW);
                break;
            case HTLEFT:
            case HTRIGHT:
                cursor = ::LoadCursor(nullptr, IDC_SIZEWE);
                break;
            default:
                break;
            }
            return cursor;
        }

        bool OnKeyPressed(const ui::KeyEvent& event)
        {
            if (event.key_code() == ui::VKEY_RETURN)
            {
                mask_view_->ApplyCapture();
                return true;
            }
            return false;
        }

    private:
        int HitTest(const gfx::Point& pt)
        {
            // 优先判断四个角
            static int kDetectThickness = GetLengthByDPIScale(4);
            if (gfx::Rect(0, 0, kDetectThickness, kDetectThickness).Contains(pt))
            {
                return HTTOPLEFT;
            }
            else if (gfx::Rect(width() - kDetectThickness, 0, kDetectThickness, kDetectThickness).Contains(pt))
            {
                return HTTOPRIGHT;
            }
            else if (gfx::Rect(0, height() - kDetectThickness, kDetectThickness, kDetectThickness).Contains(pt))
            {
                return HTBOTTOMLEFT;
            }
            else if (gfx::Rect(width() - kDetectThickness, height() - kDetectThickness, kDetectThickness, kDetectThickness).Contains(pt))
            {
                return HTBOTTOMRIGHT;
            }
            else if (gfx::Rect(0, 0, width(), kDetectThickness).Contains(pt))
            {
                return HTTOP;
            }
            else if (gfx::Rect(0, 0, kDetectThickness, height()).Contains(pt))
            {
                return HTLEFT;
            }
            else if (gfx::Rect(width() - kDetectThickness, 0, kDetectThickness, height()).Contains(pt))
            {
                return HTRIGHT;
            }
            else if (gfx::Rect(0, height() - kDetectThickness, width(), kDetectThickness).Contains(pt))
            {
                return HTBOTTOM;
            }

            return HTSIZE;
        }

    private:
        MonitorCaptureView* mask_view_ = nullptr;
        gfx::Rect limit_bounds_;
        gfx::Point down_pt_;
        gfx::Rect down_bounds_;
        int hittest_type_ = HTSIZE;
    };
}

// MonitorCaptureView
void MonitorCaptureView::ShowForm(EndDialogClosure* capture_result_callback)
{
    if (!g_single_instance)
    {
        GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->Minimize();

        g_single_instance = new MonitorCaptureView();

        views::Widget* widget = new views::Widget();
        views::Widget::InitParams params;
        params.delegate = g_single_instance;
        params.remove_taskbar_icon = true;
        params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;

        if (capture_result_callback)
        {
            g_single_instance->SetCaptureResultCallback(capture_result_callback);
        }

#if !defined(DEBUG) || !defined(_DEBUG)
        params.keep_on_top = true;// debug模式下不要始终置顶了，不然运行过程中触发断点了鼠标是点不到VS的
#endif // !DEBUG

        ShowWidget(g_single_instance, widget, params, true, false, views::ISC_NONE);
        widget->SetBounds(GetAllMonitorUnionRect());
    }
}

MonitorCaptureView::MonitorCaptureView() {
    viewport_view_ = new CaptureViewport(this);

    operate_view_ = new views::View();
    {
        operate_view_->SetLayoutManager(new views::BoxLayout(views::BoxLayout::kHorizontal,
            kPaddingColWidthForCtrlTips, 0, kPaddingRowHeightForCtrlTips));
        BililiveImageButton* cancel = new BililiveImageButton(this);
        cancel->SetAllStateImage(GetImageSkiaNamed(IDR_LIVEHIME_SCREEN_CAPTURE_CLOSE));
        cancel->SetTooltipText(GetLocalizedString(IDS_SRCPROP_COMMON_CANCEL));
        cancel->set_id(Button_Cancel);
        BililiveImageButton* ok = new BililiveImageButton(this);
        ok->SetAllStateImage(GetImageSkiaNamed(IDR_LIVEHIME_SCREEN_CAPTURE_CONFIRM));
        ok->SetTooltipText(GetLocalizedString(IDS_SRCPROP_COMMON_OK));
        ok->set_id(Button_OK);
        operate_view_->AddChildView(cancel);
        operate_view_->AddChildView(ok);
        operate_view_->set_background(views::Background::CreateSolidBackground(SK_ColorWHITE));
        operate_view_->SetVisible(false);
    }

    tip_view_ = new views::View();
    {
        tip_view_->SetLayoutManager(new views::BoxLayout(views::BoxLayout::kVertical,
            kPaddingColWidthForCtrlTips, kPaddingRowHeightForCtrlTips, 0));
        tip_view_->set_background(views::Background::CreateSolidBackground(kClrBk));

        tip_rect_label_ = new LivehimeContentLabel(L"0 x 0");
        tip_rect_label_->SetTextColor(SK_ColorWHITE);
        tip_rect_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);

        auto new_label = [](const base::string16& text)->views::View*
        {
            BililiveLabel* label = new LivehimeContentLabel(text);
            label->SetTextColor(SK_ColorWHITE);
            label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
            return label;
        };

        tip_view_->AddChildView(tip_rect_label_);
        tip_view_->AddChildView(new_label(L"单击鼠标左键捕获当前预选区域"));
        tip_view_->AddChildView(new_label(L"按住鼠标左键并拖拽以框选捕获区域"));
        tip_view_->AddChildView(new_label(L"确认捕获区域后双击鼠标左键完成捕获"));
        tip_view_->AddChildView(new_label(L"鼠标右键或键盘ESC键取消/退出捕获"));
    }

    rect_label_ = new LivehimeContentLabel();
    rect_label_->set_background(views::Background::CreateSolidBackground(kClrBk));
    rect_label_->SetTextColor(SK_ColorWHITE);
    rect_label_->SetVisible(false);

    AddChildView(viewport_view_);
    AddChildView(rect_label_);
    AddChildView(tip_view_);
    AddChildView(operate_view_);

    set_focusable(true);
    set_focus_border(nullptr);
}

MonitorCaptureView::~MonitorCaptureView()
{
    g_single_instance = nullptr;
}

void MonitorCaptureView::SetCaptureResultCallback(EndDialogClosure* capture_result_callback)
{
    capture_result_callback_ = *capture_result_callback;
}

void MonitorCaptureView::AdjustViewport(gfx::Rect* capture_rect/* = nullptr*/)
{
    if (viewport_view_)
    {
        // 选好之后通过微调进行裁剪的要嘛不允许缩小到空尺寸，要嘛当做放弃本次选取
        if (capture_rect)
        {
            capture_rect_ = *capture_rect;

            // 当前采取不允许缩到空的做法
            capture_rect_.set_width(std::max(1, capture_rect_.width()));
            capture_rect_.set_height(std::max(1, capture_rect_.height()));
            capture_rect_.set_x(std::min(display_map_view_rect_.right() - 1, capture_rect_.x()));
            capture_rect_.set_y(std::min(display_map_view_rect_.bottom() - 1, capture_rect_.y()));
        }

        viewport_view_->SetBoundsRect(capture_rect_);
        viewport_view_->SetVisible(valid_capture_);

        if (!drag_size_now_)
        {
            gfx::Size size = operate_view_->GetPreferredSize();
            gfx::Rect rect(capture_rect_.right() - size.width(), capture_rect_.bottom() + GetLengthByDPIScale(2),
                size.width(), size.height());
            rect.set_x(std::max(display_map_view_rect_.x(), rect.x()));
            rect.set_x(std::min(display_map_view_rect_.right() - rect.width(), rect.x()));
            // 捕获区下面放不下就放上面
            if (rect.bottom() > display_map_view_rect_.bottom())
            {
                rect.set_y(capture_rect_.y() - GetLengthByDPIScale(2) - size.height());
            }
            rect.set_y(std::max(display_map_view_rect_.y(), rect.y()));
            operate_view_->SetBoundsRect(rect);
        }
        // 没在drag状态且区域有效才显示确认/取消view
        if (valid_capture_)
        {
            operate_view_->SetVisible(!drag_size_now_);
        }
        else
        {
            operate_view_->SetVisible(false);
        }

        if (valid_capture_)
        {
            rect_label_->SetText(base::StringPrintf(L"%d x %d", capture_rect_.width(), capture_rect_.height()));

            // 框选尺寸label放在区域外左上角贴边，区域外尺寸不够放就直接侵占到区域内
            gfx::Size size = rect_label_->GetPreferredSize();
            gfx::Rect rect(capture_rect_.x(), capture_rect_.y() - GetLengthByDPIScale(2) - size.height(),
                size.width(), size.height());
            rect.set_x(std::min(display_map_view_rect_.right() - rect.width(), rect.x()));
            rect.set_y(std::max(display_map_view_rect_.y(), rect.y()));
            rect_label_->SetBoundsRect(rect);
        }
        rect_label_->SetVisible(valid_capture_);

        SchedulePaint();
    }
}

void MonitorCaptureView::ApplyCapture()
{
    if (capture_display_.is_valid() && !capture_rect_.IsEmpty())
    {
        // 截取区域是窗口坐标，转为和capture_display_一样参照系的屏幕坐标
        gfx::Point pt(capture_rect_.origin());
        views::View::ConvertPointToScreen(this, &pt);
        {
            bililive::MonitorCaptureData extra_data(capture_display_, gfx::Rect(pt, capture_rect_.size()));
            extra_data.silence_mode = true;
            bililive::CreatingSourceParams params(bililive::SourceType::Monitor, &extra_data);
            bililive::ExecuteCommandWithParams(
                GetBililiveProcess()->bililive_obs(),
                IDC_LIVEHIME_ADD_SOURCE,
                CommandParams<bililive::CreatingSourceParams>(&params));
        }
        GetWidget()->Close();
		livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::SourceSettingSubmitClick,
			"material_type:" + std::to_string((size_t)secret::BehaviorEventMaterialType::Monitor));
        if (!capture_result_callback_.is_null())
        {
            capture_result_callback_.Run(IDOK,nullptr);
        }
    }
}

void MonitorCaptureView::AdjustTipView()
{
    // 拖拽框选中或者区域尚无效时显示操作提示view
    if (!valid_capture_ || drag_size_now_)
    {
        if (!display_map_view_rect_.IsEmpty())
        {
            gfx::Point pt = gfx::Screen::GetNativeScreen()->GetCursorScreenPoint();
            views::View::ConvertPointFromScreen(this, &pt);
            gfx::Size size = tip_view_->GetPreferredSize();
            static const int kCursorCX = GetLengthByDPIScale(16);
            static const int kCursorCY = GetLengthByDPIScale(56);
            gfx::Rect rect(pt.x() + kCursorCX / 2, pt.y() + kCursorCY / 2,
                size.width(), size.height());
            if (rect.right() > display_map_view_rect_.right())
            {
                rect.set_x(pt.x() - kCursorCX / 2 - size.width());
            }
            if (rect.bottom() > display_map_view_rect_.bottom())
            {
                rect.set_y(pt.y() - kCursorCY / 2 - size.height());
            }
            tip_view_->SetBoundsRect(rect);

            size = capture_rect_.IsEmpty() ? hovered_wnd_rect_.size() : capture_rect_.size();
            tip_rect_label_->SetText(base::StringPrintf(L"%d x %d", size.width(), size.height()));
        }
        tip_view_->SetVisible(true);
    }
    else
    {
        tip_view_->SetVisible(false);
    }
}

views::NonClientFrameView* MonitorCaptureView::CreateNonClientFrameView(views::Widget* widget)
{
    CaptureWindowFrameView* frame_view = new CaptureWindowFrameView();
    return frame_view;
}

void MonitorCaptureView::WindowClosing()
{
    GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->Restore();
    TabAreaMaterialsWidget::Close();
    __super::WindowClosing();
}

void MonitorCaptureView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add && GetWidget())
        {
            if (GetWidget()->GetFocusManager())
            {
                GetWidget()->GetFocusManager()->SetFocusedView(this);
            }
        }
    }
}

gfx::Size MonitorCaptureView::GetPreferredSize()
{
    return GetAllMonitorUnionRect().size();
}

void MonitorCaptureView::OnPaintBackground(gfx::Canvas* canvas)
{
    canvas->FillRect(GetLocalBounds(), SkColorSetA(SK_ColorWHITE, 1));

    if (!valid_capture_)
    {
        if (!hovered_wnd_rect_.IsEmpty())
        {
            static int kStrokeWidth = GetLengthByDPIScale(4);
            SkPaint paint;
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(kStrokeWidth);
            paint.setColor(kClrBorder);
            canvas->DrawRect(hovered_wnd_rect_, paint);
        }
    }
    else
    {
        // 周边蒙版，捕获区高亮
        SkPath path;
        path.addRect(SkRect::MakeWH(width(), height()));
        SkPath capture_path;
        capture_path.addRect(SkRect::MakeXYWH(capture_rect_.x(), capture_rect_.y(),
            capture_rect_.width(), capture_rect_.height()));
        path.reverseAddPath(capture_path);

        static U8CPU a = 180;
        SkPaint paint;
        paint.setColor(SkColorSetA(SK_ColorDKGRAY, a));
        canvas->DrawPath(path, paint);
    }
}

void MonitorCaptureView::OnMouseMoved(const ui::MouseEvent& event)
{
    if (!valid_capture_)
    {
        gfx::Display hoverd_display = gfx::Screen::GetNativeScreen()->GetDisplayNearestPoint(
            gfx::Screen::GetNativeScreen()->GetCursorScreenPoint());
        if (capture_display_.bounds() != hoverd_display.bounds())
        {
            capture_display_ = hoverd_display;

            gfx::Point pt = capture_display_.bounds().origin();
            views::View::ConvertPointFromScreen(this, &pt);
            display_map_view_rect_ = gfx::Rect(pt, capture_display_.bounds().size());
        }

        // 预选鼠标悬浮处的窗口区域
        bool repaint = false;
        gfx::Point pt = gfx::Screen::GetNativeScreen()->GetCursorScreenPoint();
        HWND hwnd = GetWidget()->GetNativeWindow();
        do
        {
            hwnd = ::GetWindow(hwnd, GW_HWNDNEXT);
            if (hwnd)
            {
                // 只在乎可见的顶层窗口
                if (::IsWindowVisible(hwnd) && (::GetParent(hwnd) == nullptr))
                {
                    RECT rt;
                    ::GetWindowRect(hwnd, &rt);
                    gfx::Rect rect(rt);
                    if (rect.Contains(pt))
                    {
                        gfx::Point pt(rect.origin());
                        views::View::ConvertPointFromScreen(this, &pt);
                        rect.set_origin(pt);

                        // 超出当前屏幕的窗口区域的不要
                        rect.Intersect(display_map_view_rect_);

                        if (rect != hovered_wnd_rect_)
                        {
                            hovered_wnd_rect_ = rect;
                            repaint = true;
                        }

                        break;
                    }
                }
            }
            else
            {
                if (hovered_wnd_rect_ != display_map_view_rect_)
                {
                    hovered_wnd_rect_ = display_map_view_rect_;
                    repaint = true;
                }
            }
        } while (hwnd);

        if (repaint)
        {
            SchedulePaint();
        }
    }

    AdjustTipView();
}

bool MonitorCaptureView::OnMousePressed(const ui::MouseEvent& event)
{
    can_drag_ = false;
    if (event.IsLeftMouseButton())
    {
        static base::Time pre_click_stamp;
        base::Time click_stamp = base::Time::Now();

        // 没截取区域就开始进行截取，已经截取了的话那么在双击的时候就确认选取
        if (!valid_capture_)
        {
            down_pt_ = event.location();
            capture_rect_ = gfx::Rect();
            can_drag_ = true;
            viewport_view_->SetLimitBounds(display_map_view_rect_);
            return true;
        }
        else
        {
            if (base::TimeDelta(click_stamp - pre_click_stamp).InMilliseconds() < kDoubleClickTimeMS)
            {
                ApplyCapture();
            }
        }

        pre_click_stamp = click_stamp;
    }
    else if (event.IsRightMouseButton())
    {
        if (valid_capture_)
        {
            valid_capture_ = false;
            capture_rect_ = gfx::Rect();
            AdjustViewport();
        }
        else
        {
            GetWidget()->Close();
        }
    }
    return true;
}

bool MonitorCaptureView::OnMouseDragged(const ui::MouseEvent& event)
{
    if (!can_drag_)
    {
        return false;
    }

    // 框选范围不能超过鼠标点击时所在的监视器的范围
    if (event.IsLeftMouseButton())
    {
        if (!display_map_view_rect_.IsEmpty())
        {
            valid_capture_ = true;
            drag_size_now_ = true;

            gfx::Point cur_pt = event.location();
            cur_pt.set_x(std::max(cur_pt.x(), display_map_view_rect_.x()));
            cur_pt.set_x(std::min(cur_pt.x(), display_map_view_rect_.right()));
            cur_pt.set_y(std::max(cur_pt.y(), display_map_view_rect_.y()));
            cur_pt.set_y(std::min(cur_pt.y(), display_map_view_rect_.bottom()));
            capture_rect_ = gfx::BoundingRect(down_pt_, cur_pt);
            AdjustViewport();
            AdjustTipView();
            return true;
        }
    }
    return false;
}

void MonitorCaptureView::OnMouseReleased(const ui::MouseEvent& event)
{
    if (!can_drag_)
    {
        return;
    }

    // 如果是原地点击的则直接选中当前预选区域
    if (event.IsLeftMouseButton())
    {
        if (down_pt_ == event.location())
        {
            if (capture_rect_.IsEmpty())
            {
                capture_rect_ = hovered_wnd_rect_;
            }
        }
    }
    drag_size_now_ = false;
    valid_capture_ = !capture_rect_.IsEmpty();
    AdjustViewport();
    AdjustTipView();
}

bool MonitorCaptureView::OnKeyPressed(const ui::KeyEvent& event)
{
    if (event.key_code() == ui::VKEY_RETURN)
    {
        ApplyCapture();
        return true;
    }
    return false;
}

void MonitorCaptureView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (sender->id() == Button_OK)
    {
        ApplyCapture();
        return;
    }
    GetWidget()->Close();
}
