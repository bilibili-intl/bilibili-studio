#include "source_fliter_select_color_view.h"

#include <stdio.h>

#include "base/strings/utf_string_conversions.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/layout/box_layout.h"
#include "ui/gfx/display.h"
#include "ui/gfx/screen.h"
#include "ui/views/view.h"
#include "ui/base/resource/resource_bundle.h"

#include "bililive/public/bililive/bililive_process.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_filter_base_property_view.h"

SourceFliterSelectColorView* g_single_instance = nullptr;
const SkColor kClrBk = SkColorSetA(0x65C4FF, 180);

BOOL CALLBACK EnumAllMonitor(HMONITOR monitor, HDC hdc, LPRECT rc, LPARAM data) {
    gfx::Rect* rect = (gfx::Rect*)data;
    *rect = gfx::UnionRects(*rect, gfx::Rect(*rc));
    return true;
}

gfx::Rect GetAllMonitorUnionRect() {
    static gfx::Rect rect;
    if (rect.IsEmpty()) {
        bool success = !!::EnumDisplayMonitors(NULL, NULL, EnumAllMonitor, (LPARAM)&rect);
        DCHECK(success);
    }
    return rect;
}

void SourceFliterSelectColorView::ShowWindow(SourceFilterBaseView* view) {
    if (!g_single_instance) {

        g_single_instance = new SourceFliterSelectColorView(view);
        views::Widget* widget = new views::Widget();
        views::Widget::InitParams params;
        params.delegate = g_single_instance;
        params.remove_taskbar_icon = true;
        params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;
        params.type = views::Widget::InitParams::TYPE_WINDOW_FRAMELESS;
        params.keep_on_top = true;

        BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(widget);
        native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_CLOSE);

        params.native_widget = native_widget;

        DoModalWidget(g_single_instance,widget,params);
        //ShowWidget(g_single_instance, widget, params, true, false, views::ISC_NONE);
        widget->SetBounds(GetAllMonitorUnionRect()/*gfx::Rect(0,0,800,600)*/);
    }
    else {
        if (g_single_instance->GetWidget()) {
            g_single_instance->GetWidget()->Activate();
        }
    }
}

SourceFliterSelectColorView::SourceFliterSelectColorView(SourceFilterBaseView* view)
    :tip_view_(nullptr),
    text_label_(nullptr),
    filter_view_(view){

}

SourceFliterSelectColorView::~SourceFliterSelectColorView() {
   g_single_instance = nullptr;
}

bool SourceFliterSelectColorView::OnMousePressed(const ui::MouseEvent& event) {
    if (event.IsLeftMouseButton()) {
        //更新颜色
        if (filter_view_ != nullptr) {
            this->SetVisible(false);
            filter_view_->UpdateFilterCustomColor(GetCursorPosColor());
        }
        GetWidget()->Close();
    }
    return true;
}

void SourceFliterSelectColorView::OnMouseMoved(const ui::MouseEvent& event) {
    gfx::Display hoverd_display = gfx::Screen::GetNativeScreen()->GetDisplayNearestPoint(
        gfx::Screen::GetNativeScreen()->GetCursorScreenPoint());
 
    gfx::Point pt = hoverd_display.bounds().origin();
    views::View::ConvertPointFromScreen(this, &pt);
    display_view_rect_ = gfx::Rect(pt, hoverd_display.bounds().size());

    AdjustTipViewPos();
    UpdateTipsViewColorVal(GetCursorPosColor());
}

void SourceFliterSelectColorView::OnPaintBackground(gfx::Canvas* canvas) {
    //背景透明处理
    canvas->FillRect(GetLocalBounds(), SkColorSetA(SK_ColorWHITE,1));
}


gfx::Size SourceFliterSelectColorView::GetPreferredSize() {
    return /*gfx::Size(800,600)*/GetAllMonitorUnionRect().size();
}

gfx::NativeCursor SourceFliterSelectColorView::GetCursor(const ui::MouseEvent& event) {
    gfx::NativeCursor cursor = ::LoadCursor(nullptr, IDC_CROSS);
    return cursor;
}

// View
void SourceFliterSelectColorView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) {
    if (details.child == this) {
        if (details.is_add) {
            InitViews();
        }
    }
}

void SourceFliterSelectColorView::WindowClosing() {
    __super::WindowClosing();
}

void SourceFliterSelectColorView::InitViews() {
    tip_view_ = new views::View();
    {
        tip_view_->SetLayoutManager(new views::BoxLayout(views::BoxLayout::kVertical,
            kPaddingColWidthForCtrlTips, kPaddingRowHeightForCtrlTips, 0));
        tip_view_->set_background(views::Background::CreateSolidBackground(kClrBk));

        text_label_ = new LivehimeContentLabel(L"RGB(255,255,255)");
        text_label_->SetTextColor(SK_ColorWHITE);
        text_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);

        BililiveLabel* tip_label = new LivehimeContentLabel(ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_LEFT_CLICK_SELECT_COLOR));
        tip_label->SetTextColor(SK_ColorWHITE);
        tip_label->SetHorizontalAlignment(gfx::ALIGN_LEFT);

        tip_view_->AddChildView(text_label_);
        tip_view_->AddChildView(tip_label);

    }

    AddChildView(tip_view_);
    set_focusable(true);
    set_focus_border(nullptr);
}

void SourceFliterSelectColorView::AdjustTipViewPos() {
    if (!display_view_rect_.IsEmpty()) {

        gfx::Point pt = gfx::Screen::GetNativeScreen()->GetCursorScreenPoint();
        views::View::ConvertPointFromScreen(this, &pt);

        gfx::Size size = tip_view_->GetPreferredSize();
        int cursor_x = GetLengthByDPIScale(16);
        int cursor_y = GetLengthByDPIScale(56);

        gfx::Rect rect(pt.x() + cursor_x / 2, pt.y() + cursor_y / 2,
            size.width(), size.height());
        if (rect.right() > display_view_rect_.right()) {
            rect.set_x(pt.x() - cursor_x / 2 - size.width());
        }
        if (rect.bottom() > display_view_rect_.bottom()) {
            rect.set_y(pt.y() - cursor_y / 2 - size.height());
        }
        tip_view_->SetBoundsRect(rect);
    }
    tip_view_->SetVisible(true);
}

SkColor SourceFliterSelectColorView::GetCursorPosColor() {
    HDC hdc = ::GetDC(NULL);

    gfx::Point pt = gfx::Screen::GetNativeScreen()->GetCursorScreenPoint();
    COLORREF windows_hdc_rgb = ::GetPixel(hdc, pt.x(), pt.y());	//获取指定DC上的像素点RGB值

    WORD r = (windows_hdc_rgb & 0x00FF0000) >> 16;
    WORD g = (windows_hdc_rgb & 0x0000FF00) >> 8;
    WORD b = windows_hdc_rgb & 0x000000FF;

    if (r < 127 || r == 129) {
        r--;
    }
    if (g < 127 || g == 129) {
        g--;
    }
    if (b < 127 || b == 129) {
        b--;
    }

    windows_hdc_rgb = SkColorSetRGB(b, g, r);

    return windows_hdc_rgb;
}

void SourceFliterSelectColorView::UpdateTipsViewColorVal(const SkColor& color) {
    if (text_label_ != nullptr) {
        char color_text[64] = { 0 };
        sprintf_s(color_text,
                " RGB (%d,%d,%d) ", 
                SkColorGetR(color),
                SkColorGetG(color), 
                SkColorGetB(color));

        text_label_->SetText(UTF8ToUTF16(color_text));
    }
}