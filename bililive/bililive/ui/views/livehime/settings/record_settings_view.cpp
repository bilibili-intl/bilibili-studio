#include "record_settings_view.h"

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
        int item_text_resid;
        std::string item_data;
        int tip_resid;
    };

    std::vector<RecordingFormatTableItem> recording_format_table{
        {false, 0, IDS_CONFIG_VIDEO_CMB_RECORDING_FORMAT_FLV, prefs::kRecordingFormatFLV, IDS_CONFIG_VIDEO_RECORDING_FMT_FLV_TIP},
        {true, 1, IDS_CONFIG_VIDEO_CMB_RECORDING_FORMAT_FMP4, prefs::kRecordingFormatFMP4, IDS_CONFIG_VIDEO_RECORDING_FMT_FMP4_TIP},
        {false, 2, IDS_CONFIG_VIDEO_CMB_RECORDING_FORMAT_MP4, prefs::kRecordingFormatMP4, IDS_CONFIG_VIDEO_RECORDING_FMT_MP4_TIP}
    };

}


RecordSettingsView::RecordSettingsView() {
    presenter_ = std::make_unique<SettingsPresenterImpl>(this);
}

void RecordSettingsView::InitViews() {
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
        views::GridLayout::LEADING, views::GridLayout::CENTER,
        0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingRowHeightForCtrlTips);
    column_set->AddColumn(
        views::GridLayout::LEADING, views::GridLayout::BASELINE,
        0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);

    // 录像保存格式
    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForDiffGroups);
    BililiveLabel* label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_CONFIG_VIDEO_RECORDING_FORMAT));
    layout->AddView(label);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    layout->AddView(recording_format_combobox_);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForCtrlTips);
    label = recording_format_tip_label_ = new LivehimeTipLabel(L"");
    recording_format_tip_label_->SetMultiLine(true);
    label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    layout->AddView(label);

    // 录像保存路径
    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForDiffGroups);
    label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_CONFIG_VIDEO_SAVEPATH));
    layout->AddView(label);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    views::View* cmbandbrowser = new views::View();
    {
        views::GridLayout* path_layout = new views::GridLayout(cmbandbrowser);
        cmbandbrowser->SetLayoutManager(path_layout);

        views::ColumnSet* path_column_set = path_layout->AddColumnSet(0);
        path_column_set->AddColumn(
            views::GridLayout::Alignment::FILL, views::GridLayout::Alignment::FILL,
            1.0f, views::GridLayout::SizeType::USE_PREF, 0, 0);
        path_column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        path_column_set->AddColumn(
            views::GridLayout::Alignment::FILL, views::GridLayout::Alignment::FILL,
            0, views::GridLayout::SizeType::USE_PREF, 0, 0);

        LivehimeLinkButton* opendir_btn = new LivehimeLinkButton(this, rb.GetLocalizedString(IDS_CONFIG_VIDEO_OPENDIR));
        opendir_btn->set_id(kOpenDirButtonId);
        folder_textfield_ = new LivehimeNativeEditView(opendir_btn, BilibiliNativeEditView::BD_RIGHT);

        LivehimeFunctionLabelButton* labelbtn = new LivehimeFunctionLabelButton(this, rb.GetLocalizedString(IDS_CONFIG_UPDATE_PATH));
        labelbtn->set_id(kBrowserButtonId);

        path_layout->StartRow(0, 0);
        path_layout->AddView(folder_textfield_);
        path_layout->AddView(labelbtn);

        folder_textfield_->SetReadOnly(true);
    }
    layout->AddView(cmbandbrowser);

    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
    record_sync_checkbox_ = new LivehimeCheckbox(rb.GetLocalizedString(IDS_CONFIG_VIDEO_SYNCRECORDING));
    layout->AddView(record_sync_checkbox_);

    layout->StartRowWithPadding(0, 2, 0, kPaddingRowHeightForDiffGroups);
    BililiveLabel* path_label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_CONFIG_OTHER_PATH));
    layout->AddView(path_label);
    path_label = new LivehimeSmallContentLabel(rb.GetLocalizedString(IDS_CONFIG_OTHER_PATH_TIP));
    layout->AddView(path_label);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    views::View* pathbrowser = new views::View();
    {
        views::GridLayout* path_layout = new views::GridLayout(pathbrowser);
        pathbrowser->SetLayoutManager(path_layout);

        views::ColumnSet* path_column_set = path_layout->AddColumnSet(0);
        path_column_set->AddColumn(
            views::GridLayout::Alignment::FILL, views::GridLayout::Alignment::FILL,
            1.0f, views::GridLayout::SizeType::USE_PREF, 0, 0);
        path_column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
        path_column_set->AddColumn(
            views::GridLayout::Alignment::FILL, views::GridLayout::Alignment::FILL,
            0, views::GridLayout::SizeType::USE_PREF, 0, 0);

        LivehimeLinkButton* opendir_btn = new LivehimeLinkButton(this, rb.GetLocalizedString(IDS_CONFIG_VIDEO_OPENDIR));
        opendir_btn->set_id(kOpenCacheButtonId);
        unity_folder_textfield_ = new LivehimeNativeEditView(opendir_btn, BilibiliNativeEditView::BD_RIGHT);

        LivehimeFunctionLabelButton* labelbtn = new LivehimeFunctionLabelButton(this, rb.GetLocalizedString(IDS_CONFIG_UPDATE_PATH));
        labelbtn->set_id(kCachePathButtonId);

        path_layout->StartRow(0, 0);
        path_layout->AddView(unity_folder_textfield_);
        path_layout->AddView(labelbtn);

        unity_folder_textfield_->SetReadOnly(true);
    }
    layout->AddView(pathbrowser);

    // ComboBox通知
    OnSelectedIndexChanged(recording_format_combobox_);
}

void RecordSettingsView::InitData() {
    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();

    // 录像格式
    auto format = pref->GetString(prefs::kOutputRecordingFormat);
    auto nIndex = recording_format_combobox_->FindItemData(format);
    if (-1 != nIndex) {
        recording_format_combobox_->SetSelectedIndex(nIndex);
        OnSelectedIndexChanged(recording_format_combobox_);
    }

    // 录像路径
    auto szACfg = pref->GetString(prefs::kOutputFolder);
    folder_textfield_->SetText(base::UTF8ToUTF16(szACfg));
    //folder_textfield_->text_field()->SelectRange(ui::Range(0, 0));

    // 同步录制
    record_sync_checkbox_->SetChecked(pref->GetBoolean(prefs::kOutputRecordingSync));

    bool path_exist = false;
    auto cache_path = pref->GetString(prefs::kLivehimeCachePath);
    if (!cache_path.empty()) {
        base::ThreadRestrictions::ScopedAllowIO scoped_allow;
        if (base::PathExists(base::FilePath::FromUTF8Unsafe(cache_path))) {
            other_cache_path_ = base::UTF8ToUTF16(cache_path);
            path_exist = true;
        }
    }

    if (!path_exist) {
        base::FilePath path;
        bililive::GetDefaultUserDataDirectory(&path);
        other_cache_path_ = path.AsUTF16Unsafe();
    }

    unity_folder_textfield_->SetText(other_cache_path_);
}

bool RecordSettingsView::CheckSettingsValid() {
    return true;
}

bool RecordSettingsView::SaveNormalSettingsChange() {
    return true;
}

void RecordSettingsView::SaveOrCheckStreamingSettingsChange(bool check, ChangeType& result) {
    std::map<std::string, std::string> device_string;
    std::map<std::string, int> device_integer;
    std::map<std::string, bool> device_boolean;

    auto format = recording_format_combobox_->GetItemData<std::string>(
        recording_format_combobox_->selected_index());
    device_string.insert(std::make_pair(prefs::kOutputRecordingFormat, format));

    auto szCfg = folder_textfield_->text();
    device_string.insert(std::make_pair(prefs::kOutputFolder, base::UTF16ToUTF8(szCfg)));

    device_boolean.insert(std::make_pair(prefs::kOutputRecordingSync,
        record_sync_checkbox_->checked()));

    presenter_->VideoSaveChange(device_string, device_integer, device_boolean, check, result);

    auto unity_path = unity_folder_textfield_->text();
    if (other_cache_path_ != unity_path) {
        other_cache_path_ = unity_path;
        PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
        pref->SetString(prefs::kLivehimeCachePath, base::UTF16ToUTF8(unity_path));

        livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::CachePathSetClick, "");
    }
}

void RecordSettingsView::OnSelectedIndexChanged(BililiveComboboxEx* combobox) {
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();
    if (combobox == recording_format_combobox_)
    {
        int rec_fmt_index = combobox->selected_index();
        if (rec_fmt_index >= 0 && static_cast<size_t>(rec_fmt_index) < recording_format_table.size())
        {
            auto tip_text = rb.GetLocalizedString(recording_format_table[rec_fmt_index].tip_resid);
            recording_format_tip_label_->SetText(tip_text);
        }
    }
}

void RecordSettingsView::ButtonPressed(views::Button* sender, const ui::Event& event) {
    int id = sender->id();
    switch (id)
    {
    case kOpenDirButtonId:
    case kOpenCacheButtonId:
    {
        string16 path;
        if (id == kOpenDirButtonId) {
            path = folder_textfield_->text();
        }
        else {
            path = unity_folder_textfield_->text();
        }

        base::ThreadRestrictions::ScopedAllowIO allow;
        if (!base::DirectoryExists(base::FilePath(path)))
        {
            ResourceBundle& rb = ResourceBundle::GetSharedInstance();
            livehime::ShowMessageBox(
                GetWidget()->GetNativeView(),
                rb.GetLocalizedString(IDS_TIP_DLG_TIP_INFO),
                rb.GetLocalizedString(IDS_MESSAGEBOX_CONTENTS_INVALIDFOLDER),
                rb.GetLocalizedString(IDS_SURE));
        }
        else
        {
            ::ShellExecuteW(nullptr, L"open", path.c_str(), nullptr, nullptr, SW_SHOW);
        }
        break;
    }

    case kBrowserButtonId:
    {
        bililive::ISelectFileDialog* dlg = bililive::CreateSelectFileDialog(sender->GetWidget());
        dlg->SetDefaultPath(folder_textfield_->text());
        bool selected = dlg->DoModel(ui::SelectFileDialog::Type::SELECT_FOLDER);
        if (selected)
        {
            std::wstring wfolder = dlg->GetSelectedFileName();
            folder_textfield_->SetText(wfolder);
            folder_textfield_->SelectRange(MAKELONG(0, 0));
        }
        delete dlg;
        break;
    }

    case kCachePathButtonId:
    {
        bililive::ISelectFileDialog* dlg = bililive::CreateSelectFileDialog(sender->GetWidget());
        dlg->SetDefaultPath(unity_folder_textfield_->text());
        bool selected = dlg->DoModel(ui::SelectFileDialog::Type::SELECT_FOLDER);
        if (selected)
        {
            std::wstring wfolder = dlg->GetSelectedFileName();
            unity_folder_textfield_->SetText(wfolder);
            unity_folder_textfield_->SelectRange(MAKELONG(0, 0));
        }
        delete dlg;
        break;
    }
    default:
        break;
    }
}

void RecordSettingsView::InitCmbModel() {
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();

    recording_format_combobox_ = new LivehimeCombobox();
    recording_format_combobox_->set_listener(this);
    for (auto x : recording_format_table)
    {
        recording_format_combobox_->AddItem(rb.GetLocalizedString(x.item_text_resid), x.item_data);
        if (x.is_default)
            recording_format_combobox_->SetSelectedIndex(x.index);
    }
}