#ifndef OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_VOLUME_CONTROLLER_IMPL_H_
#define OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_VOLUME_CONTROLLER_IMPL_H_

#include "base/memory/weak_ptr.h"
#include "base/time/time.h"

#include "bilibase/basic_macros.h"

#include "obs/obs-studio/libobs/obs.hpp"

#include "obs/obs_proxy/core_proxy/common/basic_types.h"
#include "obs/obs_proxy/public/proxy/obs_volume_controller.h"

namespace obs_proxy {

// Class `VolumeControllerImpl` internally shares ownership of bound audio source, i.e. source that contains
// volume, on obs-core level.

class VolumeControllerImpl : public VolumeController {
public:
    VolumeControllerImpl(obs_source_t* source_to_bind, const std::string& source_name);

    ~VolumeControllerImpl();

    DISABLE_COPY(VolumeControllerImpl);

    void RegisterVolumeChangedHandler(VolumeChangedHandler handler) override;

    void RegisterVolumeLevelUpdatedHandler(VolumeLevelUpdatedHandler handler) override;

    void RegisterVolumeMuteChangedHandler(VolumeMuteChangedHandler handler) override;

    bool IsMuted() const override;

    void SetMuted(bool muted) override;

    float GetDB() const override;

    void SetDB(float gain_db) override;

    float GetDeflection() const override;

    void SetDeflection(float value) override;

    const std::string& GetBoundSourceName() const override;

    int GetRenderError() const override;

    void SetMonitoringType(int status) override;

    int GetMonitoringType() const override;

    obs_source_t* LeakBoundAudioSource() const
    {
        return bound_audio_source_;
    }

private:
    static void OnVolumeMuteChanged(void* data, calldata_t* calldata);

    static void OnVolumeChanged(void* data, calldata_t* calldata);

    static void OnVolumeLevelUpdated(void* data, const float magnitude[MAX_AUDIO_CHANNELS],
        const float peak[MAX_AUDIO_CHANNELS],
        const float inputPeak[MAX_AUDIO_CHANNELS]);

    static void NotifyVolumeLevelUpdated(const base::WeakPtr<VolumeControllerImpl>& instance, 
        const std::vector<float>& magnitude,
        const std::vector<float>& peak,
        const std::vector<float>& inputPeak);

private:
    base::Time last_volume_updated_time_;
    OBSSource bound_audio_source_;
    std::string bound_source_name_;
    obs_fader_t* fader_;
    obs_volmeter_t* volmeter_;
    VolumeChangedHandler volume_changed_handler_;
    VolumeLevelUpdatedHandler volume_level_updated_handler_;
    SignalHandler<VolumeMuteChangedHandler> volume_mute_changed_signal_handler_;
    SignalHandler<VolumeChangedHandler> volume_changed_signal_handler_;
    base::WeakPtrFactory<VolumeControllerImpl> weak_ptr_factory_;
    base::WeakPtr<VolumeControllerImpl> weak_bound_;
};

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_VOLUME_CONTROLLER_IMPL_H_