#ifndef OBS_OBS_PROXY_PUBLIC_PROXY_OBS_AUDIO_DEVICES_H_
#define OBS_OBS_PROXY_PUBLIC_PROXY_OBS_AUDIO_DEVICES_H_

#include "obs/obs_proxy/public/proxy/obs_source_properties.h"
#include "obs/obs_proxy/public/proxy/obs_source_property_values.h"
#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"

namespace obs_proxy {

// One `AudioDevice` represents an audio source from a specific channel.

extern const char* kDefaultInputAudio;
extern const char* kDefaultOutputAudio;

class AudioDevice {
public:
    virtual ~AudioDevice() {}

    virtual const std::string& name() const = 0;

    virtual uint32_t bound_channel_number() const = 0;

    virtual Filter* AddNewFilter(FilterType type, const std::string& name) = 0;

    virtual void RemoveFilter(const std::string& name) = 0;

    virtual Filter* GetFilter(const std::string& name) const = 0;

    virtual std::vector<Filter*> GetFilters() const = 0;

    virtual bool ReorderFilters(const std::vector<Filter*>& filters) = 0;

    virtual PropertyValue& GetPropertyValues() = 0;

    virtual void UpdatePropertyValues() = 0;

    virtual const Properties& GetProperties() const = 0;

    virtual void StartPreview(bool disable_output) = 0;

    virtual void StopPreview() = 0;
};

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_PUBLIC_PROXY_OBS_AUDIO_DEVICES_H_