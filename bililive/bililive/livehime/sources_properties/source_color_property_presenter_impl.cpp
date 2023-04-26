#include "bililive/bililive/livehime/sources_properties/source_color_property_presenter_impl.h"

#include "base/logging.h"

SourceColorPropertyPresenterImpl::SourceColorPropertyPresenterImpl(obs_proxy::SceneItem* scene_item)
    : contracts::SourceColorPropertyPresenter(scene_item),
    color_scene_item_(scene_item)
{}

void SourceColorPropertyPresenterImpl::SetColor(SkColor value)
{
    color_scene_item_.color(value);
}

SkColor SourceColorPropertyPresenterImpl::GetColor()
{
    return color_scene_item_.color();
}

void SourceColorPropertyPresenterImpl::SetWidth(int width)
{
    color_scene_item_.width(width);
}

int SourceColorPropertyPresenterImpl::GetWidth()
{
    return color_scene_item_.width();
}

void SourceColorPropertyPresenterImpl::SetHeight(int height)
{
    color_scene_item_.height(height);
}

int SourceColorPropertyPresenterImpl::GetHeight()
{
    return color_scene_item_.height();
}

float SourceColorPropertyPresenterImpl::GetTransparent()
{
    return MapFloatFromInt(color_scene_item_.Transparent());
}

void SourceColorPropertyPresenterImpl::SetTransparent(float value)
{
    color_scene_item_.Transparent(MapFloatToInt(value));
}

void SourceColorPropertyPresenterImpl::Snapshot()
{
    snapshot_ = livehime::ColorPropertySnapshot::NewTake(&color_scene_item_);
}

void SourceColorPropertyPresenterImpl::Restore()
{
    DCHECK(snapshot_);

    snapshot_->Restore();
}