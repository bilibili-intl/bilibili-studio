#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_COLOR_PROPERTY_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_COLOR_PROPERTY_PRESENTER_IMPL_H_

#include "bililive/bililive/livehime/sources_properties/source_property_snapshot.h"
#include "bililive/bililive/livehime/sources_properties/source_color_property_contract.h"

class SourceColorPropertyPresenterImpl
    : public contracts::SourceColorPropertyPresenter
{
public:
    explicit SourceColorPropertyPresenterImpl(obs_proxy::SceneItem* scene_item);

    ~SourceColorPropertyPresenterImpl() {}

    void SetColor(SkColor value) override;
    SkColor GetColor() override;

    void SetWidth(int width) override;
    int GetWidth() override;

    void SetHeight(int height) override;
    int GetHeight() override;

    float GetTransparent() override;
    void SetTransparent(float value) override;

    void Snapshot() override;
    void Restore() override;
private:
    ColorItemHelper color_scene_item_;
    std::unique_ptr<livehime::ColorPropertySnapshot> snapshot_;

    DISALLOW_COPY_AND_ASSIGN(SourceColorPropertyPresenterImpl);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_COLOR_PROPERTY_PRESENTER_IMPL_H_