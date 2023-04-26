#include "obs_livehime_ipc_service.h"

#include <util/base.h>
#include <obs.hpp>
#include "UI/obs-frontend-api/obs-frontend-api.h"

#include "base/logging.h"

#include "bililive/public/bililive/livehime_obs_ipc_message.h"


OBSPluginLivehimeIPCService::OBSPluginLivehimeIPCService(scoped_refptr<base::MessageLoopProxy> message_loop_proxy)
    : message_loop_proxy_(message_loop_proxy)
{
    DCHECK(message_loop_proxy_);
}

OBSPluginLivehimeIPCService::~OBSPluginLivehimeIPCService()
{
    Uninitialize();
}

void OBSPluginLivehimeIPCService::Initialize()
{
    DCHECK(!channel_);
    
    message_loop_proxy_->PostTask(FROM_HERE,
        base::Bind(&OBSPluginLivehimeIPCService::ConnectOnWorkerThread, this));
}

void OBSPluginLivehimeIPCService::Uninitialize()
{
    shutdown_ = true;
    if (channel_)
    {
        channel_->SetListener(nullptr);
        channel_->Shutdown();
        if (message_loop_proxy_)
        {
            message_loop_proxy_->DeleteSoon(FROM_HERE, channel_.release());
            message_loop_proxy_ = nullptr;
        }
    }
}

bool OBSPluginLivehimeIPCService::SendLivehimeMessage(int cmd_type)
{
    if (!message_loop_proxy_ || !channel_)
    {
        return false;
    }

    std::unique_ptr<IPC::Message> msg = std::make_unique<IPC::Message>(
        MSG_ROUTING_NONE,
        cmd_type,
        IPC::Message::PRIORITY_NORMAL);

    message_loop_proxy_->PostTask(FROM_HERE,
        base::Bind(base::IgnoreResult(&BililiveIPCChannel::Send), base::Unretained(channel_.get()), msg.release()));

    return true;
}

void OBSPluginLivehimeIPCService::ConnectOnWorkerThread()
{
    if (!channel_)
    {
        channel_.reset(new BililiveIPCChannel(
            ipc_message::kIPCChannelLiveHimeOBSPlugins, ipc_message::kInstanceMutexLiveHimeOBSPlugins, this));
        bool ret = channel_->Init();
        if (!ret)
        {
            channel_.reset();
            NOTREACHED();
            blog(LOG_INFO, "livehime ipc service init failed.");
        }
        else
        {
            blog(LOG_INFO, "livehime ipc service init success, mode=%s.", 
                (channel_->server_mode() ? "server" : "client"));
        }
    }
}

// BililiveIPCChannelListener，invoke on io thread
void OBSPluginLivehimeIPCService::OnChannelConnected(BililiveIPCChannel* channel, int32 peer_pid)
{
    if (shutdown_)
    {
        return;
    }

    livehime_connected_ = true;
    blog(LOG_INFO, "livehime ipc connected.");
}

void OBSPluginLivehimeIPCService::OnChannelError(BililiveIPCChannel* channel)
{
    livehime_connected_ = false;
    blog(LOG_INFO, "livehime ipc disconnected.");

    /*if (shutdown_)
    {
        return;
    }*/
}

bool OBSPluginLivehimeIPCService::OnMessageReceived(BililiveIPCChannel* channel, const IPC::Message& message)
{
    if (shutdown_)
    {
        return false;
    }

    switch (message.type())
    {
    case ipc_message::IPC_LIVEHIME_TO_OBS_STREAM_SETTINGS:
    {
        std::string server;
        std::string key;
        PickleIterator iter(message);
        iter.ReadString(&server);
        iter.ReadString(&key);

        obs_service_t* service = obs_frontend_get_streaming_service();
        if (service)
        {
            obs_data_t* data = obs_service_get_settings(service);
            if (data)
            {
                //const char* ser = obs_data_get_string(data, "service");
                obs_data_set_string(data, "server", server.c_str());
                obs_data_set_string(data, "key", key.c_str());

                obs_service_update(service, data);

                // 推流
                blog(LOG_INFO, "livehime start streaming.");
                obs_frontend_streaming_start();
            }
        }
    }
        break;
    case ipc_message::IPC_LIVEHIME_TO_OBS_STOP_STREAMING:
        {
            std::string server;
            std::string key;
            PickleIterator iter(message);
            iter.ReadString(&server);
            iter.ReadString(&key);

            obs_service_t* service = obs_frontend_get_streaming_service();
            if (service)
            {
                obs_data_t* data = obs_service_get_settings(service);
                if (data)
                {
                    std::string pref_server = obs_data_get_string(data, "server");
                    std::string pref_key = obs_data_get_string(data, "key");

                    // 地址完全对得上才断流，防止直播姬断了完全不相关的OBS流
                    if (server == pref_server && 
                        key == pref_key)
                    {
                        blog(LOG_INFO, "livehime stop streaming.");
                        obs_frontend_streaming_stop();
                    }
                }
            }
        }
        break;

    case ipc_message::IPC_LIVEHIME_TO_OBS:
    {
        blog(LOG_INFO, "IPC_LIVEHIME_TO_OBS");

        std::unique_ptr<IPC::Message> msg = std::make_unique<IPC::Message>(
            MSG_ROUTING_NONE,
            ipc_message::IPC_OBS_TO_LIVEHIME,
            IPC::Message::PRIORITY_NORMAL);

        message_loop_proxy_->PostTask(FROM_HERE,
            base::Bind(base::IgnoreResult(&BililiveIPCChannel::Send), base::Unretained(channel_.get()), msg.release()));
    }
    break;
    case ipc_message::IPC_BOTHWAY:
        {
            blog(LOG_INFO, "IPC_BOTHWAY");

            std::unique_ptr<IPC::Message> msg = std::make_unique<IPC::Message>(
                MSG_ROUTING_NONE,
                ipc_message::IPC_MSG_END,
                IPC::Message::PRIORITY_NORMAL);

            message_loop_proxy_->PostTask(FROM_HERE,
                base::Bind(base::IgnoreResult(&BililiveIPCChannel::Send), base::Unretained(channel_.get()), msg.release()));
        }
        break;
    default:
        blog(LOG_WARNING, "not support ipc msg type: %d", message.type());
        break;
    }
    return true;
}
