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
    // ���ڿ����Ĳ�Ӧ�ý������������ģʽ
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

            // ֪ͨ���������ģʽ����UIģ��ø������
            FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_, OnEnterIntoThirdPartyStreamingMode());

            // ��ThirdPartyOutputController�������״̬
            bool ret = ThirdPartyStreamingStartListen();
            if (!ret)
            {
                // ����ʧ��ֱ���˳�������ģʽ��
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

        // ֹͣ���������ʱ��UIֻ�ǻص���������ӣ����ǿ�/�ز���ť��¼�ư�ť��ȻӦ��
        // �ȵ���ֹͣ/����رյ�֪ͨ��������״̬
        if (!IsLiveReady())
        {
            CloseLiveRoom();
        }
        else
        {
            // ��������ײ��Ѿ��ڹ����˾�ֹͣ�乤���������乤��ֹͣ�Ļص��������ʽ�˳�����������ģʽ
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

    // ������ڶ���һ���ӵȴ���Ҫ�ѵȴ��Ի���ص�
    TpsBreakWaittingView::Close();
}

void LivehimeLiveRoomController::ActualLeaveThirdPartyStreamingMode()
{
    if (-1 != g_ThirdPartyStreamingView_seq_id)
    {
        LOG(INFO) << "actual leave tps mode.";

        HideAcceptEventPreviewArea(livehime::PAEFWH_Third_Party, g_ThirdPartyStreamingView_seq_id, true);
        g_ThirdPartyStreamingView_seq_id = -1;

        // ֪ͨ�˳�������ģʽ����UIģ��ø������
        // toolbar��¼�ư�ť��������ť���ã������Զ�������
        // tabarea�������������
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
        message.empty() ? L"���ؿ�������ʧ�ܣ�\n���˳�����������ģʽ��" : message,
        GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_IKNOW));

    // �˳�����������ģʽ
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

        // ʱ�䵽��ֹͣ��ʱ���ر�ѯ�ʿ򣬹ز�
        tps_break_count_down_timer_.Stop();

        TpsBreakWaittingView::Close();

        GetInstance()->CloseLiveRoomInternal(false, nullptr);
    }
}

// ThirdPartyOutputControllerDelegate
void LivehimeLiveRoomController::OnTpocLocalListenError()
{
    // ����ʧ��ֱ���˳�������ģʽ��
    OnThirdPartyStreamingStartListenFailed();
}

void LivehimeLiveRoomController::OnTpocLocalStreamingStarted()
{
    // �Ƿ���ֹ�������̣��������ѵ���ִ֪ͨ��ǰ�û��Ȳ������˳�������ģʽ�ᴥ�����������
    if (stop_live_now_)
    {
        // ��ʱ���ܴ�������״̬��
        // 1����û�����������̣�
        // 2�����������Ѿ������ˣ���ν�����������Ϊ�ϴε������������1���ӹز�����ʱ�����е������������ƹ����ˣ�
        // ���������������ֱ����ֹ���̼��ɣ��ײ�Ĺرղ�����stop_live_now_=true��CloseLiveRoom��ʱ�ͱ�ֹͣ�ˣ����ﲻ������ʲô
        return;
    }

    // ��������������ص��첽����������
    // ������Ҫ�ǽ�OnTpocLocalStreamingStopped��!IsLiveReady()������׵ĵ���ʱ�ز�������ϵ�
    // ������������ʱ�û�ѡ����1���ӵļ�ʱ�ز��ȴ����ڼ䱾�����ֵ�������������������ʱ�ز�����ϵ���
    InvalidateThirdPartyStreamingTask();

    // �����������ˣ�
    // 1�������δ����֮ǰ�������Զ�������
    // 2��������ѿ���֮��������ô��Ӧ���ǵ�������1���ӹز�����ʱ�����������ģ�
    if (IsLiveReady())
    {
		// ��ҪΪ��֪ͨԤ���ز�ҳ�棬�����ˣ��Զ�����������
		FOR_EACH_OBSERVER(LivehimeLiveRoomObserver, observer_list_, OnThirdPartyStreamingIncoming());

        LOG(INFO) << "tps open live room.";
        StartLiveFlowResult ret = OpenLiveRoom(StartLiveType::ThirdPartyProxy, false);

        switch (ret)
        {
        case StartLiveFlowResult::InvalidParamsRetry: // ����ID�����ID��Ч��������δ�ɹ���ȡ��������Ϣ�������Ժ��ٳ���һ��
            LOG(WARNING) << "tps open live room invalid params retry, retry.";
            base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
                base::Bind(&LivehimeLiveRoomController::OnTpocLocalStreamingStarted, weakptr_factory_.GetWeakPtr()),
                base::TimeDelta::FromSeconds(3));
            break;
        case StartLiveFlowResult::InvalidParams: // ����ID�����ID��Ч��OpenLiveRoom�л���д�����ʾ�����ﲻ��Ҫ����ʾ����
            // �û��ķ�����Ϣ������ȡ�����ˣ����ÿ����ˣ��ѵײ�ͣ�ˣ�
            // ���Ǵ�ʱ��δ�˳�������ģʽ�������ڽ������ĵײ���ֹ�¼�HandleLiveRoomClosed�л��ٴο�������
            LOG(WARNING) << "tps open live room invalid params, stop stream.";
            CloseLiveRoomInternal(true, nullptr);
            break;
        case StartLiveFlowResult::Failed:   // ֱ�Ӿ�ʧ���ˣ�OpenLiveRoom�л���д�����ʾ�����ﲻ��Ҫ����ʾ����
            // �ϲ㿪��״̬���ԣ�״̬��ת���ˣ����ó��Կ����ˣ��ѵײ�ͣ�ˣ�
            // ���Ǵ�ʱ��δ�˳�������ģʽ�������ڽ������ĵײ���ֹ�¼�HandleLiveRoomClosed�л��ٴο�������
            LOG(WARNING) << "tps open live room failed, stop stream.";
            CloseLiveRoomInternal(true, nullptr);
            break;
        case StartLiveFlowResult::Succeed:
            break;
        case StartLiveFlowResult::ChooseArea: // �������Ҫ��ѡ�����ľͲ��ùܣ�������Ϣ���������Լ��ٷ���OpenLiveRoom
            LOG(INFO) << "tps open live room choose area.";
            break;
        default:
            break;
        }
    }
    else
    {
        // ���������ƹ�����Ҫ�����õײ�ת����CDN
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
    // �����������ˣ���������������ص��첽����������
    // 1����OnTpocLocalStreamingStarted��InvalidParamsRetry������׵Ŀ���������ϵ���
    // ������������ʱ���ڿ�������δ������������������ڼ䱾�����ֶ��ˣ�Ҫ�������������ϵ���
    // �����ϲ㿴������ʲô��û����һ����
    // 2����startlive�Ľ���ص�����ȡ�����������ɵ����������������ģ�������������е����������ˣ�
    // ��ô��δ���Ľ���ص�����ȡ����ҵ���͵����ǿ���û�ɹ���
    InvalidateThirdPartyStreamingTask();

    // ��������������ʱ������Ѿ����𿪲������ˣ�ֻ�������̵�����ѡ������һ���ϣ�
    // ���Լ�Ȼ��ǰ�����������̵ı�����������ô�Ͱѿ������̷���ѡ��״̬���ã��Ա���ֹ����ѡ����Զ���������߼�
    choose_area_showed_by_start_live_flow_ = false;

    // 1�������ʱ�Ѿ������˿������̵Ļ����͵�����ʾ�Ƿ���Ҫ�ز���
    // 2�������ʱ��û���𿪲����̣���ô�ϲ�Ϳ���װ��ʲô��û����һ�����ײ����½������״̬��
    if (IsLiveReady())
    {
        // ��ǰ��û��ʼ�߿������̣����������ѡ�����ϣ����Ǿ͵�ʲô�¶�û����һ�����ײ�ĬĬ�ؿ���������
        LOG(INFO) << "tps break while live not start, listen again.";
        // ����ʹthird_party_output_controller�ٽ������״̬
        bool ret = ThirdPartyStreamingStartListen();
        if(!ret)
        {
            // �ؿ�����ʧ��ֱ�ӹز����˳�������ģʽ��
            OnThirdPartyStreamingStartListenFailed(L"�������������ѶϿ����ұ������¿�������ʧ�ܣ�\n���˳�����������ģʽ��");
        }
    }
    else
    {
        DCHECK(live_status_ == LiveStatus::LiveRoomOpening ||
            live_status_ == LiveStatus::StreamStarting ||
            live_status_ == LiveStatus::Streaming ||
            live_status_ == LiveStatus::StreamStopping);

        // �ز��ȴ��Ĺ����е����������˲�������
        if (live_status_ != LiveStatus::StreamStopping)
        {
            // ��������������CDN����Ҳ��Ӧ�Ķ��ˣ�
            // ���Բ����Ϲز������Ƕ���֪ͨҪ����ȥ
            HandleLiveStreamingStoped();

            // ���¿�������
            LOG(INFO) << "tps break, listen again.";
            bool ret = GetInstance()->ThirdPartyStreamingStartListen();
            if (!ret)
            {
                // �ؿ�����ʧ��ֱ�ӹز����˳�������ģʽ��
                GetInstance()->OnThirdPartyStreamingStartListenFailed();
                return;
            }

            //��������Զ�������ģʽ(����)������ģʽ�²����ز��ӿ�
            if (!CommandLine::ForCurrentProcess()->HasSwitch(bililive::kSwitchCustomRtmp))
            {
                // ����һ���ӵȴ���ͬʱ���������û������ȴ�ֱ�ӹز�
                LOG(INFO) << "tps break start count down.";
                static int kTpsBreakWaitCountDown = 60; // in sec
                tps_break_count_down_ = kTpsBreakWaitCountDown;
                tps_break_count_down_timer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(1),
                    base::Bind(&LivehimeLiveRoomController::OnThirdPartyStreamingBreakCountDownTimer, weakptr_factory_.GetWeakPtr()));

                // ֪ͨ�ϲ㣬��Ҫ���̹ز����ǵ�1���ӣ���ʱ�͹ز����ڼ������˾ͼ�����
                LOG(INFO) << "tps break ask msgbox.";
                EndDialogSignalHandler handler;
                handler.closure = base::Bind(LivehimeLiveRoomController::LocalStreamingStoppedEndDialog);
                TpsBreakWaittingView::DoModal(GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeWindow(),
                    &handler);
            }
        }
        else
        {
            // ����Ԥ�ڵ��߼�����CloseLiveRoomInternal��ʱ��������Ķ����ص�Ӧ����ͨ��OnTpocStreamingStoped
            // ������֪ͨ�����ǵײ���п���ͨ��OnTpocLocalStreamingStopped����ص���֪ͨ��Ϊ�˱������̵ıջ���
            // ������ֱ�ӵ���OnTpocStreamingStoped����������һ���ز�����
            ThirdPartyOutputControllerDelegate::StreamingErrorInfos
                infos{ obs_proxy::StreamingErrorCode::Success };
            OnTpocStreamingStoped(infos);
        }
    }
}

void LivehimeLiveRoomController::LocalStreamingStoppedEndDialog(int code, void* data)
{
    // ��������û���ʱ�䲻ȷ�ϲ�������ֱ���䱻����˼��þ�û����Ч��������ǿ�ƹرյĻ���
    // �û�ȷ���˲���Ҳ�����ߺ���������
    if (GetInstance()->stop_live_now_)
    {
        LOG(INFO) << "tps break ask msgbox closed after stop live.";
        return;
    }

    if (code == 1)
    {
        LOG(INFO) << "tps break choose stop live.";
        // �ز����˳�������ģʽ
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

    // �������ƹ�����ʱ��ֱ��������ֱ������CDN�Ϳ����ˣ�ǰһ����Ȼѡ�������ˣ���
    // �����ʱֱ�����Ǵ��������治�ɼ�������µĻ�����С���������������ߵ�ס����
    // ��ô�û������Ƿ��Ѿ�������û�и�֪�ģ�����Ҫ���ǰ�ֱ������������ǰ��
    // Ҫ�����ö������ݽ���֪ͨ������ѡ��ϵͳ����֪ͨ����������ǰ�����û��ڵ����������ϵĲ���
    std::tuple<std::wstring, std::wstring> infos{ L"��������ֱ����", L"�����������ɹ���ֱ���ѿ�ʼ��" };
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
    // ready״̬�´�����OnTpocStreamingStoped��ǰֻ�����˳�������ģʽʱֹͣ�ײ�������ֳ���
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

    // ����֪ͨ���Ǵ����µĶ�����stoped֪ͨ�������µ���error֪ͨ
    // ����ͣ�������ĵײ����֪ͨ����Ϊ�������Ķ���
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

    // �ز�
    LOG(INFO) << "tps stream stopped, stop live.";
    std::unique_ptr<livehime::ThirdPartyOutputControllerDelegate::StreamingErrorInfos> err_details =
        std::make_unique<livehime::ThirdPartyOutputControllerDelegate::StreamingErrorInfos>(std::move(info));
    CloseLiveRoomInternal(false, err_details.release());
}
