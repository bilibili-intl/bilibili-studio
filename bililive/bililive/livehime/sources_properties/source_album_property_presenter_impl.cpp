#include "bililive/bililive/livehime/sources_properties/source_album_property_presenter_impl.h"

#include "base/logging.h"

namespace
{

const int kSlidetimeMin = 0;
const int kSlidetimeMax = 10000;

const int kTransitiontimeMin = 0;
const int kTransitiontimeMax = 5000;

}   // namespace


// SourceAlbumPropertyPresenterImpl
SourceAlbumPropertyPresenterImpl::SourceAlbumPropertyPresenterImpl(
    obs_proxy::SceneItem* scene_item)
    : contracts::SourceAlbumPropertyPresenter(scene_item),
      slide_show_item_(scene_item)
{}

float SourceAlbumPropertyPresenterImpl::GetSlideTime()
{
    return MapFloatFromInt(
        slide_show_item_.SlideTime(),
        kSlidetimeMin,
        kSlidetimeMax);
}

void SourceAlbumPropertyPresenterImpl::SetSlideTime(float value)
{
    slide_show_item_.SlideTime(
        MapFloatToInt(
        value,
        kSlidetimeMin,
        kSlidetimeMax));
}

float SourceAlbumPropertyPresenterImpl::GetTransitionTime()
{
    return MapFloatFromInt(
        slide_show_item_.TransitionTime(),
        kTransitiontimeMin,
        kTransitiontimeMax);
}

void SourceAlbumPropertyPresenterImpl::SetTransitionTime(float value)
{
    slide_show_item_.TransitionTime(
        MapFloatToInt(
        value,
        kTransitiontimeMin,
        kTransitiontimeMax));
}

SlideShowSceneItemHelper::TransitionT SourceAlbumPropertyPresenterImpl::GetTransition()
{
    return slide_show_item_.Transition();
}

void SourceAlbumPropertyPresenterImpl::SetTransition(SlideShowSceneItemHelper::TransitionT value)
{
    slide_show_item_.Transition(value);
}

bool SourceAlbumPropertyPresenterImpl::GetRandom()
{
    return slide_show_item_.Random();
}

void SourceAlbumPropertyPresenterImpl::SetRandom(bool value)
{
    slide_show_item_.Random(value);
}

bool SourceAlbumPropertyPresenterImpl::GetLoop()
{
    return slide_show_item_.Loop();
}

void SourceAlbumPropertyPresenterImpl::SetLoop(bool value)
{
    slide_show_item_.Loop(value);
}

std::vector<std::wstring> SourceAlbumPropertyPresenterImpl::GetFileList()
{
    return slide_show_item_.FileList();
}

void SourceAlbumPropertyPresenterImpl::SetFileList()
{
    slide_show_item_.FileList(file_list_);
}

void SourceAlbumPropertyPresenterImpl::Snapshot()
{
    snapshot_ = livehime::AlbumPropertySnapshot::NewTake(&slide_show_item_);
}

void SourceAlbumPropertyPresenterImpl::Restore()
{
    DCHECK(snapshot_);

    snapshot_->Restore();
}

void SourceAlbumPropertyPresenterImpl::AddFile(const string16& file_path)
{
    file_list_.push_back(file_path);
}

void SourceAlbumPropertyPresenterImpl::RemoveFile(int index)
{
    if (!file_list_.empty() && index >= 0)
    {
        file_list_.erase(file_list_.begin() + index);
    }
}

void SourceAlbumPropertyPresenterImpl::EmptyFileList()
{
    file_list_.clear();
}