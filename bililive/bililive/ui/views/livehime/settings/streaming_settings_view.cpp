#include "streaming_settings_view.h"

#include "base/prefs/pref_service.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/livehime/settings/settings_presenter_impl.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_combobox.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_textfield.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/login/bilibili_login_control.h"
#include "bililive/public/bililive/bililive_process.h"

#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"

void StreamingSettingsView::SaveOrCheckStreamingSettingsChange(bool check, ChangeType &result) {
    
}

void StreamingSettingsView::InitViews()
{
    ++radio_button_groupid_;
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    auto column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);
    column_set->AddColumn(
        views::GridLayout::FILL, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);
    
    layout->StartRow(0, 0);
    InitCdnViews();
    layout->AddView(cdn_views_);
}

void StreamingSettingsView::InitData()
{
    cdn_combobox_->ClearItems();
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    cdn_views_->SetVisible(true);
    Layout();
}

void StreamingSettingsView::InitCdnViews() {
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    cdn_views_ = new HideAwareView();
    views::GridLayout *layout = new views::GridLayout(cdn_views_);
    cdn_views_->SetLayoutManager(layout);

    auto column_set = layout->AddColumnSet(0);
    column_set->AddColumn(
        views::GridLayout::LEADING, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);

    column_set = layout->AddColumnSet(1);
    column_set->AddColumn(
        views::GridLayout::FILL, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);

    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightEnds);
    BililiveLabel *label = new LivehimeTitleLabel(
        rb.GetLocalizedString(IDS_CONFIG_SWITCH_CDN));
    layout->AddView(label);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    cdn_combobox_ = new LivehimeCombobox();
    layout->AddView(cdn_combobox_);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForCtrlTips);
    label = new LivehimeTipLabel(rb.GetLocalizedString(IDS_CONFIG_SWITCH_CDN_TIP));
    label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    label->SetMultiLine(true);
    layout->AddView(label);

    cdn_views_->SetVisible(true);
}
