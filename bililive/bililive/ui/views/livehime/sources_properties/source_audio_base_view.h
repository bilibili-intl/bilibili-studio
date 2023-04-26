#pragma once

#include "bililive/bililive/ui/views/controls/combox/bililive_combobox_ex.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_slider.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_property_base_view.h"
#include "bililive/bililive/livehime/sources_properties/source_camera_property_contract.h"

#include "obs/obs_proxy/public/proxy/obs_proxy.h"
#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"

#include "ui/views/view.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/slider.h"


class SourceAudioBaseView :
    public views::View,
    public views::ButtonListener,
    public views::SliderListener,
    public BililiveComboboxExListener,
    public contracts::SourceCameraPropertyView,
    public obs_proxy_ui::OBSUIProxyObserver
{
public:
    enum CameraEffectiveControl
    {
        CAMERA_AUDIO_OUTPUT = 0,
        CAMERA_FLIP_VERTICAL,
        CAMERA_FLIP_HORIZINTAL,
        CAMERA_AUDIO_DEVICE
    };

public:
    SourceAudioBaseView(obs_proxy::SceneItem* scene_item);
    virtual ~SourceAudioBaseView();

    void InitData();
    void SetCheck(bool check);
    bool Checked();

protected:
    //view
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    void OnPaint(gfx::Canvas* canvas);

    //bution
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;
    // SliderListener
    void SliderValueChanged(views::Slider* sender, float value, float old_value, views::SliderChangeReason reason) override;

    //BililiveComboboxExListener
    void OnBililiveComboboxExDropDown(BililiveComboboxEx* combobox) override;
    void OnSelectedIndexChanged(BililiveComboboxEx* combobox) override;

    //SourceCameraPropertyView
    void RefreshResolutionListCallback() override;

    //OBSUIProxyObserver
    void OnAudioSourceActivate(obs_proxy::VolumeController* audio_source) override;

private:
    void InitView();
    void RefreshAudioDeviceList();
    void RefreshAudioOutputList();
    void SetVolume(float val);
    void EffectiveImmediately(CameraEffectiveControl effetive_control);
    void SetMuteBtnState(bool mute);

    obs_proxy::SceneItem*       scene_item_ = nullptr;
    BililiveImageButton*        mute_btn_ = nullptr;
    BililiveImageButton*        audio_setting_checkbox_ = nullptr;
    LivehimeSrcPropCombobox*    audio_output_combox_ = nullptr;
    BililiveImageButton*        mic_mute_btn_ = nullptr;
    LivehimeSlider*             volume_slider_ = nullptr;
    BililiveLabel*              volume_label_ = nullptr;
    LivehimeSrcPropCombobox*    audio_device_combox_ = nullptr;
    LivehimeSrcPropCombobox*    monitor_combobox_ = nullptr;

    views::View*                main_view_ = nullptr;

    bool                        need_listening_ = false;

    std::unique_ptr<contracts::SourceCameraPropertyPresenter> presenter_;

};

