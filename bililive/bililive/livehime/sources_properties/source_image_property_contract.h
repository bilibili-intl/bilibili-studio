#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_IMAGE_PROPERTY_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_IMAGE_PROPERTY_CONTRACT_H_

#include "bililive/bililive/livehime/sources_properties/source_public_property_presenter.h"

namespace contracts {

class SourceImagePropertyPresenter
    : public SourcePublicPropertyPresenter
{
public:
    explicit SourceImagePropertyPresenter(obs_proxy::SceneItem* scene_item)
        : SourcePublicPropertyPresenter(scene_item){}

    ~SourceImagePropertyPresenter() {}

    virtual string16 GetFilePath() = 0;
    virtual void SetFilePath(const string16& path) = 0;

    virtual float GetTransparent() = 0;
    virtual void SetTransparent(float value) = 0;

    virtual void RotateLeft() = 0;

    virtual void RotateRight() = 0;

    virtual void VerticalFlip() = 0;

    virtual void HorizontalFlip() = 0;
};

}   // namespace contracts

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_IMAGE_PROPERTY_CONTRACT_H_