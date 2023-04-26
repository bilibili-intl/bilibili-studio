#ifndef OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_AUDIO_DEVICES_IMPL_H_
#define OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_AUDIO_DEVICES_IMPL_H_

#include <string>

#include "bilibase/basic_macros.h"

#include "obs/obs-studio/libobs/obs.h"

#include "obs/obs_proxy/core_proxy/scene_collection/obs_filter_wrapper_impl.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_source_properties_impl.h"
#include "obs/obs_proxy/public/proxy/obs_audio_devices.h"
#include "obs/obs_proxy/public/proxy/obs_source_property_values.h"

namespace obs_proxy {

class AudioDeviceImpl : public AudioDevice {
public:
    AudioDeviceImpl(obs_source_t* audio_source, uint32_t channel, const std::string& name, bool brand_new);

    ~AudioDeviceImpl();

    DISABLE_COPY(AudioDeviceImpl);

    const std::string& name() const override;

    uint32_t bound_channel_number() const override;

    Filter* AddNewFilter(FilterType type, const std::string& name) override;

    void AddExistingFilter(std::unique_ptr<FilterImpl> filter);

    void RemoveFilter(const std::string& name) override;

    Filter* GetFilter(const std::string& name) const override;

    std::vector<Filter*> GetFilters() const override;

    bool ReorderFilters(const std::vector<Filter*>& filters) override;

    PropertyValue& GetPropertyValues() override;

    void UpdatePropertyValues() override;

    const Properties& GetProperties() const override;

    obs_source_t* LeakUnderlyingSource() const
    {
        return channel_audio_source_;
    }

    void StartPreview(bool disable_output) override;

    void StopPreview() override;

private:
    obs_source_t* channel_audio_source_;
    uint32_t channel_;
    std::string name_;
    std::unique_ptr<PropertiesImpl> properties_;
    PropertyValue property_values_;
    std::vector<std::unique_ptr<FilterImpl>> filters_;
};

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_AUDIO_DEVICES_IMPL_H_