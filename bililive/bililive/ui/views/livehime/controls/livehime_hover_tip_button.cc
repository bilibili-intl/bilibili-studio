#include "livehime_hover_tip_button.h"

#include "ui/views/layout/grid_layout.h"

#include "ui/base/resource/resource_bundle.h"
#include "grit/generated_resources.h"
#include "grit/theme_resources.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"


// LivehimeTipsBubble
LivehimeTipsBubble * LivehimeTipsBubble::instance_ = nullptr;

void LivehimeTipsBubble::Show(views::View *anchor, const base::string16 &title,
    const base::string16 &tips, views::BubbleBorder::Arrow arrow)
{
    if (instance_)
    {
        if (instance_->anchor_view() != anchor)
        {
            Close(instance_->anchor_view());
            instance_ = nullptr;
        }
    }
    if (!instance_)
    {
        instance_ = new LivehimeTipsBubble(anchor, title, tips, arrow);
        views::Widget* widget = BubbleDelegateView::CreateBubble(instance_);
        instance_->SetArrowPaintType(views::BubbleBorder::ArrowPaintType::PAINT_NONE);
        widget->ShowInactive();
    }
}

void LivehimeTipsBubble::Close(views::View *anchor)
{
    if (instance_ && instance_->anchor_view() == anchor)
    {
        instance_->GetWidget()->Close();
        instance_ = nullptr;
    }
}

void LivehimeTipsBubble::OnAnchorViewVisibleBoundsChanged()
{
    if (instance_)
    {
        instance_->OnAnchorViewBoundsChangedEx();
    }
}

LivehimeTipsBubble::LivehimeTipsBubble(views::View *anchor, const base::string16 &title, const base::string16 &tips,
    views::BubbleBorder::Arrow arrow)
    : BililiveBubbleView(anchor, arrow)
    , title_(title)
    , tips_(tips)
{
    set_shadow(views::BubbleBorder::Shadow::NO_SHADOW);
    set_close_on_deactivate(false);
}

LivehimeTipsBubble::~LivehimeTipsBubble()
{
    if (instance_ == this)
    {
        instance_ = nullptr;
    }
}

void LivehimeTipsBubble::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            ResourceBundle &rb = ResourceBundle::GetSharedInstance();

            views::GridLayout *layout = new views::GridLayout(this);
            SetLayoutManager(layout);

            views::ColumnSet *column_set = layout->AddColumnSet(0);
            column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
            column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
            column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);

            title_label_ = new LivehimeTitleLabel(title_);
            title_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);

            tips_label_ = new LivehimeTipLabel(tips_, clrLabelContent);
            tips_label_->SetMultiLine(true);
            tips_label_->SetAllowCharacterBreak(true);
            tips_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);

            if (title_.size())
            {
                layout->AddPaddingRow(0, kPaddingRowHeightForCtrlTips);
                layout->StartRow(0, 0);
                layout->AddView(title_label_);
                layout->AddPaddingRow(0, kPaddingRowHeightForCtrlTips);
            }

            layout->StartRow(0, 0);
            layout->AddView(tips_label_);

            if (title_.size())
            {
                layout->AddPaddingRow(0, kPaddingRowHeightForCtrlTips);
            }
        }
    }
}

gfx::Size LivehimeTipsBubble::GetPreferredSize()
{
    static int cx = LivehimePaddingCharWidth(ftPrimary) * 25;
    gfx::Size size = __super::GetPreferredSize();
    size.set_width(std::min(cx, size.width()));
    return size;
}

void LivehimeTipsBubble::OnAnchorViewBoundsChangedEx()
{
    if (anchor_view())
    {
        gfx::Rect rect = anchor_view()->GetVisibleBounds();
        if (rect.IsEmpty())
        {
            GetWidget()->Close();
        }
    }
    __super::OnAnchorViewBoundsChanged();
}


// LivehimeHoverTipButton
LivehimeHoverTipButton::LivehimeHoverTipButton(const base::string16 &title, const base::string16 &tips,
    views::BubbleBorder::Arrow arrow/* = views::BubbleBorder::LEFT_TOP*/,
    views::ButtonListener* listener/* = nullptr*/)
    : ImageButton(listener)
    , bubble_(nullptr)
    , title_(title)
    , tips_(tips)
    , arrow_(arrow)
{
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();
    gfx::ImageSkia *skia = rb.GetImageSkiaNamed(IDR_LIVEHIME_TIP);
    gfx::ImageSkia *skia_hv = rb.GetImageSkiaNamed(IDR_LIVEHIME_TIP_HV);
    SetImage(views::Button::STATE_NORMAL, skia);
    SetImage(views::Button::STATE_HOVERED, skia_hv);
    SetImage(views::Button::STATE_PRESSED, skia_hv);
    SetImage(views::Button::STATE_DISABLED, skia);
    SetImageAlignment(views::ImageButton::ALIGN_CENTER, views::ImageButton::ALIGN_MIDDLE);
}

LivehimeHoverTipButton::~LivehimeHoverTipButton()
{
}

void LivehimeHoverTipButton::SetTooltipText(const base::string16 &title, const base::string16 &tips)
{
    title_ = title;
    tips_ = tips;
}

void LivehimeHoverTipButton::SetButtonImage(const gfx::ImageSkia* nor, const gfx::ImageSkia* hv)
{
    SetImage(views::Button::STATE_NORMAL, nor);
    SetImage(views::Button::STATE_HOVERED, hv);
    SetImage(views::Button::STATE_PRESSED, hv);
    SetImage(views::Button::STATE_DISABLED, nor);
}

void LivehimeHoverTipButton::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (!details.is_add)
        {
            LivehimeTipsBubble::Close(this);
        }
    }
}

void LivehimeHoverTipButton::StateChanged()
{
    if (state() == Button::STATE_HOVERED || state() == Button::STATE_PRESSED)
    {
        LivehimeTipsBubble::Show(this, title_, tips_, arrow_);
    }
    else
    {
        LivehimeTipsBubble::Close(this);
    }
};
