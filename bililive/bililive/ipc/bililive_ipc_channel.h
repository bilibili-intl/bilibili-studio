#ifndef BILILIVE_BILILIVE_BILILIVE_IPC_CHANNEL_H_
#define BILILIVE_BILILIVE_BILILIVE_IPC_CHANNEL_H_

#include <mutex>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread_checker.h"
#include "base/win/scoped_handle.h"

#include "ipc/ipc_channel_proxy.h"
#include "ipc/ipc_listener.h"

class BililiveIPCChannel;
class BililiveIPCChannelListener
{
public:
    virtual bool OnMessageReceived(BililiveIPCChannel* channel, const IPC::Message& message) = 0;
    virtual void OnChannelConnected(BililiveIPCChannel* channel, int32 peer_pid) {}
    virtual void OnChannelError(BililiveIPCChannel* channel) {}

protected:
    virtual ~BililiveIPCChannelListener() = default;
};

class BililiveIPCChannel
    : public IPC::Listener
    , public IPC::Sender
{
public:
    BililiveIPCChannel(const std::string& pipe_name, const std::wstring& pipe_instance_mutex_name,
        BililiveIPCChannelListener* listener);

    ~BililiveIPCChannel();

    bool Init();
    void Shutdown();

    bool Send(IPC::Message* message) override;

    bool server_mode() const
    {
        return server_mode_;
    }

    void SetListener(BililiveIPCChannelListener* listener);

private:
    bool RedetermineCSType();
    void Connect(bool server_mode);

    // Override IPC::Listener£¬invoke on worker thread
    bool OnMessageReceived(const IPC::Message& message) override;
    void OnChannelConnected(int32 peer_pid) override;
    void OnChannelError() override;

private:
    std::string pipe_name_;
    std::wstring pipe_instance_mutex_name_;
    base::win::ScopedHandle instance_mutex_;
    bool server_mode_ = false;
    std::atomic_bool shutdown_ = false;
    std::unique_ptr<IPC::Channel> channel_;

    std::recursive_mutex listener_mutex_;
    BililiveIPCChannelListener* listener_ = nullptr;

    scoped_ptr<base::ThreadChecker> thread_check_;

    base::WeakPtrFactory<BililiveIPCChannel> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(BililiveIPCChannel);
};

#endif  // BILILIVE_BILILIVE_BILILIVE_IPC_CHANNEL_H_