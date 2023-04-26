#ifndef BILILIVE_BILILIVE_LIVEHIME_VOLUME_SOUND_EFFECT_PROPERTY_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_VOLUME_SOUND_EFFECT_PROPERTY_CONTRACT_H_

#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "base/strings/string16.h"


namespace contracts
{

class SoundEffectPropertyPresenter
{
public:
    struct DeviceInfo 
    {
        base::string16  dev_name;
        std::string     dev_id;
    };

public:
    virtual ~SoundEffectPropertyPresenter() = default;

    virtual void UpdateFilters() = 0;

    virtual void StartPreview() = 0;
    virtual void EndPreview() = 0;

    virtual bool GetIsReverbEnabled() = 0;
    virtual void SetIsReverbEnabled(bool value) = 0;

    virtual void SetAllValueToDefault() = 0;

    virtual float GetRoomSize() = 0;
    virtual void SetRoomSize(float value) = 0;

    virtual float GetDamping() = 0;
    virtual void SetDamping(float value) = 0;

    virtual float GetWetLevel() = 0;
    virtual void SetWetLevel(float value) = 0;

    virtual float GetDryLevel() = 0;
    virtual void SetDryLevel(float value) = 0;

    virtual float GetWidth() = 0;
    virtual void SetWidth(float value) = 0;

    virtual float GetFreezeMode() = 0;
    virtual void SetFreezeMode(float value) = 0;

    virtual long long GetDenoiseLevel() = 0;
    virtual void SetDenoiseLevel(int device_integer) = 0;

    virtual int GetMonoStatus() = 0;
    virtual void SetMonoStatus(const std::string& device_string) = 0;

    virtual void AudioSettingSaveOrCancel(const std::pair<std::string, int>* device_integer,
        const std::pair<std::string, std::string>* device_string) = 0;

    virtual void Snapshot() = 0;
    virtual void Restore() = 0;
};

}   // contracts

#endif  // BILILIVE_BILILIVE_LIVEHIME_VOLUME_SOUND_EFFECT_PROPERTY_CONTRACT_H_