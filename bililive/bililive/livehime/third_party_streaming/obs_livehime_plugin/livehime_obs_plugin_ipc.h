#ifndef BILILIVE_BILILIVE_LIVEHIME_OBS_LIVEHIME_PLUGIN_IPC_H_
#define BILILIVE_BILILIVE_LIVEHIME_OBS_LIVEHIME_PLUGIN_IPC_H_

#include "base/basictypes.h"

#include "ipc/ipc_channel.h"
#include "ipc/ipc_listener.h"
#include "ipc/ipc_message.h"

#include "bililive/bililive/ipc/bililive_ipc_channel.h"
#include "bililive/bililive/livehime/third_party_streaming/third_party_streaming_datatypes.h"

class LivehimeOBSPluginIPCODelegate
{
public:
    virtual void OnMessageReceived(const IPC::Message& message) = 0;
    virtual void OnChannelConnected() {}
    virtual void OnChannelError() {}

protected:
    virtual ~LivehimeOBSPluginIPCODelegate() = default;
};

class LivehimeOBSPluginIPCService
    : public BililiveIPCChannelListener
    , public base::RefCountedThreadSafe<LivehimeOBSPluginIPCService>
{
public:
    explicit LivehimeOBSPluginIPCService(LivehimeOBSPluginIPCODelegate* delegate);

    void Initialize();
    void Uninitialize();

    void SendAddrInfos(const livehime::ThirdPartyStreamingAddrInfos& infos);
    void SendLiveStop(const livehime::ThirdPartyStreamingAddrInfos& infos);

private:
    ~LivehimeOBSPluginIPCService();

    void ConnectOnWorkerThread();

    // BililiveIPCChannelListener£¬invoke on io thread
    bool OnMessageReceived(BililiveIPCChannel* channel, const IPC::Message& message) override;
    void OnChannelConnected(BililiveIPCChannel* channel, int32 peer_pid);
    void OnChannelError(BililiveIPCChannel* channel);

    // invoke on main thread
    void InformObserverMessageReceived(const IPC::Message& message);
    void InformObserverChannelConnected();
    void InformObserverChannelError();

private:
    friend class base::RefCountedThreadSafe<LivehimeOBSPluginIPCService>;

    bool shutdown_ = false;
    std::unique_ptr<BililiveIPCChannel> channel_;
    LivehimeOBSPluginIPCODelegate* delegate_ = nullptr;
};


#endif