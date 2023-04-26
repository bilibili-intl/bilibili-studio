#include "livehime_obs_plugin_ipc.h"

#include "base/logging.h"

#include "bililive/bililive/command_updater_delegate.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/livehime/obs/output_controller.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/livehime_obs_ipc_message.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"


LivehimeOBSPluginIPCService::LivehimeOBSPluginIPCService(LivehimeOBSPluginIPCODelegate* delegate)
    : delegate_(delegate)
{
}

LivehimeOBSPluginIPCService::~LivehimeOBSPluginIPCService()
{
}

void LivehimeOBSPluginIPCService::Initialize()
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    DCHECK(!channel_);
    BililiveThread::PostTask(BililiveThread::IO, FROM_HERE,
        base::Bind(&LivehimeOBSPluginIPCService::ConnectOnWorkerThread, this));
}

void LivehimeOBSPluginIPCService::Uninitialize()
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    shutdown_ = true;
    delegate_ = nullptr;
    if (channel_)
    {
        channel_->SetListener(nullptr);
        channel_->Shutdown();
        BililiveThread::DeleteSoon(BililiveThread::IO, FROM_HERE, channel_.release());
    }
}

void LivehimeOBSPluginIPCService::ConnectOnWorkerThread()
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::IO));
    if (!channel_)
    {
        channel_.reset(new BililiveIPCChannel(
            ipc_message::kIPCChannelLiveHimeOBSPlugins, ipc_message::kInstanceMutexLiveHimeOBSPlugins, this));
        bool ret = channel_->Init();
        if (!ret)
        {
            channel_.reset();
            LOG(WARNING) << "tps ipc service init failed, will auto try until succeed.";
        }
        else
        {
            LOG(INFO) << "tps ipc service init success, mode=" <<
                (channel_->server_mode() ? "server" : "client");
        }
    }
}

void LivehimeOBSPluginIPCService::SendAddrInfos(const livehime::ThirdPartyStreamingAddrInfos& infos)
{
    if (channel_)
    {
        std::unique_ptr<IPC::Message> msg = std::make_unique<IPC::Message>(
            MSG_ROUTING_NONE,
            ipc_message::IPC_LIVEHIME_TO_OBS_STREAM_SETTINGS,
            IPC::Message::PRIORITY_NORMAL);
        msg->WriteString(infos.addr);
        msg->WriteString(infos.code);

        LOG(INFO) << "tps ipc send obs streaming addr.";
        BililiveThread::PostTask(BililiveThread::IO, FROM_HERE,
            base::Bind(base::IgnoreResult(&BililiveIPCChannel::Send), base::Unretained(channel_.get()), msg.release()));
    }
}

void LivehimeOBSPluginIPCService::SendLiveStop(const livehime::ThirdPartyStreamingAddrInfos& infos)
{
    std::unique_ptr<IPC::Message> msg = std::make_unique<IPC::Message>(
        MSG_ROUTING_NONE,
        ipc_message::IPC_LIVEHIME_TO_OBS_STOP_STREAMING,
        IPC::Message::PRIORITY_NORMAL);
    msg->WriteString(infos.addr);
    msg->WriteString(infos.code);

    LOG(INFO) << "tps ipc send obs stop streaming.";
    BililiveThread::PostTask(BililiveThread::IO, FROM_HERE,
        base::Bind(base::IgnoreResult(&BililiveIPCChannel::Send), base::Unretained(channel_.get()), msg.release()));
}

// BililiveIPCChannelListener£¬invoke on io thread
void LivehimeOBSPluginIPCService::OnChannelConnected(BililiveIPCChannel* channel, int32 peer_pid)
{
    BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
        base::Bind(&LivehimeOBSPluginIPCService::InformObserverChannelConnected, this));
}

void LivehimeOBSPluginIPCService::OnChannelError(BililiveIPCChannel* channel)
{
    BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
        base::Bind(&LivehimeOBSPluginIPCService::InformObserverChannelError, this));
}

bool LivehimeOBSPluginIPCService::OnMessageReceived(BililiveIPCChannel* channel, const IPC::Message& message)
{
    BililiveThread::PostTask(BililiveThread::UI, FROM_HERE, 
        base::Bind(&LivehimeOBSPluginIPCService::InformObserverMessageReceived, this, message));

    return true;
}

// invoke on main thread
void LivehimeOBSPluginIPCService::InformObserverChannelConnected()
{
    if (shutdown_)
    {
        return;
    }

    if (delegate_)
    {
        delegate_->OnChannelConnected();
    }
}

void LivehimeOBSPluginIPCService::InformObserverChannelError()
{
    if (shutdown_)
    {
        return;
    }

    if (delegate_)
    {
        delegate_->OnChannelError();
    }
}

void LivehimeOBSPluginIPCService::InformObserverMessageReceived(const IPC::Message& message)
{
    if (shutdown_)
    {
        return;
    }

    bool handle = false;
    switch (message.type())
    {
    case ipc_message::IPC_OBS_TO_LIVEHIME:
    {
        handle = true;
        DLOG(INFO) << "IPC_OBS_TO_LIVEHIME";

        std::unique_ptr<IPC::Message> msg = std::make_unique<IPC::Message>(
            MSG_ROUTING_NONE,
            ipc_message::IPC_BOTHWAY,
            IPC::Message::PRIORITY_NORMAL);

        BililiveThread::PostTask(BililiveThread::IO, FROM_HERE,
            base::Bind(base::IgnoreResult(&BililiveIPCChannel::Send), base::Unretained(channel_.get()), msg.release()));
    }
        break;
    case ipc_message::IPC_BOTHWAY:
    {
        handle = true;
        DLOG(INFO) << "IPC_BOTHWAY";

        std::unique_ptr<IPC::Message> msg = std::make_unique<IPC::Message>(
            MSG_ROUTING_NONE,
            ipc_message::IPC_BOTHWAY,
            IPC::Message::PRIORITY_NORMAL);

        BililiveThread::PostTask(BililiveThread::IO, FROM_HERE,
            base::Bind(base::IgnoreResult(&BililiveIPCChannel::Send), base::Unretained(channel_.get()), msg.release()));
    }
        break;
    case ipc_message::IPC_MSG_END:
        {
            DLOG(INFO) << "IPC_MSG_END";
            handle = true;
        }
        break;
    default:
        break;
    }

    if (!handle && delegate_)
    {
        delegate_->OnMessageReceived(message);
    }
}
