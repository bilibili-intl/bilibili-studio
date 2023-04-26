#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_IMAGE_PROPERTY_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_IMAGE_PROPERTY_PRESENTER_IMPL_H_

#include "bililive/bililive/livehime/sources_properties/source_property_snapshot.h"
#include "bililive/bililive/livehime/sources_properties/source_image_property_contract.h"

class SourceImagePropertyPresenterImpl
    : public contracts::SourceImagePropertyPresenter
{
public:
    explicit SourceImagePropertyPresenterImpl(obs_proxy::SceneItem* scene_item);

    ~SourceImagePropertyPresenterImpl() {}

    string16 GetFilePath() override;
    void SetFilePath(const string16& path) override;

    float GetTransparent() override;
    void SetTransparent(float value) override;

    void RotateLeft() override;

    void RotateRight() override;

    void VerticalFlip() override;

    void HorizontalFlip() override;

    void Snapshot() override;
    void Restore() override;

private:
    ImageSceneItemHelper image_scene_item_;
    std::unique_ptr<livehime::ImagePropertySnapshot> snapshot_;

    DISALLOW_COPY_AND_ASSIGN(SourceImagePropertyPresenterImpl);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_IMAGE_PROPERTY_PRESENTER_IMPL_H_