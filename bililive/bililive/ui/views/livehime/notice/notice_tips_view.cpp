#include "bililive/bililive/ui/views/livehime/notice/notice_tips_view.h"

#include "ui/views/layout/grid_layout.h"
#include "ui/views/painter.h"
#include "ui/views/widget/widget.h"

#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "grit/theme_resources.h"

#include "base/strings/utf_string_conversions.h"

namespace {

NoticeTipsView* g_single_instance = nullptr;

}

void NoticeTipsView::ShowWindow(const std::string& icon_url, const std::string& content) {
    if (!g_single_instance) {
        views::Widget *widget_ = new views::Widget();
        views::Widget::InitParams params;
        params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;
        params.native_widget = new BililiveNativeWidgetWin(widget_);
        params.parent = nullptr;
        params.keep_on_top = true;
        params.remove_taskbar_icon = true;
        g_single_instance = new NoticeTipsView(icon_url, content);

        BililiveWidgetDelegate::ShowWidget(g_single_instance, widget_, params, false);
        views::InsureWidgetVisible(widget_, views::InsureShowCorner::ISC_BOTTOMRIGHT);
        widget_->ShowInactive();
    }
}

NoticeTipsView::NoticeTipsView(const std::string& icon_url, const std::string& content)
    : weakptr_factory_(this)
    , icon_url_(icon_url)
    , content_(content){
}

NoticeTipsView::~NoticeTipsView() {
    g_single_instance = nullptr;
}

void NoticeTipsView::ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) {
    if (details.child == this) {
        if (details.is_add) {
            InitViews();
        }
    }
}

gfx::Size NoticeTipsView::GetPreferredSize() {
    int cy = info_view_->GetHeightForWidth(GetLengthByDPIScale(370));
    return  gfx::Size(GetLengthByDPIScale(370), std::max(GetLengthByDPIScale(80), cy));
}

void NoticeTipsView::OnPaintBackground(gfx::Canvas* canvas) {
    gfx::Rect rect = GetContentsBounds();

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorWHITE);
    paint.setStyle(SkPaint::kFill_Style);

    static int radius = GetLengthByDPIScale(4);

    canvas->DrawRoundRect(GetLocalBounds(), radius, paint);
}

int NoticeTipsView::NonClientHitTest(const gfx::Point &point) {
    if (GetEventHandlerForPoint(point) == close_view_) {
        return HTNOWHERE;
    }

    return HTCAPTION;
}

views::NonClientFrameView* NoticeTipsView::CreateNonClientFrameView(views::Widget *widget) {
    BililiveNonTitleBarFrameView* frame_view = new BililiveNonTitleBarFrameView(this);
    frame_view->SetBackgroundColor(SK_ColorTRANSPARENT);
    frame_view->SetEnableDragMove(true);
    return frame_view;
}

void NoticeTipsView::InitViews() {
    info_view_ = new views::View();
    {
        views::GridLayout *layout = new views::GridLayout(info_view_);
        info_view_->SetLayoutManager(layout);

        static int content_width = GetLengthByDPIScale(180);
        views::ColumnSet* column_set = layout->AddColumnSet(0);
        column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
        column_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, content_width, content_width);
        column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

        auto status_img = new LivehimeImageView();
        status_img->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_NOTICE_VIEW_STATUS));
        status_img->SetPreferredSize(GetSizeByDPIScale(gfx::Size(64, 64)));
        if (!icon_url_.empty()) {
            status_img->SetImageUrl(icon_url_);
        }

        auto tips_label = new LivehimeContentLabel(base::UTF8ToUTF16(content_));
        tips_label->SetFont(ftSixteen);
        tips_label->SetMultiLine(true);
        tips_label->SetElideBehavior(views::Label::ElideBehavior::NO_ELIDE);
        tips_label->SetAllowCharacterBreak(true);
        tips_label->SetHorizontalAlignment(gfx::ALIGN_LEFT);

        layout->AddPaddingRow(0, kPaddingRowHeightForDiffGroups);

        layout->StartRow(0, 0);
        layout->AddView(status_img);
        layout->AddView(tips_label);

        layout->AddPaddingRow(0, kPaddingRowHeightForDiffGroups);
    }

    close_view_ = new LivehimeImageView(this);
    close_view_->SetCursor(::LoadCursor(nullptr, IDC_HAND));
    close_view_->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_NOTICE_VIEW_CLOSE));

    AddChildView(info_view_);
    AddChildView(close_view_);

    base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
        base::Bind(&NoticeTipsView::CloseWindow, weakptr_factory_.GetWeakPtr()),
        base::TimeDelta::FromSeconds(10));
}

void NoticeTipsView::Layout() {
    gfx::Size close_size = close_view_->GetPreferredSize();

    static int inset = GetLengthByDPIScale(5);

    info_view_->SetBounds(0, 0, width(), height());
    close_view_->SetBounds(width() - close_size.width() - inset, inset, close_size.width(), close_size.height());
}

void NoticeTipsView::ButtonPressed(views::Button* sender, const ui::Event& event) {
    CloseWindow();
}

void NoticeTipsView::CloseWindow() {
    GetWidget()->Close();
}