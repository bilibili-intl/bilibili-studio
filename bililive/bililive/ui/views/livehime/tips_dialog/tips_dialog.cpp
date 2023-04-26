#include "bililive/bililive/ui/views/livehime/tips_dialog/tips_dialog.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/widget/widget.h"

#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/public/bililive/bililive_thread.h"

#include "grit/generated_resources.h"

namespace {
    TipsDialog* g_single_instance = nullptr;
}

void TipsDialog::ShowWindow(views::Widget* parent, const gfx::Rect& rect, TipsType type,const gfx::Rect* refer, const EndDialogSignalHandler* handler,int fade_duration_ms) {
    if (!g_single_instance) {
        views::Widget* widget = new views::Widget();

        views::Widget::InitParams params;
        params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;
        params.native_widget = new BililiveNativeWidgetWin(widget);
        params.parent = parent->GetNativeView();

        g_single_instance = new TipsDialog(widget,rect,type,refer);
        if (fade_duration_ms > 0)
        {
            g_single_instance->StartFade(fade_duration_ms);
        }
        DoModalWidget(g_single_instance, widget, params,handler);
    }
}

void TipsDialog::MoveDialog()
{
    if (!widget_)
        return;
    if (type_ == CameraTips && bubble_img_view_ && img_view_)
    {
        gfx::Rect widget_rect = widget_->GetWindowBoundsInScreen();
        gfx::Rect bubble_img_rect = bubble_img_view_->GetBoundsInScreen();
        gfx::Rect img_rect = img_view_->GetBoundsInScreen();
        widget_->SetBounds(gfx::Rect(refer_.x() - (left_view_->width() - refer_.width()) / 2 - kPaddingColWidthEndsSmall,
            refer_.bottom() - std::max(0, bubble_img_rect.y() - img_rect.y()), widget_rect.width(), widget_rect.height()));
    }
    else if (type_ == LiveReplayTips)
    {
        gfx::Rect widget_rect = widget_->GetWindowBoundsInScreen();

        widget_->SetBounds(gfx::Rect(widget_rect.x()+ widget_rect.width()*0.6,
            widget_rect.y()+ widget_rect.height()*0.8, widget_rect.width(), widget_rect.height()));
    }
}

TipsDialog::TipsDialog(views::Widget* widget, const gfx::Rect& rect, TipsType type,const gfx::Rect* refer)
    : widget_(widget)
    , weak_ptr_factory_(this)
    , rect_(rect)
    , type_(type)
{
    if (refer)
    {
        refer_.SetRect(refer->x(), refer->y(), refer->width(), refer->height());
    }
}

TipsDialog::~TipsDialog() {
    g_single_instance = nullptr;
}

void TipsDialog::ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) {
    if (details.child == this) {
        if (details.is_add) {
            InitViews();
        }
    }
}

void TipsDialog::OnPaintBackground(gfx::Canvas* canvas) {
    auto left_bound = left_view_->bounds();
    if (type_ == CameraTips)
    {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(GetColor(Theme));

        canvas->DrawRoundRect(gfx::Rect(
            0, left_bound.y() + bubble_img_view_->size().height(),
            width() - GetLengthByImgDPIScale(30), left_bound.height() - bubble_img_view_->size().height()), GetLengthByDPIScale(4), paint);
    }
    else if (type_ == LiveReplayTips)
    {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(GetColor(Theme));

        canvas->DrawRoundRect(gfx::Rect(
            0, kPaddingRowHeightEnds * 2,
            width() - GetLengthByImgDPIScale(30), left_bound.height() - kPaddingRowHeightEnds * 2), GetLengthByDPIScale(4), paint);
    }
}

views::NonClientFrameView* TipsDialog::CreateNonClientFrameView(views::Widget* widget)
{
    BililiveNonTitleBarFrameView* frame_view = new BililiveNonTitleBarFrameView(nullptr);
    frame_view->SetBackgroundColor(SK_ColorTRANSPARENT);
    frame_view->SetEnableDragMove(false);
    return frame_view;
}


void TipsDialog::ButtonPressed(views::Button* sender, const ui::Event& event) {

    if (sender == close_button_)
    {
        GetWidget()->Close();
    }
}

void TipsDialog::StartFade(int fade_duration_ms)
{
    base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
        base::Bind(&TipsDialog::FadeOut, weak_ptr_factory_.GetWeakPtr()),
        base::TimeDelta::FromMilliseconds(fade_duration_ms));
}

void TipsDialog::FadeOut()
{
    GetWidget()->Close();
}

void TipsDialog::InitViews() {
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);

    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::TRAILING, 0, views::GridLayout::FIXED,
        GetLengthByDPIScale(rect_.width()), GetLengthByDPIScale(rect_.width()));

    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);

    left_view_ = new views::View();
    {
        views::GridLayout* left_layout = new views::GridLayout(left_view_);
        left_view_->SetLayoutManager(left_layout);

        views::ColumnSet* left_column_set = left_layout->AddColumnSet(0);
        left_column_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::TRAILING, 1.0, views::GridLayout::FIXED, 0, 0);

        left_column_set = left_layout->AddColumnSet(1);
        left_column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0, views::GridLayout::FIXED, 0, 0);

        left_column_set = left_layout->AddColumnSet(2);
        left_column_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::LEADING, 1.0, views::GridLayout::FIXED, 0, 0);

        BililiveLabel *label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_SRCPROP_CAMERA_ADDSRC_BEAUTY_TIPS));
        if (type_ == LiveReplayTips)
        {
            label->SetText(rb.GetLocalizedString(IDS_TOOLBAR_REPLAY_TIPS));
        }
        label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
        label->SetMultiLine(true);
        label->SetTextColor(SkColorSetRGB(255, 255, 255));

        close_button_ = new LivehimeFunctionLabelButton(this, rb.GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_IKNOW));
        close_button_->SetStyle(LivehimeButtonStyle_V3_TitleButton);

        left_layout->StartRow(0, 0);
        if (type_ == CameraTips)
        {
            bubble_img_view_ = new views::ImageView();
            bubble_img_view_->SetImage(ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_V3_TIPS_DIALOG_BUBBLE));
            left_layout->AddView(bubble_img_view_);
        }
        else
        {
            left_layout->AddPaddingRow(0, kPaddingRowHeightEnds *2);
        }


        left_layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
        left_layout->AddPaddingRow(1, 0);
        left_layout->StartRow(0, 1);
        left_layout->AddView(label);
        if (type_ == CameraTips)
        {
            left_layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
        }
        else
        {
            left_layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForCtrlTips);
        }
        left_layout->StartRow(0, 2);
        left_layout->AddView(close_button_);
        left_layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    }
    layout->StartRow(0, 0);
    layout->AddView(left_view_);

    img_view_ = new views::ImageView();
    img_view_->set_interactive(false);
    img_view_->SetImage(ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_V3_TIPS_DIALOG_IMG33));
    layout->AddView(img_view_);


    BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
        base::Bind(&TipsDialog::MoveDialog,
            base::Unretained(this)));
}