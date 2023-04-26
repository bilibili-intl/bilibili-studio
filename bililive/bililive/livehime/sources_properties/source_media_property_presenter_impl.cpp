#include "bililive/bililive/livehime/sources_properties/source_media_property_presenter_impl.h"

#include "base/ext/bind_lambda.h"
#include "base/logging.h"

#include "bililive/public/bililive/bililive_thread.h"

#include "obs/obs_proxy/utils/obs_wrapper_impl_cast.h"


namespace {
    const char kMediaCallback[] = "media_callback";
    const char kMediaState[] = "media_state";
}

SourceMediaPropertyPresenterImpl::SourceMediaPropertyPresenterImpl(obs_proxy::SceneItem* scene_item,
    contracts::SourceMediaPropertyView* view)
    : contracts::SourceMediaPropertyPresenter(scene_item),
      media_scene_item_(scene_item),
      view_(view)
{
    canary_.reset(new int(0));
}

SourceMediaPropertyPresenterImpl::~SourceMediaPropertyPresenterImpl()
{
    media_signal_.Disconnect();
}

bool SourceMediaPropertyPresenterImpl::Initialize() {
    auto source = impl_cast(media_scene_item_.GetItem())->AsSource();
    RegisterSignals(source);
    return true;
}

string16 SourceMediaPropertyPresenterImpl::GetFilePath()
{
    return media_scene_item_.FilePath();
}

void SourceMediaPropertyPresenterImpl::SetFilePath(const string16& path)
{
    media_scene_item_.FilePath(path);
}

void SourceMediaPropertyPresenterImpl::SetMP4MotionFilePath(const string16& path)
{
    media_scene_item_.MP4MotionFilePath(path);
}

float SourceMediaPropertyPresenterImpl::GetVolumeValue()
{
    return MapFloatFromInt(media_scene_item_.Volume());
}

void SourceMediaPropertyPresenterImpl::SetVolumeValue(float value)
{
    media_scene_item_.Volume(MapFloatToInt(value));
}

bool SourceMediaPropertyPresenterImpl::GetIsLoop()
{
    return media_scene_item_.IsLoop();
}

void SourceMediaPropertyPresenterImpl::SetIsLoop(bool value)
{
    media_scene_item_.IsLoop(value);
}

void SourceMediaPropertyPresenterImpl::Snapshot()
{
    snapshot_ = livehime::MediaPropertySnapshot::NewTake(&media_scene_item_);
}

void SourceMediaPropertyPresenterImpl::Restore()
{
    DCHECK(snapshot_);

    snapshot_->Restore();
}

bool SourceMediaPropertyPresenterImpl::IsMuted()
{
    return media_scene_item_.IsMuted();
}

void SourceMediaPropertyPresenterImpl::SetMuted(bool muted)
{
    media_scene_item_.SetMuted(muted);
}

void SourceMediaPropertyPresenterImpl::RegisterSignals(const obs_source_t* source)
{
    auto signal_handle = obs_source_get_signal_handler(source);
    media_signal_.Connect(
        signal_handle, kMediaCallback, &SourceMediaPropertyPresenterImpl::OnMediaSignal, this);
}

// static
void SourceMediaPropertyPresenterImpl::OnMediaSignal(void* data, calldata_t* params)
{
    auto This = static_cast<SourceMediaPropertyPresenterImpl*>(data);
    int state = static_cast<int>(calldata_int(params, kMediaState));

    std::weak_ptr<int> weak_self = This->canary_;
    BililiveThread::PostTask(
        BililiveThread::UI,
        FROM_HERE,
        base::BindLambda([weak_self, state, This]
            {
                auto ptr = weak_self.lock();
                if (ptr && This->view_)
                {
                    This->view_->OnMediaStateChanged(contracts::MediaState(state));
                }
            }));
}