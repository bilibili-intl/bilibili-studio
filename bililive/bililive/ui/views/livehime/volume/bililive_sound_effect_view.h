#pragma once

#include "base/notification/notification_observer.h"
#include "base/notification/notification_registrar.h"
#include "base/notification/notification_source.h"

#include "bililive/bililive/ui/views/controls/bililive_floating_scroll_view.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/controls/combox/bililive_combobox_ex.h"
#include "bililive/bililive/ui/views/livehime/settings/base_settings_view.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_combobox.h"
#include "bililive/bililive/ui/views/livehime/volume/sound_reverb_view.h"
#include "bililive/bililive/livehime/settings/settings_contract.h"

#include "bililive/bililive/livehime/volume/bililive_volume_controllers_devices_contract.h"
#include "bililive/bililive/livehime/volume/sound_effect_property_contract.h"

#include "obs/obs_proxy/public/proxy/obs_proxy.h"

#include "ui/views/controls/slider.h"
#include "ui/views/controls/button/button.h"


class LivehimeCheckbox;
class LivehimeSlider;
class LivehimeContentLabel;
class LivehimeTipLabel;
class BililiveLabelButton;
class LivehimeFunctionLabelButton;
class DenoiseSlider;
class ToolBarVolumeCtrl;

// BililiveSoundEffectView
class BililiveSoundEffectView : 
    public BaseSettingsView,
    public views::ButtonListener,
    public views::SliderListener,
    public obs_proxy_ui::OBSUIProxyObserver,
    public base::NotificationObserver,
    public BililiveComboboxExListener,
    public contracts::AudioSettingsView
{
private:
    enum EffectiveControl
    {
        SOUND_EFFECT_INVALID = -1,
        SOUND_EFFECT_DENOISE,
        SOUND_EFFECT_ROOM_SIZE,
        SOUND_EFFECT_DAMPING,
        SOUND_EFFECT_WET,
        SOUND_EFFECT_DRY,
        SOUND_EFFECT_WIDTH,
        SOUND_EFFECT_FREEZE_MODE,
    };

public:
    BililiveSoundEffectView();

    virtual ~BililiveSoundEffectView();

    void OnCancel() override;

    //BaseSettingsView
    void SaveOrCheckStreamingSettingsChange(bool check, ChangeType& result);

    void SetInsets(const gfx::Insets& insets);

protected:
    // View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) override;
    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;
    // SliderListener
    void SliderValueChanged(views::Slider* sender, float value, float old_value, views::SliderChangeReason reason) override;
    // OBSUIProxyObserver
    void OnAudioSourceActivate(obs_proxy::VolumeController* audio_source) override;
    // NotificationObserver
    void Observe(int type, const base::NotificationSource& source, const base::NotificationDetails& details) override;
    // BililiveComboboxExListener
    void OnSelectedIndexChanged(BililiveComboboxEx* combobox) override;

    //BaseSettingsView
    bool CheckSettingsValid() override;
    bool SaveNormalSettingsChange() override;

    // contracts::AudioSettingsView
    void OnInitComboData(
        const std::string& device,
        const std::wstring& devname,
        const std::string& devid) override;

private:
    void InitViews();
    void InitData();
    void InitVolumeControllerData(bool is_mic);
    void SetEnableSystemVoice(bool enabled);
    void GetMonoSettings(std::pair<std::string, std::string>& device_string);
    views::View* CreateSubView(views::View* label, views::View* buuton, int padding);

    void SaveOrCheckAudioDeviceSetting(bool check, ChangeType& result);
    void SaveReverbSetting();

    void OnVolumeSliderValueChanged(contracts::BililiveVolumeControllersContract* controller, views::Slider* sender, BililiveLabel* label, float value, float old_value);

    void OnVolumeChanged(const std::string& source_name, float db);
    void OnVolumeMuteChanged(const std::string& source_name, bool muted);
    void OnVolumeChangedInternal(bool is_mic);
    void SetVolumeSliderValue(bool is_mic);

    void ShowMicWidget(bool show);

    float MergeDeflectionAndGainToVolume(float deflection, float gain);
    bool UpdateVolumeSliderKeyboardIncrement(views::Slider* volSlider, float oldValue);
    int VolumeSliderValueToInteger(float value);
    void SplitVolumeSliderValueToDeflectionAndGain(float volume, float* deflection, float* gain);
    std::wstring VolumeSliderValueToString(float value);

    void SetVolumeBtnState(bool is_mic, bool mute);
    void SetVolumeBtnEnable(bool is_mic, bool enable);

    gfx::Insets             insets_;
    BililiveLabel* mic_adv_label_ = nullptr;
    BililiveLabel* mic_denoise_label_ = nullptr;
    BililiveLabel* audio_rate_label_ = nullptr;
    BililiveLabel* sound_track_label_ = nullptr;
    LivehimeHoverTipButton* volume_tip_ = nullptr;
    LivehimeHoverTipButton* bit_rate_tip_ = nullptr;

    LivehimeCombobox*       mic_combobox_ = nullptr;            //麦克风设备组合框
    LivehimeCombobox*       speaker_combobox_ = nullptr;        //扬声器设备组合框
    LivehimeSlider*         mic_volume_slider_ = nullptr;       //麦克风音量滑动条
    LivehimeSlider*         speaker_volume_slider_ = nullptr;   //扬声器音量滑动条
    BililiveLabel*          mic_volume_label_ = nullptr;        //麦克风音量标签
    BililiveLabel*          speaker_volume_label_ = nullptr;    //扬声器音量标签
    SoundReverbView*        reverb_view_ = nullptr;             //混响view
    BililiveImageButton*    mic_mute_btn_ = nullptr;            //麦克风静音
    BililiveImageButton*    speaker_mute_btn_ = nullptr;        //扬声器静音
    DenoiseSlider*          denoise_slider_ = nullptr;          //降噪等级
    LivehimeCombobox*       bitrate_combobox_ = nullptr;        //音频码率
    LivehimeCombobox*       track_combobox_ = nullptr;          //声道

    base::NotificationRegistrar notifation_register_;

    int audio_inited_ = 0;
    bool is_self_operation_ = false;

    float mic_deflection_ = 0.0f;
    float mic_db_ = 0.0f;

    float speaker_deflection_ = 0.0f;
    float speaker_db_ = 0.0f;

    std::unique_ptr<contracts::BililiveVolumeControllersContract> mic_controllers_;
    std::unique_ptr<contracts::BililiveVolumeControllersContract> speaker_controllers_;
    std::unique_ptr<contracts::SettingsPresenter> audio_setting_presenter_;
    std::unique_ptr<contracts::SoundEffectPropertyPresenter> presenter_;
    base::WeakPtrFactory<BililiveSoundEffectView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(BililiveSoundEffectView);
};
