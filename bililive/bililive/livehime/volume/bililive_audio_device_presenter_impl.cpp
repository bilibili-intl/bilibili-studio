#include "bililive/bililive/livehime/volume/bililive_audio_device_presenter_impl.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"

#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"

std::vector<obs_proxy::AudioDevice*> BililiveAudioDeviceImpl::InitAudioDeviceList()
{
    std::vector<obs_proxy::AudioDevice*> device_list;

    obs_proxy::SceneCollection *scene_collection
        = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    if (scene_collection)
    {
        device_list = scene_collection->GetAudioDevices();
    }

    return device_list;
}

obs_proxy::AudioDevice* BililiveAudioDeviceImpl::GetSystemAudioDevice()
{
    obs_proxy::AudioDevice* system_audio_device = nullptr;
    std::vector<obs_proxy::AudioDevice*> device_list = InitAudioDeviceList();

    auto device = std::find_if(device_list.begin(), device_list.end(),
        [](obs_proxy::AudioDevice* dev)->bool {
        return dev->name() == obs_proxy::kDefaultOutputAudio; });

    if (device != device_list.end())
    {
        system_audio_device = *device;
    }

    return system_audio_device;
}

obs_proxy::AudioDevice* BililiveAudioDeviceImpl::GetMicAudioDevice()
{
    obs_proxy::AudioDevice* mic_audio_device = nullptr;
    std::vector<obs_proxy::AudioDevice*> device_list = InitAudioDeviceList();

    auto device = std::find_if(device_list.begin(), device_list.end(),
        [](obs_proxy::AudioDevice* dev)->bool {
        return dev->name() == obs_proxy::kDefaultInputAudio; });

    if (device != device_list.end())
    {
        mic_audio_device = *device;
    }

    return mic_audio_device;
}