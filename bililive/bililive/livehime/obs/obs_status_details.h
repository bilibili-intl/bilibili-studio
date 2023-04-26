/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_BILILIVE_LIVEHIME_OBS_OBS_STATUS_DETAILS_H_
#define BILILIVE_BILILIVE_LIVEHIME_OBS_OBS_STATUS_DETAILS_H_

#include <string>

#include "base/files/file_path.h"

#include "obs/obs_proxy/public/proxy/obs_proxy.h"
#include "bililive/secret/public/app_service.h"

namespace bililive {

struct RecordingDetails {
    base::FilePath video_path;
    obs_proxy::RecordingErrorCode error_code;
    std::wstring error_msg;

    explicit RecordingDetails(const base::FilePath& path);

    RecordingDetails(const base::FilePath& path,
                     obs_proxy::RecordingErrorCode code,
                     const std::wstring& msg);
};

struct StartStreamingDetails {
    bool is_colive = false; // 本次开始的是连麦推流（用于连麦自推流）
    bool after_colive = false;
    std::string host_name;
    std::string addr;
    std::string code;

    bool quality_abnormal_streaming = false;
    std::string live_key;
    std::string stream_name;
    std::string stream_key;
};

struct LiveStreamingErrorDetails
{
    enum ErrorCategory
    {
        None = 0,
        Local,
        Server
    };

    ErrorCategory category;
    std::wstring error_msg;
    int error_code;

    LiveStreamingErrorDetails(ErrorCategory type, const std::wstring& msg, int code);
};

// 推流质量数据
struct LiveDataDetails {
    int config_bitrate_a = 0;
    int config_bitrate_v = 0;
    std::string config_fps;
    uint64_t total_bytes = 0;
    int total_frames = 0;
    int frames_dropped = 0;
    std::string live_host;
    std::string live_addr;
    std::string protocol;
    int64 duration = 0;
    int64_t encoder_generated_bytes = 0;
    int64_t drop_filler_bytes = 0;
};

struct StreamingMetricsDetails {
    double bandwidth_speed;
    double frame_loss_rate;

    StreamingMetricsDetails(double bandwidth_speed, double frame_loss_rate);
};

}   // namespace bililive

#endif  // BILILIVE_BILILIVE_LIVEHIME_OBS_OBS_STATUS_DETAILS_H_
