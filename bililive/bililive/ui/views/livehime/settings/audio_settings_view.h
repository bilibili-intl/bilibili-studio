#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_AUDIO_SETTINGS_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_AUDIO_SETTINGS_VIEW_H_

#include "bililive/bililive/livehime/settings/settings_contract.h"
#include "bililive/bililive/ui/views/livehime/settings/base_settings_view.h"


class DenoiseSlider;
class LivehimeCombobox;

// “Ù∆µ…Ë÷√“≥
class AudioSettingsView :
    public BaseSettingsView,
    public contracts::AudioSettingsView
{
public:
    AudioSettingsView();

    void OnInitComboData(
        const std::string& device,
        const std::wstring& devname,
        const std::string& devid) override;

protected:
    // BaseSettingsView
    void InitViews() override;
    void InitData() override;
    bool CheckSettingsValid() override { return true; }
    bool SaveNormalSettingsChange() override { return true; }
    void SaveOrCheckStreamingSettingsChange(bool check, ChangeType &result) override;

private:
    void InitCmbModel();

private:
    LivehimeCombobox* mic_combobox_;
    LivehimeCombobox* sysvol_combobox_;
    LivehimeCombobox* corerate_combobox_;

    std::unique_ptr<contracts::SettingsPresenter> presenter_;

    DISALLOW_COPY_AND_ASSIGN(AudioSettingsView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_AUDIO_SETTINGS_VIEW_H_