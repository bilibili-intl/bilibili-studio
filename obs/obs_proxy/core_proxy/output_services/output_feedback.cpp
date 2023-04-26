#include "obs/obs_proxy/core_proxy/output_services/output_feedback.h"

#include "base/ext/bind_lambda.h"
#include "base/logging.h"
#include "base/time/time.h"

#include "bililive/public/bililive/bililive_thread.h"

#include "obs/obs-studio/libobs/util/platform.h"


namespace {
    // 单位为秒
    const int kFrameLossRateQueueLength = 30;
}

namespace obs_proxy {

StreamingFeedbacker::StreamingFeedbacker(obs_output_t* stream_output)
{
	stream_output_ = obs_output_get_ref(stream_output);
    DCHECK(stream_output != nullptr);
}

StreamingFeedbacker::~StreamingFeedbacker()
{
	obs_output_release(stream_output_);
    StopWatching();
}

void StreamingFeedbacker::RegisterFeedbackHandler(FeedbackHandler handler)
{
    DCHECK(thread_checker_.CalledOnValidThread());
    feedback_handler_ = handler;
}

void StreamingFeedbacker::StartWatching(uint64_t interval_in_second)
{
    DCHECK(thread_checker_.CalledOnValidThread());
    timer_.Start(FROM_HERE,
                 base::TimeDelta::FromSeconds(interval_in_second),
                 this,
                 &StreamingFeedbacker::OnTimerTick);
}

void StreamingFeedbacker::StopWatching()
{
    DCHECK(thread_checker_.CalledOnValidThread());
    timer_.Stop();
    ResetDataMetrics();
}

void StreamingFeedbacker::OnTimerTick()
{
    auto feedback_metrics = CalculateMetrics();
    if (feedback_handler_) {
        feedback_handler_(feedback_metrics);
    }
}

StreamingFeedbackMetrics StreamingFeedbacker::CalculateMetrics()
{
    uint64_t bytes_sent = obs_output_get_total_bytes(stream_output_);
    int frames_dropped = obs_output_get_frames_dropped(stream_output_);
    int total_frames = obs_output_get_total_frames(stream_output_);
    return StreamingFeedbackMetrics(CalculateBandwidthSpeed(bytes_sent),
                                    CalculateFrameLossRate(frames_dropped, total_frames),
                                    bytes_sent,
                                    total_frames,
                                    frames_dropped);
}

double StreamingFeedbacker::CalculateBandwidthSpeed(uint64_t bytes_sent)
{
    uint64_t current_time_stamp = os_gettime_ns();

    if (bytes_sent == 0)
    {
        bandwidth_metrics_.last_bytes_sent = 0;
    }

    const double kNS2S = 1000000000.0;
    auto time_span = static_cast<double>(current_time_stamp - bandwidth_metrics_.last_sent_time) / kNS2S;
    auto bytes_per_sec = static_cast<double>(bytes_sent - bandwidth_metrics_.last_bytes_sent) / time_span;

    bandwidth_metrics_.last_bytes_sent = bytes_sent;
    bandwidth_metrics_.last_sent_time = current_time_stamp;

    return bytes_per_sec / 1000.0;
}

double StreamingFeedbacker::CalculateFrameLossRate(int frames_dropped, int total_frames)
{
    DLOG(INFO) << "--------->>>> dropped: " << frames_dropped << " total: " << total_frames;

    // 刚开始推流或断网重连时可能会出现总帧数为0，
    // 这时清掉队列，重新统计
    if (total_frames == 0) {
        fd_queue_.clear();
        return 0.0;
    }

    // 如果发现总帧数回退，则清掉队列，重新统计
    if (!fd_queue_.empty()) {
        if (fd_queue_.back().total_frames > total_frames) {
            fd_queue_.clear();
        }
    }

    // 新数据插入队列
    FDRecorder fd_info;
    fd_info.frames_dropped = frames_dropped;
    fd_info.total_frames = total_frames;
    fd_info.timestamp = ::GetTickCount64();
    fd_queue_.push_back(fd_info);
    if (fd_queue_.size() < 2) {
        return static_cast<double>(frames_dropped) / total_frames * 100.0;
    }

    // 检查队列项是否过期
    auto back_fd = fd_queue_.back();
    auto frone_fd = fd_queue_.front();
    auto time_interval = back_fd.timestamp - frone_fd.timestamp;
    while (time_interval > kFrameLossRateQueueLength * 1000 && fd_queue_.size() > 1) {
        fd_queue_.pop_front();
        frone_fd = fd_queue_.front();
        time_interval = back_fd.timestamp - frone_fd.timestamp;
    }
    if (fd_queue_.size() < 2) {
        return static_cast<double>(frames_dropped) / total_frames * 100.0;
    }

    // 如果发现丢帧数回退，则清掉队列，重新统计
    auto tf_interval = back_fd.total_frames - frone_fd.total_frames;
    auto fd_interval = back_fd.frames_dropped - frone_fd.frames_dropped;
    if (fd_interval < 0) {
        fd_queue_.clear();
        return static_cast<double>(frames_dropped) / total_frames * 100.0;
    }
    if (tf_interval == 0) {
        return static_cast<double>(frames_dropped) / total_frames * 100.0;
    }

    return static_cast<double>(fd_interval) / tf_interval * 100.0;
}

void StreamingFeedbacker::ResetDataMetrics()
{
    fd_queue_.clear();
    bandwidth_metrics_.last_bytes_sent = 0;
    bandwidth_metrics_.last_sent_time = 0;
}

}   // namespace obs_proxy
