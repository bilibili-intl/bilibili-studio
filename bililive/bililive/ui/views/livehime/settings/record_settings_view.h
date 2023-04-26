#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_RECORD_SETTINGS_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_RECORD_SETTINGS_VIEW_H_

#include "bililive/bililive/livehime/settings/settings_contract.h"
#include "bililive/bililive/ui/views/controls/combox/bililive_combobox_ex.h"
#include "bililive/bililive/ui/views/livehime/settings/base_settings_view.h"


class LivehimeCombobox;
class LivehimeCheckbox;
class LivehimeTextfield;
class LivehimeTipLabel;

class RecordSettingsView :
    public BaseSettingsView,
    public BililiveComboboxExListener,
    public views::ButtonListener,
    public contracts::RecordSettingsView
{
public:
    RecordSettingsView();

protected:
    void InitViews() override;
    void InitData() override;
    bool CheckSettingsValid() override;
    bool SaveNormalSettingsChange() override;
    void SaveOrCheckStreamingSettingsChange(bool check, ChangeType& result) override;

    // BililiveComboboxExListener
    void OnSelectedIndexChanged(BililiveComboboxEx* combobox) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

private:
    void InitCmbModel();

    LivehimeCombobox* recording_format_combobox_ = nullptr;
    LivehimeTipLabel* recording_format_tip_label_ = nullptr;
    BilibiliNativeEditView* folder_textfield_ = nullptr;
    BilibiliNativeEditView* unity_folder_textfield_ = nullptr;
    LivehimeCheckbox* record_sync_checkbox_ = nullptr;

    base::string16 other_cache_path_;

    std::unique_ptr<contracts::SettingsPresenter> presenter_;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_RECORD_SETTINGS_VIEW_H_