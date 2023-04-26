#pragma once

#include "ui/views/view.h"

#include "bililive/bililive/ui/views/livehime/controls/custom_style_button.h"
#include "bililive/bililive/ui/views/livehime/controls/custom_style_checkbox.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_hover_tip_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_slider.h"
#include "bililive/bililive/ui/views/livehime/volume/bililive_volume_bar.h"
#include "bililive/bililive/livehime/volume/bililive_volume_controllers_devices_contract.h"
#include "bililive/bililive/livehime/volume/sound_effect_property_contract.h"


class SoundReverbView :
    public views::View,
    public views::ButtonListener,
    public views::SliderListener
{
public:
    SoundReverbView(views::ButtonListener *listener);
    ~SoundReverbView();

    views::View* GetMixerMainView();
    LivehimeCheckbox* GetEnableCheckBox();

    void SetPresenter(contracts::SoundEffectPropertyPresenter* presenter);
    void OnEnableCheckBoxChanged();
    void OnSaveChanged();
    void InitMicVolumeController(contracts::BililiveVolumeControllersContract* controller);

protected:
    //views
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;
    void SliderValueChanged(views::Slider* sender, float value, float old_value, views::SliderChangeReason reason) override;


private:
    void InitView();
    void InitData();
    void UpdateControlStateAndData();
    views::View* CreateSubView(views::View* label, views::View* buuton, int padding);


    contracts::BililiveVolumeControllersContract*   volume_controllers_ = nullptr;
    contracts::SoundEffectPropertyPresenter*        presenter_ = nullptr;
    views::ButtonListener*                          sound_effect_listener_ = nullptr;
    LivehimeCheckbox*                               enable_mixer_checkbox_ = nullptr;
    CustomStyleButton*                              reset_btn_ = nullptr;
    LivehimeSlider*                                 room_size_slider_ = nullptr;
    BililiveLabel*                                  room_size_label_ = nullptr;
    LivehimeSlider*                                 wet_slider_ = nullptr;
    BililiveLabel*                                  wet_label_ = nullptr;
    LivehimeSlider*                                 dry_slider_ = nullptr;
    BililiveLabel*                                  dry_label_ = nullptr;
    LivehimeSlider*                                 width_slider_ = nullptr;
    BililiveLabel*                                  width_label_ = nullptr;
    LivehimeSlider*                                 damp_slider_ = nullptr;
    BililiveLabel*                                  damp_label_ = nullptr;
    LivehimeSlider*                                 freeze_mode_slider_ = nullptr;
    BililiveLabel*                                  freeze_mode_label_ = nullptr;

    CustomStyleCheckbox*                            preview_btn_ = nullptr;
    BililiveVolumeBar*                              volume_bar_ = nullptr;

    views::View*                                    main_view_ = nullptr;
};
