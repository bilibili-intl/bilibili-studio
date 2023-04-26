#ifndef BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_VOLUME_CONTROLLERS_DEVICES_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_VOLUME_CONTROLLERS_DEVICES_CONTRACT_H_

#include "base/strings/string16.h"

#include "obs/obs_proxy/public/proxy/obs_audio_devices.h"
#include "obs/obs_proxy/public/proxy/obs_volume_controller.h"

namespace contracts
{

class BililiveVolumeControllersContract
{
public:
    using VolumeChangedHandler = obs_proxy::VolumeController::VolumeChangedHandler;
    using VolumeLevelUpdatedHandler = obs_proxy::VolumeController::VolumeLevelUpdatedHandler;
    using VolumeMuteChangedHandler = obs_proxy::VolumeController::VolumeMuteChangedHandler;

    virtual ~BililiveVolumeControllersContract() = default;

    virtual void SelectController(const string16& controller_name) = 0;

    virtual bool ControllerIsValid() = 0;

    virtual bool GetMuted() = 0;

    virtual void SetMuted(bool muted) = 0;

    virtual float GetDB() const = 0;

    virtual void SetDB(float gain_db) = 0;

    virtual float GetDeflection() const = 0;

    virtual void SetDeflection(float value) = 0;

    virtual string16 GetBoundSourceName() const = 0;

    virtual void RegisterVolumeChangedHandler(VolumeChangedHandler handler) = 0;

    virtual void RegisterVolumeLevelUpdatedHandler(VolumeLevelUpdatedHandler handler) = 0;

    virtual void RegisterVolumeMuteChangedHandler(VolumeMuteChangedHandler handler) = 0;

    virtual int GetRenderError() const = 0;
};

class BililiveAudioDevicesContract
{
public:
    virtual ~BililiveAudioDevicesContract() = default;

    virtual void SelectAudioDevice(const string16 &device_name) = 0;

    virtual obs_proxy::AudioDevice* GetAudioDevice() const = 0;

    virtual bool DeviceIsValid() = 0;

    virtual string16 GetDeviceName() const = 0;

    virtual uint32_t BoundChannelNumber() const = 0;

    virtual obs_proxy::Filter* AddNewFilter(obs_proxy::FilterType type, const string16& name) = 0;

    virtual void RemoveFilter(const string16& name) = 0;

    virtual obs_proxy::Filter* GetFilter(const string16& name) const = 0;

    virtual std::vector<obs_proxy::Filter*> GetFilters() const = 0;

    virtual bool ReorderFilters(const std::vector<obs_proxy::Filter*>& filters) = 0;

    virtual obs_proxy::PropertyValue& GetPropertyValues() = 0;

    virtual void UpdatePropertyValues() = 0;

    virtual const obs_proxy::Properties& GetProperties() const = 0;
};

}   // contracts

#endif  // BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_VOLUME_CONTROLLERS_DEVICES_CONTRACT_H_