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
        //�����Զ���������ֹͣ��ʱ�����������Ϣ
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
        // ���ڶ��̵߳�Ե�ʣ��²����֪ͨ�ϲ㣬�ϲ�Ҫ���¿�������ʱ�������²���̶߳���û�˳���
        // ����Ӧ���ȵ��ϴε��߳���ȫ�˳����ٿ��µ�
        DCHECK(!IsRunning());
        if (IsRunning())
        {
            // �����߳��첽��ԭ�򣬵ײ㹤���̶߳�����֪ͨ��UI��UI������Ҫ�ؿ��ײ�ʱ��
            // ԭ�ȵĹ����߳̿��ܻ�������ȫ�˳�����ʱ���ؿ��ײ��߼���detach�����̣߳�
            // ��ʽ�������ţ�����ʵ��Ч
            LOG(WARNING) << "[tps_oc] old thead still running, ignore.";
            //return false;
        }

        // �����㿪ʼ����
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

        // �����ն˵�ַ
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
        // ����������ģ�鵱ǰ�ݲ�����ַ�б�����ʧ�������߼���ֻʹ�õ�һ��rtmpЭ��ĵ�ַ��
        // new_linkҪ����Э�����Ҫ��
        start_live_info_ = start_live_info;

        // ����������ַ�б�ȡ��һ��rtmp��ַ
        bool match = false;
        std::string prtcs;
        auto pl = start_live_info.protocol_list;
        // �ӿڲ�Ŀǰ��ʵ�Ѿ�����ѡ��Ĭ�ϵ�ַ�嵽�б����˵ģ����ﱣ�յ��ٲ�һ�ζ��߼��ж�Ҳû���Ӱ��
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
        //�����Զ���������ֹͣ��ʱ�����������Ϣ
        if (CommandLine::ForCurrentProcess()->HasSwitch(bililive::kSwitchCustomRtmp))
        {
            get_sei_need_stop_ = true;
            get_sei_is_running_ = false;
        }

        // ֪ͨ�ײ㣬����ֹͣ������ת�ַ�
        if (IsRunning())
        {
            ShutdownTpsProcessorOnWorkerThread(processor_);
        }

        // ���ܴ˴η��������ģʽ���������ǲ���OBS����ģ���֪ͨOBS�������ŵĶ�����
        // OBS���ͨ���ж�������ַ�������Ƿ����
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

        // ������������OBS�����ʶ
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

        //������Զ�������ģʽ(����)
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
        //streaming_error_details.rtmp_host; // ������ģʽ��CDN������ip��ַ�Ͳ�����
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
            // ��������������ת��ģʽ
            if (LivehimeLiveRoomController::GetInstance()->IsLiveReady())
            {
                LOG(INFO) << "enter into obs stream mode.";
                bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_ENTER_INTO_THIRD_PARTY_STREAM_MODE);

                // ��ȡ����������ַ���ط�OBS
                if (ipc_ && local_addr_infos_)
                {
                    running_by_obs_ = true;
                    ipc_->SendAddrInfos(*local_addr_infos_);
                }
            }
            else
            {
                // ������ڿ�������ô�����ǰ�ǵ�����ģʽ�Ҵ��ڶ���״̬�¾Ϳ��Խ���OBS������
                if (LivehimeLiveRoomController::IsInTheThirdPartyStreamingMode())
                {
                    if (LivehimeLiveRoomController::GetInstance()->live_status() ==
                        LivehimeLiveRoomController::LiveStatus::StreamStoped)
                    {
                        LOG(INFO) << "continue streaming in obs stream mode.";

                        // ��ȡ����������ַ���ط�OBS
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
        case streaming_processor::WAIT_FOR_TP_LINK: // ���ڼ���
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformTpListening));
            break;
        case streaming_processor::OPEN_LOCAL_ERROR: // ����ʧ�ܣ��ײ��߳��˳����ϲ��˳�������ģʽ
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformTpListenError));
            break;
        case streaming_processor::TP_STREAM_RUNNING:    // �����������ˣ����Կ�����
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformTpStreamIncoming));
            break;
        case streaming_processor::TP_STREAMING_STOPPED: // �����������ˣ��ײ��߳��˳����ϲ�Ҫ�����Ƿ�ز�
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformTpStreamingStopped));
            break;

        case streaming_processor::START_CDN_STREAMING_FAILED: // �ײ㹤���̶߳�û������������ʧ��
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformStartStreamingFailed));
            break;
        case streaming_processor::START_CDN_STREAMING:  // �ײ���������CDN
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformCDNConnecting));
            break;
        case streaming_processor::CDN_STREAMING_CONNNECT_FAILED:   // ����CDNʧ�ܣ��ײ��߳��˳�
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformCDNConnectError));
            break;
        case streaming_processor::CDN_STREAMING_RUNNING:// ������CDN����
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformCDNStreaming));
            break;
        case streaming_processor::STOP_STREAMING:   // ������ֹ�ײ㹤��
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformPreShutdown));
            break;
        case streaming_processor::CDN_STREAMING_STOPPED:  // CDN���������ˣ��ײ��߳��˳�
            BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
                base::Bind(ThirdPartyOutputController::InformCDNStreamingStopped));
            break;
        case streaming_processor::STREAMING_STOPPED:  // ���������ײ㹤����ֹ��
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
        // ������������OBS�����ʶ
        GetInstance()->running_by_obs_ = false;

        // ����ʧ��
        LOG(INFO) << "[tps_oc] listen failed.";
        if (GetInstance()->delegate_)
        {
            GetInstance()->delegate_->OnTpocLocalListenError();
        }
    }

    void ThirdPartyOutputController::InformTpStreamIncoming()
    {
        // �����������ˣ����Կ�����
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
        // ������������OBS�����ʶ
        GetInstance()->running_by_obs_ = false;

        // ���ϣ�ͣ��
        LOG(INFO) << "[tps_oc] streaming break off.";
        if (GetInstance()->delegate_)
        {
            GetInstance()->delegate_->OnTpocLocalStreamingStopped();
        }
    }

    void ThirdPartyOutputController::InformStartStreamingFailed()
    {
        // ��������CDN
        LOG(INFO) << "[tps_oc] does't running.";
        GetInstance()->StreamingAnnihilated(obs_proxy::StreamingErrorCode::StartFailure, {});
    }

    void ThirdPartyOutputController::InformCDNConnecting()
    {
        // ��������CDN
        LOG(INFO) << "[tps_oc] connecting cdn.";
        if (GetInstance()->delegate_)
        {
            GetInstance()->delegate_->OnTpocPreStartStreaming();
        }
    }

    void ThirdPartyOutputController::InformCDNConnectError()
    {
        // ����CDNʧ��
        LOG(INFO) << "[tps_oc] connect cdn failed.";

        // ����ʧ��Ӧ�е�ַϸ��
        obs_proxy::StreamingErrorDetails streaming_error_details = GetInstance()->GenerateStreamingDetails();
        GetInstance()->StreamingAnnihilated(obs_proxy::StreamingErrorCode::ConnectFailed, streaming_error_details);
    }

    void ThirdPartyOutputController::InformCDNStreaming()
    {
        // CDN������
        LOG(INFO) << "[tps_oc] cdn streaming.";
        if (GetInstance()->delegate_)
        {
            // ���������ɹ��ĵ�ַϸ�ڣ��ϲ�Ҫ�񿪲��ɹ���
            obs_proxy::StreamingErrorDetails streaming_error_details = GetInstance()->GenerateStreamingDetails();
            GetInstance()->delegate_->OnTpocStreamingStarted(streaming_error_details);
        }

        // ������Ͷ���ɹ����ɹ����ӵ�CDN��
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
        // ����ֹͣ�ײ�ȫ������
        LOG(INFO) << "[tps_oc] begin shutdown.";
        if (GetInstance()->delegate_)
        {
            GetInstance()->delegate_->OnTpocPreStopStreaming();
        }
    }

    void ThirdPartyOutputController::InformCDNStreamingStopped()
    {
        // ��CDN��ֹ��
        LOG(INFO) << "[tps_oc] cdn streaming break off.";

        // �����ж���Ӧ�е�ַϸ��
        obs_proxy::StreamingErrorDetails streaming_error_details = GetInstance()->GenerateStreamingDetails();
        GetInstance()->StreamingAnnihilated(obs_proxy::StreamingErrorCode::InvalidStream, streaming_error_details);
    }

    void ThirdPartyOutputController::InformShutdown()
    {
        // �ײ㹤��ȫ����ֹ
        LOG(INFO) << "[tps_oc] shutdown.";
        GetInstance()->StreamingAnnihilated(obs_proxy::StreamingErrorCode::Success, {});
    }

    void ThirdPartyOutputController::AddSEIInfo(const uint8_t* data, int size)
    {
        if (processor_)
            processor_->AddSEIInfo(data, size);
    }
}
