#include "bililive_ipc_channel.h"

#include "base/bind.h"
#include "base/threading/thread_restrictions.h"

#include "ipc/ipc_message_macros.h"


BililiveIPCChannel::BililiveIPCChannel(const std::string& pipe_name, const std::wstring& pipe_instance_mutex_name,
    BililiveIPCChannelListener* listener)
    : pipe_name_(pipe_name)
    , pipe_instance_mutex_name_(pipe_instance_mutex_name)
    , listener_(listener)
    , weakptr_factory_(this)
{
}

BililiveIPCChannel::~BililiveIPCChannel()
{
    if (thread_check_)
    {
        DCHECK(thread_check_->CalledOnValidThread());
        LOG_IF(WARNING, !thread_check_->CalledOnValidThread()) << 
            "ipc channel release on invalid thread.";
    }
    Shutdown();
    instance_mutex_.Close();
    channel_.reset();
    thread_check_.reset();
}

bool BililiveIPCChannel::Init()
{
    DCHECK(base::MessageLoopForIO::current());
    DCHECK(!thread_check_.get());
    if (!thread_check_.get())
    {
        thread_check_.reset(new base::ThreadChecker());
    }

    if (!RedetermineCSType())
    {
        return false;
    }

    return true;
}

void BililiveIPCChannel::Shutdown()
{
    shutdown_ = true;
}

bool BililiveIPCChannel::RedetermineCSType()
{
    if (shutdown_)
    {
        return false;
    }

    // 关闭原先的角色定位互斥量
    instance_mutex_.Close();

    instance_mutex_.Set(::CreateMutexW(nullptr, false, pipe_instance_mutex_name_.c_str()));
    if (!instance_mutex_)
    {
        LOG(INFO) << "ipc channel determine role failed.";

        // 只要上层业务不终止IPC服务，那么在这里创建管道或连接管道失败了就要再次重试
        base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
            base::Bind(base::IgnoreResult(&BililiveIPCChannel::RedetermineCSType),
                weakptr_factory_.GetWeakPtr()),
            base::TimeDelta::FromSeconds(2));

        return false;
    }

    server_mode_ = (::GetLastError() != ERROR_ALREADY_EXISTS);

    Connect(server_mode_);

    return true;
}

void BililiveIPCChannel::Connect(bool server_mode)
{
    DCHECK(thread_check_->CalledOnValidThread());

    bool ret = false;
    if (server_mode)
    {
        channel_.reset(new IPC::Channel(pipe_name_, IPC::Channel::MODE_SERVER, this));
    }
    else
    {
        channel_.reset(new IPC::Channel(pipe_name_, IPC::Channel::MODE_CLIENT, this));
    }
    if (channel_)
    {
        ret = channel_->Connect();
        LOG(INFO) << "ipc channel init " << (ret ? "success" : "failed") <<
            ", mode=" << (server_mode ? "server" : "client");
        DCHECK(ret);
        if (!ret)
        {
            channel_.reset();

            // 只要上层业务不终止IPC服务，那么在这里创建管道或连接管道失败了就要再次重试
            base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
                base::Bind(base::IgnoreResult(&BililiveIPCChannel::RedetermineCSType), 
                    weakptr_factory_.GetWeakPtr()),
                base::TimeDelta::FromSeconds(2));
        }
    }
}

bool BililiveIPCChannel::Send(IPC::Message* message)
{
    std::unique_ptr<IPC::Message> msg(message);
    DCHECK(thread_check_->CalledOnValidThread());
    if (!channel_)
    {
        return false;
    }

    return channel_->Send(msg.release());
}

void BililiveIPCChannel::SetListener(BililiveIPCChannelListener* listener)
{
    std::lock_guard<std::recursive_mutex> lock(listener_mutex_);
    listener_ = listener;
}

// Override IPC::Listener，invoke on worker thread
void BililiveIPCChannel::OnChannelConnected(int32 peer_pid)
{
    std::lock_guard<std::recursive_mutex> lock(listener_mutex_);
    if (listener_)
    {
        listener_->OnChannelConnected(this, peer_pid);
    }
}

void BililiveIPCChannel::OnChannelError()
{
    if (shutdown_)
    {
        return;
    }

    {
        std::lock_guard<std::recursive_mutex> lock(listener_mutex_);
        if (listener_)
        {
            listener_->OnChannelError(this);
        }
    }

    // IPC::Channel不管是SERVER还是CLIENT都是只有一条通信实例，所以不管己方当前是什么角色C/S，
    // 只要对端一断开，本方立刻重新定位自身角色，并重新建立通信实例
    LOG(INFO) << "ipc channel error, redetermine channel type.";
    base::MessageLoop::current()->PostTask(FROM_HERE, 
        base::Bind(base::IgnoreResult(&BililiveIPCChannel::RedetermineCSType), weakptr_factory_.GetWeakPtr()));
}

bool BililiveIPCChannel::OnMessageReceived(const IPC::Message& message)
{
    {
        std::lock_guard<std::recursive_mutex> lock(listener_mutex_);
        if (listener_)
        {
            return listener_->OnMessageReceived(this, message);
        }
    }

    return true;
}
