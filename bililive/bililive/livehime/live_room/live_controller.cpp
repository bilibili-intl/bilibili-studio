#include "bililive/bililive/livehime/live_room/live_controller.h"

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
#include "bililive/bililive/viddup/live_room/viddup_live_controller.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/public/log_ext/log_constants.h"
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

    // 一些特殊的服务端错误不需要埋点
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

    // 本次开播的结果埋点（成功打开直播间并推流成功埋成功点，打开直播间失败、推不上流埋失败点，数据维度相对粗糙）
    void ReportLiveStreamingOutcome(bool streaming_succeeded,
        bililive::LiveStreamingErrorDetails::ErrorCategory category,
        int err_code,   // 服务端的开播接口返回码或是本地StreamingErrorCode错误类型码
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

        // 新增一个第三方推流模式的标识，开播结果的埋点也顺带当做第三方模式的使用率的埋点
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

        // 新增横竖屏模式的埋点，可以得出横竖屏的开播率
        vec_outcome.push_back(base::StringPrintf("livemodel:%d",
            (LiveModelController::GetInstance() && LiveModelController::GetInstance()->IsLandscapeModel()) ? 0 : 1));

        std::string outcome;
        outcome.append(JoinString(vec_outcome, ";"));

        auto secret_core = GetBililiveProcess()->secret_core();
        auto mid = secret_core->account_info().mid();
        secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
            secret::LivehimeBehaviorEvent::LivehimeStreamingOutcome, mid, outcome).Call();

        // 服务端埋点
        livehime::BehaviorEventReportViaServer(secret::LivehimeViaServerBehaviorEvent::StartLive, {});

        LOG(INFO) << (streaming_succeeded ? app_log::kLogStreamSuccess : app_log::kLogStreamFail) << outcome;
    }
}

LivehimeLiveRoomController* LivehimeLiveRoomController::GetInstance()
{
    return ViddupLiveRoomController::GetInstance();
}

void LivehimeLiveRoomController::GetNewLinkAddr(const secret::LiveStreamingService::ProtocolInfo& live_info,
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

LivehimeLiveRoomController::LivehimeLiveRoomController()
    : weakptr_factory_(this)
{
    g_single_instance = this;

    notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_ROOMINFO_UPDATING,
        base::NotificationService::AllSources());
    notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_ROOMINFO_UPDATE_SUCCESS,
        base::NotificationService::AllSources());
    notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_ROOMINFO_UPDATE_FAIL,
        base::NotificationService::AllSources());
    notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_CREATE_LIVEROOM_SUCCESS,
        base::NotificationService::AllSources());

    output_controller_ = bililive::OutputController::GetInstance();
    output_controller_->set_delegate(this);
    third_party_output_controller_ = livehime::ThirdPartyOutputController::GetInstance();
    third_party_output_controller_->set_delegate(this);
}

LivehimeLiveRoomController::~LivehimeLiveRoomController()
{
    g_single_instance = nullptr;
}

void LivehimeLiveRoomController::Initialize()
{
    DCHECK(false);
}

void LivehimeLiveRoomController::Uninitialize()
{
    // output_controller_/third_party_output_controller_不需要置null，livectrl实例的维护方式和output_controller/third_party_output_controller_不一样，
    // livectrl比output_controller/third_party_output_controller_析构的时机早得多，
    // 不用担心output_controller/third_party_output_controller_实例无效的问题；
    // 需要关心的应该是当livectrl进行Uninitialize之后，异步结果回来调用到output_controller/third_party_output_controller_时
    // 会不会产生额外的不必要的流程控制、状态流转问题，
    // 比如此时程序已经进入终止流程了，但是推流层在结果回调回来后又开启了新一轮的内部某种流程的工作，
    // 目前暂没有这种场景，所以直接在这里不置null最方便，省得在用到output_controller/third_party_output_controller_的地方要非空判断一下

    output_controller_->set_delegate(nullptr);
    //output_controller_ = nullptr;
    third_party_output_controller_->Uninitialize();
    third_party_output_controller_->set_delegate(nullptr);
    //third_party_output_controller_ = nullptr;
    notifation_register_.RemoveAll();
}

void LivehimeLiveRoomController::AddObserver(LivehimeLiveRoomObserver *observer)
{
    observer_list_.AddObserver(observer);
}

void LivehimeLiveRoomController::RemoveObserver(LivehimeLiveRoomObserver *observer)
{
    observer_list_.RemoveObserver(observer);
}

void LivehimeLiveRoomController::RestartLive(LiveRestartMode mode)
{
    switch (start_type_)
    {
    case StartLiveType::Normal: // OBS推流路径
    {
        output_controller()->Restart(mode);
    }
    break;
    case StartLiveType::ThirdPartyProxy:    // 第三方代理转发路径，不需要推
    {
        
    }
    break;
    default:
        break;
    }
}

bool LivehimeLiveRoomController::CheckIfInSelfStreamingStatus()
{
    if (IsInTheThirdPartyStreamingMode()) {
        return false;
    }

    if (!output_controller()->IsStreaming()) {
        return false;
    }

    return true;
}

bool LivehimeLiveRoomController::TryToChangeStreamAddr(const std::string& addr, const std::string &code, const std::string &prtc)
{
    // 正在自推流
    if (CheckIfInSelfStreamingStatus())
    {
        LiveRestartMode mode;
        mode.SetUseSpecialAddr();
        LiveRestartMode::UseSpecialAddrParam tp{ addr, code, prtc };
        mode.set_use_special_addr_param(tp);
        BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
            base::Bind(&LivehimeLiveRoomController::RestartLive, weakptr_factory_.GetWeakPtr(), mode));
        return true;
    }
    return false;
}

void LivehimeLiveRoomController::GetStreamAddrList(GetStreamAddrListHandler handler)
{

}

StartLiveFlowResult LivehimeLiveRoomController::OpenLiveRoom(StartLiveType start_type, bool start_by_hotkey,
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

    // ★检查房间ID是否有效，getroominfo接口没返回或是用户根本就没有开通直播间是开不了播的
    auto room_id = secret->user_info().room_id();
    if (room_id < 0)
    {
        // 房间信息未初始化前用户就急不可耐的开播的话，在其尝试开播并失败时我们再拉一遍房间信息
        ON_SCOPE_EXIT{
            GetBililiveProcess()->bililive_obs()->user_info_service()->GetRoomInfo();
        };

        LOG(WARNING) << "[LC] live room id " << room_id << " invalid";

        // 尝试三次直播都因为房间信息未初始化而失败，那还是把这种情况上报一下
        static const int kInvalidRoomIdTryTimes = 3;
        static int failed_times = 0;
        if (failed_times < kInvalidRoomIdTryTimes)
        {
            // 通知一下开播过程中断了，开播必要信息没加载完
            NotificationNotify(bililive::NOTIFICATION_LIVEHIME_LOADING_ROOMINFO,
                base::NotificationService::NoDetails());

            ++failed_times;

            return StartLiveFlowResult::InvalidParamsRetry;
        }
        else
        {
            // 尝试N次都拿不到房间信息，那可能网络或是接口本身有问题了，直接提示错误
            NotificationNotify(bililive::NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_FAILED,
                base::NotificationService::NoDetails());

            return StartLiveFlowResult::InvalidParams;
        }
    }

    auto channel_id = secret->anchor_info().current_area();
    if (channel_id <= secret::LiveStreamingService::kInvalidChannelId)
    {
        LOG(WARNING) << "[LC] invalid channel id " << channel_id;
        if (IsInTheThirdPartyStreamingMode() && !leave_third_party_mode_)//如果是在第三方推流模式下，则需要弹框选择分区后才能主动继续开播
        {
            
        }
        else{
           
            // 用户没选过分区，先自行选一下，不要主动帮他弹出
            NotificationNotify(bililive::NOTIFICATION_LIVEHIME_LIVEROOM_NO_AREA,
                base::NotificationService::NoDetails());
            
        }
        return StartLiveFlowResult::ChooseArea;
    }

    //2022-01-19产品需求(预设素材)，如果选择过了，就不再弹框确认了，只有当前id无效时再弹
	std::string area_name = GetBililiveProcess()->bililive_obs()->user_info_service()->GetAreaNameById(channel_id);
	if (area_name.empty())
	{
		// ★分区选择过期了没有，程序开启后初次开播前必须先选分区，选完后超过5分钟不开播，之后开播要重选分区
		static base::TimeDelta dt = base::TimeDelta::FromMinutes(bililive::kIntervalOfSetChannel);
		if ((base::Time::Now() - last_set_channel_time_) > dt)
		{
			return StartLiveFlowResult::ChooseArea;
		}
	}

    //这里更新下分区的码率配置,之前选分区的时候没有更新是因为有可能开播过程切换分区，
    //这时就不能更新,等这里开播的时候再更新

    UpdateStreamVideoBitrateAtLiveArea(
        GetBililiveProcess()->bililive_obs()->user_info_service()->GetParentId(channel_id));

    OpenLiveRoomInternal();

    return StartLiveFlowResult::Succeed;
}

void LivehimeLiveRoomController::UpdateStreamVideoBitrateAtLiveArea(int parent_id) {


}

void LivehimeLiveRoomController::OpenLiveRoomInternal()
{
    // 房间打开中
    SetLiveStatus(LiveStatus::LiveRoomOpening);

    // 通知开播中
    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnOpenLiveRoom());

    // 调开播接口
    StartLive();
}

void LivehimeLiveRoomController::CloseLiveRoomPre()
{
	//关播拦截：有马上要完成的任务时，提示是否完成任务再关播。
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
    //否则还是走之前的关播流程
    CloseLiveRoom();
}

void LivehimeLiveRoomController::CloseLiveRoom()
{
    DCHECK(live_status_ != LiveStatus::Ready && !stop_live_now_);
    if (live_status_ != LiveStatus::Ready && !stop_live_now_)
    {
        // 先置关播标识位，以便中止各种异步回调的逻辑
        stop_live_now_ = true;

        // 分区选完了也要中止自动开播流程
        choose_area_showed_by_start_live_flow_ = false;

        // 只要关播就退出第三方模式，不然类似OBS这样的会不断自动重试的软件会一直重试推流
        // 推上了又会再次自动开播
        leave_third_party_mode_ = true;

        // 通知关播中
        FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
            OnCloseLiveRoom());

        // 根据当前的状态来决定具体的关播操作，除了正处于推流中需要主动断流外，
        // 其他的状态要嘛不需要主动做中止操作要嘛只需在其异步状态的回调中对stop_live_now_判断进行中止即可
        if (live_status_ == LiveStatus::Streaming)
        {
            LOG(INFO) << "[LC] streaming while stop live, stop stream.";
            CloseLiveRoomInternal(true, nullptr);
        }
        else
        {
            // 1、底层错误已经调了StopStreaming，结果未返回的时候会处于LiveStatus::StreamStopping；
            // 2、第三方底层本地流断了，还在一分钟续流等待，则正处于LiveStatus::StreamStoped中；
            // 3、底层错误引发的关播已经调了关播接口了，只是结果未返回，处于LiveStatus::LiveRoomClosing中；
            DCHECK(live_status_ == LiveStatus::StreamStopping ||
                live_status_ == LiveStatus::StreamStoped ||
                live_status_ == LiveStatus::LiveRoomClosing);

            if (IsInTheThirdPartyStreamingMode())
            {
                // 将第三方推流相关的异步任务抛弃，当前用于这几种异步任务：
                // 1、本地流推过来了，要自动开播时发现房间ID等开播必要信息还没成功获取，延时进行开播重试任务；
                // 2、本地流断之后的1分钟关播倒计时任务；
                InvalidateThirdPartyStreamingTask();

                // 如果底层还在跑把底层停掉，没在跑了直接关播
                if (live_status_ == LiveStatus::StreamStoped)
                {
                    // 此时若还处于一分钟续流等待中则IsRunning=false，若已然决定等待了就是true
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

void LivehimeLiveRoomController::CloseLiveRoomInternal(bool stop_streaming, void* param)
{
    // 优雅的关播应该是先断流，流断了再关直播间，关播接口的实际调用就要放在流断的通知里去做
    // 根据开播类型决定断流路径
    if (stop_streaming)
    {
        // 目前的主动关播是不需要也不应该上层提供主动关播的详细信息的
        DCHECK(!param);

        SetLiveStatus(LiveStatus::StreamStopping);

        switch (start_type_)
        {
        case StartLiveType::Normal: // OBS推流路径
        {
            output_controller()->StopStreaming();
        }
        break;
        case StartLiveType::ThirdPartyProxy:    // 第三方代理转发路径，不需要推
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
        // 1、常规模式下不需要断流的关播应该只发生在推流层断流导致的关播场景下，
        // 此时关播推流层应该把本次失败的相关信息传上来，以便上层异步关播完成之后能
        // 通过此信息来做一些关播后的针对性处理；
        // 2、第三方模式下关播的触发有两种情况，一是第三方流断了，二是推CDN断了，对于第一种情况
        // 的关播当前是不带本次推流的相关信息，第二种情况才会带
        if (start_type_ == StartLiveType::Normal)
        {
            DCHECK(param);
        }

        SetLiveStatus(LiveStatus::LiveRoomClosing);

        //答题自定义推流的情况下，断网也会走到这里，不调关播接口，防止影响另一台电脑,并重新开启监听
        if (!CommandLine::ForCurrentProcess()->HasSwitch(bililive::kSwitchCustomRtmp))
        {
            // 不需要断流的关播，一般是推流底层先断了流再通知上层关播的场景
            StopLive(param);
        }
        else
        {
            std::unique_ptr<OutputControllerDelegate::StreamingErrorInfos> info(
                (OutputControllerDelegate::StreamingErrorInfos*)param);
            if (info && info->streaming_errno == obs_proxy::StreamingErrorCode::Success)
            {
                StopLive(info.release());
            }
            else
            {
                HandleLiveRoomClosed(info.release());
            }
        }
    }
}

void LivehimeLiveRoomController::StartLive()
{

}

void LivehimeLiveRoomController::StopLive(void* param)
{

}

void LivehimeLiveRoomController::TerminateStartLiveFlow()
{
    // 常规模式下就把开播按钮状态重置
    if (IsInTheThirdPartyStreamingMode())
    {
        LeaveThirdPartyStreamingMode();
    }
    else
    {
        // 重置开播按钮状态
        NotificationNotify(bililive::NOTIFICATION_LIVEHIME_ENABLE_START_LIVE,
            base::NotificationService::NoDetails());
    }
}

void LivehimeLiveRoomController::OnChoosePartition(PartitionUpdateStatus status, void* param)
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

        // 选分区是由开播流程发起的则结果成功后开始走实际的开播/推流流程
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

        // 旧流程是如果信息更新是由开播流程发起的，那么更新完毕后启用开播按钮，以便用户能再次发起开播流程
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

void LivehimeLiveRoomController::OnStartLive(bool success, int result_code, const std::string& err_msg,
    const secret::LiveStreamingService::StartLiveInfo& start_live_info)
{
    // 开播接口的调用结果打出来
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

        // 接口整个炸了，http底层直接404，success=false，其他字段是无值的
        auto error_info = success ?
            std::make_pair(base::UTF8ToWide(err_msg), result_code) :
            std::make_pair(GetLocalizedString(IDS_STREAMING_GET_LIVE_STATUS_ERROR), kInvalidResponseErrorCode);

        OnStartLiveError(obs_proxy::StreamingErrorCode::StartFailure, error_info.first, error_info.second, start_live_info);

        // 实名认证相关,截止目前(v3.12)只有实名认证补充认证使用这个扩展notice数据段
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

    // 通知房间已打开（当前已实际处于开播状态，只是还没有推流）
    HandleLiveRoomOpened();

    // 是否中止开播流程
    if (stop_live_now_)
    {
        LOG(INFO) << "live room opened while live stop, stop live.";
        CloseLiveRoomInternal(false, nullptr);
        return;
    }

    // 如果没有开播地址就直接视为开播失败，即开播接口成功却不给任何开播地址
    // (从视频云拿地址失败了，但服务端的逻辑是开播和推流分离的，推流相关信息拿不到不影响房间开播成功)
    if (start_live_info.protocol_list.empty())
    {
        auto error_info = std::make_pair(GetLocalizedString(IDS_STREAMING_START_FAILED), 0);
        OnStartLiveError(obs_proxy::StreamingErrorCode::BadPath, error_info.first, error_info.second, start_live_info);

        // 虽然开播接口成功了，但这里不调关播接口，防止是因为其他端正在开播所以这里服务端才不给推流地址
        // 之类的情况导致直播姬直接关播

        SetLiveStatus(LiveStatus::Ready);
        return;
    }

    // 当前开播成功的埋点仅埋在推流底层连接服务端并成功把流推出去的时候，而不是开播接口成功调用的时候；
    // 开播失败的埋点是埋在两个地方，一是开播接口调用失败，而是推流底层尝试连接/推流失败的时候
    SetLiveStatus(LiveStatus::StreamStarting);

    is_stop_live_intercept_ = false;//重置标识

    // 根据开播类型决定推流路径开始推流
    switch (start_type_)
    {
    case StartLiveType::Normal: // OBS推流路径
        {
            LOG(INFO) << "[LC] start streaming.";
            output_controller()->StartStreaming(start_live_info_);
        }
        break;
    case StartLiveType::ThirdPartyProxy:    // 第三方代理转发路径，不需要推
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

void LivehimeLiveRoomController::OnStartLiveError(obs_proxy::StreamingErrorCode streaming_error_type,
    const std::wstring& error_msg, int error_code,
    const secret::LiveStreamingService::StartLiveInfo& start_live_info)
{
    // 一些用户鉴权和房间锁定等特殊的错误就不埋了，防止用户拼命点击贡献失败数据
    if (ShouldReportStreamingFailure(error_code))
    {
        // 把错误类型和错误码处理一下，区分是本地错误还是服务端错误
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

        // 开播失败埋点，纯粹接口失败，没有地址细节
        ReportLiveStreamingOutcome(false, category, error_code, {});
    }

    // 把错误类型和错误码处理一下，构造对应的错误的说明文案
    base::string16 msg = error_msg;
    int code = error_code;
    GetLiveErrorMsg(streaming_error_type, msg, code);

    bool need_face_auth = false;
    // 账号异常,需要开启人脸识别认证
    if ((error_code == (int)StartLiveRoomErrorCode::AuthAccountAnomaly)&& start_live_info.need_face_auth){
        need_face_auth = true;
    }

    // 通知开播失败
    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnOpenLiveRoomError(msg, code,need_face_auth, start_live_info.qr));
}

void LivehimeLiveRoomController::OnStopLive(bool success, int result_code, const std::string& err_msg,
    void* param)
{
    // 关播接口调用结果打出来
    LOG(INFO) << "stop live interface result: " <<
        "ret= " << success << ", code=" << result_code << ", msg=" << err_msg;

    HandleLiveRoomClosed(param);
}

void LivehimeLiveRoomController::HideBookLiveTime()
{
    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnBookLiveInfoChanged(0, L"0"));
}

void LivehimeLiveRoomController::GetBookLiveTimeInfos()
{

}

void LivehimeLiveRoomController::Observe(int type,
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
                if (4 == from_type){  // 开播
                    OpenLiveRoom(start_type_, start_by_hotkey_);
                }
                else if (5 == from_type) { //第三方推流
                    bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_ENTER_INTO_THIRD_PARTY_STREAM_MODE);
                }
            }
        }
        break;
    default:
        break;
    }
}

void LivehimeLiveRoomController::HandleStartLiveNotice(const secret::LiveStreamingService::StartLiveNotice& notice)
{
    switch ((StartLiveNoticeType)notice.type)
    {
    case StartLiveNoticeType::Undefine:
        break;
    case StartLiveNoticeType::AuthNeedSupplement: // 实名认证需资料补充的要弹窗提示
    {
        if (notice.status == (int)StartLiveNoticeStatus::ShouldHandle)
        {
            FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
                OnNoticeAuthNeedSupplement(notice));
        }
    }
        break;
    default:
        // 客户端未发版服务端先上线的通知类型要不要进行通用处理
        //////////////////////////////////////////////////////////////////////////
        NOTREACHED() << "unhandled start live notice type " << notice.type;
        break;
    }
}

void LivehimeLiveRoomController::SetLiveStatus(LiveStatus status)
{
    live_status_ = status;

    if (live_status_ == LiveStatus::Ready)
    {
        stop_live_now_ = false;
    }
}

// OutputControllerDelegate
void LivehimeLiveRoomController::OnPreStartStreaming(bool is_restart)
{
    HandlePreStartStreaming(is_restart);
}

void LivehimeLiveRoomController::OnStreamingStarted(const bililive::StartStreamingDetails& details,
    const obs_proxy::StreamingErrorDetails& streaming_error_details)
{
    // 当前连麦 PK 比较特殊，为保证 UI 的连贯，连麦发起时，
    // 在从推流到B站切换到声网时是不往上层（LiveController）通知的（主要是 OnStreamingStopped）；
    // 但当连麦结束，从推流到声网切换到直推B站时，却会往这里通知（主要是 OnStreamingStarted），
    // 造成这里触发状态检查的 DCHECK，不过不影响流程。于是在这里对连麦放行不检查。
    DCHECK(details.is_colive || details.after_colive || live_status_ == LiveStatus::StreamStarting);

    HandleStreamingStarted(details, streaming_error_details);
}

void LivehimeLiveRoomController::OnRetryStreaming(int index, int total)
{
    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnLiveRoomStreamingRetryNextAddr(index, total));
}

void LivehimeLiveRoomController::OnPreStopStreaming(bool is_restart)
{
    HandlePreStopStreaming(is_restart);
}

void LivehimeLiveRoomController::OnStreamingStopped(OutputControllerDelegate::StreamingErrorInfos& info)
{
    DCHECK(live_status_ == LiveStatus::StreamStarting ||
        live_status_ == LiveStatus::Streaming ||
        live_status_ == LiveStatus::StreamStopping);

    // 断流通知，非错误导致的断流走stoped通知，错误导致的走error通知
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

    // 纯关播，这里是推流层流断后的通知，不需要再切流了
    std::unique_ptr<OutputControllerDelegate::StreamingErrorInfos> err_details =
        std::make_unique<OutputControllerDelegate::StreamingErrorInfos>(std::move(info));
    if (info.should_close_live_room)
    {
        LOG(INFO) << "stream stopped, stop live.";
        CloseLiveRoomInternal(false, err_details.release());
    }
    else
    {
        // 如果底层不需要本层调关播接口，本层就直接在这里通知底层已经关播了，
        // 以便底层自己完整走完断流关播流程完成业务闭环
        HandleLiveRoomClosed(err_details.release());
    }
}

void LivehimeLiveRoomController::NeedNewStreamingAddrInfos()
{
    base::MessageLoop::current()->PostTask(FROM_HERE,
        base::Bind(base::IgnoreResult(&LivehimeLiveRoomController::OpenLiveRoom), weakptr_factory_.GetWeakPtr(),
            StartLiveType::Normal, false, true));
}

void LivehimeLiveRoomController::GetLiveErrorMsg(obs_proxy::StreamingErrorCode streaming_error_code, base::string16& error_msg, int& code)
{
    std::wstring msg;  obs_proxy::FormatStreamingError(streaming_error_code);

    if (code != kNoneServerErrorCode)
    {
        switch (code)
        {
            //这部分错误信息不显示错误码
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

#pragma region(回调业务聚合处理)

void LivehimeLiveRoomController::HandleLiveRoomOpened()
{
    SetLiveStatus(LiveStatus::LiveRoomOpened);

    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnOpenLiveRoomSuccessed(start_live_info_));
}

void LivehimeLiveRoomController::HandlePreStartStreaming(bool is_restart)
{
    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnPreLiveRoomStartStreaming(is_restart));
}

void LivehimeLiveRoomController::HandleStreamingStarted(const bililive::StartStreamingDetails& details,
    const obs_proxy::StreamingErrorDetails& streaming_error_details)
{
    // 开播成功埋点，带地址细节
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

    // 是否中止开播流程
    if (stop_live_now_)
    {
        LOG(INFO) << "streaming started while live stop, stop stream.";
        CloseLiveRoomInternal(true, nullptr);
    }
}

void LivehimeLiveRoomController::HandleLiveStreamingError(obs_proxy::StreamingErrorCode streaming_errno, const std::wstring& error_msg, int error_code)
{
    LOG(INFO) << app_log::kLogStreamFail << "code:" << (int)streaming_errno << "; msg:" << base::UTF16ToUTF8(error_msg);

    SetLiveStatus(LiveStatus::StreamStoped);

    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnLiveRoomStreamingError(streaming_errno, error_msg, error_code));
}

void LivehimeLiveRoomController::HandlePreStopStreaming(bool is_restart)
{
    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnPreLiveRoomStopStreaming());
}

void LivehimeLiveRoomController::HandleLiveStreamingStoped()
{
    LOG(INFO) << app_log::kLogStreamStop;

    SetLiveStatus(LiveStatus::StreamStoped);

    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnLiveRoomStreamingStopped());
}

void LivehimeLiveRoomController::HandleLiveRoomClosed(void* param)
{
    SetLiveStatus(LiveStatus::LiveRoomClosed);

    bool cause_for_restreaming = false;

    // 通知推流层直播间已经关闭，提供一个时机以便其重置内部状态等操作
    switch (start_type_)
    {
    case StartLiveType::Normal: // OBS推流路径
    {
        // 1、常规模式的关播是由推流层断流之后通知到上层进行关房间操作，一定会带有效的断流信息，
        // 否则关播流程是不正确的（或者是上层不无限等待推流层断流通知在等待超时的时候关房间）
        // DCHECK(param);
        // 2、如果是测速过程中关播的，此时推流层是已然停止（等待上层交付测速结果）的状态，
        // 上层直接关播是不带关播信息的
        std::unique_ptr<OutputControllerDelegate::StreamingErrorInfos> info(
            (OutputControllerDelegate::StreamingErrorInfos*)param);
        if (info)
        {
            // 开播失败埋点，断流引发的直播间关闭，有地址细节
            if (info->streaming_errno != obs_proxy::StreamingErrorCode::Success)
            {
                ReportLiveStreamingOutcome(false, bililive::LiveStreamingErrorDetails::Local,
                    bilibase::enum_cast(info->streaming_errno), info->streaming_error_details);

                LOG(INFO) << app_log::kLogLiveStop << "cause by stream error";
            }
            else
            {
                LOG(INFO) << app_log::kLogLiveStop << "user invoke";

                //用户正常关播埋点
                base::StringPairs event_ext;
                event_ext.push_back(std::pair<std::string, std::string>("close_reason", "2")); 
                livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::CloseLiveTracker, event_ext);
            }


            cause_for_restreaming = info->cause_for_restreaming;
        }

        output_controller()->InformLiveRoomClosed(info.get());
    }
    break;
    case StartLiveType::ThirdPartyProxy:    // 第三方代理转发路径，不需要推
    {
        // 第三方模式下关播的触发有两种情况，一是第三方流断了，二是推CDN断了，对于第一种情况
        // 的关播当前是不带本次推流的相关信息，第二种情况才会带
        //DCHECK(param);

        std::unique_ptr<livehime::ThirdPartyOutputControllerDelegate::StreamingErrorInfos> info(
            (livehime::ThirdPartyOutputControllerDelegate::StreamingErrorInfos*)param);

        if (info)
        {
            // 开播失败埋点，断流引发的直播间关闭，有地址细节
            if (info->streaming_errno != obs_proxy::StreamingErrorCode::Success)
            {
                int type = (int)StartLiveType::ThirdPartyProxy;
                type |= info->running_by_obs ? (int)StartLiveType::ThirdPartyProxy_OBS : 0;
                ReportLiveStreamingOutcome(false, bililive::LiveStreamingErrorDetails::Local,
                    bilibase::enum_cast(info->streaming_errno), info->streaming_error_details,
                    type);

                LOG(INFO) << app_log::kLogLiveStop << "tps cause by stream error";
            }
            else
            {
                LOG(INFO) << app_log::kLogLiveStop << "tps user invoke";
            }
        }

        // 如果不是退出第三方模式引发的关播，那么关播后继续开启监听，以便发起新的开播流程
        if (!leave_third_party_mode_)
        {
            // 判断一波当前是否已然处于监听状态，正在监听就不要再开了，
            // 上一次第三方断流后用户选择等待一分钟，选择的时候已经开启新一轮监听了，
            // 这里再开会有问题
            if (!third_party_output_controller()->IsRunning())
            {
                // 重新开启监听等待第三方推流
                bool ret = ThirdPartyStreamingStartListen();
                if (!ret)
                {
                    // 监听失败直接退出第三方模式了
                    base::MessageLoop::current()->PostTask(FROM_HERE,
                        base::Bind(&LivehimeLiveRoomController::OnThirdPartyStreamingStartListenFailed,
                            weakptr_factory_.GetWeakPtr(), L""));
                }
            }
        }
        else
        {
            base::MessageLoop::current()->PostTask(FROM_HERE,
                base::Bind(&LivehimeLiveRoomController::ActualLeaveThirdPartyStreamingMode,
                    weakptr_factory_.GetWeakPtr()));
        }
    }
    break;
    default:
        break;
    }

    GetBililiveProcess()->secret_core()->anchor_info().set_live_status(0);

    // 服务端埋点
    livehime::BehaviorEventReportViaServer(secret::LivehimeViaServerBehaviorEvent::StopLive, {});

    // 通知关播了
    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnLiveRoomClosed(cause_for_restreaming, start_live_info_));

    // 流的断开是重推引发不要急着将状态置为ready
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
bool LivehimeLiveRoomController::NeedSpeedTest()
{
    SetLiveStatus(LiveStatus::WaitForSpeedTestResultWhileRestream);
    return true;
}

#pragma endregion
