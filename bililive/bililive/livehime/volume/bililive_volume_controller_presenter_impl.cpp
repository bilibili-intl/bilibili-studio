#include "bililive/bililive/livehime/volume/bililive_volume_controller_presenter_impl.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"

#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"

std::vector<obs_proxy::VolumeController*> BililiveVolumeControllerPresenterImpl::InitVolumeControllerList()
{
    std::vector<obs_proxy::VolumeController*> volume_controller;

    obs_proxy::SceneCollection *scene_collection
        = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    if (scene_collection)
    {
        volume_controller = scene_collection->GetVolumeControllers();
    }

    return volume_controller;
}

obs_proxy::VolumeController* BililiveVolumeControllerPresenterImpl::GetSystemControllerDevice()
{
    obs_proxy::VolumeController* system_controller = nullptr;
    std::vector<obs_proxy::VolumeController*> volume_controller = InitVolumeControllerList();

    auto volume = std::find_if(volume_controller.begin(), volume_controller.end(),
        [](obs_proxy::VolumeController* dev)-> bool {
        return (dev->GetBoundSourceName() == obs_proxy::kDefaultOutputAudio);
    });
    if (volume != volume_controller.end())
    {
        system_controller = *volume;
    }

    return system_controller;
}

obs_proxy::VolumeController* BililiveVolumeControllerPresenterImpl::GetMicControllerDevice()
{
    obs_proxy::VolumeController* mic_controller = nullptr;
    std::vector<obs_proxy::VolumeController*> volume_controller = InitVolumeControllerList();

    auto volume = std::find_if(volume_controller.begin(), volume_controller.end(),
        [](obs_proxy::VolumeController* dev)-> bool {
        return (dev->GetBoundSourceName() == obs_proxy::kDefaultInputAudio);
    });
    if (volume != volume_controller.end())
    {
        mic_controller = *volume;
    }

    return mic_controller;
}