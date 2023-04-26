#ifndef OBS_OBS_PROXY_CORE_PROXY_COMMON_BASIC_TYPES_H_
#define OBS_OBS_PROXY_CORE_PROXY_COMMON_BASIC_TYPES_H_

#include "obs/obs-studio/libobs/obs.hpp"

#include "obs/obs_proxy/public/common/basic_types.h"

// Common types shared among core proxy.

namespace obs_proxy {

template<typename Handler>
using SignalHandler = std::pair<Handler, OBSSignal>;

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_CORE_PROXY_COMMON_BASIC_TYPES_H_