#include "bililive/bililive/livehime/third_party_streaming/third_party_output_controller.h"

#include "bilibase/basic_types.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/ext/bind_lambda.h"
#include "base/ext/callable_callback.h"
#include "base/file_util.h"
#include "base/notification/notification_service.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/command_updater_delegate.h"
#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/utils/net_util.h"
#include "bililive/bililive/utils/setting_util.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/bililive/livehime_obs_ipc_message.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/secret/public/event_tracking_service.h"


namespace
{
    void ShutdownTpsProcessorOnWorkerThread(std::shared_ptr<streaming_processor::IStreamingProcessor> processor)
    {
        if (processor->IsRunning())
        {
            LOG(INFO) << "[tps_oc] shutting down.";
            processor->StopProcess();
        }
    }
}

namespace livehime
{
    using namespace std::placeholders;

    ThirdPartyOutputController* ThirdPartyOutputController::GetInstance()
    {
        return Singleton<ThirdPartyOutputController>::get();
    }

    ThirdPartyOutputController::ThirdPartyOutputController()
        : weakptr_factory_(this)
    {
        processor_.reset(streaming_processor::CreateStreamingProcessor(
            CommandLine::ForCurrentProcess()->HasSwitch(bililive::kSwitchCustomRtmp)));
        processor_->AddObserver(this);

        Initialize();
    }

    ThirdPartyOutputController::~ThirdPartyOutputController()
    {
        //答题自定义推流，停止定时器请求答题信息
        if (CommandLine::ForCurrentProcess()->HasSwitch(bililive::kSwitchCustomRtmp))
        {
            get_sei_need_stop_ = true;
            get_sei_is_running_ = false;
        }
        Uninitialize();
    }

    void ThirdPartyOutputController::Initialize()
    {
        DCHECK(!ipc_);
        ipc_ = new LivehimeOBSPluginIPCService(this);
        ipc_->Initialize();
    }

    void ThirdPartyOutputController::Uninitialize()
    {
        if (ipc_)
        {
            ipc_->Uninitialize();
            ipc_ = nullptr;
        }
        if (processor_)
        {
            processor_->RemoveObserver(this);

            ShutdownTpsProcessorOnWorkerThread(processor_);
            processor_ = nullptr;
        }
    }

    bool ThirdPartyOutputController::IsRunning() const
    {
        if (processor_)
        {
            return processor_->IsRunning();
        }
        return false;
    }

    std::string ThirdPartyOutputController::GetEncoderInfo()
    {
        std::string ret;
        if (processor_)
        {
            ret =  processor_->GetEncoderInfo();
        }
        return ret;
    }

    bool ThirdPartyOutputController::StartListen(std::vector<std::string>* ip_list, int* port)
    {
        // 由于多线程的缘故，下层出错通知上层，上层要重新开启监听时，可能下层的线程都还没退出，
        // 所以应该先等上次的线程完全退出才再开新的
        DCHECK(!IsRunning());
        if (IsRunning())
        {
            // 由于线程异步的原因，底层工作线程断流抛通知到UI，UI处理完要重开底层时，
            // 原先的工作线程可能还不能完全退出，这时候重开底层逻辑，detach掉旧线程，
            // 方式不够优雅，但切实有效
            LOG(WARNING) << "[tps_oc] old thead still running, ignore.";
            //return false;
        }

        // 推流层开始监听
        LOG(INFO) << "[tps_oc] start listen.";
        int local_port = processor_->InitProcess();
        if (local_port == 0)
        {
            LOG(WARNING) << "[tps_oc] start listen failed.";
            return false;
        }

        if (port)
        {
            *port = local_port;
        }

        // 生成终端地址
        auto ipl = bililive::GetLocalValidIpList();
        if (ip_list)
        {
            *ip_list = ipl;
        }

        if (!ipl.empty())
        {
            std::string rtmp = base::StringPrintf("rtmp://%s:%d", ipl[0].c_str(), local_port);
            local_addr_infos_.reset(new ThirdPartyStreamingAddrInfos({ rtmp, "livehime" }));
        }

        return true;
    }

    bool ThirdPartyOutputController::StartStreaming(const secret::LiveStreamingService::StartLiveInfo& start_live_info)
    {
        // 第三方推流模块当前暂不做地址列表连接失败重试逻辑，只使用第一个rtmp协议的地址，
        // new_link要做，协议过滤要做
        start_live_info_ = start_live_info;

        // 遍历推流地址列表，取第一个rtmp地址
        bool match = false;
        std::string prtcs;
        auto pl = start_live_info.protocol_list;
        // 接口层目前其实已经把首选的默认地址插到列表中了的，这里保险点再插一次对逻辑判断也没多大影响
        pl.insert(pl.begin(), start_live_info.rtmp_info);
        for (auto& iter : pl)
        {
            prtcs.append(iter.protocol);
            prtcs.append(",");

            if (iter.protocol.empty() || stricmp(iter.protocol.c_str(), "rtmp") == 0)
            {
                DetermineStreamingAddr(iter);
                match = true;
                break;
            }
        }

        if (!match)
        {
            LOG(INFO) << "[tps_oc] only support rtmp, startlive protocols is " << prtcs.c_str();
            StreamingAnnihilated(obs_proxy::StreamingErrorCode::InvalidProtocol, {});
        }

        return match;
    }

    void ThirdPartyOutputController::Shutdown()
    {
        //答题自定义推流，停止定时器请求答题信息
        if (CommandLine::ForCurrentProcess()->HasSwitch(bililive::kSwitchCustomRtmp))
        {
            get_sei_need_stop_ = true;
            get_sei_is_running_ = false;
        }

        // 通知底层，让其停止流的中转分发
        if (IsRunning())
        {
            ShutdownTpsProcessorOnWorkerThread(processor_);
        }

        // 不管此次发起第三方模式推流流程是不是OBS发起的，都通知OBS让其优雅的断流，
        // OBS插件通过判断推流地址来决定是否断流
        if (ipc_)
        {
            livehime::ThirdPartyStreamingAddrInfos infos;
            if (local_addr_infos_)
            {
                infos = *local_addr_infos_;
            }
            ipc_->SendLiveStop(infos);
        }

        local_addr_infos_.reset();
    }

    void ThirdPartyOutputController::StreamingAnnihilated(obs_proxy::StreamingErrorCode streaming_errno,
        const obs_proxy::StreamingErrorDetails& streaming_error_details)
    {
        if (GetInstance()->delegate_)
        {
            ThirdPartyOutputControllerDelegate::StreamingErrorInfos
                infos{ streaming_errno, streaming_error_details, running_by_obs_ };
            GetInstance()->delegate_->OnTpocStreamingStoped(infos);
        }

        // 重置推流流程OBS发起标识
        running_by_obs_ = false;
    }

    void ThirdPartyOutputController::DetermineStreamingAddr(const secret::LiveStreamingService::ProtocolInfo& live_info)
    {
        LOG(INFO) << "[tps_oc] determine streaming addr.";
        auto handler = base::MakeCallable(base::Bind(
            &ThirdPartyOutputController::OnDetermineStreamingAddr, weakptr_factory_.GetWeakPtr()));

        LivehimeLiveRoomController::GetNewLinkAddr(live_info, handler);
    }

    void ThirdPartyOutputController::OnDetermineStreamingAddr(
        const std::string& cdn_server_addr, const std::string& cdn_server_key, bool new_link_mode,
        const secret::LiveStreamingService::ProtocolInfo& live_info)
    {
        stream_address_.new_link_mode = new_link_mode;
        stream_address_.cdn_address = cdn_server_addr;
        stream_address_.cdn_key = cdn_server_key;
        stream_address_.server_address = live_info.addr;
        stream_address_.server_key = live_info.key;
        stream_address_.new_link = live_info.new_link;
        stream_address_.protocol = live_info.protocol;
        stream_address_.live_key = live_info.live_key;

        LOG(INFO) << "[tps_oc] start streaming.";
        std::string rtmp = cdn_server_addr + cdn_server_key;

        //如果是自定义推流模式(答题)
        if (CommandLine::ForCurrentProcess()->HasSwitch(bililive::kSwitchCustomRtmp))
        {
            CommandLine::StringVector args = CommandLine::ForCurrentProcess()->GetArgs();
            if (args.size() == 1)
            {
                auto iter = args.begin();
                rtmp = WideToUTF8(*iter);
                if (!get_sei_is_running_)
                {
                    get_sei_is_running_ = true;
                    get_sei_need_stop_ = false;
                }
            }
        }
        processor_->StartProcess(rtmp);
    }

    obs_proxy::StreamingErrorDetails ThirdPartyOutputController::GenerateStreamingDetails()
    {
        obs_proxy::StreamingErrorDetails streaming_error_details;
        //streaming_error_details.rtmp_host; // 第三方模式下CDN推流的ip地址就不报了
        streaming_error_details.rtmp_host_name = stream_address_.server_address;
        streaming_error_details.new_link = stream_address_.new_link_mode ? 1 : 0;
        streaming_error_details.custom_streaming = (start_live_info_.room_type == (int)StartLiveRoomType::Studio);
        streaming_error_details.protocol = stream_address_.protocol;
        streaming_error_details.addr = stream_address_.cdn_address;
        streaming_error_details.key = stream_address_.cdn_key;
        return streaming_error_details;
    }

    // LivehimeOBSPluginIPCODelegate
    void ThirdPartyOutputController::OnMessageReceived(const IPC::Message& message)
    {
        switch (message.type())
        {
        case ipc_message::IPC_OBS_TO_LIVEHIME_START_LIVE:
        {
            LOG(INFO) << "obs request livehime start live.";
            // 开启第三方推流转发模式
            if (LivehimeLiveRoomController::GetInstance()->IsLiveReady())
            {
                LOG(INFO) << "enter into obs stream mode.";
                bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_ENTER_INTO_THIRD_PARTY_STREAM_MODE);

                // 获取本地推流地址并回发OBS
                if (ipc_ && local_addr_infos_)
                {
                    running_by_obs_ = true;
                    ipc_->SendAddrInfos(*local_addr_infos_);
                }
            }
            else
            {
                // 如果正在开播，那么如果当前是第三方模式且处于断流状态下就可以接受OBS的推流
                if (LivehimeLiveRoomController::IsInTheThirdPartyStreamingMode())
                {
                    if (LivehimeLiveRoomController::GetInstance()->live_status() ==
                        LivehimeLiveRoomController::LiveStatus::StreamStoped)
                    {
                        LOG(INFO) << "continue streaming in obs stream mode.";

                        // 获取本地推流地址并回发OBS
                        if (ipc_ && local_addr_infos_)
                        {
                            running_by_obs_ = true;
                            ipc_->SendAddrInfos(*local_addr_infos_);
                        }
                    }
                    else
                    {
                        LOG(INFO) << "livehime streaming now, can't accept obs stream.";
                    }
                }
                else
                {
                    LOG(INFO) << "livehime streaming now, can't enter into obs stream mode.";
                }
            }
        }
        break;

        default:
            LOG(WARNING) << "[tps_oc] not support ipc msg type: " << message.type();
            break;
        }
    }

    void ThirdPartyOutputController::OnChannelConnected()
    {
        LOG(INFO) << "[tps_oc] obs ipc connected.";
    }

    void ThirdPartyOutputController::OnChannelError()
    {
        LOG(INFO) << "[tps_oc] obs ipc disconnected.";
    }

    // ProcessorCallback
    void ThirdPartyOutputController::ProcessorStatus(streaming_processor::ProcessStatus status)
    {
        switch (status)
        {
        case streaming_processor::WAIT_FOR_TP_LINK: // 正在监听
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformTpListening));
            break;
        case streaming_processor::OPEN_LOCAL_ERROR: // 监听失败，底层线程退出，上层退出第三方模式
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformTpListenError));
            break;
        case streaming_processor::TP_STREAM_RUNNING:    // 第三方流到了，可以开播了
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformTpStreamIncoming));
            break;
        case streaming_processor::TP_STREAMING_STOPPED: // 第三方流断了，底层线程退出，上层要决定是否关播
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformTpStreamingStopped));
            break;

        case streaming_processor::START_CDN_STREAMING_FAILED: // 底层工作线程都没开，开启推流失败
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformStartStreamingFailed));
            break;
        case streaming_processor::START_CDN_STREAMING:  // 底层正在连接CDN
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformCDNConnecting));
            break;
        case streaming_processor::CDN_STREAMING_CONNNECT_FAILED:   // 连接CDN失败，底层线程退出
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformCDNConnectError));
            break;
        case streaming_processor::CDN_STREAMING_RUNNING:// 正在往CDN推流
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformCDNStreaming));
            break;
        case streaming_processor::STOP_STREAMING:   // 正在终止底层工作
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformPreShutdown));
            break;
        case streaming_processor::CDN_STREAMING_STOPPED:  // CDN的推流断了，底层线程退出
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformCDNStreamingStopped));
            break;
        case streaming_processor::STREAMING_STOPPED:  // 整个推流底层工作终止了
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformShutdown));
            break;
        default:
            break;
        }
    }

    // Inform
    void ThirdPartyOutputController::InformTpListening()
    {
        if (GetInstance()->local_addr_infos_)
        {
            LOG(INFO) << "[tps_oc] listening in " << GetInstance()->local_addr_infos_->addr;
        }
        else
        {
            LOG(INFO) << "[tps_oc] listening.";
        }
    }

    void ThirdPartyOutputController::InformTpListenError()
    {
        // 重置推流流程OBS发起标识
        GetInstance()->running_by_obs_ = false;

        // 监听失败
        LOG(INFO) << "[tps_oc] listen failed.";
        if (GetInstance()->delegate_)
        {
            GetInstance()->delegate_->OnTpocLocalListenError();
        }
    }

    void ThirdPartyOutputController::InformTpStreamIncoming()
    {
        // 第三方流到了，可以开播了
        if (LivehimeLiveRoomController::GetInstance()->IsLiveReady() ||
            LivehimeLiveRoomController::GetInstance()->live_status() == LivehimeLiveRoomController::LiveStatus::StreamStoped)
        {
            if (LivehimeLiveRoomController::GetInstance()->IsLiveReady())
            {
                LOG(INFO) << "[tps_oc] stream incoming, start live.";
            }
            else
            {
                LOG(INFO) << "[tps_oc] stream incoming, keep live.";
            }

            if (GetInstance()->delegate_)
            {
                GetInstance()->delegate_->OnTpocLocalStreamingStarted();
            }
        }
        else
        {
            LOG(WARNING) << "[tps_oc] stream incoming, but live not ready.";
            NOTREACHED();
        }
    }

    void ThirdPartyOutputController::InformTpStreamingStopped()
    {
        // 重置推流流程OBS发起标识
        GetInstance()->running_by_obs_ = false;

        // 流断，停播
        LOG(INFO) << "[tps_oc] streaming break off.";
        if (GetInstance()->delegate_)
        {
            GetInstance()->delegate_->OnTpocLocalStreamingStopped();
        }
    }

    void ThirdPartyOutputController::InformStartStreamingFailed()
    {
        // 正在连接CDN
        LOG(INFO) << "[tps_oc] does't running.";
        GetInstance()->StreamingAnnihilated(obs_proxy::StreamingErrorCode::StartFailure, {});
    }

    void ThirdPartyOutputController::InformCDNConnecting()
    {
        // 正在连接CDN
        LOG(INFO) << "[tps_oc] connecting cdn.";
        if (GetInstance()->delegate_)
        {
            GetInstance()->delegate_->OnTpocPreStartStreaming();
        }
    }

    void ThirdPartyOutputController::InformCDNConnectError()
    {
        // 连接CDN失败
        LOG(INFO) << "[tps_oc] connect cdn failed.";

        // 连接失败应有地址细节
        obs_proxy::StreamingErrorDetails streaming_error_details = GetInstance()->GenerateStreamingDetails();
        GetInstance()->StreamingAnnihilated(obs_proxy::StreamingErrorCode::ConnectFailed, streaming_error_details);
    }

    void ThirdPartyOutputController::InformCDNStreaming()
    {
        // CDN推流中
        LOG(INFO) << "[tps_oc] cdn streaming.";
        if (GetInstance()->delegate_)
        {
            // 开播推流成功的地址细节，上层要埋开播成功点
            obs_proxy::StreamingErrorDetails streaming_error_details = GetInstance()->GenerateStreamingDetails();
            GetInstance()->delegate_->OnTpocStreamingStarted(streaming_error_details);
        }

        // 第三方投屏成功并成功连接到CDN了
        if (GetInstance()->processor_)
        {
            std::string ec = GetInstance()->processor_->GetEncoderInfo();
            int blink = livehime::CheckThirdpartyStreamEncoderType(ec);
            livehime::BehaviorEventReportViaServer(secret::LivehimeViaServerBehaviorEvent::ScanQRCode,
                base::StringPrintf("screencast_type=%d", (blink == 0) ? 2 : 1));

            LOG(INFO) << "[tps_oc] receive stream from " << ec;
        }
    }

    void ThirdPartyOutputController::InformPreShutdown()
    {
        // 正在停止底层全部工作
        LOG(INFO) << "[tps_oc] begin shutdown.";
        if (GetInstance()->delegate_)
        {
            GetInstance()->delegate_->OnTpocPreStopStreaming();
        }
    }

    void ThirdPartyOutputController::InformCDNStreamingStopped()
    {
        // 推CDN终止了
        LOG(INFO) << "[tps_oc] cdn streaming break off.";

        // 推流中断流应有地址细节
        obs_proxy::StreamingErrorDetails streaming_error_details = GetInstance()->GenerateStreamingDetails();
        GetInstance()->StreamingAnnihilated(obs_proxy::StreamingErrorCode::InvalidStream, streaming_error_details);
    }

    void ThirdPartyOutputController::InformShutdown()
    {
        // 底层工作全部终止
        LOG(INFO) << "[tps_oc] shutdown.";
        GetInstance()->StreamingAnnihilated(obs_proxy::StreamingErrorCode::Success, {});
    }

    void ThirdPartyOutputController::AddSEIInfo(const uint8_t* data, int size)
    {
        if (processor_)
            processor_->AddSEIInfo(data, size);
    }
}
