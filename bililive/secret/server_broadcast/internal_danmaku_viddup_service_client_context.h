#ifndef BILILIVE_SECRET_SERVER_BROADCAST_INTERNAL_DANMAKU_VIDDUP_SERVICE_CLIENT_CONTEXT_H
#define BILILIVE_SECRET_SERVER_BROADCAST_INTERNAL_DANMAKU_VIDDUP_SERVICE_CLIENT_CONTEXT_H

#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/synchronization/lock.h"
#include "base/timer/timer.h"

//#include "obs/obs-studio/deps/w32-pthreads/pthread.h"

#include "bililive/public/common/refcounted_dictionary.h"

#include "bililive/public/protobuf/intl/broadcast.pb.h"
#include "bililive/public/protobuf/intl/room.pb.h"
#include "bililive/public/protobuf/intl/DMData.pb.h"

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef std::shared_ptr<boost::asio::ssl::context> context_ptr;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// pull out the type of messages sent by our config
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;


namespace base {
    class Value;
}

namespace livehime
{
    class GoimPacket;
    class GoimClient;
    class DanmakuPacketStatistics;
};

struct ChatData
{
    enum ChatType
    {
        System = 0,         //系统弹幕
        User,               //用户发送
        Gift,               //礼物弹幕
        LiveKeyChange,      //场次变更
        MuteMsg,            //禁言广播
        WarnMs,             //警告广播
        PopupMsg,           //弹窗广播
        BlockMsg,           //禁播广播
        LikesMsg,           //点赞广播
        UltimatePrizeMsg,   //大奖广播
        RechargeBarMsg      //充能条广播
    };

    enum UserIdentity
    {
        Normal = 0,
        Rank1 = 1,
        Rank2 = 2,
        Rank3 = 3,
    };

    ChatType type = System;
    UserIdentity id = Normal;
    std::string content;
    std::string userName;
    bool isAdmin = false;
    std::string gifFileUrl;  //url
    std::string giftName;
    std::string comboString;
    int treasureID = 0;

    // 禁播信息
    std::string notice;
    int64 end_time;

    std::string popupTitle;
    std::string popupContent;
    int duration = 0;
};

namespace internal {
    class DanmakuViddupServiceClientContext :
            public base::RefCountedThreadSafe <DanmakuViddupServiceClientContext> {
    public:
        enum WebSocketConnectStatus
        {
            Empty = 0,
            Connecting,
            TlsInit,
            Open,
            Closed,
            Failed,
            Interrupt
        };

        //do on UI
        DanmakuViddupServiceClientContext(
            int64_t room_id, int64_t uid, const std::string& token,
            const std::vector<std::pair<std::string, unsigned short>>& goim_server_list,
            RefDictionaryParamClosure msg_notify,
            base::Callback<void(int)> audience_num_notify,
            std::function<void()> token_exp_notify,
            std::function<void(const std::set<uint32_t>&)> danmaku_ack);

        //can't be concurrent
        //don't invoke these more than one time;
        bool Start();
        void Stop();

        //do on UI
        //don't presume that IO thread is quit before UI thread
        void Invalid();

    private:
        static void* DanmakuThread(void* p);
        static void* HeartBeatThread(void* p);

        void InitWebSocket();
        bool ConnectWebSocket();
        void RunWebSocket();
        void StopWebSocket();
        void DisConnectWebSocket();
        void ReConnectWebSocket();
        void SendAuthBuffer(client* c, websocketpp::connection_hdl hdl);
        void SendEnterRoomBuffer(client* c, websocketpp::connection_hdl hdl);
        void SendHeartBeat(websocketpp::connection_hdl hdl);

        // websocket handler
        context_ptr OnTlsInit(client* c, websocketpp::connection_hdl hdl);
        void OnMessage(client* c, websocketpp::connection_hdl hdl, message_ptr msg);
        void OnOpen(client* c, websocketpp::connection_hdl hdl);
        void OnClose(client* c, websocketpp::connection_hdl hdl);
        void OnFail(client* c, websocketpp::connection_hdl hdl);
        void OnInterrupt(client* c, websocketpp::connection_hdl hdl);

        void OnHeartBeat(websocketpp::connection_hdl hdl);
        void OnCheckHeartBeat(websocketpp::connection_hdl hdl);

        void ProcessReceviceMsg(const bilibili::broadcast::v1::BroadcastFrame& broadcast_frame);
        void ProcessDMReply(const bilibili::broadcast::message::intl::DmEventReply& reply);

        void TestDMReply();

        ChatData TransferProtoData(const bilibili::broadcast::message::intl::DmItem& item);

        char* FormatMessagePayload(message_ptr msg);

        void CloseConnections();

        std::string GetConnectStatus(WebSocketConnectStatus status);

    private:
        std::vector<std::pair<std::string, unsigned short> > goim_server_list_;
        std::unordered_map<std::string, int> connected_history_;

        std::mutex danmakuSocketVarMutex;
        std::weak_ptr<livehime::GoimClient> danmakuSocket;
        int64_t room_id_;
        int64_t uid_;
        std::string token_;

        //pthread_t networkThread;

        base::Lock valid_lock_;
        volatile bool valid_;
        base::Callback<void(int)> audience_num_notify_;
        RefDictionaryParamClosure msg_notify_;
        std::function<void()> token_exp_notify_;

        std::set<uint32_t> seq_set_;
        std::function<void(const std::set<uint32_t>&)> danmaku_ack_;

        volatile bool isStopping;

        client websocket_client_;
        bool is_connected_;
        int last_message_id_;
        int try_heart_times_;
        bool recive_heart_;
        int64_t lastHeartbeatTime_;
        int64_t lastCheckHeartbeatTime_;
        //std::vector<websocketpp::connection_hdl> con_list_;
        client::connection_ptr con_current_;
        volatile bool is_Heartbeat_;
        volatile bool is_CheckHeartbeat_;
        volatile bool is_reconnect_;
        WebSocketConnectStatus con_status_;

        base::RepeatingTimer<DanmakuViddupServiceClientContext> timer_;
        base::OneShotTimer<DanmakuViddupServiceClientContext> heart_check_timer_;
        base::RepeatingTimer<DanmakuViddupServiceClientContext> repeat_heart_check_timer_;
        base::OneShotTimer<DanmakuViddupServiceClientContext> reconnect_timer_;

        base::RepeatingTimer<DanmakuViddupServiceClientContext> timer_test_danmaku_;

        //base::WeakPtrFactory<DanmakuViddupServiceClientContext> weak_factory_;

        DISALLOW_COPY_AND_ASSIGN(DanmakuViddupServiceClientContext);
    };
}

#endif  // BILILIVE_SECRET_SERVER_BROADCAST_INTERNAL_DANMAKU_VIDDUP_SERVICE_CLIENT_CONTEXT_H