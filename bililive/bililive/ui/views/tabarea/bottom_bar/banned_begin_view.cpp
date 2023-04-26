#include "bililive/bililive/ui/views/tabarea/bottom_bar/banned_begin_view.h"

#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/tabarea/tabarea_toast_view.h"

#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"



BannedBeginView::BannedBeginView(BannedBeginDelegate *delegate)
    : delegate_(delegate) {
    DCHECK(delegate_ != nullptr);
}

BannedBeginView::~BannedBeginView() {
}

void BannedBeginView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) {
    if (details.child == this) {
        if (details.is_add) {
            InitView();
        }
    }
}

void BannedBeginView::InitView() {
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    switch_icon_ = new BililiveImageButton(this);
    switch_icon_->SetAllStateImage(rb.GetImageSkiaNamed(IDR_LIVEMAIN_TOOLBAR_SWITCH_OFF));

    banned_label_ = new LivehimeContentLabel(rb.GetLocalizedString(IDS_DANMUKU_BANNED_NO_BANNED));
    banned_label_->SetHorizontalAlignment(gfx::ALIGN_RIGHT);

    banned_time_label_ = new LivehimeContentLabel(rb.GetLocalizedString(IDS_DANMUKU_BANNED_TIME));
    banned_time_label_->SetHorizontalAlignment(gfx::ALIGN_RIGHT);

    banned_type_label_ = new LivehimeContentLabel(L"范围");
    banned_type_label_->SetHorizontalAlignment(gfx::ALIGN_RIGHT);

    banned_level_ = new LivehimeContentLabel(L"屏蔽等级");

    banned_type_combobox_ = new LivehimeCombobox();
    banned_type_combobox_->set_listener(this);
    int index = banned_type_combobox_->AddItem(L"全员", (int)BannedType::TYPE_ALL);
    banned_type_combobox_->AddItem(L"用户等级", (int)BannedType::TYPE_LEVEL);
    banned_type_combobox_->set_default_width_in_chars(8, rb.GetLocalizedString(IDS_PADDING_CHAR));
    banned_type_combobox_->SetSelectedIndex(index);

    slider_view_ = new views::View();
    auto slider_view_layout = new views::GridLayout(slider_view_);
    slider_view_->SetLayoutManager(slider_view_layout);

    auto cs = slider_view_layout->AddColumnSet(0);
    cs->AddColumn(
        views::GridLayout::FILL, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    cs->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    cs->AddColumn(
        views::GridLayout::FILL, views::GridLayout::FILL,
        0, views::GridLayout::USE_PREF, 0, 0);

    slider_view_layout->StartRow(0, 0);

    level_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
    level_slider_->set_id(0);
    level_slider_->SetKeyboardIncrement(0.017f);
    level_slider_->SetMouseWheelIncrement(0.017f);
    slider_view_layout->AddView(level_slider_);

    auto label = new LivehimeContentLabel(L"1");
    label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    label->SetPreferredSize(gfx::Size(
        LivehimePaddingCharWidth(LivehimeContentLabel::GetFont()) * 2, 1));
    level_slider_->SetAssociationLabel(label);
    slider_view_layout->AddView(label);

    level_slider_->SetEnabled(false);

    banned_combobox_ = new LivehimeCombobox();
    index = banned_combobox_->AddItem(L"3分钟", 3);
    banned_combobox_->AddItem(L"10分钟", 10);
    banned_combobox_->AddItem(L"30分钟", 30);
    banned_combobox_->AddItem(L"本场直播", 0);
    banned_combobox_->set_default_width_in_chars(8, rb.GetLocalizedString(IDS_PADDING_CHAR));
    banned_combobox_->SetSelectedIndex(index);

    banned_tips_ = new LivehimeTipLabel(rb.GetLocalizedString(IDS_DANMUKU_BANNED_BEGIN_TIPS));

    views::ColumnSet *columnset = layout->AddColumnSet(0);
    columnset->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    columnset->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    columnset->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    columnset->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    columnset = layout->AddColumnSet(1);
    columnset->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    columnset->AddPaddingColumn(1.0f, 0);

    layout->StartRow(0, 0);
    layout->AddView(banned_label_);
    layout->AddView(switch_icon_);

    layout->AddPaddingRow(0, kPaddingRowHeightForDiffGroups);
    layout->StartRow(0, 0);
    layout->AddView(banned_type_label_);
    layout->AddView(banned_type_combobox_);

    layout->AddPaddingRow(0, kPaddingRowHeightForDiffGroups);
    layout->StartRow(0, 0);
    layout->AddView(banned_level_);
    layout->AddView(slider_view_);

    layout->AddPaddingRow(0, kPaddingRowHeightForDiffGroups);
    layout->StartRow(0, 0);
    layout->AddView(banned_time_label_);
    layout->AddView(banned_combobox_);

    layout->AddPaddingRow(0, kPaddingRowHeightForDiffGroups);
    layout->StartRow(0, 1);
    layout->AddView(banned_tips_);

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
}

void BannedBeginView::ButtonPressed(views::Button* sender, const ui::Event& event) {
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    if (switch_icon_ == sender) {
        bool streaming = !LivehimeLiveRoomController::GetInstance()->IsLiveReady();

        if (!streaming) {
            ShowTabAreaToast(TabAreaToastType_Warning,
                rb.GetLocalizedString(IDS_DANMUKU_BANNED_TOAST_UNOPENED));
        } else {
            std::string banned_type;
            int level;

            auto type =
                banned_type_combobox_->GetItemData<int>(banned_type_combobox_->selected_index());

            if (type == BannedType::TYPE_ALL) {
                banned_type = "member";
                level = kDefaultLevel;
            } else {
                banned_type = "level";
                level = std::atoi(base::UTF16ToUTF8(level_slider_->GetAssociationLabel()->text()).c_str());
            }

            delegate_->BannedOn(
                banned_combobox_->GetItemData<int>(banned_combobox_->selected_index()),
                banned_type,
                level
                );
        }
    }
}

void BannedBeginView::ResetCombobox() {
    if (banned_combobox_) banned_combobox_->SetSelectedIndex(0);
}

void BannedBeginView::SliderValueChanged(
    views::Slider* sender, float value, float old_value, views::SliderChangeReason reason) {
    if (value < 0.017f) {
        sender->SetValue(0.017f);
        return;
    }

    int pos = sender->value() * 60;
    level_slider_->GetAssociationLabel()->SetText(base::StringPrintf(L"%d", pos));
}

void BannedBeginView::OnSelectedIndexChanged(BililiveComboboxEx* combobox) {
    if (combobox != banned_type_combobox_) {
        return;
    }

    auto type = banned_type_combobox_->GetItemData<int>(banned_type_combobox_->selected_index());

    if (type == BannedType::TYPE_ALL) {
        level_slider_->SetEnabled(false);
    } else {
        level_slider_->SetEnabled(true);
    }
}