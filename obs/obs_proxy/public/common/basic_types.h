#ifndef OBS_OBS_PROXY_PUBLIC_COMMON_BASIC_TYPES_H_
#define OBS_OBS_PROXY_PUBLIC_COMMON_BASIC_TYPES_H_

#include <cstdint>

#include "obs/obs_proxy/app/obs_proxy_export.h"

namespace obs_proxy {

struct OBS_PROXY_EXPORT StreamingFeedbackMetrics {
    StreamingFeedbackMetrics(double bandwidth_speed, double frame_loss_rate, uint64_t total_bytes_transmitted,
                             uint64_t total_frames_generated, uint64_t total_frames_dropped)
        : bandwidth_speed(bandwidth_speed),
          frame_loss_rate(frame_loss_rate),
          total_bytes_transmitted(total_bytes_transmitted),
          total_frames_generated(total_frames_generated),
          total_frames_dropped(total_frames_dropped)
    {}

    double bandwidth_speed;
    double frame_loss_rate;
    uint64_t total_bytes_transmitted;
    uint64_t total_frames_generated;
    uint64_t total_frames_dropped;
};

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_PUBLIC_COMMON_BASIC_TYPES_H_