#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_ALBUM_PROPERTY_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_ALBUM_PROPERTY_VIEW_H_

#include "bililive/bililive/livehime/sources_properties/source_album_property_contract.h"
#include "bililive/bililive/ui/views/controls/listview.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_combobox.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_slider.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_property_base_view.h"

class AlbumPropDetailView
    : public livehime::BasePropertyView,
      public BililiveComboboxExListener,
      public views::SliderListener,
      public views::ButtonListener
{
private:
    enum EffectiveControl
    {
        ALBUM_INVALID = -1,
        ALBUM_TRANSITION,
        ALBUM_PLAY_TIME,
        ALBUM_TRANSITION_TIME,
        ALBUM_LOOP,
        ALBUM_RANDOM
    };

public:
    explicit AlbumPropDetailView(obs_proxy::SceneItem* scene_item);

    virtual ~AlbumPropDetailView() {}

private:
    void InitView() override;

    void InitData() override;

    bool CheckSetupValid() override;

    bool SaveSetupChange() override;

    bool Cancel() override;

    gfx::ImageSkia* GetSkiaIcon() override;

    std::wstring GetCaption() override;

    // BililiveComboboxExListener
    void OnSelectedIndexChanged(BililiveComboboxEx* combobox) override;

    // SliderListener
    void SliderValueChanged(
        views::Slider* sender,
        float value,
        float old_value,
        views::SliderChangeReason reason) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

private:
    void InitFileList();

    void InitTransitionComb();

    void EffectiveImmediately(EffectiveControl effective_control);

private:
    bool data_loaded_;

    LivehimeNativeEditView* name_edit_;

    LivehimeFunctionLabelButton* add_file_button_;
    LivehimeFunctionLabelButton* add_directory_button_;
    LivehimeFunctionLabelButton* delete_button_;
    LivehimeFunctionLabelButton* empty_button_;

    ListView *file_list_view_;

    LivehimeCombobox* transition_combobox_;

    LivehimeSlider* play_time_slider_;
    LivehimeContentLabel* play_time_value_label_;
    LivehimeSlider* transition_slider_;
    LivehimeContentLabel* transition_value_label_;

    BililiveCheckbox* loop_checkbox_;
    BililiveCheckbox* random_checkbox_;

    std::unique_ptr<contracts::SourceAlbumPropertyPresenter> presenter_;

    DISALLOW_COPY_AND_ASSIGN(AlbumPropDetailView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_ALBUM_PROPERTY_VIEW_H_