#include "bililive/bililive/livehime/sources_properties/source_audio_device_property_presenter_impl.h"

#include "base/strings/stringprintf.h"
#include "obs/obs_proxy/utils/obs_wrapper_impl_cast.h"

#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"



SourceAudioDevicePropertyPresenterImpl::SourceAudioDevicePropertyPresenterImpl(obs_proxy::SceneItem* scene_item,
    contracts::AudioDevicePropDetailView* audio_device_prop_view)
    : contracts::SourceAudioDevicePropertyPresenter(scene_item)
    , audio_device_scene_item_(scene_item)
    , audio_device_prop_view_(audio_device_prop_view)
{
}

SourceAudioDevicePropertyPresenterImpl::~SourceAudioDevicePropertyPresenterImpl()
{
}



float SourceAudioDevicePropertyPresenterImpl::GetVolumeValue()
{
    return MapFloatFromInt(audio_device_scene_item_.Volume());
}

void SourceAudioDevicePropertyPresenterImpl::SetVolumeValue(float value)
{
    return audio_device_scene_item_.Volume(MapFloatToInt(value));
}


bool SourceAudioDevicePropertyPresenterImpl::IsMuted()
{
    return audio_device_scene_item_.IsMuted();
}

void SourceAudioDevicePropertyPresenterImpl::SetMuted(bool muted)
{
    audio_device_scene_item_.SetMuted(muted);
}


void SourceAudioDevicePropertyPresenterImpl::Snapshot()
{
    snapshot_ = livehime::AudioDevicePropertySnapshot::NewTake(&audio_device_scene_item_);
}

void SourceAudioDevicePropertyPresenterImpl::Restore()
{
    DCHECK(snapshot_);

    snapshot_->Restore();
}

void SourceAudioDevicePropertyPresenterImpl::InitComboData(const std::string& device)
{
    obs_proxy::SceneCollection *scene_collection
        = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    std::vector<obs_proxy::AudioDevice*> vct = scene_collection->GetAudioDevices();

    auto value = std::find_if(vct.begin(), vct.end(), [device](obs_proxy::AudioDevice* dev)->bool { return dev->name() == device; });
    if (value != vct.end()) {
        std::string name;
        const obs_proxy::Properties &props = (*value)->GetProperties();
        for (obs_proxy::Properties::iterator iter = props.begin(); iter != props.end(); ++iter) {
            auto pt = iter->type();
            name = iter->name();//"device_id"
            if ((pt == obs_proxy::PropertyType::List) && (stricmp(name.c_str(), "device_id") == 0)) {
                auto deviceList = GetPropertyStringList(props, name);

                std::wstring devname;
                std::string devid;
                for (unsigned int i = 0; i < deviceList.size(); i++) {
                    devname = std::get<0>(deviceList[i]);
                    devid = std::get<1>(deviceList[i]);

                    audio_device_prop_view_->OnInitComboData(device, devname, devid);
                }
            }
        }
    }
}

std::string SourceAudioDevicePropertyPresenterImpl::GetSelectedAudioDevice()
{
    return audio_device_scene_item_.SelectedAudioDevice();
}

void SourceAudioDevicePropertyPresenterImpl::SetSelectedAudioDevice(std::string val)
{
    audio_device_scene_item_.SelectedAudioDevice(val);
}

bool SourceAudioDevicePropertyPresenterImpl::IsUseDeviceTiming()
{
    return audio_device_scene_item_.IsUseDeviceTiming();
}

void SourceAudioDevicePropertyPresenterImpl::SetUseDeviceTiming(bool use)
{
    audio_device_scene_item_.SetUseDeviceTiming(use);
}

int SourceAudioDevicePropertyPresenterImpl::GetAudioDeviceMonitor()
{
    return audio_device_scene_item_.GetMoinitoringType();
}

void SourceAudioDevicePropertyPresenterImpl::SetAudioDeviceMonitor(int status)
{
    audio_device_scene_item_.SetMonitoringType(status);
}