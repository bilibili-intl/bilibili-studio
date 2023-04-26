#ifndef OBS_OBS_PROXY_PUBLIC_PROXY_OBS_VOLUME_CONTROLLER_H_
#define OBS_OBS_PROXY_PUBLIC_PROXY_OBS_VOLUME_CONTROLLER_H_

#include <functional>

#include "obs/obs-studio/libobs/media-io/audio-io.h"

namespace obs_proxy {
const float kPeekRange = 60.0;

static float GetMaxPeek(const float peak[MAX_AUDIO_CHANNELS])
{
    float ret_val = -kPeekRange;
    for (int channel = 0; channel < MAX_AUDIO_CHANNELS; channel++)
    {
        if (!isnan(peak[channel]))
        {
            ret_val = std::max(ret_val, peak[channel]);
        }
    }
    return ret_val;
}
// Each `VolumeController` instance binds with a audio source (i.e. source that contains volume).

extern const float kAudioGain;

class VolumeController {
public:
    using VolumeChangedHandler = std::function<void(const std::string& source_name, float db)>;
    using VolumeLevelUpdatedHandler = std::function<void(const std::string& source_name,
        const std::vector<float>& magnitude,
        const std::vector<float>& peak,
        const std::vector<float>& inputPeak)>;
    using VolumeMuteChangedHandler = std::function<void(const std::string& source_name, bool muted)>;

    virtual ~VolumeController() {}

    virtual void RegisterVolumeChangedHandler(VolumeChangedHandler handler) = 0;

    virtual void RegisterVolumeLevelUpdatedHandler(VolumeLevelUpdatedHandler handler) = 0;

    virtual void RegisterVolumeMuteChangedHandler(VolumeMuteChangedHandler handler) = 0;

    virtual bool IsMuted() const = 0;

    virtual void SetMuted(bool muted) = 0;

    virtual float GetDB() const = 0;

    virtual void SetDB(float gain_db) = 0;

    virtual float GetDeflection() const = 0;

    virtual void SetDeflection(float value) = 0;

    virtual const std::string& GetBoundSourceName() const = 0;

    virtual int GetRenderError() const = 0;

    virtual void SetMonitoringType(int status) = 0;

    virtual int GetMonitoringType() const = 0;
};

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_PUBLIC_PROXY_OBS_VOLUME_CONTROLLER_H_