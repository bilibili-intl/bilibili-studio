#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_COLOR_PROPERTY_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_COLOR_PROPERTY_CONTRACT_H_

#include "bililive/bililive/livehime/sources_properties/source_public_property_presenter.h"

namespace contracts {

    class SourceColorPropertyPresenter
        : public SourcePublicPropertyPresenter
    {
    public:
        explicit SourceColorPropertyPresenter(obs_proxy::SceneItem* scene_item)
            : SourcePublicPropertyPresenter(scene_item){}

        ~SourceColorPropertyPresenter() {}

        virtual void SetColor(SkColor value) = 0;
        virtual SkColor GetColor() = 0;

        virtual void SetWidth(int width) = 0;
        virtual int GetWidth() = 0;

        virtual void SetHeight(int height) = 0;
        virtual int GetHeight() = 0;

        virtual float GetTransparent() = 0;
        virtual void SetTransparent(float value) = 0;

    };

}   // namespace contracts

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_COLOR_PROPERTY_CONTRACT_H_