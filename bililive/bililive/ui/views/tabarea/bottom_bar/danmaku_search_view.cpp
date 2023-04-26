#include "bililive/bililive/ui/views/tabarea/bottom_bar/danmaku_search_view.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/tabarea/bottom_bar/danmaku_search_notification_details.h"
#include "bililive/public/bililive/bililive_notification_types.h"

#include "base/notification/notification_service.h"
#include "base/strings/utf_string_conversions.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"



DanmakuSearchView::DanmakuSearchView() {
}

DanmakuSearchView::~DanmakuSearchView() {
}

void DanmakuSearchView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) {
    if (details.child == this) {
        if (details.is_add) {
            InitView();
        }
    }
}

void DanmakuSearchView::InitView() {
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    search_btn_ = new views::ImageButton(this);
    search_btn_->SetImage(
        views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEHIME_SEARCH));
    search_btn_->SetImage(
        views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_LIVEHIME_SEARCH_HV));

    search_edit_ = new LivehimeTextfield(search_btn_);
    search_edit_->set_placeholder_text(rb.GetLocalizedString(IDS_DANMUKU_SEARCH_HOLDER_TEXT));

    search_label_ = new LivehimeContentLabel(
        rb.GetLocalizedString(IDS_DANMUKU_SEARCH_TIME_RANGE));

    search_combobox_ = new LivehimeCombobox();
    int index = search_combobox_->AddItem(rb.GetLocalizedString(IDS_DANMUKU_SEARCH_TIME_RANGE_ALL),
        (int)TimeRange::ALL);
    search_combobox_->AddItem(rb.GetLocalizedString(IDS_DANMUKU_SEARCH_TIME_RANGE_WEEK),
        (int)TimeRange::WEEK);
    search_combobox_->AddItem(rb.GetLocalizedString(IDS_DANMUKU_SEARCH_TIME_RANGE_MONTH),
        (int)TimeRange::MONTH);
    search_combobox_->AddItem(rb.GetLocalizedString(IDS_DANMUKU_SEARCH_TIME_RANGE_QUARTER),
        (int)TimeRange::THREE_MONTH);
    search_combobox_->set_default_width_in_chars(8, rb.GetLocalizedString(IDS_PADDING_CHAR));
    search_combobox_->SetSelectedIndex(index);

    views::ColumnSet *columnset = layout->AddColumnSet(0);
    columnset->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    columnset->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    columnset = layout->AddColumnSet(1);
    columnset->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    columnset->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    columnset->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    layout->StartRow(0, 0);
    layout->AddView(search_edit_);

    layout->AddPaddingRow(0, kPaddingRowHeightForDiffGroups);
    layout->StartRow(0, 1);
    layout->AddView(search_label_);
    layout->AddView(search_combobox_);

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
}

void DanmakuSearchView::ButtonPressed(views::Button* sender, const ui::Event& event) {
    if (sender == search_btn_) {
        Search();
    }
}

bool DanmakuSearchView::OnKeyPressed(const ui::KeyEvent& event) {
    if (event.key_code() == ui::KeyboardCode::VKEY_RETURN) {
        Search();
        return true;
    }

    return false;
}

void DanmakuSearchView::Search() {
    DanmakuSearchDetails details;

    details.time_range = static_cast<TimeRange>(search_combobox_->GetItemData<int>(search_combobox_->selected_index()));
    details.content = base::UTF16ToUTF8(search_edit_->text_field()->text());

    base::NotificationService::current()->Notify(
        bililive::NOTIFICATION_LIVEHIME_TABAREA_SEARCH,
        base::NotificationService::AllSources(),
        base::Details<DanmakuSearchDetails>(&details));
}