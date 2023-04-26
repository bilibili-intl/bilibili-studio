#include "bililive/bililive/livehime/sources_properties/source_image_property_presenter_impl.h"

#include "base/logging.h"

SourceImagePropertyPresenterImpl::SourceImagePropertyPresenterImpl(obs_proxy::SceneItem* scene_item)
    : contracts::SourceImagePropertyPresenter(scene_item),
      image_scene_item_(scene_item)
{}

string16 SourceImagePropertyPresenterImpl::GetFilePath()
{
    return image_scene_item_.FilePath();
}

void SourceImagePropertyPresenterImpl::SetFilePath(const string16& path)
{
    image_scene_item_.FilePath(path);
}

float SourceImagePropertyPresenterImpl::GetTransparent()
{
    return MapFloatFromInt(image_scene_item_.Transparent());
}

void SourceImagePropertyPresenterImpl::SetTransparent(float value)
{
    image_scene_item_.Transparent(MapFloatToInt(value));
}

void SourceImagePropertyPresenterImpl::RotateLeft()
{
    image_scene_item_.RotateLeft();
}

void SourceImagePropertyPresenterImpl::RotateRight()
{
    image_scene_item_.RotateRight();
}

void SourceImagePropertyPresenterImpl::VerticalFlip()
{
    image_scene_item_.VerticalFlip();
}

void SourceImagePropertyPresenterImpl::HorizontalFlip()
{
    image_scene_item_.HorizontalFlip();
}

void SourceImagePropertyPresenterImpl::Snapshot()
{
    snapshot_ = livehime::ImagePropertySnapshot::NewTake(&image_scene_item_);
}

void SourceImagePropertyPresenterImpl::Restore()
{
    DCHECK(snapshot_);

    snapshot_->Restore();
}