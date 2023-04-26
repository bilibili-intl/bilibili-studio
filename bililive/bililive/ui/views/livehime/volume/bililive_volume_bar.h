#pragma once

#include "bililive/bililive/ui/views/livehime/theme/livehime_theme_constants.h"

#include "ui/views/view.h"
#include "obs/obs_proxy/public/proxy/obs_audio_devices.h"
#include "obs/obs_proxy/public/proxy/obs_proxy.h"
#include "obs/obs_proxy/public/proxy/obs_volume_controller.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"


// BililiveVolumeBar
class BililiveVolumeBar
    : public views::View
{
public:
    BililiveVolumeBar();
    virtual ~BililiveVolumeBar();

    void SetVolumeState(bool state);
    void SetMuted(bool muted);

    void VolumeLevelUpdate(const std::string& source_name,
        const std::vector<float>& magnitude,
        const std::vector<float>& peak,
        const std::vector<float>& inputPeak);

protected:
    // views::View
    void OnPaint(gfx::Canvas* canvas) override;
    gfx::Size GetPreferredSize() override;

private:
    bool is_muted_;
    float currentPeak_[MAX_AUDIO_CHANNELS];
    bool volume_state_;

    DISALLOW_COPY_AND_ASSIGN(BililiveVolumeBar);
};

