#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_COLOR_PROPERTY_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_COLOR_PROPERTY_VIEW_H_

#include "bililive/bililive/livehime/sources_properties/source_color_property_contract.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_slider.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_property_base_view.h"

class ColorPropDetailView
    : public livehime::BasePropertyView,
      public views::SliderListener,
      public views::ButtonListener,
      public BilibiliNativeEditController
{
private:
    enum EffectiveControl
    {
        COLOR_INVALID = -1,
        COLOR_COLOR,
        COLOR_WIDTH,
        COLOR_HEIGHT,
        COLOR_TRANSPARENT
    };

public:
    explicit ColorPropDetailView(obs_proxy::SceneItem* scene_item);

    virtual ~ColorPropDetailView();

    void ContentsChanged(BilibiliNativeEditView* sender, const string16& new_contents) override;
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

private:
    void EffectiveImmediately(EffectiveControl effective_control);

private:
    bool data_loaded_;

    LivehimeNativeEditView* name_edit_;

    LivehimeFunctionLabelButton* color_button_;

    LivehimeNativeEditView* width_edit_;

    LivehimeNativeEditView* height_edit_;

    LivehimeSlider* transparent_slider_;

    LivehimeContentLabel* transparent_value_label_;

    std::unique_ptr<contracts::SourceColorPropertyPresenter> presenter_;

    DISALLOW_COPY_AND_ASSIGN(ColorPropDetailView);

};

#endif // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_COLOR_PROPERTY_VIEW_H_