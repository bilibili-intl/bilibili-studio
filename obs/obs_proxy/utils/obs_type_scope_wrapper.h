#ifndef OBS_OBS_PROXY_UTILS_OBS_TYPE_SCOPE_WRAPPER_H_
#define OBS_OBS_PROXY_UTILS_OBS_TYPE_SCOPE_WRAPPER_H_

#include <memory>

#include "obs/obs-studio/libobs/obs-data.h"
#include "obs/obs-studio/libobs/obs-properties.h"

namespace obs_proxy {

using scoped_obs_data = std::unique_ptr<obs_data_t, decltype(&obs_data_release)>;
using scoped_obs_property = std::unique_ptr<obs_properties_t, decltype(&obs_properties_destroy)>;
using scoped_obs_data_array = std::unique_ptr<obs_data_array_t, decltype(&obs_data_array_release)>;

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_UTILS_OBS_TYPE_SCOPE_WRAPPER_H_