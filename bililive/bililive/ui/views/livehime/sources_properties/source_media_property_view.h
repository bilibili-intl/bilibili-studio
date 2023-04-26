#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_MEDIA_PROPERTY_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_MEDIA_PROPERTY_VIEW_H_

#include "bililive/bililive/livehime/sources_properties/source_media_property_contract.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_property_base_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_volume_control_view.h"

class MediaPropDetailView
    : public livehime::BasePropertyView,
      public views::ButtonListener,
      public contracts::SourceMediaPropertyView
{
public:
    explicit MediaPropDetailView(obs_proxy::SceneItem* scene_item);

    ~MediaPropDetailView() OVERRIDE;

    using OnMediaStateCallHandler = std::function<void(bool)>;
    void SetVtuber(const std::string& mp4_path, OnMediaStateCallHandler handler);

private:
    void InitView() override;

    void InitData() override;

    bool CheckSetupValid() override;

    bool SaveSetupChange() override;

    bool Cancel() override;

    gfx::ImageSkia* GetSkiaIcon() override;

    std::wstring GetCaption() override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // contracts::SourceMediaPropertyView
    void OnMediaStateChanged(contracts::MediaState state) override;

private:
    bool data_loaded_;

    LivehimeNativeEditView* name_edit_;
    SouceVolumeControlView* volume_control_view_;
    LivehimeNativeEditView* file_path_;
    BililiveLabelButton* browse_button_;
    BililiveCheckbox* loop_checkbox_;

    OnMediaStateCallHandler media_state_handler_;

    std::shared_ptr<contracts::SourceMediaPropertyPresenter> presenter_;

    DISALLOW_COPY_AND_ASSIGN(MediaPropDetailView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_MEDIA_PROPERTY_VIEW_H_