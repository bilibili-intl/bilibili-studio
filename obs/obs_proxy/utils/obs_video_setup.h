#ifndef OBS_OBS_PROXY_UTILS_OBS_VIDEO_SETUP_H_
#define OBS_OBS_PROXY_UTILS_OBS_VIDEO_SETUP_H_

#include <cstdint>
#include <tuple>

class PrefService;
enum video_format;
enum video_colorspace;
enum obs_scale_type;
enum video_range_type;

namespace obs_proxy {

using FPSValue = std::tuple<uint32_t, uint32_t>;        // <numerator, denominator>
using VideoMetrics = std::tuple<uint32_t, uint32_t>;    // <cx, cy>

const char* GetConfiguredRendererModule(PrefService* pref_service);

FPSValue GetConfiguredFPS(PrefService* pref_service);

VideoMetrics GetConfiguredVideoOutputMetrics(PrefService* pref_service);

video_colorspace GetConfiguredVideoColorspace(PrefService* pref_service);

video_range_type GetConfiguredVideoRange(PrefService* pref_service);

video_format GetConfiguredVideoFormat(PrefService* pref_service);

obs_scale_type GetConfiguredScaleType(PrefService* pref_service);

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_UTILS_OBS_VIDEO_SETUP_H_