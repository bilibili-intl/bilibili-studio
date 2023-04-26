/*
 @ 0xCCCCCCCC
*/

#include "bililive/bililive/livehime/obs/obs_status_details.h"

namespace bililive {

RecordingDetails::RecordingDetails(const base::FilePath& path)
    : video_path(path), error_code(obs_proxy::RecordingErrorCode::Success)
{}

RecordingDetails::RecordingDetails(const base::FilePath& path,
                                   obs_proxy::RecordingErrorCode code,
                                   const std::wstring& msg)
    : video_path(path), error_code(code), error_msg(msg)
{}

LiveStreamingErrorDetails::LiveStreamingErrorDetails(ErrorCategory type, const std::wstring& msg, int code)
    : category(type), error_msg(msg), error_code(code)
{}

StreamingMetricsDetails::StreamingMetricsDetails(double bandwidth_speed, double frame_loss_rate)
    : bandwidth_speed(bandwidth_speed), frame_loss_rate(frame_loss_rate)
{}

}   // namespace bililive
