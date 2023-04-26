#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_TEXT_PROPERTY_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_TEXT_PROPERTY_VIEW_H_

#include "bililive/bililive/livehime/sources_properties/source_text_property_contract.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_combobox.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_slider.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_textfield.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_property_base_view.h"

class TextPropDetailView
    : public livehime::BasePropertyView,
      public views::ButtonListener,
      public views::SliderListener,
      public BililiveComboboxExListener
{
private:
    enum EffectiveControl
    {
        TEXT_INVALID = -1,
        TEXT_FONT,                  // font effective
        TEXT_OUTLINE,               // outline effective
        TEXT_HORIZONTAL_SCROLL,
        TEXT_VERTICAL_SCROLL,
        TEXT_TRANSPARENT
    };

public:
    explicit TextPropDetailView(obs_proxy::SceneItem* scene_item);

    virtual ~TextPropDetailView() {}

    using CheckSorceTextHandler = std::function<void(bool)>;
    CheckSorceTextHandler check_sensitive_;

    void CheckSourceText(CheckSorceTextHandler handle);

private:
    void InitView() override;

    void InitData() override;

    bool CheckSetupValid() override;

    bool SaveSetupChange() override;

    bool Cancel() override;

    gfx::ImageSkia* GetSkiaIcon() override;

    std::wstring GetCaption() override;

    //views
    gfx::Size GetPreferredSize() override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // SliderListener
    void SliderValueChanged(
        views::Slider* sender,
        float value,
        float old_value,
        views::SliderChangeReason reason) override;

    // BililiveComboboxExListener
    void OnSelectedIndexChanged(BililiveComboboxEx* combobox) override;

    base::string16 OnBililiveComboboxExEditShow(BililiveComboboxEx* combobox, const base::string16 &label_text) override;

    void OnBililiveComboboxExEditBlur(
        BililiveComboboxEx* combobox,
        const base::string16 &text,
        int reason) override;

    void OnBililiveComboboxExEditContentsChanged(BililiveComboboxEx* combobox, const string16& new_contents) override;
private:
    void InitFontData();

    void UpdateControlStatus();

    void EffectiveImmediately(EffectiveControl effective_control);

    void EffectiveFontData();

    void EffectiveOutlineData();

    void SearchStringInData(base::string16 source_str);

    void OnCheckSourceText(bool valid_response, int code, bool is_sensitive);

private:
    bool data_loaded_;

    LivehimeNativeEditView* name_edit_;

    LivehimeNativeEditView* text_area_;

    BililiveCheckbox* load_file_checkbox_;
    BililiveLabelButton* browse_file_button_;
    LivehimeTextfield* file_path_edit_;

    LivehimeCombobox* font_name_combo_;
    LivehimeCombobox* font_size_combo_;
    LivehimeCombobox* font_style_combo_;

    BililiveLabelButton* color_button_;

    BililiveCheckbox* use_outline_checkbox_;
    BililiveLabelButton* outline_color_button_;
    LivehimeSlider* outline_slider_;
    LivehimeContentLabel* outline_value_label_;

    LivehimeSlider* horizontal_scroll_slider_;
    LivehimeContentLabel* horizontal_scroll_value_label_;
    LivehimeSlider* vertical_scroll_slider_;
    LivehimeContentLabel* vertical_scroll_value_label_;
    LivehimeSlider* transparent_slider_;
    LivehimeContentLabel* transparent_value_label_;

    SkColor font_color_;
    SkColor outline_color_;

    std::unique_ptr<contracts::SourceTextPropertyPresenter> presenter_;
    base::WeakPtrFactory<TextPropDetailView> weakptr_factory_;

    base::string16 cur_sel_font_name_;
    DISALLOW_COPY_AND_ASSIGN(TextPropDetailView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_TEXT_PROPERTY_VIEW_H_