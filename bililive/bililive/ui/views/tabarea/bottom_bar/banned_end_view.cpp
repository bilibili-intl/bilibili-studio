#include "bililive/bililive/ui/views/tabarea/bottom_bar/banned_end_view.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"



BannedEndView::BannedEndView(BannedEndDelegate *delegate)
    : delegate_(delegate) {
    DCHECK(delegate_ != nullptr);
}

BannedEndView::~BannedEndView() {
}

void BannedEndView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) {
    if (details.child == this) {
        if (details.is_add) {
            InitView();
        }
    }
}

void BannedEndView::InitView() {
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    switch_icon_ = new BililiveImageButton(this);
    switch_icon_->SetAllStateImage(rb.GetImageSkiaNamed(IDR_LIVEMAIN_TOOLBAR_SWITCH_ON));

    banned_label_ = new LivehimeContentLabel(L"");
    banned_label_->SetTextColor(clrLivehime);

    banned_time_label_ = new LivehimeContentLabel(rb.GetLocalizedString(IDS_DANMUKU_BANNED_TIME));

    banned_tips_ = new LivehimeTipLabel(rb.GetLocalizedString(IDS_DANMUKU_BANNED_END_TIPS));

    //banned_combobox_->GetItemData<int>(banned_combobox_->selected_index());

    views::ColumnSet *columnset = layout->AddColumnSet(0);
    columnset->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    columnset->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    columnset->AddPaddingColumn(1.0f, 0);

    columnset = layout->AddColumnSet(1);
    columnset->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    columnset->AddPaddingColumn(1.0f, 0);

    layout->AddPaddingRow(1.0f, 0);
    layout->StartRow(0, 0);
    layout->AddView(switch_icon_);
    layout->AddView(banned_label_);

    layout->AddPaddingRow(0, kPaddingRowHeightForDiffGroups);
    layout->StartRow(0, 1);
    layout->AddView(banned_tips_);

    layout->AddPaddingRow(1.0f, 0);
    layout->AddPaddingRow(0, kPaddingRowHeightEnds);

}

void BannedEndView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (switch_icon_ == sender)
        delegate_->BannedOff();
}

void BannedEndView::set_label_text(const string16& text) {
    banned_label_->SetText(text);
}