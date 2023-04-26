#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_LANGUAGE_SETTINGS_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_LANGUAGE_SETTINGS_VIEW_H_

#include "bililive/bililive/livehime/settings/settings_contract.h"
#include "bililive/bililive/ui/views/controls/combox/bililive_combobox_ex.h"
#include "bililive/bililive/ui/views/livehime/settings/base_settings_view.h"


class LivehimeCombobox;
class LivehimeCheckbox;
class LivehimeTextfield;
class LivehimeTipLabel;

class LanguageSettingsView :
    public BaseSettingsView,
    public BililiveComboboxExListener,
    public views::ButtonListener,
    public contracts::RecordSettingsView
{
public:
    LanguageSettingsView();
    bool IsLanguageChanged() { return is_language_changed_; };
    void ClearLanguageChangeFlag() { is_language_changed_ = false; };

protected:
    void InitViews() override;
    void InitData() override;
    bool CheckSettingsValid() override;
    bool SaveNormalSettingsChange() override;

    // BililiveComboboxExListener
    void OnSelectedIndexChanged(BililiveComboboxEx* combobox) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

private:
    void InitCmbModel();

    LivehimeCombobox* recording_format_combobox_ = nullptr;
    std::string language_code_;

    std::unique_ptr<contracts::SettingsPresenter> presenter_;

    bool is_language_changed_ = false;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_RECORD_SETTINGS_VIEW_H_