#pragma once

#include "ui/views/controls/button/button.h"

#include "bililive/bililive/livehime/sources_properties/source_camera_property_contract.h"
#include "bililive/bililive/ui/views/controls/navigation_bar.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_hover_tip_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_textfield.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_slider.h"
#include "bililive/bililive/ui/views/livehime/controls/custom_style_button.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/base_setup_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_audio_base_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_volume_control_view.h"
#include "bililive/bililive/ui/views/login/bilibili_login_control.h"


class obs_proxy::SceneItem;
class SourceFilterBaseView;

// ª˘¥°…Ë÷√“≥
//source_camera_base_settings_view
class SourceCameraBaseSettingsView :
    public BaseSetupView,
    public contracts::SourceCameraPropertyView,
    public BililiveComboboxExListener,
    public views::ButtonListener
{
public:
    enum CameraEffectiveControl
    {
        CAMERA_DPI = 0,
        CAMERA_FLIP_VERTICAL,
        CAMERA_FLIP_HORIZINTAL,
    };

    enum CommonButtonId
    {
        HORIZINTAL_CHECKBOX = 1,
        VERTICAL_CHECKBOX,
        DANMAKU_CHECKBOX,
        BACKGROUND_CHECKBOX,
    };

    SourceCameraBaseSettingsView(obs_proxy::SceneItem* scene_item);

    ~SourceCameraBaseSettingsView();

public:
    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // BililiveComboboxExListener
    void OnBililiveComboboxPressed(BililiveComboboxEx* combobox) override;
    void OnSelectedIndexChanged(BililiveComboboxEx* combobox) override;
    void OnBililiveComboboxExDropDown(BililiveComboboxEx* combobox) override;

    void InitView() override;
    void UninitView() override;
    void InitData() override;

    bool SaveSetupChange() override;
    void PostSaveSetupChange(obs_proxy::SceneItem* scene_item) override;

    bool Cancel() override;

    // contracts::SourceCameraPropertyView
    void RefreshResolutionListCallback() override;

    void RefreshCameraList();
    void RefreshResolutionList();
    void RefreshAudioOutputList();

    CameraSceneItemHelper::ColorKeyColorT GetSelectedColor();
    void EffectiveImmediately(CameraEffectiveControl effetive_control);
    void ShowDanmakuMaskCtrls(bool show);

    gfx::Size GetPreferredSize() override;

private:
    bool data_loaded_ = false;

    LivehimeImageView* logo_image_ = nullptr;
    LivehimeLinkButton* auto_beauty_settings_ = nullptr;

    LivehimeSrcPropCombobox* device_combox_ = nullptr;
    CustomStyleButton* device_set_button_ = nullptr;
    LivehimeSrcPropCombobox* dpi_combox_ = nullptr;
    LivehimeSrcPropCombobox* audio_output_combox_ = nullptr;

    LivehimeCheckbox* beauty_checkbox_ = nullptr;
    CustomStyleButton* beauty_param_btn_ = nullptr;
    CustomStyleButton* horizintal_flip_btn_ = nullptr;
    CustomStyleButton* vertical_flip_btn_ = nullptr;
    CustomStyleButton* rotation_btn_ = nullptr;

    LivehimeCheckbox* danmaku_checkbox_ = nullptr;
    LivehimeHoverTipButton* danmaku_switch_tip_ = nullptr;
    LivehimeHoverTipButton* background_switch_tip_ = nullptr;
    views::View* danmaku_mask_contaner_ = nullptr;

    obs_proxy::SceneItem*   scene_item_;
    SourceFilterBaseView*   source_filter_base_view_;
    SourceAudioBaseView*    source_audio_base_view_ = nullptr;

    std::shared_ptr<contracts::SourceCameraPropertyPresenter> presenter_;

    DISALLOW_COPY_AND_ASSIGN(SourceCameraBaseSettingsView);
};

