#include "bililive/bililive/viddup/live_room/viddup_live_controller.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/ext/bind_lambda.h"
#include "base/ext/callable_callback.h"
#include "base/file_util.h"
#include "base/message_loop/message_loop.h"
#include "base/notification/notification_details.h"
#include "base/notification/notification_service.h"
#include "base/prefs/pref_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"

#include "bilibase/basic_types.h"
#include "bilibase/scope_guard.h"

#include "bililive/bililive/command_updater_delegate.h"
#include "bililive/bililive/livehime/obs/output_controller.h"
#include "bililive/bililive/livehime/gift_image/image_fetcher.h"
#include "bililive/bililive/livehime/user_info/user_info_service.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_hotkey_notify_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/livehime/new_live_room/new_live_room_guide_view.h"
#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_widget.h"
#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_datatype.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/public/log_ext/log_constants.h"
//#include "bililive/bililive/ui/views/toolbar/livehime_live_close_intercept_dialog.h"
#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"

namespace
{
    using namespace std::placeholders;

    const int kInvalidResponseErrorCode = (1 << 31);

    LivehimeLiveRoomController* g_single_instance = nullptr;

    void NotificationNotify(int type, const base::NotificationDetails& details)
    {
        base::NotificationService::current()->Notify(
            type, base::NotificationService::AllSources(), details);
    }

    // The best link queried from CDN providers contains both rtmp server address and authentication
    // key, while obs rtmp service requires separated server address and key.
    std::pair<std::string, std::string> SplitRecommendedCDNAddress(const std::string& server_addr)
    {
        auto query_leading_pos = server_addr.find('?');
        if (query_leading_pos == std::string::npos)
        {
            return { server_addr, std::string() };
        }

        auto new_addr = server_addr.substr(0, query_leading_pos);
        auto new_key = server_addr.substr(query_leading_pos);

        return { new_addr, new_key };
    }

    void AddErrorMsg(std::vector<std::string>& vec, const std::string& key, const std::string& value)
    {
        std::string new_value;
        if (!value.empty())
        {
            std::string output;
            TrimString(value, ":", &output);
            TrimString(output, ";", &new_value);
        }

        if (!new_value.empty())
        {
            std::string format_msg;
            format_msg.append(key).append(":").append(new_value);
            vec.push_back(format_msg);
        }
    }

    bool ShouldReportStreamingFailure(int error_code)
    {
        static const std::set<int> spared_error_codes{
            0,    // success
            -802,    // account expired
            60002,   // room locked
            60008,   // room cut off
            60010,   // under age
            60024,   // Abnormal account,need face auth
        };
        return (spared_error_codes.count(error_code) == 0);
    }

    void ReportLiveStreamingOutcome(bool streaming_succeeded,
        bililive::LiveStreamingErrorDetails::ErrorCategory category,
        int err_code,
        const obs_proxy::StreamingErrorDetails& error_details,
        int live_type = (int)StartLiveType::Normal)
    {
        if (error_details.custom_streaming)
        {
            return;
        }

        int result = -1;
        if (streaming_succeeded)
        {
            result = 0;
        }
        else if (category == bililive::LiveStreamingErrorDetails::Local)
        {
            result = 1;
        }
        else if (category == bililive::LiveStreamingErrorDetails::Server)
        {
            result = 2;
        }
        else
        {
            NOTREACHED();
        }

        std::vector<std::string> vec_outcome;

        vec_outcome.push_back(base::StringPrintf("result:%d", result));
        vec_outcome.push_back(base::StringPrintf("code:%d", err_code));
        vec_outcome.push_back(base::StringPrintf("new_link:%d", error_details.new_link));
        if (error_details.streaming_timespan > 0)
        {
            vec_outcome.push_back(base::StringPrintf("timespan:%ld", error_details.streaming_timespan));
        }

        AddErrorMsg(vec_outcome, "addr", error_details.addr/* + error_details.key*/);
        AddErrorMsg(vec_outcome, "rtmp_errfmt", error_details.rtmp_errfmt);
        AddErrorMsg(vec_outcome, "rtmp_errmsg", error_details.rtmp_errmsg);
        AddErrorMsg(vec_outcome, "errmsg", error_details.errmsg);
        AddErrorMsg(vec_outcome, "rtmp_host_name", error_details.rtmp_host_name);
        AddErrorMsg(vec_outcome, "rtmp_host", error_details.rtmp_host);

        if (live_type == (int)StartLiveType::Normal)
        {
            vec_outcome.push_back("livetype:0");
        }
        else if (live_type & (int)StartLiveType::ThirdPartyProxy)
        {
            vec_outcome.push_back("livetype:1");

            if (live_type & (int)StartLiveType::ThirdPartyProxy_OBS)
            {
                vec_outcome.push_back("tpstype:1");
            }
        }
        else
        {
            NOTREACHED() << "not support live type";
        }

        vec_outcome.push_back(base::StringPrintf("livemodel:%d",
            (LiveModelController::GetInstance() && LiveModelController::GetInstance()->IsLandscapeModel()) ? 0 : 1));

        std::string outcome;
        outcome.append(JoinString(vec_outcome, ";"));

        auto secret_core = GetBililiveProcess()->secret_core();
        auto mid = secret_core->account_info().mid();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
            secret::LivehimeBehaviorEvent::LivehimeStreamingOutcome, mid, outcome).Call();

        livehime::BehaviorEventReportViaServer(secret::LivehimeViaServerBehaviorEvent::StartLive, {});

        LOG(INFO) << (streaming_succeeded ? app_log::kLogStreamSuccess : app_log::kLogStreamFail) << outcome;
    }
}

LivehimeLiveRoomController* ViddupLiveRoomController::GetInstance()
{
    return g_single_instance;
}

void ViddupLiveRoomController::GetNewLinkAddr(const secret::LiveStreamingService::ProtocolInfo& live_info,
    GetNewLinkAddrHandler handler)
{
    if (!live_info.new_link.empty())
    {
      
    }
    else
    {
        std::string server_addr = live_info.addr;
        std::string server_key = live_info.key;
        handler(server_addr, server_key, false, live_info);
    }
}

ViddupLiveRoomController::ViddupLiveRoomController()
    : weakptr_factory_(this)
{
    g_single_instance = this;
}

ViddupLiveRoomController::~ViddupLiveRoomController()
{
    g_single_instance = nullptr;
}

void ViddupLiveRoomController::Initialize()
{
    DCHECK(false);
}

void ViddupLiveRoomController::Uninitialize()
{
    output_controller()->set_delegate(nullptr);
    third_party_output_controller()->Uninitialize();
    third_party_output_controller()->set_delegate(nullptr);
    notifation_register_.RemoveAll();
}

void ViddupLiveRoomController::AddObserver(LivehimeLiveRoomObserver *observer)
{
    observer_list_.AddObserver(observer);
}

void ViddupLiveRoomController::RemoveObserver(LivehimeLiveRoomObserver *observer)
{
    observer_list_.RemoveObserver(observer);
}

void ViddupLiveRoomController::ChooseAreaEndDialog(int code, void* data)
{
}

void ViddupLiveRoomController::RestartLive(LiveRestartMode mode)
{
    switch (start_type_)
    {
    case StartLiveType::Normal:
    {
        output_controller()->Restart(mode);
    }
    break;
    case StartLiveType::ThirdPartyProxy:
    {
    }
    break;
    default:
        break;
    }
}

bool ViddupLiveRoomController::CheckIfInSelfStreamingStatus()
{
    if (IsInTheThirdPartyStreamingMode()) {
        return false;
    }

    if (!output_controller()->IsStreaming()) {
        return false;
    }

    return true;
}

bool ViddupLiveRoomController::TryToChangeStreamAddr(const std::string& addr, const std::string &code, const std::string &prtc)
{
    if (CheckIfInSelfStreamingStatus())
    {
        LiveRestartMode mode;
        mode.SetUseSpecialAddr();
        LiveRestartMode::UseSpecialAddrParam tp{ addr, code, prtc };
        mode.set_use_special_addr_param(tp);
        BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
            base::Bind(&ViddupLiveRoomController::RestartLive, weakptr_factory_.GetWeakPtr(), mode));
        return true;
    }

    return false;
}

void ViddupLiveRoomController::GetStreamAddrList(GetStreamAddrListHandler handler)
{

}

StartLiveFlowResult ViddupLiveRoomController::OpenLiveRoom(StartLiveType start_type, bool start_by_hotkey,
    bool ignore_cover_check/* = false*/)
{
    LOG(INFO) << "[LC] User start live, hk = " << start_by_hotkey;

    DCHECK(live_status_ == LiveStatus::Ready);
    if (live_status_ != LiveStatus::Ready)
    {
        return StartLiveFlowResult::Failed;
    }

    start_type_ = start_type;
    start_by_hotkey_ = start_by_hotkey;

    auto secret = GetBililiveProcess()->secret_core();
    auto room_id = secret->user_info().room_id();
    if (room_id < 0)
    {
        ON_SCOPE_EXIT{
            GetBililiveProcess()->bililive_obs()->user_info_service()->GetRoomInfo();
        };

        LOG(WARNING) << "[LC] live room id " << room_id << " invalid";

        static const int kInvalidRoomIdTryTimes = 3;
        static int failed_times = 0;
        if (failed_times < kInvalidRoomIdTryTimes)
        {
            NotificationNotify(bililive::NOTIFICATION_LIVEHIME_LOADING_ROOMINFO,
                base::NotificationService::NoDetails());

            ++failed_times;
            return StartLiveFlowResult::InvalidParamsRetry;
        }
        else
        {
            NotificationNotify(bililive::NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_FAILED,
                base::NotificationService::NoDetails());

            return StartLiveFlowResult::InvalidParams;
        }
    }
    else if (room_id == 0 )
    {
        LOG(WARNING) << "[LC] live room not grant." << room_id;
        static int from_type = 4;
        NotificationNotify(bililive::NOTIFICATION_LIVEHIME_LIVEROOM_NOT_GRANT,
            base::Details<int>(&from_type));

        return StartLiveFlowResult::InvalidParams;
    }

    OpenLiveRoomInternal();

    return StartLiveFlowResult::Succeed;
}

void ViddupLiveRoomController::UpdateStreamVideoBitrateAtLiveArea(int parent_id) {

}

void ViddupLiveRoomController::OpenLiveRoomInternal()
{
    SetLiveStatus(LiveStatus::LiveRoomOpening);

    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnOpenLiveRoom());

    StartLive();
}

void ViddupLiveRoomController::CloseLiveRoomPre()
{
	if (!is_stop_live_intercept_ && !IsLiveReady())
	{
		const std::vector<secret::AnchorInfo::TaskScheduleInfo>& schedule_list =
			GetBililiveProcess()->secret_core()->anchor_info().get_anchor_task_schedule_list();
        for (auto& it : schedule_list)
        {
            if (it.touch_type == static_cast<int>(secret::AnchorInfo::TaskScheduleInfo::TouchType::Close_Live))
            {
                    return;
            }
        }
	}

    CloseLiveRoom();
}

void ViddupLiveRoomController::CloseLiveRoom()
{
    DCHECK(live_status_ != LiveStatus::Ready && !stop_live_now_);
    if (live_status_ != LiveStatus::Ready && !stop_live_now_)
    {
        stop_live_now_ = true;
        choose_area_showed_by_start_live_flow_ = false;
        leave_third_party_mode_ = true;

        FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
            OnCloseLiveRoom());

        if (live_status_ == LiveStatus::Streaming)
        {
            LOG(INFO) << "[LC] streaming while stop live, stop stream.";
            CloseLiveRoomInternal(true, nullptr);
        }
        else
        {
            DCHECK(live_status_ == LiveStatus::StreamStopping ||
                live_status_ == LiveStatus::StreamStoped ||
                live_status_ == LiveStatus::LiveRoomClosing);

            if (IsInTheThirdPartyStreamingMode())
            {
                InvalidateThirdPartyStreamingTask();

                if (live_status_ == LiveStatus::StreamStoped)
                {
                    if (third_party_output_controller()->IsRunning())
                    {
                        LOG(INFO) << "tps listening while stop live, stop stream.";
                    }
                    else
                    {
                        LOG(INFO) << "tps listening while stop live, stop live.";
                    }

                    CloseLiveRoomInternal(third_party_output_controller()->IsRunning(), nullptr);
                }
            }
        }
    }
}

void ViddupLiveRoomController::CloseLiveRoomInternal(bool stop_streaming, void* param)
{
    if (stop_streaming)
    {
        DCHECK(!param);

        SetLiveStatus(LiveStatus::StreamStopping);

        switch (start_type_)
        {
        case StartLiveType::Normal:
        {
            output_controller()->StopStreaming();
        }
        break;
        case StartLiveType::ThirdPartyProxy:
        {
            third_party_output_controller()->Shutdown();
        }
        break;
        default:
            break;
        }
    }
    else
    {
        if (start_type_ == StartLiveType::Normal)
        {
            DCHECK(param);
        }

        SetLiveStatus(LiveStatus::LiveRoomClosing);
        std::unique_ptr<OutputControllerDelegate::StreamingErrorInfos> info(
                    (OutputControllerDelegate::StreamingErrorInfos*)param);
        HandleLiveRoomClosed(info.release());
    }
}

void ViddupLiveRoomController::StartLive()
{
    auto handler = base::MakeCallable(base::Bind(
        &ViddupLiveRoomController::OnStartLive,
        weakptr_factory_.GetWeakPtr()));

    auto secret = GetBililiveProcess()->secret_core();

    secret->live_streaming_service()->StartViddupLive(handler).Call();

}

void ViddupLiveRoomController::StopLive(void* param)
{
    auto handler = base::MakeCallable(base::Bind(
        &ViddupLiveRoomController::OnStopLive,
        weakptr_factory_.GetWeakPtr()));
}

void ViddupLiveRoomController::TerminateStartLiveFlow()
{
    if (IsInTheThirdPartyStreamingMode())
    {
        LeaveThirdPartyStreamingMode();
    }
    else
    {
        NotificationNotify(bililive::NOTIFICATION_LIVEHIME_ENABLE_START_LIVE,
            base::NotificationService::NoDetails());
    }
}

void ViddupLiveRoomController::OnChoosePartition(PartitionUpdateStatus status, void* param)
{
    switch (status)
    {
    case PartitionUpdateStatus::Updating:
    {
        updating_room_info_ = true;

        if (!choose_area_showed_by_start_live_flow_ &&
            !IsLiveReady())
        {
            return;
        }

        NotificationNotify(bililive::NOTIFICATION_LIVEHIME_DISABLE_START_LIVE,
            base::NotificationService::NoDetails());

        NotificationNotify(bililive::NOTIFICATION_LIVEHIME_DISABLE_PARTITION_BTN,
            base::NotificationService::NoDetails());
    }
    break;
    case PartitionUpdateStatus::Success:
    {
        updating_room_info_ = false;
        last_set_channel_time_ = base::Time::Now();

        if (choose_area_showed_by_start_live_flow_)
        {
            if (start_by_hotkey_)
            {
                HotkeyNotifyView::ShowForm(HotkeyNotifyView::HN_LiveStart,
                    GetBililiveProcess()->bililive_obs()->obs_view()->get_preview_rect());
            }

            OpenLiveRoom(start_type_, start_by_hotkey_, true);
        }
        else
        {
            if (IsLiveReady())
            {
                NotificationNotify(bililive::NOTIFICATION_LIVEHIME_ENABLE_START_LIVE,
                    base::NotificationService::NoDetails());
            }

            if (IsStreaming())
            {
                NotificationNotify(bililive::NOTIFICATION_LIVEHIME_SET_TAGS,
                    base::NotificationService::NoDetails());
            }
        }

        NotificationNotify(bililive::NOTIFICATION_LIVEHIME_ENABLE_PARTITION_BTN,
            base::NotificationService::NoDetails());
    }
    break;
    case PartitionUpdateStatus::Failure:
    {
        updating_room_info_ = false;

        const base::string16* error_msg = (base::string16*)param;
        if (error_msg && !error_msg->empty())
        {
            livehime::ShowMessageBox(
                GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeWindow(),
                GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_TITLE),
                *error_msg,
                GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_IKNOW));
        }

        NotificationNotify(bililive::NOTIFICATION_LIVEHIME_ENABLE_PARTITION_BTN,
            base::NotificationService::NoDetails());

        if (choose_area_showed_by_start_live_flow_ ||
            IsLiveReady())
        {
            NotificationNotify(bililive::NOTIFICATION_LIVEHIME_ENABLE_START_LIVE,
                base::NotificationService::NoDetails());
        }
    }
    break;
    default:
        break;
    }
}

void ViddupLiveRoomController::OnStartLive(bool success, int result_code, const std::string& err_msg,
    const secret::LiveStreamingService::StartLiveInfo& start_live_info)
{
    LOG(INFO) << app_log::kLogLiveStart <<
        base::StringPrintf("result:%s; mode:%s; start_type:%s; landscape:%s",
            (success && (result_code == 0)) ? "true" : "false",
            start_type_ == StartLiveType::Normal ? "normal" : "tps",
            start_by_hotkey_ ? "hotkey" : "click",
            LiveModelController::GetInstance()->IsLandscapeModel() ? "true" : "false");
    LOG(INFO) << "start live interface result: " <<
        "ret= " << success << ", code=" << result_code << ", msg=" << err_msg << ", room_type=" << start_live_info.room_type <<
        ",\n addr=" << start_live_info.rtmp_info.addr << ", newlink=" << start_live_info.rtmp_info.new_link <<
        ", addr_count=" << start_live_info.protocol_list.size();

    if (!success || (result_code != secret::LiveStreamingService::OK))
    {
        SetLiveStatus(LiveStatus::LiveRoomOpenFailed);

        auto error_info = success ?
            std::make_pair(base::UTF8ToWide(err_msg), result_code) :
            std::make_pair(GetLocalizedString(IDS_STREAMING_GET_LIVE_STATUS_ERROR), kInvalidResponseErrorCode);

        OnStartLiveError(obs_proxy::StreamingErrorCode::StartFailure, error_info.first, error_info.second, start_live_info);
        if (result_code == (int)StartLiveRoomErrorCode::Authentication){
            HandleStartLiveNotice(start_live_info.notice);
        }

        SetLiveStatus(LiveStatus::Ready);
        return;
    }

    GetBililiveProcess()->secret_core()->anchor_info().set_live_status(1);

    HandleStartLiveNotice(start_live_info.notice);

    start_live_info_ = start_live_info;
    start_live_time_ = base::Time::Now();

    HandleLiveRoomOpened();

    if (stop_live_now_)
    {
        LOG(INFO) << "live room opened while live stop, stop live.";
        CloseLiveRoomInternal(false, nullptr);
        return;
    }

    if (start_live_info.protocol_list.empty())
    {
        auto error_info = std::make_pair(GetLocalizedString(IDS_STREAMING_START_FAILED), 0);
        OnStartLiveError(obs_proxy::StreamingErrorCode::BadPath, error_info.first, error_info.second, start_live_info);

        SetLiveStatus(LiveStatus::Ready);
        return;
    }

    SetLiveStatus(LiveStatus::StreamStarting);

    is_stop_live_intercept_ = false;

    switch (start_type_)
    {
    case StartLiveType::Normal:
        {
            LOG(INFO) << "[LC] start streaming.";
            output_controller()->StartStreaming(start_live_info_);
        }
        break;
    case StartLiveType::ThirdPartyProxy:
        {
            LOG(INFO) << "[LC] tps start streaming.";
            third_party_output_controller()->StartStreaming(start_live_info_);
        }
        break;
    default:
        NOTREACHED() << "invalid live type " << (int)start_type_;
        break;
    }
}

void ViddupLiveRoomController::OnStartLiveError(obs_proxy::StreamingErrorCode streaming_error_type,
    const std::wstring& error_msg, int error_code,
    const secret::LiveStreamingService::StartLiveInfo& start_live_info)
{
    if (ShouldReportStreamingFailure(error_code))
    {
        bililive::LiveStreamingErrorDetails::ErrorCategory category = bililive::LiveStreamingErrorDetails::None;
        if (error_code != kNoneServerErrorCode)
        {
            category = bililive::LiveStreamingErrorDetails::Server;
        }
        else
        {
            category = bililive::LiveStreamingErrorDetails::Local;
            error_code = bilibase::enum_cast(streaming_error_type);
        }

        ReportLiveStreamingOutcome(false, category, error_code, {});
    }

    base::string16 msg = error_msg;
    int code = error_code;
    GetLiveErrorMsg(streaming_error_type, msg, code);

    bool need_face_auth = false;
    if ((error_code == (int)StartLiveRoomErrorCode::AuthAccountAnomaly)&& start_live_info.need_face_auth){
        need_face_auth = true;
    }

    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnOpenLiveRoomError(msg, code,need_face_auth, start_live_info.qr));
}

void ViddupLiveRoomController::OnStopLive(bool success, int result_code, const std::string& err_msg,
    void* param)
{
    LOG(INFO) << "stop live interface result: " <<
        "ret= " << success << ", code=" << result_code << ", msg=" << err_msg;

    HandleLiveRoomClosed(param);
}

void ViddupLiveRoomController::OnBindLiveTopic(bool success, int result_code, const std::string& err_msg)
{
    if (result_code != 0 || !success) {
        LOG(WARNING) << "bind live topic interface result: " <<
            "ret= " << success << ", code=" << result_code << ", msg=" << err_msg;
    }
}

void ViddupLiveRoomController::HideBookLiveTime()
{
}

void ViddupLiveRoomController::GetBookLiveTimeInfos()
{
}

void ViddupLiveRoomController::Observe(int type,
    const base::NotificationSource& source, const base::NotificationDetails& details)
{
    switch (type)
    {
    case bililive::NOTIFICATION_LIVEHIME_ROOMINFO_UPDATING:
        OnChoosePartition(PartitionUpdateStatus::Updating, nullptr);
        break;
    case bililive::NOTIFICATION_LIVEHIME_ROOMINFO_UPDATE_SUCCESS:
        OnChoosePartition(PartitionUpdateStatus::Success, nullptr);
        break;
    case bililive::NOTIFICATION_LIVEHIME_ROOMINFO_UPDATE_FAIL:
        OnChoosePartition(PartitionUpdateStatus::Failure, (void*)details.map_key());
        break;
    case bililive::NOTIFICATION_LIVEHIME_CREATE_LIVEROOM_SUCCESS:
        {
            int from_type = -1;
            if (!!details.map_key()){
                from_type = *reinterpret_cast<int*>(details.map_key());
                if (4 == from_type){  
                    OpenLiveRoom(start_type_, start_by_hotkey_);
                }
                else if (5 == from_type) {
                    bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_ENTER_INTO_THIRD_PARTY_STREAM_MODE);
                }
            }
        }
        break;
    default:
        break;
    }
}

void ViddupLiveRoomController::HandleStartLiveNotice(const secret::LiveStreamingService::StartLiveNotice& notice)
{
    switch ((StartLiveNoticeType)notice.type)
    {
    case StartLiveNoticeType::Undefine:
        break;
    case StartLiveNoticeType::AuthNeedSupplement:
    {
    }
        break;
    default:
        NOTREACHED() << "unhandled start live notice type " << notice.type;
        break;
    }
}

void ViddupLiveRoomController::SetLiveStatus(LiveStatus status)
{
    live_status_ = status;

    if (live_status_ == LiveStatus::Ready)
    {
        stop_live_now_ = false;
    }
}

// OutputControllerDelegate
void ViddupLiveRoomController::OnPreStartStreaming(bool is_restart)
{
    HandlePreStartStreaming(is_restart);
}

void ViddupLiveRoomController::OnStreamingStarted(const bililive::StartStreamingDetails& details,
    const obs_proxy::StreamingErrorDetails& streaming_error_details)
{
    DCHECK(details.is_colive || details.after_colive || live_status_ == LiveStatus::StreamStarting);

    HandleStreamingStarted(details, streaming_error_details);
}

void ViddupLiveRoomController::OnRetryStreaming(int index, int total)
{
    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnLiveRoomStreamingRetryNextAddr(index, total));
}

void ViddupLiveRoomController::OnPreStopStreaming(bool is_restart)
{
    HandlePreStopStreaming(is_restart);
}

void ViddupLiveRoomController::OnStreamingStopped(OutputControllerDelegate::StreamingErrorInfos& info)
{
    DCHECK(live_status_ == LiveStatus::StreamStarting ||
        live_status_ == LiveStatus::Streaming ||
        live_status_ == LiveStatus::StreamStopping);

    if (info.streaming_errno == obs_proxy::StreamingErrorCode::Success)
    {
        HandleLiveStreamingStoped();
    }
    else
    {
        base::string16 msg;
        int code = kNoneServerErrorCode;
        GetLiveErrorMsg(info.streaming_errno, msg, code);
        HandleLiveStreamingError(info.streaming_errno, msg, code);
    }

    std::unique_ptr<OutputControllerDelegate::StreamingErrorInfos> err_details =
        std::make_unique<OutputControllerDelegate::StreamingErrorInfos>(std::move(info));
    if (info.should_close_live_room)
    {
        LOG(INFO) << "stream stopped, stop live.";
        CloseLiveRoomInternal(false, err_details.release());
    }
    else
    {
        HandleLiveRoomClosed(err_details.release());
    }
}

void ViddupLiveRoomController::NeedNewStreamingAddrInfos()
{
    base::MessageLoop::current()->PostTask(FROM_HERE,
        base::Bind(base::IgnoreResult(&ViddupLiveRoomController::OpenLiveRoom), weakptr_factory_.GetWeakPtr(),
            StartLiveType::Normal, false, true));
}

void ViddupLiveRoomController::GetLiveErrorMsg(obs_proxy::StreamingErrorCode streaming_error_code, base::string16& error_msg, int& code)
{
    std::wstring msg;  obs_proxy::FormatStreamingError(streaming_error_code);

    if (code != kNoneServerErrorCode)
    {
        switch (code)
        {
            case -1:
            case 60002:
            case 1:
            case 60010:
            case -802:
            case 60024:
            break;

            default:
                msg = obs_proxy::FormatStreamingError(streaming_error_code);
                msg = std::to_wstring(code) + L" " + msg;
            break;
        }
    }

    if (!error_msg.empty())
    {
        msg.append(L" ").append(error_msg);
    }

    if (streaming_error_code == obs_proxy::StreamingErrorCode::StartFailure &&
        code == kNoneServerErrorCode)
    {
        msg += L"\n\n";
        msg += GetLocalizedString(IDS_RECORDING_TIP_NOTE);
    }

    // Server failure has higher precedence.
    if (code == kNoneServerErrorCode)
    {
        code = bilibase::enum_cast(streaming_error_code);
    }

    error_msg = std::move(msg);
}

void ViddupLiveRoomController::HandleLiveRoomOpened()
{
    SetLiveStatus(LiveStatus::LiveRoomOpened);

    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnOpenLiveRoomSuccessed(start_live_info_));
}

void ViddupLiveRoomController::HandlePreStartStreaming(bool is_restart)
{
    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnPreLiveRoomStartStreaming(is_restart));
}

void ViddupLiveRoomController::HandleStreamingStarted(const bililive::StartStreamingDetails& details,
    const obs_proxy::StreamingErrorDetails& streaming_error_details)
{
    int type = (int)start_type_;
    if (start_type_ == StartLiveType::ThirdPartyProxy)
    {
        type |= third_party_output_controller()->running_by_obs() ? (int)StartLiveType::ThirdPartyProxy_OBS : 0;
    }

    ReportLiveStreamingOutcome(true, bililive::LiveStreamingErrorDetails::None, kNoneServerErrorCode,
        streaming_error_details, type);

    SetLiveStatus(LiveStatus::Streaming);

    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnLiveRoomStreamingStarted(details));

    if (stop_live_now_)
    {
        LOG(INFO) << "streaming started while live stop, stop stream.";
        CloseLiveRoomInternal(true, nullptr);
    }
}

void ViddupLiveRoomController::HandleLiveStreamingError(obs_proxy::StreamingErrorCode streaming_errno, const std::wstring& error_msg, int error_code)
{
    LOG(INFO) << app_log::kLogStreamFail << "code:" << (int)streaming_errno << "; msg:" << base::UTF16ToUTF8(error_msg);

    SetLiveStatus(LiveStatus::StreamStoped);

    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnLiveRoomStreamingError(streaming_errno, error_msg, error_code));
}

void ViddupLiveRoomController::HandlePreStopStreaming(bool is_restart)
{
    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnPreLiveRoomStopStreaming());
}

void ViddupLiveRoomController::HandleLiveStreamingStoped()
{
    LOG(INFO) << app_log::kLogStreamStop;

    SetLiveStatus(LiveStatus::StreamStoped);

    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnLiveRoomStreamingStopped());
}

void ViddupLiveRoomController::HandleLiveRoomClosed(void* param)
{
    SetLiveStatus(LiveStatus::LiveRoomClosed);

    bool cause_for_restreaming = false;

    switch (start_type_)
    {
    case StartLiveType::Normal:
    {
        std::unique_ptr<OutputControllerDelegate::StreamingErrorInfos> info(
            (OutputControllerDelegate::StreamingErrorInfos*)param);
        if (info)
        {
            if (info->streaming_errno != obs_proxy::StreamingErrorCode::Success)
            {
                ReportLiveStreamingOutcome(false, bililive::LiveStreamingErrorDetails::Local,
                    bilibase::enum_cast(info->streaming_errno), info->streaming_error_details);

                LOG(INFO) << app_log::kLogLiveStop << "cause by stream error";
            }
            else
            {
                LOG(INFO) << app_log::kLogLiveStop << "user invoke";

                base::StringPairs event_ext;
                event_ext.push_back(std::pair<std::string, std::string>("close_reason", "2")); 
                livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::CloseLiveTracker, event_ext);
            }


            cause_for_restreaming = info->cause_for_restreaming;
        }

        output_controller()->InformLiveRoomClosed(info.get());
    }
    break;
    case StartLiveType::ThirdPartyProxy:
    {
    }
    break;
    default:
        break;
    }

    GetBililiveProcess()->secret_core()->anchor_info().set_live_status(0);

    livehime::BehaviorEventReportViaServer(secret::LivehimeViaServerBehaviorEvent::StopLive, {});

    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnLiveRoomClosed(cause_for_restreaming, start_live_info_));

    if (!cause_for_restreaming)
    {
        SetLiveStatus(LiveStatus::Ready);
    }
    else
    {
        SetLiveStatus(LiveStatus::StreamStarting);
    }
}

// SpeedTest
bool ViddupLiveRoomController::NeedSpeedTest()
{
    SetLiveStatus(LiveStatus::WaitForSpeedTestResultWhileRestream);

    return true;
}
