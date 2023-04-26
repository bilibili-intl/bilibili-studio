#include "source_filter_property_presenter.h"
#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"
#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"

SourcePublicFilterPropertyPresenterImp::SourcePublicFilterPropertyPresenterImp(obs_proxy::SceneItem* scene_item)
    :SourcePublicFilterPropertyPresenter(scene_item),
    base_filter_item_helper_(std::make_shared<BaseFilterItemHelper>(scene_item)){

}

bool SourcePublicFilterPropertyPresenterImp::GetIsUseColorKey() {
    return base_filter_item_helper_->IsUseColorKey();
}

void SourcePublicFilterPropertyPresenterImp::SetIsUseColorKey(bool value) {
    base_filter_item_helper_->IsUseColorKey(value);
}

BaseFilterItemHelper::ColorKeyColorT SourcePublicFilterPropertyPresenterImp::GetColorKeyColor() {
    return base_filter_item_helper_->ColorKeyColor();
}

void SourcePublicFilterPropertyPresenterImp::SetColorKeyColor(BaseFilterItemHelper::ColorKeyColorT type, SkColor val) {
    base_filter_item_helper_->ColorKeyColor(type,val);
}

std::tuple<std::string, SkColor> SourcePublicFilterPropertyPresenterImp::GetColorKeyColorVal() {
    return base_filter_item_helper_->GetColorKeyVal();
}

void SourcePublicFilterPropertyPresenterImp::SetColorKeyColorVal(const std::tuple<std::string, SkColor>& val) {
    base_filter_item_helper_->SetColorKeyVal(val);
}

float SourcePublicFilterPropertyPresenterImp::GetColorKeySimilar() {
    return MapFloatFromInt(base_filter_item_helper_->ColorKeySimilar(), kChromaKeySimilarMin, kChromaKeySimilarMax);
}

void SourcePublicFilterPropertyPresenterImp::SetColorKeySimilar(float value) {
    base_filter_item_helper_->ColorKeySimilar(MapFloatToInt(value, kChromaKeySimilarMin, kChromaKeySimilarMax));
}

float SourcePublicFilterPropertyPresenterImp::GetColorKeySmooth() {
    return MapFloatFromInt(base_filter_item_helper_->ColorKeySmooth(), kChromaKeySmoothMin, kChromaKeySmoothMax);
}

void SourcePublicFilterPropertyPresenterImp::SetColorKeySmooth(float value) {
    base_filter_item_helper_->ColorKeySmooth(MapFloatToInt(value, kChromaKeySmoothMin, kChromaKeySmoothMax));
}

float SourcePublicFilterPropertyPresenterImp::GetColorKeySpill() {
    return MapFloatFromInt(base_filter_item_helper_->ColorKeySpill(), kChromaKeySpillMin, kChromaKeySpillMax);
}

void SourcePublicFilterPropertyPresenterImp::SetColorKeySpill(float value) {
    base_filter_item_helper_->ColorKeySpill(MapFloatToInt(value, kChromaKeySpillMin, kChromaKeySpillMax));
}

float SourcePublicFilterPropertyPresenterImp::GetColorKeyTransparent() {
    return MapFloatFromInt(base_filter_item_helper_->ColorKeyTransparent());
}

void SourcePublicFilterPropertyPresenterImp::SetColorKeyTransparent(float value) {
    base_filter_item_helper_->ColorKeyTransparent(MapFloatToInt(value));
}

void SourcePublicFilterPropertyPresenterImp::Snapshot() {
    snapshot_ = livehime::ColorKeyPropertySnapshot::NewTake(base_filter_item_helper_.get());
}

void SourcePublicFilterPropertyPresenterImp::Restore() {
    //DCHECK(snapshot_);
    snapshot_->Restore();
}
