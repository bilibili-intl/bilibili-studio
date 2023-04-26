#ifndef OBS_OBS_PROXY_CORE_PROXY_OUTPUT_SERVICES_OUTPUT_FEEDBACK_H_
#define OBS_OBS_PROXY_CORE_PROXY_OUTPUT_SERVICES_OUTPUT_FEEDBACK_H_

#include <deque>

#include "base/timer/timer.h"
#include "base/threading/thread_checker.h"

#include "bilibase/basic_macros.h"

#include "obs/obs-studio/libobs/obs.h"

#include "obs/obs_proxy/core_proxy/common/basic_types.h"

namespace obs_proxy {

struct BandwidthMetrics {
    uint64_t last_bytes_sent = 0;
    uint64_t last_sent_time = 0;
};

class StreamingFeedbacker {
public:
    using FeedbackHandler = std::function<void(const StreamingFeedbackMetrics& metrics)>;

    explicit StreamingFeedbacker(obs_output_t* stream_output);

    ~StreamingFeedbacker();

    DISABLE_COPY(StreamingFeedbacker);

    void RegisterFeedbackHandler(FeedbackHandler handler);

    void StartWatching(uint64_t interval_in_second);

    void StopWatching();

private:
    // 用于记录丢帧信息的结构体
    struct FDRecorder {
        int frames_dropped;
        int total_frames;
        uint64_t timestamp;
    };

    void OnTimerTick();

    StreamingFeedbackMetrics CalculateMetrics();

    // Returns in KB/s.
    double CalculateBandwidthSpeed(uint64_t bytes_sent);

    double CalculateFrameLossRate(int frames_dropped, int total_frames);

    void ResetDataMetrics();

private:
    std::deque<FDRecorder> fd_queue_;
    obs_output_t* stream_output_;   // non-owning.
    BandwidthMetrics bandwidth_metrics_;
    FeedbackHandler feedback_handler_;
    base::RepeatingTimer<StreamingFeedbacker> timer_;
    base::ThreadChecker thread_checker_;
};

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_CORE_PROXY_OUTPUT_HANDLER_OUTPUT_FEEDBACK_H_