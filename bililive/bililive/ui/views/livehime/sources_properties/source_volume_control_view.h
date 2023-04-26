#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCE_VOLUME_CONTROL_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCE_VOLUME_CONTROL_VIEW_H_

#include "ui/views/view.h"
#include "ui/views/controls/button/image_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_slider.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_property_base_view.h"
#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"

#include "obs/obs_proxy/public/proxy/obs_proxy.h"

class BililiveImageButton;

enum class SourceVolumeCtrlType
{
    Camera,
    Media,
    Mobile,
    AudioInputDevice,
    AudioOutputDevice,
    Browser,
};

class SouceVolumeControlView:
    public views::View,
    public views::SliderListener,
    public views::ButtonListener,
    public obs_proxy_ui::OBSUIProxyObserver,
    public BililiveComboboxExListener
{
    enum ImageButtonId
    {
        ID_SYSTEM_VOICE,
        ID_MIC_VOICE
    };
public:
    SouceVolumeControlView(SourceVolumeCtrlType type);

    SouceVolumeControlView(obs_proxy::SceneItem* sceneItem, SourceVolumeCtrlType type, bool for_camera = false);

    ~SouceVolumeControlView();

    void SliderValueChanged(views::Slider* sender, float value,
        float old_value, views::SliderChangeReason reason) override;

    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) override;

    // OBSUIProxyObserver
    void OnAudioSourceActivate(obs_proxy::VolumeController* audio_source) override;

    // BililiveComboboxExListener
    void OnSelectedIndexChanged(BililiveComboboxEx* combobox) override;

    void SetSceneItem(obs_proxy::SceneItem* sceneItem);

private:
    void SetVolume(float val);

    void SetMutedButtonState(bool is_muted);

    void InitViews();
private:
    BililiveImageButton* muted_button_;

    LivehimeSlider* volume_slider_;

    LivehimeContentLabel* volume_value_label_;

    SourceVolumeCtrlType type_;

    LivehimeSrcPropCombobox* monitor_combobox_;

    obs_proxy::SceneItem* sceneItem_;

    bool need_listening_;

    bool for_camera_ = false;

    DISALLOW_COPY_AND_ASSIGN(SouceVolumeControlView);
};
#endif //BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCE_VOLUME_CONTROL_VIEW_H_