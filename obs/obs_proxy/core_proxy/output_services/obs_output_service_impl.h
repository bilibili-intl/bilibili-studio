/*
 @ 0xCCCCCCCC
*/

#ifndef OBS_OBS_PROXY_CORE_PROXY_OUTPUT_SERVICES_OBS_OUTPUT_SERVICE_IMPL_H_
#define OBS_OBS_PROXY_CORE_PROXY_OUTPUT_SERVICES_OBS_OUTPUT_SERVICE_IMPL_H_

#include <memory>
#include <mutex>

#include "base/prefs/pref_service.h"

#include "bilibase/basic_macros.h"

#include "obs/obs-studio/libobs/obs.hpp"

#include "obs/obs_proxy/core_proxy/output_services/output_feedback.h"
#include "obs/obs_proxy/public/proxy/obs_output_service.h"

namespace obs_proxy {

	OBSOutputService* CreateOBSOutputService();

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_CORE_PROXY_OUTPUT_SERVICES_OBS_OUTPUT_SERVICE_IMPL_H_
