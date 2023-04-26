#include "bililive/bililive/ui/views/livehime/settings/audio_settings_view.h"

#include "base/prefs/pref_service.h"

#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/settings/settings_presenter_impl.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_combobox.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/public/bililive/bililive_process.h"

#include "obs/obs_proxy/public/common/pref_names.h"
#include "obs/obs_proxy/public/proxy/obs_audio_devices.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"
#include "base/strings/utf_string_conversions.h"


AudioSettingsView::AudioSettingsView()
    : BaseSettingsView(),
    mic_combobox_(nullptr),
    sysvol_combobox_(nullptr),
    corerate_combobox_(nullptr)
{
    presenter_ = std::make_unique<SettingsPresenterImpl>(this);
}

void AudioSettingsView::SaveOrCheckStreamingSettingsChange(bool check, ChangeType &result)
{
    std::map<std::string, std::string> device_string;
    std::map<std::string, int> device_integer;

    // 麦克风设备
    int index = mic_combobox_->selected_index();
    if (-1 != index) {
        std::string str = mic_combobox_->GetItemData<std::string>(index);
        if (!str.empty()) {
            device_string.insert(std::make_pair(prefs::kAudioMicDeviceInUse, str));
        }
    }


    // 扬声器设备
    index = sysvol_combobox_->selected_index();
    if (-1 != index) {
        std::string str = sysvol_combobox_->GetItemData<std::string>(index);
        if (!str.empty()) {
            device_string.insert(std::make_pair(prefs::kAudioSpeakerDeviceInUse, str));
        }
    }

    // 码率
    device_integer.insert(std::make_pair(prefs::kAudioBitRate,
        corerate_combobox_->GetItemData<int>(corerate_combobox_->selected_index())));

    // 要修改这里时，注意一下 device_string 和 device_integer 对应的 ChangeType 值
    presenter_->AudioSaveChange(device_string, device_integer, check, result);
}

void AudioSettingsView::InitViews() {
    InitCmbModel();
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    views::GridLayout *layout = new views::GridLayout(this);
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

    // 麦克风
    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightEnds);
    BililiveLabel *label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_CONFIG_AUDIO_MIC));
    layout->AddView(label);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    layout->AddView(mic_combobox_);


    // 系统声音
    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForDiffGroups);
    label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_CONFIG_AUDIO_SYSVOL));
    layout->AddView(label);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    layout->AddView(sysvol_combobox_);

    // 音频码率
    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForDiffGroups);
    label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_CONFIG_AUDIO_CORERATE));
    layout->AddView(label);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    layout->AddView(corerate_combobox_);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForCtrlTips);
    label = new LivehimeTipLabel(rb.GetLocalizedString(IDS_CONFIG_AUDIO_CORERATE_TIP));
    label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    layout->AddView(label);

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
}

void AudioSettingsView::InitCmbModel() {
    int index = 0;
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    mic_combobox_ = new LivehimeCombobox();
    mic_combobox_->SetDefaultItem(rb.GetLocalizedString(IDS_COMMON_NO_CAMERA), std::string());
    presenter_->InitComboData(obs_proxy::kDefaultInputAudio);

    sysvol_combobox_ = new LivehimeCombobox();
    sysvol_combobox_->SetDefaultItem(rb.GetLocalizedString(IDS_COMMON_NO_SYS_VOICE), std::string());
    presenter_->InitComboData(obs_proxy::kDefaultOutputAudio);

    auto media_prefs = presenter_->GetMediaPrefsInfo();

    corerate_combobox_ = new LivehimeCombobox();
    for (auto& iter : media_prefs.audio_settings.audio_bitrate_list)
    {
        int i = corerate_combobox_->AddItem(base::UTF8ToUTF16(iter.second), iter.first);
        if (iter.first == media_prefs.audio_settings.audio_bitrate_default)
        {
            index = i;
        }
    }
    corerate_combobox_->SetSelectedIndex(index);
}

void AudioSettingsView::OnInitComboData(
    const std::string& device,
    const std::wstring& devname,
    const std::string& devid)
{
    LivehimeCombobox *combo;
    if (device == obs_proxy::kDefaultInputAudio) {
        combo = mic_combobox_;
    } else {
        combo = sysvol_combobox_;
    }

    if (wcsicmp(devname.c_str(), L"default") == 0) {
        int index = combo->AddItem(
            ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_CONFIG_AUDIO_DEFAULTDEVICE),
            devid);
        combo->SetSelectedIndex(index);
    } else {
        combo->AddItem(devname, devid);
    }
}


void AudioSettingsView::InitData()
{
    int index = mic_combobox_->FindItemData(presenter_->GetCurMicId());
    if (-1 != index)
    {
        mic_combobox_->SetSelectedIndex(index);
    }

    index = sysvol_combobox_->FindItemData(presenter_->GetCurSpeakerId());
    if (-1 != index)
    {
        sysvol_combobox_->SetSelectedIndex(index);
    }

    PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();
    int nIndex = corerate_combobox_->FindItemData(pref->GetInteger(prefs::kAudioBitRate));
    corerate_combobox_->SetSelectedIndex(nIndex != -1 ? nIndex : 0);
}