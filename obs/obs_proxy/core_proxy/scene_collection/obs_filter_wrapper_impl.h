#ifndef OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_FILTER_WRAPPER_IMPL_H_
#define OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_FILTER_WRAPPER_IMPL_H_

#include <string>

#include "bilibase/basic_macros.h"

#include "obs/obs-studio/libobs/obs.h"

#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"

namespace obs_proxy {

class FilterImpl : public Filter {
public:
    FilterImpl(FilterType type, const std::string& name);

    FilterImpl(obs_source_t* raw_filter, FilterType type, const std::string& name);

    ~FilterImpl();

    DISABLE_COPY(FilterImpl);

    const std::string& name() const override;

    FilterType type() const override;

    PropertyValue& GetPropertyValues() override;

    void UpdatePropertyValues() override;

    obs_source_t* LeakUnderlyingSource() const
    {
        return filter_;
    }

private:
    FilterType type_;
    std::string name_;
    obs_source_t* filter_;
    PropertyValue property_values_;
};

const char* FilterTypeToID(FilterType type);

FilterType FilterIDToType(const char* id);

class CreatingFilterError : public std::runtime_error {
public:
    explicit CreatingFilterError(const std::string& msg)
        : runtime_error(msg)
    {}

    explicit CreatingFilterError(const char* msg)
        : runtime_error(msg)
    {}
};

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_FILTER_WRAPPER_IMPL_H_