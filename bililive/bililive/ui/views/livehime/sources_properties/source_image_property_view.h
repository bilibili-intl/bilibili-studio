#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_IMAGE_PROPERTY_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_IMAGE_PROPERTY_VIEW_H_

#include "bililive/bililive/livehime/sources_properties/source_image_property_contract.h"
#include "bililive/bililive/ui/views/livehime/controls/custom_style_button.h"
#include "bililive/bililive/ui/views/livehime/controls/custom_style_checkbox.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_slider.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_property_base_view.h"
#include "bililive/bililive/ui/views/livehime/preset_material/preset_material_background_preview_view.h"


class ImagePropDetailView : 
    public livehime::BasePropertyView,
    public views::SliderListener,
    public views::ButtonListener,
    public PresetMaterialBackgroundPreviewObserver
{
private:
    enum EffectiveControl
    {
        IMAGE_INVALID = -1,
        IMAGE_TRANSPARENT,
        IMAGE_ROTATE_CLOCKWISE,
        IMAGE_ROTATE_ANTICLOCKWISE,
        IMAGE_FLIP_VERTICAL,
        IMAGE_FLIP_HORIZINTAL
    };

public:
    explicit ImagePropDetailView(obs_proxy::SceneItem* scene_item);

    virtual ~ImagePropDetailView();

    //gfx::Size GetPreferredSize() override;

private:
    void InitView() override;

    void InitData() override;

    bool CheckSetupValid() override;

    bool SaveSetupChange() override;

    bool Cancel() override;

    gfx::ImageSkia* GetSkiaIcon() override;

    std::wstring GetCaption() override;

    void SliderValueChanged(views::Slider* sender, float value, float old_value, views::SliderChangeReason reason) override;

    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    void OnBkImageSelected(const BkImageInfo& bk_info) override;
    void OnBkImageInitialized() override;

private:
    void EffectiveImmediately(EffectiveControl effective_control);

private:
    bool data_loaded_;

    //LivehimeNativeEditView* name_edit_;
    LivehimeNativeEditView* file_path_;
    BililiveLabelButton* browse_button_;
    LivehimeSlider* transparent_slider_;
    LivehimeContentLabel* transparent_value_label_;

    CustomStyleButton* rotate_clockwise_button_;
    CustomStyleButton* rotate_anticlockwise_button_;

    CustomStyleButton* flip_vertical_button_;
    CustomStyleButton* flip_horizintal_button_;

    PresetMaterialBackgroundPreviewView* bk_preview_ = nullptr;

    std::unique_ptr<contracts::SourceImagePropertyPresenter> presenter_;

    DISALLOW_COPY_AND_ASSIGN(ImagePropDetailView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_IMAGE_PROPERTY_VIEW_H_