#ifndef BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_AUDIO_DEVICE_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_AUDIO_DEVICE_CONTRACT_H_

#include "obs/obs_proxy/public/proxy/obs_audio_devices.h"

namespace contracts
{

class BililiveAudioDeviceContract
{
public:
    virtual ~BililiveAudioDeviceContract() = default;

    virtual obs_proxy::AudioDevice* GetSystemAudioDevice() = 0;

    virtual obs_proxy::AudioDevice* GetMicAudioDevice() = 0;
};

}   // contracts

#endif  // BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_AUDIO_DEVICE_CONTRACT_H_