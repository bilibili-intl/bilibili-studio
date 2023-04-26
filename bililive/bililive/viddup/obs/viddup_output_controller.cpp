/*
 @ 0xCCCCCCCC
*/

#include "bililive/bililive/viddup/obs/viddup_output_controller.h"

#include <algorithm>

#include "base/command_line.h"
#include "base/ext/callable_callback.h"
#include "base/logging.h"
#include "base/notification/notification_service.h"
#include "base/prefs/pref_service.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/threading/thread_restrictions.h"

#include "ui/base/resource/resource_bundle.h"

#include "bilibase/basic_types.h"
#include "bilibase/scope_guard.h"

#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/ui/views/login/livehime_login_util.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/public/log_ext/log_constants.h"
#include "bililive/bililive/livehime/event_report/event_report_center.h"

#include "obs/obs_proxy/app/obs_proxy_access_stub.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"
#include "obs/obs_proxy/utils/obs_video_setup.h"

#include "grit/generated_resources.h"


namespace {

using namespace std::placeholders;

const int kNoneServerErrorCode = 0;

const char kMutexRecording[] = "1";

const char* ORTHO_AGORA_APPID = "9d8b280958bd4a2ea4db2364605954e7";
const char* TEST_AGORA_APPID = "c8b5b196c3064c6199557e61b1b27f7c";

const base::TimeDelta kRestartDelayInterval = base::TimeDelta::FromMilliseconds(100);
const base::TimeDelta kRestartDelayIntervalForColive = base::TimeDelta::FromMilliseconds(100);
const base::TimeDelta kRestartLiveStreamingSafePeriod = base::TimeDelta::FromSeconds(5);

void AddErrorMsg(std::vector<std::string>& vec, const std::string& key, const std::string& value) {
    std::string new_value;
    if (!value.empty()) {
        std::string output;
        TrimString(value, ":", &output);
        TrimString(output, ";", &new_value);
    }

    if (!new_value.empty()) {
        std::string format_msg;
        format_msg.append(key).append(":").append(new_value);
        vec.push_back(format_msg);
    }
}

std::wstring LocalStr(int message_id)
{
    return ResourceBundle::GetSharedInstance().GetLocalizedString(message_id);
}

std::wstring FormatRecordingError(obs_proxy::RecordingErrorCode code)
{
    switch (code) {
        case obs_proxy::RecordingErrorCode::Unsupported:
            return LocalStr(IDS_RECORDING_TIP_UNSUPPORTED);

        case obs_proxy::RecordingErrorCode::NoSpace:
            return LocalStr(IDS_RECORDING_TIP_NOSPACE);

        case obs_proxy::RecordingErrorCode::StartFailure:
            return LocalStr(IDS_RECORDING_START_FAILED);

        case obs_proxy::RecordingErrorCode::Unknown:
            return LocalStr(IDS_RECORDING_TIP_UNKNOWN);

        default:
            NOTREACHED() << "Unknown error code";
            return LocalStr(IDS_RECORDING_TIP_UNKNOWN);
    }
}

void SendNotification(bililive::NotificationType notification, base::NotificationDetails details)
{
    auto service = base::NotificationService::current();
    DCHECK(service != nullptr);
    service->Notify(notification, base::NotificationService::AllSources(), details);
}

void SendNotification(bililive::NotificationType notification)
{
    SendNotification(notification, base::NotificationService::NoDetails());
}

// 每一次推流尝试的结果埋点（一次开播流程可能会尝试多次CDN的连接和推流，每次都埋，数据维度更细致）
void ReportLiveStreamingResult(bool succeeded,
    const obs_proxy::StreamingErrorCode streaming_errno,
    const obs_proxy::StreamingErrorDetails& streaming_error_details)
{
    auto code = bilibase::enum_cast(streaming_errno);
    int result = succeeded ? 0 : 1;

    std::vector<std::string> vec_outcome;

    vec_outcome.push_back(base::StringPrintf("result:%d", result));
    vec_outcome.push_back(base::StringPrintf("code:%d", code));
    vec_outcome.push_back(base::StringPrintf("new_link:%d", streaming_error_details.new_link));
    vec_outcome.push_back(base::StringPrintf("output_type:%d", int(streaming_error_details.output_type)));

    vec_outcome.push_back(base::StringPrintf("qas:%d",
        streaming_error_details.quality_abnormal_streaming ? 1 : 0));

    AddErrorMsg(vec_outcome, "rtmp_errfmt", streaming_error_details.rtmp_errfmt);
    AddErrorMsg(vec_outcome, "rtmp_errmsg", streaming_error_details.rtmp_errmsg);
    AddErrorMsg(vec_outcome, "errmsg", streaming_error_details.errmsg);
    AddErrorMsg(vec_outcome, "rtmp_host_name", streaming_error_details.rtmp_host_name);
    AddErrorMsg(vec_outcome, "rtmp_host", streaming_error_details.rtmp_host);
    AddErrorMsg(vec_outcome, "protocol", streaming_error_details.protocol);
    AddErrorMsg(vec_outcome, "streamname", streaming_error_details.key);

    std::string outcome = JoinString(vec_outcome, ";");

    auto secret_core = GetBililiveProcess()->secret_core();
    auto mid = secret_core->account_info().mid();
    secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
        secret::LivehimeBehaviorEvent::LivehimeQuicStreaming, mid, outcome).Call();
}

// 推流质量埋点
void ReportLiveDataDetails(const bililive::LiveDataDetails& details)
{
    std::vector<std::string> vec_livedata;

    vec_livedata.push_back(base::StringPrintf("config_bitrate_a:%d", details.config_bitrate_a));
    vec_livedata.push_back(base::StringPrintf("config_bitrate_v:%d", details.config_bitrate_v));
    vec_livedata.push_back(base::StringPrintf("config_fps:%s", details.config_fps.c_str()));
    vec_livedata.push_back(base::StringPrintf("output_frames:%d", details.total_frames));
    vec_livedata.push_back(base::StringPrintf("output_bytes:%lld", details.total_bytes));
    vec_livedata.push_back(base::StringPrintf("drop_frames:%d", details.frames_dropped));
    vec_livedata.push_back(base::StringPrintf("livehost:%s", details.live_host.c_str()));
    vec_livedata.push_back(base::StringPrintf("liveaddr:%s", details.live_addr.c_str()));
    vec_livedata.push_back(base::StringPrintf("duration:%lld", details.duration));
    vec_livedata.push_back(base::StringPrintf("protocol:%s", details.protocol.c_str()));
    vec_livedata.push_back(base::StringPrintf("encoder_generated_bytes:%lld", details.encoder_generated_bytes));
    vec_livedata.push_back(base::StringPrintf("drop_filler_bytes:%lld", details.drop_filler_bytes));

    std::string live_data(JoinString(vec_livedata, ";"));
    auto secret_core = GetBililiveProcess()->secret_core();
    auto mid = secret_core->account_info().mid();
    secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
        secret::LivehimeBehaviorEvent::LivehimeLiveData, mid, live_data).Call();
}

}   // namespace


namespace bililive {

ViddupOutputController::ViddupOutputController()
    : obs_core_(nullptr),
      recording_status_(RecordingStatus::Ready),
      streaming_status_(LiveStreamingStatus::Ready),
      colive_streaming_status_(ColiveStreamingStatus::Ready),
      recording_while_streaming_(false),
      restart_count_down_(0),
      self_colive_id_(0),
      remote_colive_id_(0),
      error_code_(obs_proxy::StreamingErrorCode::Unknown) {
}

ViddupOutputController::~ViddupOutputController()
{}

// static
ViddupOutputController* ViddupOutputController::GetInstance()
{
    return Singleton<ViddupOutputController>::get();
}

bool ViddupOutputController::IsStreamProtocolSupported(const std::string& protocol)
{
    std::string prtc = StringToLowerASCII(protocol);
    static const std::vector<std::string> kSupportStreamProtocols{
        "rtmp",
        "quic",
        "srt"
    };

    return kSupportStreamProtocols.end() != std::find(kSupportStreamProtocols.begin(), kSupportStreamProtocols.end(), prtc);
}

void ViddupOutputController::Bind(obs_proxy::OBSCoreProxy* core)
{
    DCHECK(core);

    bind_obs_core(core);

    auto& obs_output_events = obs_core()->GetOBSOutputService()->events();

    obs_output_events.start_recording = std::bind(&ViddupOutputController::OnStartRecording, this);
    obs_output_events.stop_recording = std::bind(&ViddupOutputController::OnStopRecording, this, _1);
    obs_output_events.recording_error = std::bind(&ViddupOutputController::OnRecordingError, this, _1, _2);

    obs_output_events.start_streaming = std::bind(&ViddupOutputController::OnStartStreaming, this);
    obs_output_events.stop_streaming = std::bind(&ViddupOutputController::OnStopStreaming, this, _1);
    obs_output_events.streaming_error = std::bind(&ViddupOutputController::OnStreamingError, this, _1, _2);

    obs_output_events.streaming_feedback = std::bind(&ViddupOutputController::OnStreamingFeedback, _1);
    obs_output_events.encoded_qp_feedback = std::bind(&ViddupOutputController::OnStreamEncodedQPFeedback, _1);

    obs_output_events.post_send_failing = std::bind(&ViddupOutputController::OnPostSendFailing, this);
}

void ViddupOutputController::UnBind()
{
    auto& obs_output_events = obs_core()->GetOBSOutputService()->events();

    obs_output_events.start_recording = {};
    obs_output_events.stop_recording = {};
    obs_output_events.recording_error = {};

    obs_output_events.start_streaming = {};
    obs_output_events.stop_streaming = {};
    obs_output_events.streaming_error = {};

    obs_output_events.streaming_feedback = {};
    obs_output_events.encoded_qp_feedback = {};

    obs_output_events.post_send_failing = {};

    unbind_obs_core();
}

bool ViddupOutputController::IsCodecReady() const
{
    return !obs_core()->GetOBSOutputService()->IsOutputActive();
}

void ViddupOutputController::StartRecording()
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    LOG(INFO) << "[OC] Start recording, rs: " << int(recording_status_);

    if (!IsRecordingReady()) {
        NOTREACHED();
        return;
    }

    set_recording_status(RecordingStatus::Starting);

    SendNotification(bililive::NOTIFICATION_LIVEHIME_RECORDING_STARTING);

    if (!obs_core()->GetOBSOutputService()->StartRecording()) {
        OnRecordingError(obs_proxy::RecordingErrorCode::StartFailure, base::FilePath());
    }
}

void ViddupOutputController::StopRecording()
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    LOG(INFO) << "[OC] Stop recording, rs: " << int(recording_status_);

    if (!IsRecording()) {
        NOTREACHED();
        return;
    }

    set_recording_status(RecordingStatus::Stopping);

    SendNotification(bililive::NOTIFICATION_LIVEHIME_RECORDING_STOPPING);

    obs_core()->GetOBSOutputService()->StopRecording();
}

void ViddupOutputController::OnStartRecording()
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    LOG(INFO) << "[OC] OnStartRecording, prev_rs: " << int(recording_status_);

    set_recording_status(RecordingStatus::Recording);

    SendNotification(bililive::NOTIFICATION_LIVEHIME_START_RECORDING);
}

void ViddupOutputController::OnStopRecording(const base::FilePath& video_path)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    LOG(INFO) << "[OC] OnStopRecording, prev_rs: " << int(recording_status_);

    set_recording_status(RecordingStatus::Ready);

    RecordingDetails details(video_path);
    SendNotification(bililive::NOTIFICATION_LIVEHIME_STOP_RECORDING,
                        base::Details<RecordingDetails>(&details));

    if (restart_mode_.IsRecordingSet()) {
        DCHECK(restart_count_down_ > 0);
        if (--restart_count_down_ == 0) {
            DoRestart();
        }
    }
}

void ViddupOutputController::OnRecordingError(obs_proxy::RecordingErrorCode error_code,
    const base::FilePath& video_path)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    LOG(INFO) << "[OC] OnRecordingError, prev_rs: " << int(recording_status_);

    restart_mode_.ClearRecording();

    set_recording_status(RecordingStatus::Ready);

    RecordingDetails details(video_path, error_code, FormatRecordingError(error_code));
    SendNotification(bililive::NOTIFICATION_LIVEHIME_RECORDING_ERROR,
                        base::Details<RecordingDetails>(&details));
}

void ViddupOutputController::StartStreaming(const secret::LiveStreamingService::StartLiveInfo& start_live_info)
{
    LOG(INFO) << "[OC] Start streaming, ss: " << int(streaming_status_)
        << ", css: " << int(colive_streaming_status_);

    start_live_info_ = start_live_info;
    studio_streaming_ = (start_live_info_.room_type == (int)StartLiveRoomType::Studio);
    retry_count_ = 0;
    new_link_mode_ = false;

    IgniteStreamUntilAddrDetermined(false);
}

void ViddupOutputController::StopStreaming()
{
    LOG(INFO) << "[OC] Stop streaming, ss: " << int(streaming_status_)
        << ", css: " << int(colive_streaming_status_);

    StopStreamingInternal(false);
}

void ViddupOutputController::OnStartStreaming()
{

    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    LOG(INFO) << "[OC] OnStartStreaming, ss: " << int(streaming_status_)
        << ", css: " << int(colive_streaming_status_);

    if (trans_to_stop_live_) {
        /**
         * 调 TransToStopLive 时，如果当前推流还没推上，就标记一下，这里断流
         */
        trans_to_stop_live_ = false;
        set_streaming_status(LiveStreamingStatus::Streaming);
        StopStreaming();
        return;
    }

    StartStreamingDetails details;
    details.host_name = saved_addresses_.server_address;
    details.addr = saved_addresses_.cdn_address;
    details.code = saved_addresses_.cdn_key;
    details.stream_name = saved_addresses_.stream_name;
    details.stream_key = saved_addresses_.stream_key;
    details.live_key = saved_addresses_.live_key;
    details.quality_abnormal_streaming = quality_abnormal_streaming_;

    obs_proxy::StreamingErrorDetails streaming_error_details;
    streaming_error_details.new_link = (new_link_mode_ && !saved_addresses_.new_link.empty()) ? 1 : 0;
    streaming_error_details.rtmp_host = obs_core()->GetOBSOutputService()->GetRtmpHost();
    streaming_error_details.rtmp_host_name = saved_addresses_.server_address;
    streaming_error_details.custom_streaming = studio_streaming_;
    streaming_error_details.protocol = saved_addresses_.protocol;
    streaming_error_details.addr = saved_addresses_.cdn_address;
    streaming_error_details.key = saved_addresses_.cdn_key;
    streaming_error_details.output_type = obs_proxy::StreamingOutputType::Normal;
    streaming_error_details.quality_abnormal_streaming = quality_abnormal_streaming_;

    quality_abnormal_streaming_ = false;

    if (streaming_status_ == LiveStreamingStatus::IgnitingStreamAfterColive)
    {
        set_streaming_status(LiveStreamingStatus::Streaming);
        details.after_colive = true;
    }
    else
    {
        set_streaming_status(LiveStreamingStatus::Streaming);
        details.after_colive = false;

        // 推流成功埋点
        ReportLiveStreamingResult(true, obs_proxy::StreamingErrorCode::Success, streaming_error_details);
    }

    // 通知上层推流成功，上层应该在此回调处埋点开播成功
    if (delegate_)
    {
        delegate_->OnStreamingStarted(details, streaming_error_details);
    }

    // 开播时同步录制
    recording_while_streaming_ =
        GetBililiveProcess()->profile()->GetPrefs()->GetBoolean(prefs::kOutputRecordingSync);
    if (recording_while_streaming_ && IsRecordingReady())
    {
        StartRecording();
    }
}

void ViddupOutputController::OnStreamingError(obs_proxy::StreamingErrorCode error_code,
    obs_proxy::StreamingErrorDetails error_details)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    LOG(WARNING) << "[OC] OnStreamingError! ss: " << int(streaming_status_)
        << ", css: " << int(colive_streaming_status_)
        << ", mode: " << restart_mode_.to_string();

    trans_to_stop_live_ = false;

    error_details.new_link = (new_link_mode_ && !saved_addresses_.new_link.empty()) ? 1 : 0;
    error_details.rtmp_host = obs_core()->GetOBSOutputService()->GetRtmpHost();
    error_details.rtmp_host_name = saved_addresses_.server_address;
    error_details.custom_streaming = studio_streaming_;
    //此处rtmp_host_name取值为new_link前的server_address，是因为在newlink生效的前提下，name和host实际上都是host
    //同时从ip地址查询到域名又比较困难，所以这里保留域名。
    error_details.protocol = saved_addresses_.protocol;
    error_details.addr = saved_addresses_.cdn_address;
    error_details.key = saved_addresses_.cdn_key;
    error_details.output_type = obs_proxy::StreamingOutputType::Normal; /*(streaming_status_ == LiveStreamingStatus::IgnitingStreamAfterColive)
        ? obs_proxy::StreamingOutputType::Colive : obs_proxy::StreamingOutputType::Normal;*/
    error_details.quality_abnormal_streaming = quality_abnormal_streaming_;

    // 连麦之前先断流，断流可能失败，不管，直接拉起连麦流
    if (DoRestartAfterStreamingStoppedIfNeeded()) {
        return;
    }

    if (error_code == obs_proxy::StreamingErrorCode::ConnectFailed ||
        error_code == obs_proxy::StreamingErrorCode::InvalidStream ||
        error_code == obs_proxy::StreamingErrorCode::InvalidProtocol)
    {
        // 推流失败埋点
        ReportLiveStreamingResult(false, error_code, error_details);

        // 如果这次失败是从普通地址切换到推流检测异常推荐的地址，那么重新回到原先在用的普通地址
        if (quality_abnormal_streaming_)
        {
            LOG(INFO) << app_log::kLogStreamPushurlChange << "from quality_abnormal recommand fallback to before.";
            // 以旧的被判定为异常的地址重推
            quality_abnormal_streaming_ = false;
            saved_addresses_ = last_normal_address_;
            StartStreamingInternal(saved_addresses_.cdn_address, saved_addresses_.cdn_key, saved_addresses_.protocol);

            return;
        }

        if ((streaming_status_ == LiveStreamingStatus::IgnitingStreamAfterColive ||
            (streaming_status_ == LiveStreamingStatus::IgnitingStream && retry_count_after_colive_ > 0)) &&
            retry_count_after_colive_ < 5)
        {
            retry_timer_.Start(
                FROM_HERE, base::TimeDelta::FromSeconds(2),
                base::Bind(&ViddupOutputController::OnRetryStreamingTimer, base::Unretained(this)));
            ++retry_count_after_colive_;
            return;
        }

        if (retry_count_ < static_cast<int>(start_live_info_.protocol_list.size() - 1))
        {
            // 如果失败时用的是newlink，那么此次重试先fallback到当前地址项的addr，不急着试下一条地址项
            if (!new_link_mode_)
            {
                retry_count_++;

                LOG(INFO) << "[OC] Streaming failed, last addr is dns server, try next addr item " << retry_count_;
                LOG(INFO) << app_log::kLogStreamPushurlChange << "last dns push failed, try next addr item " << retry_count_;
            }
            else
            {
                LOG(INFO) << "[OC] Streaming failed, last addr is newlink addr, "
                    "fallback to current addr item's dns server " << retry_count_;
                LOG(INFO) << app_log::kLogStreamPushurlChange <<
                    "last newlink push failed, fallback to current addr item's dns server " << retry_count_;
            }
            RetryStreaming();
            return;
        }
    }

    set_streaming_status(LiveStreamingStatus::ClosingLiveRoom);
    StreamingAnnihilated(error_code, error_details);
}

void ViddupOutputController::OnStopStreaming(obs_proxy::StreamingStopDetails stop_details)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    LOG(INFO) << "[OC] OnStopStreaming! ss: " << int(streaming_status_)
        << ", css: " << int(colive_streaming_status_)
        << ", mode: " << restart_mode_.to_string();

    trans_to_stop_live_ = false;

    if (DoRestartAfterStreamingStoppedIfNeeded()) {
        return;
    }

    if (recording_while_streaming_ && IsRecording())
    {
        StopRecording();
    }

    // 如果是主动断开重连引发的断流操作，那么两种情况：
    // 1、如果需要关播重开（推流线路改了），那么走关播逻辑，重连操作放在关播回调中进行；
    // 2、如果仅需断流重推（本地推流参数变更），那么不走关播逻辑，直接本层重连即可；
    // ※ 当前startlive接口即使是直播间已处于开播状态也是可以调用成功的，所以旧有的逻辑
    //    在第1种重连的情况下不是先关房间再开房间的而是直接重调以便startlive，惊呆了，
    //    v3.12还是继续保持这个逻辑不变
    if (!restart_mode_.IsJustRestartStreamSet() && !restart_mode_.IsUseSpecialAddrSet())
    {
        set_streaming_status(LiveStreamingStatus::ClosingLiveRoom);
    } else
    {
        set_streaming_status(LiveStreamingStatus::Ready);
    }

    // 通知断流关房间，需要关播重连的话在直播间关闭回调InformLiveRoomClosed中进行重连操作
    StreamingAnnihilated(obs_proxy::StreamingErrorCode::Success, {});

    int64_t encoder_generated_bytes = 0;
    int64_t drop_filler_bytes = 0;
    // 推流质量埋点
    LiveDataDetails live_data;
    live_data.config_bitrate_a = stop_details.config_bitrate_a;
    live_data.config_bitrate_v = stop_details.config_bigrate_v;
    live_data.config_fps = stop_details.config_fps;
    live_data.live_host = saved_addresses_.server_address;
    live_data.live_addr = obs_core()->GetOBSOutputService()->GetRtmpHost();
    live_data.frames_dropped = stop_details.frames_dropped;
    live_data.total_bytes = stop_details.total_bytes;
    live_data.total_frames = stop_details.total_frames;
    live_data.duration = stop_details.duration;
    live_data.encoder_generated_bytes = stop_details.encoder_generated_bytes;
    live_data.drop_filler_bytes = stop_details.drop_filler_bytes;
    live_data.protocol = saved_addresses_.protocol;
    ReportLiveDataDetails(live_data);
}

void ViddupOutputController::OnStreamingFeedback(const obs_proxy::StreamingFeedbackMetrics& metrics)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    StreamingMetricsDetails details(metrics.bandwidth_speed, metrics.frame_loss_rate);

    SendNotification(bililive::NOTIFICATION_LIVEHIME_STREAMING_FEEDBACK,
        base::Details<StreamingMetricsDetails>(&details));
}

void ViddupOutputController::OnStreamEncodedQPFeedback(int qp)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    SendNotification(bililive::NOTIFICATION_LIVEHIME_STREAM_ENCODED_QP, base::Details<int>(&qp));
}

void ViddupOutputController::RetryStreaming() {
    if (delegate_) {
        delegate_->OnRetryStreaming(retry_count_, start_live_info_.protocol_list.size());
    }
    DetermineStreamingAddr();
}

void ViddupOutputController::OnRetryStreamingTimer() {
    // 校验状态
    if (streaming_status_ == LiveStreamingStatus::IgnitingStreamAfterColive ||
        streaming_status_ == LiveStreamingStatus::IgnitingStream)
    {
        RetryStreaming();
    }
}

void ViddupOutputController::IgniteStreamUntilAddrDetermined(bool restarted)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    LOG(INFO) << "[OC] Ignite LiveStreaming, ss: " << int(streaming_status_)
        << ", css: " << int(colive_streaming_status_);

    if (!IsStreamingReady())
    {
        NOTREACHED();
        return;
    }

    if(delegate_)
    {
        delegate_->OnPreStartStreaming(restarted);
    }

    // 开播是由重推流程发起的要重新调开播接口拿新的地址信息
    if (!restarted)
    {
        set_streaming_status(LiveStreamingStatus::OpeningLiveRoom);

        // 直接以当前缓存的地址信息进行推流
        DetermineStreamingAddr();
    }
    else
    {
        // 重新获取开播地址，其实就是要求上层重新发起开播流程
        if (delegate_)
        {
            LOG(INFO) << "[OC] Restart stream need new stream addr info.";

            delegate_->NeedNewStreamingAddrInfos();
        }
    }
}

void ViddupOutputController::IgniteStream(bool after_colive)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    LOG(INFO) << "[OC] Ignite stream, ss: " << int(streaming_status_)
        << ", css: " << int(colive_streaming_status_) << ", after_colive: " << after_colive;

    if (!IsStreamingReady())
    {
        NOTREACHED();
        return;
    }

    if (after_colive)
    {
        set_streaming_status(LiveStreamingStatus::IgnitingStreamAfterColive);
    }
    else
    {
        set_streaming_status(LiveStreamingStatus::IgnitingStream);
    }

    if (delegate_)
    {
        delegate_->OnPreStartStreaming(true);
    }

    StartStreamingInternal(saved_addresses_.cdn_address, saved_addresses_.cdn_key, saved_addresses_.protocol);
}

void ViddupOutputController::IgniteStreamBySpecialAddr(const std::string& server_addr, const std::string& server_key, const std::string& protocol)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    LOG(INFO) << "[OC] Ignite stream by special addr, ss: " << int(streaming_status_)
        << ", css: " << int(colive_streaming_status_);

    if (!IsStreamingReady())
    {
        NOTREACHED();
        return;
    }

    set_streaming_status(LiveStreamingStatus::IgnitingStream);

    if (delegate_)
    {
        delegate_->OnPreStartStreaming(true);
    }

    // 把给的参数记一下
    quality_abnormal_streaming_ = true;
    last_normal_address_ = saved_addresses_;

    saved_addresses_.cdn_address = server_addr;
    saved_addresses_.cdn_key = server_key;
    saved_addresses_.server_address = server_addr;
    saved_addresses_.server_key = server_key;
    saved_addresses_.new_link = "";
    saved_addresses_.protocol = protocol;

    StartStreamingInternal(server_addr, server_key, protocol);
}

void ViddupOutputController::IgniteStreamUntilSpeedTest()
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    LOG(INFO) << "[OC] Ignite StreamUntilSpeedTest, ss: " << int(streaming_status_)
        << ", css: " << int(colive_streaming_status_);

    if (!IsStreamingReady())
    {
        NOTREACHED();
        return;
    }

    set_streaming_status(LiveStreamingStatus::WaitForSpeedTestResult);

    if (delegate_)
    {
        delegate_->OnPreStartStreaming(true);
    }

    // 需要等测速结果出炉才能重推流的场景，这里就不主动重推了，由上层LivehimeLiveRoomController发起，
    // 上层测速完毕之后会通过InformSpeedTestCompleted对本层进行通知
    if (delegate_)
    {
        LOG(INFO) << "[OC] Start stream need speed test.";

        delegate_->NeedSpeedTest();
    }
}

void ViddupOutputController::DetermineStreamingAddr()
{
    // 正常情况不会出现进入到这里时开播地址列表是空的情况，当出现这种情况时应该是业务流程上直接跳过了常规的前置流程，
    // 在这里直接日志记录就好了，不要按照常规流程去流转到下一步了。
    if (start_live_info_.protocol_list.empty())
    {
        LOG(ERROR) << "[OC] Start live info is empty, it seem that workflow run in the wrong way.";
        return;
    }

    //if (!new_link_mode_)
    //{
    //    LOG(INFO) << "[OC] Try to get newlink addr, index = " << retry_count_ << "/" << start_live_info_.protocol_list.size();

    //    // 将即将要进行连接的地址信息回传给上层处理、确认，上层确认就是用这个地址推流就通过
    //    // OnDetermineStreamingAddr回调确认
    //    auto live_info = start_live_info_.protocol_list[retry_count_];
    //    using namespace std::placeholders;
    //    auto callback = std::bind(&ViddupOutputController::OnDetermineStreamingAddr, this, _1, _2, _3, _4);
    //    LivehimeLiveRoomController::GetNewLinkAddr(live_info, callback);
    //}
    //else
    //{
        LOG(INFO) << "[OC] Fallback to original server, index = " << retry_count_ << "/" << start_live_info_.protocol_list.size();

        auto live_info = start_live_info_.protocol_list[retry_count_];
        std::string server_addr = live_info.addr;
        std::string server_key = live_info.key;
        OnDetermineStreamingAddr(server_addr, server_key, false, live_info);
    //}
}

void ViddupOutputController::OnDetermineStreamingAddr(
    const std::string& cdn_server_addr, const std::string& cdn_server_key, bool new_link_mode,
    const secret::LiveStreamingService::ProtocolInfo& live_info)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    // Reset to default line.
    auto secret = GetBililiveProcess()->secret_core();
    secret->anchor_info().set_cdn_checked(1);

    set_streaming_status(LiveStreamingStatus::IgnitingStream);

    // 记下当前这个地址是newlink还是addr，为随后可能发生的newlink推流失败时fallback到addr提供判断依据
    new_link_mode_ = new_link_mode;

    static const StreamingAddresses empty_addr;
    saved_addresses_ = empty_addr;

    saved_addresses_.cdn_address = cdn_server_addr;
    saved_addresses_.cdn_key = cdn_server_key;
    saved_addresses_.server_address = live_info.addr;
    saved_addresses_.server_key = live_info.key;
    saved_addresses_.new_link = live_info.new_link;
    saved_addresses_.protocol = live_info.protocol;
    saved_addresses_.live_key = live_info.live_key;

    // 从接口数据code中分离流名和key
    base::StringPairs sp;
    if (base::SplitStringIntoKeyValuePairs(cdn_server_key, '=', '&', &sp))
    {
        auto streamname = std::find_if(sp.begin(), sp.end(), [](const std::pair<std::string, std::string>& p)->bool{
            return (p.first.find("streamname") != std::string::npos);
        });
        if (streamname != sp.end())
        {
            saved_addresses_.stream_name = streamname->second;
        }

        auto key = std::find_if(sp.begin(), sp.end(), [](const std::pair<std::string, std::string>& p)->bool {
            return (p.first == "key");
        });
        if (key != sp.end())
        {
            saved_addresses_.stream_key = key->second;
        }
    }

    StartStreamingInternal(cdn_server_addr, cdn_server_key, live_info.protocol);
}

void ViddupOutputController::StartStreamingInternal(
    const std::string& server_addr, const std::string& server_key, const std::string& protocol)
{
    DCHECK(!server_addr.empty());
    LOG(INFO) << "[OC] Start stream immediately, code: " << server_key << ", prtc: " << protocol;
    LOG(INFO) << app_log::kLogStreamStart << "addr:" << server_addr << "; prtc:" << protocol;

    std::string prtc = StringToLowerASCII(protocol);
    if (IsStreamProtocolSupported(protocol))
    {
        bool success = obs_core()->GetOBSOutputService()->SetupRemoteServer(server_addr, server_key, prtc) &&
            obs_core()->GetOBSOutputService()->StartStreaming();

        if (!success)
        {
            OnStreamingError(obs_proxy::StreamingErrorCode::StartFailure, {});
        }
    }
    else
    {
        LOG(WARNING) << "[OC] Start stream immediately failed, not support protocol: " << protocol;
        OnStreamingError(obs_proxy::StreamingErrorCode::InvalidProtocol, {});
    }
}

void ViddupOutputController::StopStreamingInternal(bool restart)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    LOG(INFO) << "[OC] Stop streaming immediately, ss: " << (int)streaming_status_
        << ", css: " << int(colive_streaming_status_)
        << ", mode: " << restart_mode_.to_string()
        << ", spc: " << is_self_publish_for_colive_
        << ", lfc: " << is_landscape_for_colive_
        << ", tfsppc: " << trans_from_spp_colive_
        << "restart:" << restart;

    if (!IsStreaming())
    {
        NOTREACHED();
        return;
    }

    set_streaming_status(LiveStreamingStatus::ExtinguishingStream);

    if (delegate_)
    {
        delegate_->OnPreStopStreaming(restart);
    }

    obs_core()->GetOBSOutputService()->StopStreaming();
}

void ViddupOutputController::StreamingAnnihilated(obs_proxy::StreamingErrorCode streaming_errno,
    const obs_proxy::StreamingErrorDetails& streaming_error_details)
{
    LOG(INFO) << "[OC] Stream annihilated, error: " << int(streaming_errno)
        << ", css: " << int(colive_streaming_status_)
        << ", mode: " << restart_mode_.to_string();

    // 推流错误导致的断流就不要保持当前的推流参数（主要是推流地址）继续进行重开播了
    if (streaming_errno != obs_proxy::StreamingErrorCode::Success)
    {
        LOG(INFO) << "[OC] Prev stream error occurs: " << int(streaming_errno)
            << ", clear restart mode: " << restart_mode_.to_string();

        restart_mode_.ClearRestartStreamBits();
    }

    // 断流不需要关房间有两种情况：
    // 1、需要重推流，无论是重推流还是重开播（当前重开播也是直接重新调开播接口，不调关播接口的，orz）；
    // 2、演播厅状态，因为演播厅状态下自己的直播间是非开播状态的；
    bool should_close_live_room = true;
    bool restreaming = restart_mode_.IsRestartStreamBitsSet();
    if (restreaming)
    {
        should_close_live_room = false;
    }
    else
    {
        should_close_live_room = !studio_streaming_;
    }

    // 通知上层，流已然断绝，上层应该关闭直播间了（当前只是流断了），
    // 上层应该在切实关播了之后再主动通知ViddupOutputController::InformLiveRoomClosed重置状态
    OutputControllerDelegate::StreamingErrorInfos infos{ streaming_errno, streaming_error_details,
        should_close_live_room, restreaming, restart_mode_.IsStreamUntilSpeedTestSet() };
    if (delegate_)
    {
        delegate_->OnStreamingStopped(infos);
    }
}

void ViddupOutputController::Restart(LiveRestartMode mode)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    LOG(INFO) << "[OC] Restart! ss: " << int(streaming_status_)
        << ", rs: " << int(recording_status_)
        << ", css: " << int(colive_streaming_status_)
        << ", current mode: " << restart_mode_.to_string()
        << ", require mode: " << mode.to_string();

    // 如果当前有未完结的重试流程正在进行，
    // 1、如果正在进行的重试操作完全包含了新来重试请求，那么不需要做特殊处理，因为稍后都会得到执行；
    // 2、如果正在进行的重试操作不完全包含新的重试请求，那么需要对差集做处理；
    bool need_restart_record = false;
    bool need_restart_stream = false;
    if (restart_count_down_ > 0)
    {
        LOG(WARNING) << "[OC] Restart oc when prev restart is in the process, prev mode: " << restart_mode_.to_string();

        if (mode.IsRecordingSet() && !restart_mode_.IsRecordingSet())
        {
            LOG(INFO) << "[OC] Prev restart is in the process, need to add restart record.";

            restart_mode_.SetRecording();
            ++restart_count_down_;

            need_restart_record = true;
        }

        // 需要重推流的几种情况，优先级为：需要重开播的 > 仅仅本地重推流的；
        // 需要重开播的细分优先级为：测速后重开播 > 重开播；
        // 本地重推流的细分优先级为：指定新地址重推流 > 当前地址重推流；
        // 则整体优先级为：测速后重开播 > 重开播 > 指定新地址重推流 > 当前地址重推流；
        // 那么这里采用的策略就是：
        // 新的重推流请求优先级小于正在执行的，则不需要再发起重新推流了，因为在执行的重推流流程涉及的范围涵盖了新的重推流请求；
        // 新的重推流请求优先级大于正在执行的，那么当前的重试流程继续执行，新的重试请求
        if (mode.IsRestartStreamBitsSet() && !restart_mode_.IsRestartStreamBitsSet())
        {
            LOG(INFO) << "[OC] Prev restart is in the process, need to add restart stream.";

            ++restart_count_down_;

            need_restart_stream = true;

            // 直接赋值
            unsigned long rsb = mode.restart_stream_bits_to_ulong();
            restart_mode_.SetRestartStreamBitsFromUlong(rsb);

            if (mode.IsUseSpecialAddrSet())
            {
                restart_mode_.set_use_special_addr_param(mode.use_special_addr_param());
            }
        }
        else if (mode.IsRestartStreamBitsSet() && restart_mode_.IsRestartStreamBitsSet())
        {
            LOG(INFO) << "[OC] Prev restart is in the process, new restream mode is higher than prev.";

            // 高优覆盖低优
            unsigned long new_rsb = mode.restart_stream_bits_to_ulong();
            unsigned long old_rsb = restart_mode_.restart_stream_bits_to_ulong();
            if (new_rsb > old_rsb)
            {
                need_restart_stream = true;

                restart_mode_.SetRestartStreamBitsFromUlong(new_rsb);
            }

            if (mode.IsUseSpecialAddrSet())
            {
                restart_mode_.set_use_special_addr_param(mode.use_special_addr_param());
            }
        }
    }
    else
    {
        restart_mode_ = mode;
        restart_count_down_ = restart_mode_.Count();

        if (restart_mode_.IsRecordingSet())
        {
            need_restart_record = true;
        }

        if (restart_mode_.IsRestartStreamBitsSet())
        {
            need_restart_stream = true;
        }
    }

    if (need_restart_record)
    {
        StopRecording();
    }
    if (need_restart_stream)
    {
        StopStreamingInternal(true);
    }
}

const OutputController::StreamingAddresses& ViddupOutputController::GetStreamingAddr() const
{
    return saved_addresses_;
}

bool ViddupOutputController::IsRestartLiveStreamingSafePeriodExpires() const
{
    return last_restart_live_streaming_time_ + kRestartLiveStreamingSafePeriod < base::Time::Now();
}

void ViddupOutputController::DoRestartIfNeeded()
{
    if (restart_mode_.IsRestartStreamBitsSet())
    {
        LOG(INFO) << "[OC] Try to DoRestart, ss: " << int(streaming_status_)
            << ", rs: " << int(recording_status_)
            << ", css: " << int(colive_streaming_status_)
            << ", mode: " << restart_mode_.to_string()
            << ", count_down: " << restart_count_down_;

        DCHECK(restart_count_down_ > 0);
        if (--restart_count_down_ == 0)
        {
            DoRestart();
        }
    }
}

bool ViddupOutputController::DoRestartAfterStreamingStoppedIfNeeded() {

    // 结束自推流竖屏连麦时在推流停止/错误回调后立即调 DoRestartForColive()，跳过通知上层
    // 的回调，在 UI 层营造出一种流没重推的效果。
    if (trans_from_spp_colive_) {
        LOG(INFO) << "[OC] DoRestart from colive, ss: " << int(streaming_status_)
            << ", rs: " << int(recording_status_)
            << ", css: " << int(colive_streaming_status_)
            << ", mode: " << restart_mode_.to_string()
            << ", count_down: " << restart_count_down_;

        if (restart_mode_.IsRestartStreamBitsSet()) {
            DCHECK(restart_count_down_ > 0);
            --restart_count_down_;
        }

        set_streaming_status(LiveStreamingStatus::Ready);
        DoRestart();
        return true;
    }

    return false;
}

void ViddupOutputController::DoRestart()
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    LOG(INFO) << "[OC] DoRestart, ss: " << int(streaming_status_)
        << ", rs: " << int(recording_status_)
        << ", css: " << int(colive_streaming_status_)
        << ", mode: " << restart_mode_.to_string();

    auto scope_runner = MAKE_SCOPE_GUARD{
        trans_from_spp_colive_ = false;
    };

    if (trans_to_stop_live_) {
        DCHECK(IsStreamingReady());

        trans_to_stop_live_ = false;
        restart_mode_.Clear();
        StreamingAnnihilated(obs_proxy::StreamingErrorCode::Success, {});
        return;
    }

    /**
     * 连麦开始时会先断原推流，这通过调用 DoRestart() 方法实现。但该方法里可能会 PostDelayedTask
     * 自身来确保底层不再活跃。如果之前有 Post 还没回来时，这时候连麦因各种原因断掉了。而根据当前状态，
     * 离开连麦频道时会把原推流拉起来。所以这里需要先检查当前推流状态。
     * 目前看来，除连麦以外，走到这里时推流状态应该是保持 ready 的，所以这里根据状态是否为 ready 来
     * 过滤应该没有问题。
     */
    if (!IsStreamingReady()) {
        restart_mode_.Clear();
        return;
    }

    // TODO:
    // 存在流中止的回调已经触发了但IsOutputActive()仍然返回true的情况，
    // 此时会陷入一直PostDelayedTask的死循环，需要有一个中止&&跳出这个死循环的机制。
    static int retry_restart_count = 0;
    if (obs_core()->GetOBSOutputService()->IsOutputActive())
    {
        if (retry_restart_count++ < 100)
        {
            scope_runner.Dismiss();

            LOG(WARNING) << "[OC] Do restart when output still active, try " << retry_restart_count << " times.";

            BililiveThread::PostDelayedTask(BililiveThread::UI,
                FROM_HERE,
                base::Bind(&ViddupOutputController::DoRestart, base::Unretained(this)),
                kRestartDelayInterval);
        }
        else
        {
            LOG(ERROR) << "[OC] Try to DoRestart when output still active more than " << retry_restart_count - 1 << " times.";

            // 这个时候虽然我们在流程上把重试给中止了，但是底层的OutputService仍未重置，用户主动重新开播大概率还是会失败的？？
            retry_restart_count = 0;

            restart_mode_.Clear();
            StreamingAnnihilated(obs_proxy::StreamingErrorCode::StartFailure, {});
        }
        return;
    }
    retry_restart_count = 0;

    obs_core()->UpdateVideoSettings();
    obs_core()->GetOBSOutputService()->RecreateOutputs();
    obs_core()->GetOBSOutputService()->ReloadEncoders();

    if (restart_mode_.IsRecordingSet())
    {
        restart_mode_.ClearRecording();
        StartRecording();
    }

    bool need_restream = restart_mode_.IsRestartStreamBitsSet();
    if (need_restream)
    {
        last_restart_live_streaming_time_ = base::Time::Now();
    }
    if (restart_mode_.IsJustRestartStreamSet())
    {
        IgniteStream(trans_from_spp_colive_);
    }
    else if (restart_mode_.IsUseSpecialAddrSet())
    {
        const auto& tp = restart_mode_.use_special_addr_param();
        IgniteStreamBySpecialAddr(std::get<0>(tp), std::get<1>(tp), std::get<2>(tp));
    }
    else if (restart_mode_.IsRedetermineStreamAddrSet())
    {
        IgniteStreamUntilAddrDetermined(true);
    }
    else if (restart_mode_.IsStreamUntilSpeedTestSet())
    {
        IgniteStreamUntilSpeedTest();
    }

    restart_mode_.Clear();

    DCHECK(restart_mode_.Count() == 0);
    LOG_IF(WARNING, restart_mode_.Count() != 0) << "[OC] Restart flag not completely clear after DoRestart.";
}

void ViddupOutputController::DoRestartForColive() {
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    LOG(INFO) << "## STREAM ## Do restart for colive! ss: " << int(streaming_status_)
        << " css: " << int(colive_streaming_status_);

    if (obs_core()->GetOBSOutputService()->IsOutputActive())
    {
        BililiveThread::PostDelayedTask(BililiveThread::UI,
            FROM_HERE,
            base::Bind(&ViddupOutputController::DoRestartForColive, base::Unretained(this)),
            kRestartDelayIntervalForColive);
        return;
    }

    restart_mode_.Clear();

    if (is_self_publish_for_colive_) {
        uint32_t out_width, out_height;
        std::tie(out_width, out_height) = obs_proxy::GetConfiguredVideoOutputMetrics(obs_proxy::GetPrefs());
        obs_core()->UpdateVideoSettings(
            /*std::max(out_width, out_height), std::min(out_width, out_height)*/);

        IgniteStream(false);
    } else {
        obs_core()->UpdateVideoSettings();
    }

    //obs_core()->GetColiveOutputService()->RecreateOutputs();
    //obs_core()->GetColiveOutputService()->RecreateEncoders();

    DCHECK(restart_mode_.Count() == 0);
}

int ViddupOutputController::GetOutputTotalFrames() const
{
    return obs_core_->GetOBSOutputService()->GetOutputTotalFrames();
}

int ViddupOutputController::GetOutputDroppedFrames() const
{
    return obs_core_->GetOBSOutputService()->GetOutputDroppedFrames();
}

#pragma region(invoke by LivehimeLiveRoomController)

void ViddupOutputController::InformLiveRoomClosed(const OutputControllerDelegate::StreamingErrorInfos* infos)
{
    set_streaming_status(LiveStreamingStatus::Ready);

    DCHECK(infos);
    if (infos)
    {
        // 如果是重推流引发的断流不要清除上次记录的推流信息，
        // 如果是需要重新开播的推流那么saved_addresses_是会被startlive的接口结果刷新的，
        // 如果仅仅是推流层断开重推，那么不要清除旧的推流信息；
        /*if (!infos->cause_for_restreaming)
        {
            static StreamingAddresses empty_addr;
            saved_addresses_ = empty_addr;
        }*/

        obs_proxy::StreamingErrorCode streaming_error_type = infos->streaming_errno;

        if (streaming_error_type == obs_proxy::StreamingErrorCode::Success)
        {
            // 是主动进行断流重连的关播的话在这里就可以重新开播了
            DoRestartIfNeeded();

            return;
        }
    }
    else
    {
        LOG(WARNING) << "close live room without streaming annihilated signal.";
    }
}

void ViddupOutputController::InformSpeedTestCompleted()
{
    LOG(INFO) << "[OC] SpeedTestCompleted, ss: " << int(streaming_status_)
        << ", css: " << int(colive_streaming_status_)
        << ", mode: " << restart_mode_.to_string();

    if (streaming_status_ == LiveStreamingStatus::WaitForSpeedTestResult)
    {
        set_streaming_status(LiveStreamingStatus::Ready);
        // 测速完毕，底层已经记录好了最优的IP地址，直接在这里要求上层重新提供推流地址即可
        IgniteStreamUntilAddrDetermined(true);
    }
}

#pragma endregion

void ViddupOutputController::OnPostSendFailing() {
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));
    //技术埋点推流：建连成功，发包失败
	base::StringPairs param;
   	param.push_back({ "live_session",  GetBililiveProcess()->secret_core()->anchor_info().sub_session_key()});
	param.push_back({ "push_url", this->saved_addresses_.server_address });
	livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::CDNSendPacketFail, param);
}

}   // namespace bililive
