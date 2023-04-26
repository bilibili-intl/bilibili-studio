#include "language_settings_view.h"

#include <shellapi.h>

#include "base/file_util.h"
#include "base/prefs/pref_service.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"

#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/settings/settings_presenter_impl.h"
#include "bililive/bililive/ui/dialog/shell_dialogs.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_combobox.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/bililive_paths_internal.h"
#include "bililive/secret/public/event_tracking_service.h"

#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"
#include <bililive/public/common/pref_constants.h>
#include <bililive/common/bililive_logging.h>
#include <bililive/public/common/pref_names.h>


namespace {

    enum {
        kBrowserButtonId = 1000,
        kCachePathButtonId,
        kOpenDirButtonId,
        kOpenCacheButtonId,
    };

    struct RecordingFormatTableItem {
        bool is_default;
        int index;
        std::wstring name;
        std::string item_data;
        int tip_resid;
    };

    struct LanguageItem {
        int index;
        bool is_default;
        std::wstring name;
        const char* language_code;
    };

    std::vector<LanguageItem> language_table{
        {0, true, L"English", prefs::kLocaleEnUS},
        {1, false, L"简体中文", prefs::kLocaleZhCN},
        {2, false, L"ภาษาไทย", prefs::kLocaleThTH},
        {3, false, L"Bahasa Indonesia", prefs::kLocaleIdID},
    };

}


LanguageSettingsView::LanguageSettingsView() {
    presenter_ = std::make_unique<SettingsPresenterImpl>(this);
}

void LanguageSettingsView::InitViews() {
    LOG(INFO) << "LanguageSettingsView::InitViews";
    InitCmbModel();

    ResourceBundle& rb = ResourceBundle::GetSharedInstance();
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    auto column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);
    column_set->AddColumn(
        views::GridLayout::LEADING, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);

    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);
    column_set->AddColumn(
        views::GridLayout::FILL, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);

    column_set = layout->AddColumnSet(2);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);
    column_set->AddColumn(
        views::GridLayout::LEADING, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddColumn(
        views::GridLayout::LEADING, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);

    column_set = layout->AddColumnSet(3);
    column_set->AddColumn(
        views::GridLayout::FILL, views::GridLayout::FILL,
        1, views::GridLayout::FIXED, 0, 0);

    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightEnds);
    // 画质设置
    BililiveLabel* label = new LivehimeTitleLabel(GetLocalizedString(IDS_CONFIG_LANGUAGE_TITLE));
    layout->AddView(label);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    layout->AddView(recording_format_combobox_);
    // ComboBox通知
    OnSelectedIndexChanged(recording_format_combobox_);


}

void LanguageSettingsView::InitData() {
    LOG(INFO) << "LanguageSettingsView::InitData";
    language_code_ = GetBililiveProcess()->global_profile()->GetPrefs()->GetString(prefs::kLivehimeAppLocale);
    int found_index = 0;
    language_code_ = language_code_.empty() ? language_table[0].language_code : language_code_;
    auto ch = language_code_.c_str();
    for (auto lang : language_table)
    {
        if (strcmp(lang.language_code, ch) == 0)
        {
            found_index = lang.index;
            break;
        }
    }
    this->recording_format_combobox_->SetSelectedIndex(found_index);

}

bool LanguageSettingsView::CheckSettingsValid() {
    return true;
}

bool LanguageSettingsView::SaveNormalSettingsChange() {
    is_language_changed_ = false;
    auto index = recording_format_combobox_->selected_index();
    auto newCode = language_table[index].language_code;
    if (strcmp(newCode, language_code_.c_str()) == 0)  {
        return true;
    }
    PrefService* pref = GetBililiveProcess()->global_profile()->GetPrefs();
    pref->SetString(prefs::kLivehimeAppLocale, newCode);
    is_language_changed_ = true;
    return true;
}

void LanguageSettingsView::OnSelectedIndexChanged(BililiveComboboxEx* combobox) {
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();
    if (combobox == recording_format_combobox_)
    {
        int rec_fmt_index = combobox->selected_index();
        LOG(INFO) << "combo index: " << rec_fmt_index;
    }
}

void LanguageSettingsView::ButtonPressed(views::Button* sender, const ui::Event& event) {
}

void LanguageSettingsView::InitCmbModel() {
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();

    recording_format_combobox_ = new LivehimeCombobox();
    recording_format_combobox_->set_listener(this);
    for (auto item : language_table)
    {
        recording_format_combobox_->AddItem(item.name, std::string());
        if (item.is_default)
            recording_format_combobox_->SetSelectedIndex(item.index);
    }
}