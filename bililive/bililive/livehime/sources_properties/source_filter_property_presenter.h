#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCE_FILTER_PROPERTY_PRESENTER_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCE_FILTER_PROPERTY_PRESENTER_H_

#include "bililive/bililive/livehime/sources_properties/source_public_property_presenter.h"
#include "bililive/bililive/livehime/sources_properties/source_property_snapshot.h"

const int kChromaKeySimilarMin = 1;
const int kChromaKeySimilarMax = 1000;

const int kChromaKeySmoothMin = 1;
const int kChromaKeySmoothMax = 1000;

const int kChromaKeySpillMin = 1;
const int kChromaKeySpillMax = 1000;

class obs_proxy::SceneItem;
class SceneItemHelper;

class SourcePublicFilterPropertyPresenter
    : public SourcePublicPropertyPresenter {
public:
    explicit SourcePublicFilterPropertyPresenter(obs_proxy::SceneItem* scene_item)
        : SourcePublicPropertyPresenter(scene_item) {}

    virtual ~SourcePublicFilterPropertyPresenter() {}

    virtual bool GetIsUseColorKey() = 0;
    virtual void SetIsUseColorKey(bool value) = 0;

    virtual BaseFilterItemHelper::ColorKeyColorT GetColorKeyColor() = 0;
    virtual void SetColorKeyColor(BaseFilterItemHelper::ColorKeyColorT type, SkColor val = 0) = 0;

    virtual std::tuple<std::string, SkColor> GetColorKeyColorVal() = 0;
    virtual void SetColorKeyColorVal(const std::tuple<std::string, SkColor>&) = 0;

    virtual float GetColorKeySimilar() = 0;
    virtual void SetColorKeySimilar(float value) = 0;

    virtual float GetColorKeySmooth() = 0;
    virtual void SetColorKeySmooth(float value) = 0;

    virtual float GetColorKeySpill() = 0;
    virtual void SetColorKeySpill(float value) = 0;

    virtual float GetColorKeyTransparent() = 0;
    virtual void SetColorKeyTransparent(float value) = 0;

};

class SourcePublicFilterPropertyPresenterImp 
    : public SourcePublicFilterPropertyPresenter {

public:
    explicit SourcePublicFilterPropertyPresenterImp(obs_proxy::SceneItem* scene_item);
    virtual ~SourcePublicFilterPropertyPresenterImp() {}

    virtual bool GetIsUseColorKey() override;
    virtual void SetIsUseColorKey(bool value) override;

    virtual BaseFilterItemHelper::ColorKeyColorT GetColorKeyColor() override;
    virtual void SetColorKeyColor(BaseFilterItemHelper::ColorKeyColorT type, SkColor val = 0) override;

    virtual std::tuple<std::string, SkColor> GetColorKeyColorVal() override;
    virtual void SetColorKeyColorVal(const std::tuple<std::string, SkColor>&) override;

    virtual float GetColorKeySimilar() override;
    virtual void SetColorKeySimilar(float value) override;

    virtual float GetColorKeySmooth() override;
    virtual void SetColorKeySmooth(float value) override;

    virtual float GetColorKeySpill() override;
    virtual void SetColorKeySpill(float value) override;

    virtual float GetColorKeyTransparent() override;
    virtual void SetColorKeyTransparent(float value) override;

    virtual void Snapshot() override;
    virtual void Restore() override;

private:
    std::shared_ptr<BaseFilterItemHelper> base_filter_item_helper_;
    std::unique_ptr<livehime::ColorKeyPropertySnapshot> snapshot_;
};

#endif // !BILILIVE_BILILIVE_LIVEHIME_SOURCE_FILTER_PROPERTY_PRESENTER_H_
