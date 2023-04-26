/*
 @ 0xCCCCCCCC
*/

#ifndef OBS_OBS_PROXY_PUBLIC_PROXY_OBS_OUTPUT_SERVICE_H_
#define OBS_OBS_PROXY_PUBLIC_PROXY_OBS_OUTPUT_SERVICE_H_

#include <functional>

#include "base/files/file_path.h"

#include "obs/obs_proxy/public/common/basic_types.h"

namespace obs_proxy {

enum class RecordingErrorCode {
    Unknown,
    Unsupported,
    NoSpace,
    WriteUnable,
    StartFailure,
    Success
};

enum class StreamingErrorCode {
    Unknown,
    BadPath,
    ConnectFailed,
    InvalidStream,
    Disconnected,
    StartFailure,
    InvalidMidAndChannel,
    InvalidMid,
    InvalidChannel,
    InvalidProtocol,
    Success
};

enum class StreamingOutputType {
    Normal,  // ÆÕÍ¨ÍÆÁ÷
};

std::wstring FormatStreamingError(obs_proxy::StreamingErrorCode code);

struct StreamingErrorDetails {
    std::string rtmp_errfmt;
    std::string rtmp_errmsg;
    std::string errmsg;
    std::string rtmp_host_name;
    std::string rtmp_host;
    int64 streaming_timespan = 0;
    int new_link = 0;
    bool custom_streaming = false;
    std::string protocol;
    std::string addr;
    std::string key;
    StreamingOutputType output_type;
    bool quality_abnormal_streaming = false;
};

struct StreamingStopDetails {
    int config_bitrate_a = 0;
    int config_bigrate_v = 0;
    std::string config_fps;
    uint64_t total_bytes = 0;
    int total_frames = 0;
    int frames_dropped = 0;
    int64 duration = 0;
    int64_t encoder_generated_bytes = 0;
    int64_t drop_filler_bytes = 0;
};

class OBSOutputService {
public:
    struct Events {
        using StartRecordingHandler = std::function<void()>;
        using StopRecordingHandler = std::function<void(const base::FilePath& file_path)>;
        using RecordingErrorHandler = std::function<void(RecordingErrorCode error_code,
                                                         const base::FilePath& file_path)>;
        using StreamStartingHandler = std::function<void()>;
        using StreamStoppingHandler = std::function<void()>;
        using StartStreamingHandler = std::function<void()>;
        using StopStreamingHandler = std::function<void(StreamingStopDetails details)>;
        using StreamingErrorHandler = std::function<void(StreamingErrorCode error_code,
            StreamingErrorDetails error_details)>;
        using StreamingFeedbackHandler = std::function<void(const StreamingFeedbackMetrics& metrics)>;
        using EncodedQPFeedbackHandler = std::function<void(int qp)>;
        using PostSendFailingHandler = std::function<void()>;

        StartRecordingHandler start_recording;
        StopRecordingHandler stop_recording;
        RecordingErrorHandler recording_error;
        StreamStartingHandler stream_starting;
        StreamStoppingHandler stream_stopping;
        StartStreamingHandler start_streaming;
        StopStreamingHandler stop_streaming;
        StreamingErrorHandler streaming_error;
        StreamingFeedbackHandler streaming_feedback;
        EncodedQPFeedbackHandler encoded_qp_feedback;
        PostSendFailingHandler  post_send_failing;

    };

    virtual ~OBSOutputService() = default;

    virtual Events& events() = 0;

    virtual bool SetupRemoteServer(const std::string& server_address, const std::string& access_key,
    const std::string protocol) = 0;

    virtual bool StartStreaming() = 0;

    virtual void StopStreaming() = 0;

    virtual void ForceStopStreaming() = 0;

    virtual bool IsStreamingActive() const = 0;

    virtual bool StartRecording() = 0;

    virtual void StopRecording() = 0;

    virtual bool IsRecordingActive() const = 0;

    virtual bool IsOutputActive() const = 0;

    virtual void ReloadEncoders() = 0;

    virtual void RecreateOutputs() = 0;

    virtual std::string GetRtmpHost() = 0;

    virtual int GetCurrentLeastUsedHostCount() = 0;

    virtual int GetOutputTotalFrames() const = 0;

    virtual int GetOutputDroppedFrames() const = 0;

    virtual void SetPublishBitrate(int bitrate) = 0;

    virtual void AddSEIInfo(const uint8_t* data, int size) = 0;
};

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_PUBLIC_PROXY_OBS_OUTPUT_SERVICE_H_
