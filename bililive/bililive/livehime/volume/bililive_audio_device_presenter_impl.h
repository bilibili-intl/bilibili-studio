#ifndef BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_AUDIO_DEVICE_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_AUDIO_DEVICE_PRESENTER_IMPL_H_

#include <vector>

#include "base/basictypes.h"

#include "bililive/bililive/livehime/volume/bililive_audio_device_contract.h"

class BililiveAudioDeviceImpl
    : public contracts::BililiveAudioDeviceContract
{
public:
    BililiveAudioDeviceImpl() = default;

    ~BililiveAudioDeviceImpl() = default;

    obs_proxy::AudioDevice* GetSystemAudioDevice() override;

    obs_proxy::AudioDevice* GetMicAudioDevice() override;

private:
    std::vector<obs_proxy::AudioDevice*> InitAudioDeviceList();

    DISALLOW_COPY_AND_ASSIGN(BililiveAudioDeviceImpl);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_AUDIO_DEVICE_PRESENTER_IMPL_H_