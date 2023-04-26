#ifndef BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_VOLUME_CONTROLLER_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_VOLUME_CONTROLLER_CONTRACT_H_

#include "obs/obs_proxy/public/proxy/obs_volume_controller.h"

namespace contracts
{

class BililiveVolumeControllerContract
{
public:
    virtual ~BililiveVolumeControllerContract() = default;

    virtual obs_proxy::VolumeController* GetSystemControllerDevice() = 0;

    virtual obs_proxy::VolumeController* GetMicControllerDevice() = 0;
};

}   // contracts

#endif  // BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_VOLUME_CONTROLLER_CONTRACT_H_