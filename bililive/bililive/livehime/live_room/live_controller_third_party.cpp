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
#include "bililive/bililive/livehime/user_info/user_info_service.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_hotkey_notify_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/livehime/third_party_streaming/tps_break_waitting_view.h"
#include "bililive/bililive/ui/views/livehime/third_party_streaming/tps_mode_view.h"
#include "bililive/bililive/ui/views/preview/livehime_preview_area_utils.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/secret/bililive_secret.h"

namespace
{
    using namespace std::placeholders;

    int g_ThirdPartyStreamingView_seq_id = -1;

    const char kMutexThirdPartyStreaming[] = "2";
}


bool LivehimeLiveRoomController::IsInTheThirdPartyStreamingMode()
{
    return (g_ThirdPartyStreamingView_seq_id != -1);
}

bool LivehimeLiveRoomController::EnterIntoThirdPartyStreamingMode()
{
    // 正在开播的不应该进入第三方推流模式
    DCHECK(IsLiveReady());
    if (!IsLiveReady())
    {
        return false;
    }

    if (-1 == g_ThirdPartyStreamingView_seq_id)
    {
        std::unique_ptr<ThirdPartyStreamingView> tsv(new ThirdPartyStreamingView());
        g_ThirdPartyStreamingView_seq_id = livehime::ShowAcceptEventPreviewArea(livehime::PAEFWH_Third_Party, tsv.get());
        if (-1 != g_ThirdPartyStreamingView_seq_id)
        {
            LOG(INFO) << "enter into tps mode.";

            leave_third_party_mode_ = false;
            start_type_ = StartLiveType::ThirdPartyProxy;

            // 通知进入第三方模式，各UI模块该干嘛干嘛
            FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_, OnEnterIntoThirdPartyStreamingMode());

            // 令ThirdPartyOutputController进入接流状态
            bool ret = ThirdPartyStreamingStartListen();
            if (!ret)
            {
                // 监听失败直接退出第三方模式了
                OnThirdPartyStreamingStartListenFailed();
            }

            tsv.release();
        }
    }
    return (-1 != g_ThirdPartyStreamingView_seq_id);
}

void LivehimeLiveRoomController::LeaveThirdPartyStreamingMode()
{
    if (-1 != g_ThirdPartyStreamingView_seq_id)
    {
        LOG(INFO) << "leave tps mode.";

        leave_third_party_mode_ = true;
        InvalidateThirdPartyStreamingTask();

        // 停止推流，这个时候UI只是回到常规的样子，但是开/关播按钮和录制按钮依然应该
        // 等到流停止/房间关闭的通知后再重置状态
        if (!IsLiveReady())
        {
            CloseLiveRoom();
        }
        else
        {
            // 如果推流底层已经在工作了就停止其工作，并在其工作停止的回调里面才正式退出第三方推流模式
            if (third_party_output_controller()->IsRunning())
            {
                third_party_output_controller()->Shutdown();
            }
            else
            {
                ActualLeaveThirdPartyStreamingMode();
            }
        }
    }
}

void LivehimeLiveRoomController::InvalidateThirdPartyStreamingTask()
{
    weakptr_factory_.InvalidateWeakPtrs();
    tps_break_count_down_timer_.Stop();

    // 如果正在断流一分钟等待中要把等待对话框关掉
    TpsBreakWaittingView::Close();
}

void LivehimeLiveRoomController::ActualLeaveThirdPartyStreamingMode()
{
    if (-1 != g_ThirdPartyStreamingView_seq_id)
    {
        LOG(INFO) << "actual leave tps mode.";

        HideAcceptEventPreviewArea(livehime::PAEFWH_Third_Party, g_ThirdPartyStreamingView_seq_id, true);
        g_ThirdPartyStreamingView_seq_id = -1;

        // 通知退出第三方模式，各UI模块该干嘛干嘛
        // toolbar：录制按钮、开播按钮禁用（流来自动开播）
        // tabarea：场景面板隐藏
        FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_, OnLeaveThirdPartyStreamingMode());
    }
}

void LivehimeLiveRoomController::ThirdPartyStreamingCloseLiveRoomTimeout()
{
    LOG(INFO) << "tps wait timeout, stop live.";
    CloseLiveRoomInternal(false, nullptr);
}

bool LivehimeLiveRoomController::ThirdPartyStreamingStartListen()
{
    LOG(INFO) << "tps start listen.";
    std::vector<std::string> ip_list;
    int port = 0;
    bool ret = third_party_output_controller()->StartListen(&ip_list, &port);
    if (ret)
    {
        FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
            OnThirdPartyStreamingListened(ip_list, port));
    }
    return ret;
}

void LivehimeLiveRoomController::OnThirdPartyStreamingStartListenFailed(const base::string16& message)
{
    LOG(INFO) << "tps listen failed.";
    livehime::ShowMessageBox(GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeWindow(),
        GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_TITLE),
        message.empty() ? L"本地开启监听失败，\n已退出第三方推流模式。" : message,
        GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_IKNOW));

    // 退出第三方推流模式
    bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(),
        IDC_LIVEHIME_LEAVE_THIRD_PARTY_STREAM_MODE);
}

void LivehimeLiveRoomController::OnThirdPartyStreamingBreakCountDownTimer()
{
    tps_break_count_down_--;
    FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_,
        OnThirdPartyStreamingBreakCountDown(tps_break_count_down_));

    if (tps_break_count_down_ <= 0)
    {
        LOG(INFO) << "tps break wait time out, stop live.";

        // 时间到，停止计时，关闭询问框，关播
        tps_break_count_down_timer_.Stop();

        TpsBreakWaittingView::Close();

        GetInstance()->CloseLiveRoomInternal(false, nullptr);
    }
}

// ThirdPartyOutputControllerDelegate
void LivehimeLiveRoomController::OnTpocLocalListenError()
{
    // 监听失败直接退出第三方模式了
    OnThirdPartyStreamingStartListenFailed();
}

void LivehimeLiveRoomController::OnTpocLocalStreamingStarted()
{
    // 是否中止开播流程（本地流已到来通知执行前用户迫不及待退出第三方模式会触发这个场景）
    if (stop_live_now_)
    {
        // 此时可能处于两种状态：
        // 1、还没开启开播流程；
        // 2、开播流程已经开启了，这次进到这里是因为上次第三方断流后的1分钟关播倒计时过程中第三方又重新推过来了；
        // 不管是哪种情况，直接中止流程即可，底层的关闭操作在stop_live_now_=true（CloseLiveRoom）时就被停止了，这里不用再做什么
        return;
    }

    // 将第三方推流相关的异步任务抛弃，
    // 这里主要是将OnTpocLocalStreamingStopped中!IsLiveReady()情况下抛的倒计时关播任务给废掉
    // （即本地流断时用户选择开启1分钟的计时关播等待，期间本地流又到了则继续播，把这个超时关播任务废掉）
    InvalidateThirdPartyStreamingTask();

    // 第三方流来了，
    // 1、如果是未开播之前来的则自动开播；
    // 2、如果是已开播之后来的那么这应该是第三方在1分钟关播倒计时中重连过来的；
    if (IsLiveReady())
    {
		// 主要为了通知预设素材页面，流来了，自动进入主界面
		FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_, OnThirdPartyStreamingIncoming());

        LOG(INFO) << "tps open live room.";
        StartLiveFlowResult ret = OpenLiveRoom(StartLiveType::ThirdPartyProxy, false);

        switch (ret)
        {
        case StartLiveFlowResult::InvalidParamsRetry: // 房间ID或分区ID无效，本地尚未成功获取到房间信息，可以稍后再尝试一下
            LOG(WARNING) << "tps open live room invalid params retry, retry.";
            base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
                base::Bind(&LivehimeLiveRoomController::OnTpocLocalStreamingStarted, weakptr_factory_.GetWeakPtr()),
                base::TimeDelta::FromSeconds(3));
            break;
        case StartLiveFlowResult::InvalidParams: // 房间ID或分区ID无效，OpenLiveRoom中会进行错误提示，这里不需要做提示操作
            // 用户的房间信息基本获取无望了，不用开播了，把底层停了，
            // 但是此时并未退出第三方模式，所以在接下来的底层终止事件HandleLiveRoomClosed中会再次开启监听
            LOG(WARNING) << "tps open live room invalid params, stop stream.";
            CloseLiveRoomInternal(true, nullptr);
            break;
        case StartLiveFlowResult::Failed:   // 直接就失败了，OpenLiveRoom中会进行错误提示，这里不需要做提示操作
            // 上层开播状态不对，状态流转乱了，不用尝试开播了，把底层停了，
            // 但是此时并未退出第三方模式，所以在接下来的底层终止事件HandleLiveRoomClosed中会再次开启监听
            LOG(WARNING) << "tps open live room failed, stop stream.";
            CloseLiveRoomInternal(true, nullptr);
            break;
        case StartLiveFlowResult::Succeed:
            break;
        case StartLiveFlowResult::ChooseArea: // 如果是需要先选分区的就不用管，分区信息更新完后会自己再发起OpenLiveRoom
            LOG(INFO) << "tps open live room choose area.";
            break;
        default:
            break;
        }
    }
    else
    {
        // 本地流续推过来了要重新让底层转发到CDN
        DCHECK(live_status_ == LiveStatus::StreamStoped);
        if (live_status_ == LiveStatus::StreamStoped)
        {
            LOG(INFO) << "tps start streaming.";
            third_party_output_controller()->StartStreaming(start_live_info_);
        }
        else
        {
            int status = (int)live_status_.load();
            LOG(WARNING) << "tps status unexpected " << status;
        }
    }
}

void LivehimeLiveRoomController::OnTpocLocalStreamingStopped()
{
    // 第三方流断了，将第三方推流相关的异步任务抛弃，
    // 1、将OnTpocLocalStreamingStarted中InvalidParamsRetry情况下抛的开播任务给废掉，
    // 即本地流到来时由于开播条件未成熟而抛了重试任务，期间本地流又断了，要把这个重试任务废掉，
    // 对于上层看来就是什么都没发生一样；
    // 2、将startlive的结果回调任务取消，开房是由第三方流到来触发的，如果开房过程中第三方流断了，
    // 那么把未决的结果回调任务取消，业务层就当做是开播没成功，
    InvalidateThirdPartyStreamingTask();

    // 当本地流到来的时候可能已经发起开播流程了，只不过流程等在了选分区这一步上，
    // 所以既然先前触发开播流程的本地流断了那么就把开播流程分区选择状态重置，以便中止分区选完后自动开播这个逻辑
    choose_area_showed_by_start_live_flow_ = false;

    // 1、如果此时已经发起了开播流程的话，就弹窗提示是否需要关播；
    // 2、如果此时还没发起开播流程，那么上层就可以装作什么都没发生一样，底层重新进入监听状态；
    if (IsLiveReady())
    {
        // 当前还没开始走开播流程（比如等在了选分区上），那就当什么事都没发生一样，底层默默重开监听即可
        LOG(INFO) << "tps break while live not start, listen again.";
        // 主动使third_party_output_controller再进入监听状态
        bool ret = ThirdPartyStreamingStartListen();
        if(!ret)
        {
            // 重开监听失败直接关播并退出第三方模式了
            OnThirdPartyStreamingStartListenFailed(L"第三方数据流已断开，且本地重新开启监听失败，\n已退出第三方推流模式。");
        }
    }
    else
    {
        DCHECK(live_status_ == LiveStatus::LiveRoomOpening ||
            live_status_ == LiveStatus::StreamStarting ||
            live_status_ == LiveStatus::Streaming ||
            live_status_ == LiveStatus::StreamStopping);

        // 关播等待的过程中第三方流断了不用做事
        if (live_status_ != LiveStatus::StreamStopping)
        {
            // 第三方流断了推CDN的流也相应的断了，
            // 可以不马上关播，但是断流通知要发出去
            HandleLiveStreamingStoped();

            // 重新开启监听
            LOG(INFO) << "tps break, listen again.";
            bool ret = GetInstance()->ThirdPartyStreamingStartListen();
            if (!ret)
            {
                // 重开监听失败直接关播并退出第三方模式了
                GetInstance()->OnThirdPartyStreamingStartListenFailed();
                return;
            }

            //如果不是自定义推流模式(答题)，答题模式下不调关播接口
            if (!CommandLine::ForCurrentProcess()->HasSwitch(bililive::kSwitchCustomRtmp))
            {
                // 开启一分钟等待，同时弹窗允许用户放弃等待直接关播
                LOG(INFO) << "tps break start count down.";
                static int kTpsBreakWaitCountDown = 60; // in sec
                tps_break_count_down_ = kTpsBreakWaitCountDown;
                tps_break_count_down_timer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(1),
                    base::Bind(&LivehimeLiveRoomController::OnThirdPartyStreamingBreakCountDownTimer, weakptr_factory_.GetWeakPtr()));

                // 通知上层，是要立刻关播还是等1分钟，超时就关播，期间流到了就继续播
                LOG(INFO) << "tps break ask msgbox.";
                EndDialogSignalHandler handler;
                handler.closure = base::Bind(LivehimeLiveRoomController::LocalStreamingStoppedEndDialog);
                TpsBreakWaittingView::DoModal(GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeWindow(),
                    &handler);
            }
        }
        else
        {
            // 按照预期的逻辑，在CloseLiveRoomInternal的时候，推流层的断流回调应该是通过OnTpocStreamingStoped
            // 来进行通知，但是底层会有可能通过OnTpocLocalStreamingStopped这个回调来通知，为了保持流程的闭环，
            // 在这里直接调用OnTpocStreamingStoped继续进行下一步关播流程
            ThirdPartyOutputControllerDelegate::StreamingErrorInfos
                infos{ obs_proxy::StreamingErrorCode::Success };
            OnTpocStreamingStoped(infos);
        }
    }
}

void LivehimeLiveRoomController::LocalStreamingStoppedEndDialog(int code, void* data)
{
    // 如果由于用户长时间不确认操作，而直播间被服务端检测久久没有有效的推流被强制关闭的话，
    // 用户确认了操作也不再走后续流程了
    if (GetInstance()->stop_live_now_)
    {
        LOG(INFO) << "tps break ask msgbox closed after stop live.";
        return;
    }

    if (code == 1)
    {
        LOG(INFO) << "tps break choose stop live.";
        // 关播并退出第三方模式
        bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(),
            IDC_LIVEHIME_LEAVE_THIRD_PARTY_STREAM_MODE);
    }
    else if (code == 0)
    {
        LOG(INFO) << "tps break choose waiting.";
    }
}

void LivehimeLiveRoomController::OnTpocPreStartStreaming()
{
    HandlePreStartStreaming(false);
}

void LivehimeLiveRoomController::OnTpocStreamingStarted(const obs_proxy::StreamingErrorDetails& streaming_error_details)
{
    HandleStreamingStarted({ false }, streaming_error_details);

    // 第三方推过来的时候直播姬可能直接连接CDN就开播了（前一次已然选过分区了），
    // 如果此时直播姬是处于主界面不可见的情况下的话（最小化、被第三方工具挡住），
    // 那么用户对于是否已经开播是没有感知的，所以要嘛是把直播姬主界面置前，
    // 要嘛以置顶的气泡进行通知，我们选择系统气泡通知，主界面置前会打断用户在第三方工具上的操作
    std::tuple<std::wstring, std::wstring> infos{ L"哔哩哔哩直播姬", L"第三方推流成功，直播已开始。" };
    bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_SHOW_SYS_BALLOON,
        CommandParams<std::tuple<std::wstring, std::wstring>>(&infos));
	livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::SourceSettingSubmitClick,
		"material_type:" + std::to_string((size_t)secret::BehaviorEventMaterialType::ThirdPartyStreaming));
}

void LivehimeLiveRoomController::OnTpocPreStopStreaming()
{
    HandlePreStopStreaming(false);
}

void LivehimeLiveRoomController::OnTpocStreamingStoped(livehime::ThirdPartyOutputControllerDelegate::StreamingErrorInfos& info)
{
    // ready状态下触发的OnTpocStreamingStoped当前只会是退出第三方模式时停止底层监听这种场景
    if (live_status_ == LiveStatus::Ready)
    {
        DCHECK(leave_third_party_mode_);
        if (leave_third_party_mode_)
        {
            base::MessageLoop::current()->PostTask(FROM_HERE,
                base::Bind(&LivehimeLiveRoomController::ActualLeaveThirdPartyStreamingMode,
                    weakptr_factory_.GetWeakPtr()));
        }
        else
        {
            LOG(WARNING) << "unexpected.";
        }
        return;
    }

    DCHECK(live_status_ == LiveStatus::StreamStarting ||
        live_status_ == LiveStatus::Streaming ||
        live_status_ == LiveStatus::StreamStopping);

    // 断流通知，非错误导致的断流走stoped通知，错误导致的走error通知
    // 主动停播引发的底层断流通知被认为是正常的断流
    if ((info.streaming_errno == obs_proxy::StreamingErrorCode::Success) ||
        (live_status_ == LiveStatus::StreamStopping))
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

    // 关播
    LOG(INFO) << "tps stream stopped, stop live.";
    std::unique_ptr<livehime::ThirdPartyOutputControllerDelegate::StreamingErrorInfos> err_details =
        std::make_unique<livehime::ThirdPartyOutputControllerDelegate::StreamingErrorInfos>(std::move(info));
    CloseLiveRoomInternal(false, err_details.release());
}
