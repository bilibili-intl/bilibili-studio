#include "livehime_inside_frame_titlebar.h"
#include "livehime_label.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "ui/views/layout/grid_layout.h"
#include "ui/views/widget/widget.h"
#include "ui/views/controls/button/image_button.h"

#include "ui/base/resource/resource_bundle.h"

#include "grit/theme_resources.h"
#include "grit/generated_resources.h"

LivehimeInsideFrameTitleBar::LivehimeInsideFrameTitleBar(const base::string16 &title)
{
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet *column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);

    caption_label_ = new LivehimeTitleLabel(title);

    views::ImageButton *button = new views::ImageButton(this);
    button->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEHIME_TABAREA_POPUP_CLOSE));
    button->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_LIVEHIME_TABAREA_POPUP_CLOSE_HV));
    button->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_LIVEHIME_TABAREA_POPUP_CLOSE_HV));
    button->SetTooltipText(rb.GetLocalizedString(IDS_UGC_CLOSE));

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(caption_label_);
    layout->AddView(button);
    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
}

void LivehimeInsideFrameTitleBar::ViewHierarchyChanged(const ViewHierarchyChangedDetails &details)
{
    if (details.is_add && details.child == this)
    {
        InitViews();
    }
}

void LivehimeInsideFrameTitleBar::InitViews()
{
    OnThemeChanged();
}

void LivehimeInsideFrameTitleBar::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    GetWidget()->Close();
}

void LivehimeInsideFrameTitleBar::OnThemeChanged()
{
    caption_label_->SetTextColor(GetColor(WindowChildTitleText));
}
