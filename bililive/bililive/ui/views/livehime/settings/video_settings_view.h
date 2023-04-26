#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_VIDEO_SETTINGS_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_VIDEO_SETTINGS_VIEW_H_

#include "ui/views/controls/button/button.h"

#include "bililive/bililive/livehime/settings/settings_contract.h"

#include "bililive/bililive/ui/views/controls/combox/bililive_combobox_ex.h"
#include "bililive/bililive/ui/views/livehime/settings/base_settings_view.h"


class ModelToolTip;
class LivehimeCombobox;
class LivehimeCheckbox;
class LivehimeTextfield;
class LivehimeTipLabel;
class LivehimeRadioButton;

//  ”∆µ…Ë÷√“≥
class VideoSettingsView :
    public BaseSettingsView,
    public BililiveComboboxExListener,
    public views::ButtonListener,
    public contracts::VideoSettingsView
{
public:
    VideoSettingsView();

protected:
    // BaseSettingsView
    void InitViews() override;
    void InitData() override;
    bool CheckSettingsValid() override { return true; }
    bool SaveNormalSettingsChange() override { return true; }
    void SaveOrCheckStreamingSettingsChange(bool check, ChangeType& result) override;

    // BililiveComboboxExListener
    void OnSelectedIndexChanged(BililiveComboboxEx* combobox) override;
    void OnBililiveComboboxExEditBlur(
        BililiveComboboxEx* combobox, const base::string16& text, int reason) override;
    base::string16 OnBililiveComboboxExEditShow(
        BililiveComboboxEx* combobox, const base::string16& label_text) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

private:
    void InitCustomViews();
    void InitCmbModel();

private:
    LivehimeRadioButton* auto_radio_btn_ = nullptr;
    LivehimeRadioButton* custom_radio_btn_ = nullptr;
    LivehimeTipLabel* mode_tip_label_ = nullptr;

    View* custom_view_ = nullptr;

    LivehimeCombobox* corerate_combobox_;
    LivehimeCombobox* bitrate_control_combobox_;
    LivehimeCombobox* framerate_combobox_;
    LivehimeCombobox* streaming_res_combobox_;
    LivehimeCombobox* encoder_combobox_;
    LivehimeCombobox* streaming_quality_combobox_;
    views::Label*       code_tip_ = nullptr;

    std::unique_ptr<contracts::SettingsPresenter> presenter_;

    base::WeakPtrFactory<VideoSettingsView> weak_ptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(VideoSettingsView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_VIDEO_SETTINGS_VIEW_H_