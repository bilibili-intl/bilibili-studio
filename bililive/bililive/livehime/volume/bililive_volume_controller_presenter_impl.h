#ifndef BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_VOLUME_CONTROLLER_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_VOLUME_CONTROLLER_PRESENTER_IMPL_H_

#include <vector>

#include "base/basictypes.h"

#include "bililive/bililive/livehime/volume/bililive_volume_controller_contract.h"

class BililiveVolumeControllerPresenterImpl
    : public contracts::BililiveVolumeControllerContract
{
public:
    BililiveVolumeControllerPresenterImpl() = default;

    ~BililiveVolumeControllerPresenterImpl() = default;

    obs_proxy::VolumeController* GetSystemControllerDevice() override;

    obs_proxy::VolumeController* GetMicControllerDevice() override;

private:
    std::vector<obs_proxy::VolumeController*> InitVolumeControllerList();

    DISALLOW_COPY_AND_ASSIGN(BililiveVolumeControllerPresenterImpl);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_VOLUME_CONTROLLER_PRESENTER_IMPL_H_