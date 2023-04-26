#ifndef OBS_PLUGINS_OBS_LIVEHIME_IPC_SERVICE_H_
#define OBS_PLUGINS_OBS_LIVEHIME_IPC_SERVICE_H_

#include <atomic>

#include "base/basictypes.h"

#include "ipc/ipc_channel.h"
#include "ipc/ipc_listener.h"
#include "ipc/ipc_message.h"

#include "bililive/bililive/ipc/bililive_ipc_channel.h"

class OBSPluginLivehimeIPCService
    : public BililiveIPCChannelListener
    , public base::RefCountedThreadSafe<OBSPluginLivehimeIPCService>
{
public:
    explicit OBSPluginLivehimeIPCService(scoped_refptr<base::MessageLoopProxy> message_loop_proxy);

    void Initialize();
    void Uninitialize();

    bool livehime_connected() const { return livehime_connected_; }
    bool SendLivehimeMessage(int cmd_type);


private:
    ~OBSPluginLivehimeIPCService();

    void ConnectOnWorkerThread();

    // BililiveIPCChannelListener£¬invoke on io thread
    void OnChannelConnected(BililiveIPCChannel* channel, int32 peer_pid) override;
    void OnChannelError(BililiveIPCChannel* channel) override;
    bool OnMessageReceived(BililiveIPCChannel* channel, const IPC::Message& message) override;

private:
    friend class base::RefCountedThreadSafe<OBSPluginLivehimeIPCService>;

    bool shutdown_ = false;
    std::unique_ptr<BililiveIPCChannel> channel_;
    scoped_refptr<base::MessageLoopProxy> message_loop_proxy_;

    std::atomic_bool livehime_connected_ = false;
};


#endif