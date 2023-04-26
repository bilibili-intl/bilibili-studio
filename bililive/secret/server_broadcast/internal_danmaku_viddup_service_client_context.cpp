#include "bililive/secret/server_broadcast/internal_danmaku_viddup_service_client_context.h"

#include "obs/obs-studio/deps/w32-pthreads/pthread.h"

#include "base/Bind.h"
#include "base/guid.h"
#include "base/json/json_parser.h"
#include "base/json/json_string_value_serializer.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"

#include "bililive/secret/core/bililive_secret_core_impl.h"
#include "bililive/secret/server_broadcast/livehime_goim.hpp"

#define WEB_SOCKET_THREAD_WAITTIME 1000
#define HEART_BEAT_TIME_INTERVAL 4 * 60  // 4min heartbeat
#define CHECK_HEART_BEAT_TIME_INTERVAL 29  // less than 30s check heartbeat

const int kTryHeartTimesMax = 2;
const char kURLDanmakuViddupWS[] = "wss://broadcast.biliintl.com:7826/sub?platform=web";

pthread_t networkThread;
pthread_t heartbeathread;


namespace internal { //namespace internal

DanmakuViddupServiceClientContext::DanmakuViddupServiceClientContext(
    int64_t room_id, int64_t uid, const std::string& token,
    const std::vector<std::pair<std::string, unsigned short>>& goim_server_list,
    RefDictionaryParamClosure msg_notify,
    base::Callback<void(int)> audience_num_notify,
    std::function<void()> token_exp_notify,
    std::function<void(const std::set<uint32_t>&)> danmaku_ack)
    : goim_server_list_(goim_server_list),
      room_id_(room_id),
      uid_(uid),
      token_(token),
      audience_num_notify_(audience_num_notify),
      msg_notify_(msg_notify),
      token_exp_notify_(token_exp_notify),
      danmaku_ack_(danmaku_ack),
      isStopping(false),
      is_connected_(false),
      last_message_id_(0),
      try_heart_times_(0),
      recive_heart_(false),
      lastHeartbeatTime_(0),
      lastCheckHeartbeatTime_(0),
      is_Heartbeat_(false),
      is_CheckHeartbeat_(false),
      is_reconnect_(false),
      con_status_(WebSocketConnectStatus::Empty)/*,
      weakptr_factory_(this)*/
{
    networkThread = {0};
    heartbeathread = { 0 };

    valid_ = false;
}

bool DanmakuViddupServiceClientContext::Start()
{
    LOG(INFO) << "[ws] Start";
    isStopping = false;

    //if (!timer_test_danmaku_.IsRunning())
    //{
    //    timer_test_danmaku_.Start(FROM_HERE,
    //        base::TimeDelta::FromMilliseconds(3000),
    //        base::Bind(&DanmakuViddupServiceClientContext::TestDMReply,
    //            /*weak_factory_.GetWeakPtr()*/this));
    //}
    //return true;

    //开线程后台干活
    if (0 == pthread_create(&networkThread,
        nullptr,
        &DanmakuViddupServiceClientContext::DanmakuThread,
        this)) {
        AddRef();

        if (0 == pthread_create(&heartbeathread,
            nullptr,
            &DanmakuViddupServiceClientContext::HeartBeatThread,
            this)) {
            AddRef();

            valid_ = true;
            return true;
        }
    }

    return false;
}

void DanmakuViddupServiceClientContext::TestDMReply()
{
    base::DictionaryValue* dict = new base::DictionaryValue();
    base::ListValue* list = new base::ListValue();

    base::DictionaryValue* value = new base::DictionaryValue();
    value->SetInteger("type", ChatData::WarnMs);
    value->SetInteger("id", 0);
    value->SetString("content", "6666-test-danmaku");
    value->SetString("user_name", "2233");
    value->SetBoolean("is_admin", true);
    value->SetString("gif_file_url", "https://pic.bstarstatic.com/live/treasure/2d8fa2a0ac30430516c5ead3960548a9.gif");
    value->SetString("gift_name", "flowers");
    value->SetString("combo_string", "x2 in total");
    value->SetInteger("treasure_iD", 2);
    list->Append(value);

    dict->SetString("cmd", "DANMU_MSG");
    dict->Set("info", list);
    std::string json_content;
    dict->GetAsString(&json_content);
    RefDictionary ref_dict = RefCountedDictionary::TakeDictionary(
        reinterpret_cast<base::DictionaryValue*>(dict));

    if (!msg_notify_.is_null())
    {
        msg_notify_.Run(ref_dict, json_content);
    }
}

void DanmakuViddupServiceClientContext::Stop()
{
    LOG(INFO) << "[ws] Stop begin";
    isStopping = true;

    CloseConnections();
    
    LOG(INFO) << "[ws] Stop networkThread";
    if (nullptr != networkThread.p) {
        void* useless;
        pthread_join(networkThread, &useless);
        pthread_detach(networkThread);
        networkThread = { 0 };
    }

    LOG(INFO) << "[ws] Stop heartbeathread";
    if (nullptr != heartbeathread.p) {
        void* useless;
        pthread_join(heartbeathread, &useless);
        pthread_detach(heartbeathread);
        heartbeathread = { 0 };
    }

    LOG(INFO) << "[ws] Stop end";
}

void DanmakuViddupServiceClientContext::Invalid()
{
    base::AutoLock lock(valid_lock_);
    valid_ = false;
}

void * DanmakuViddupServiceClientContext::DanmakuThread(void* p)
{
    LOG(INFO) << "[ws] DanmakuThread begin";
    auto This = static_cast<DanmakuViddupServiceClientContext*>(p);

    // 初始化WebSocket客户端
    This->InitWebSocket();

    // 连接到WebSocket服务器
    This->ConnectWebSocket();
    This->RunWebSocket();

    This->Release();
    LOG(INFO) << "[ws] DanmakuThread end";
    return nullptr;
}

void* DanmakuViddupServiceClientContext::HeartBeatThread(void* p) {
    LOG(INFO) << "[ws] HeartBeatThread begin";
    auto This = static_cast<DanmakuViddupServiceClientContext*>(p);

    while (!This->isStopping) {
        if (This->is_Heartbeat_) {
            int64_t currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
            if (currentTime - This->lastHeartbeatTime_ > HEART_BEAT_TIME_INTERVAL)
            {
                This->lastHeartbeatTime_ = currentTime;
                This->OnHeartBeat(This->con_current_->get_handle());
            }
        }

        if (This->is_CheckHeartbeat_) {
            int64_t currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
            if (currentTime - This->lastCheckHeartbeatTime_ > CHECK_HEART_BEAT_TIME_INTERVAL) {
                This->lastCheckHeartbeatTime_ = currentTime;
                This->OnCheckHeartBeat(This->con_current_->get_handle());
            }
        }

        Sleep(WEB_SOCKET_THREAD_WAITTIME);
    }

    This->Release();
    LOG(INFO) << "[ws] HeartBeatThread end";
    return nullptr;
}

void DanmakuViddupServiceClientContext::InitWebSocket() {
    LOG(INFO) << "[ws] InitWebSocket";

    // Set logging to be pretty verbose (everything except message payloads)
    websocket_client_.set_access_channels(websocketpp::log::alevel::all);
    websocket_client_.clear_access_channels(websocketpp::log::alevel::frame_payload);

    // Initialize ASIO
    websocket_client_.init_asio();
    websocket_client_.set_tls_init_handler(bind(&DanmakuViddupServiceClientContext::OnTlsInit, this, &websocket_client_, ::_1));

    // Register our message handler
    websocket_client_.set_message_handler(bind(&DanmakuViddupServiceClientContext::OnMessage, this, &websocket_client_, ::_1, ::_2));
    websocket_client_.set_open_handler(bind(&DanmakuViddupServiceClientContext::OnOpen, this, &websocket_client_, ::_1));
    websocket_client_.set_close_handler(bind(&DanmakuViddupServiceClientContext::OnClose, this, &websocket_client_, ::_1));
    websocket_client_.set_fail_handler(bind(&DanmakuViddupServiceClientContext::OnFail, this, &websocket_client_, ::_1));
    websocket_client_.set_interrupt_handler(bind(&DanmakuViddupServiceClientContext::OnInterrupt, this, &websocket_client_, ::_1));
}

bool DanmakuViddupServiceClientContext::ConnectWebSocket() {
    LOG(INFO) << "[ws] ConnectWebSocket";
    is_connected_ = false;

    try {
        websocketpp::lib::error_code ec;
        client::connection_ptr con = websocket_client_.get_connection(kURLDanmakuViddupWS, ec);
        if (ec) {
            LOG(WARNING) << "[ws] could not create connection because: " << ec.message();
            return false;
        }

        con_status_ = WebSocketConnectStatus::Connecting;
        con_current_ = con;

        // Add Sec-WebSocket-Protocol
        con->add_subprotocol("proto");

        // Note that connect here only requests a connection. No network messages are
        // exchanged until the event loop starts running in the next line.
        websocket_client_.connect(con);

        //// Start the ASIO io_service run loop
        //// this will cause a single connection to be made to the server. c.run()
        //// will exit when this connection is closed.
        //websocket_client_.run();
    } catch (websocketpp::exception const& e) {
        LOG(WARNING) << "[ws] connection exception occured: " << e.what();
        return false;
    }

    is_connected_ = true;
    return true;
}

void DanmakuViddupServiceClientContext::RunWebSocket() {
    // Start the ASIO io_service run loop
    // this will cause a single connection to be made to the server. c.run()
    // will exit when this connection is closed.
    websocket_client_.run();
}

void DanmakuViddupServiceClientContext::StopWebSocket() {
    // Stop the ASIO io_service run loop
    websocket_client_.stop();
}

void DanmakuViddupServiceClientContext::DisConnectWebSocket() {
    LOG(INFO) << "[ws] DisConnectWebSocket";
    try_heart_times_ = 0;
    recive_heart_ = false;
    is_CheckHeartbeat_ = false;
    is_Heartbeat_ = false;
    is_reconnect_ = true;

    CloseConnections();
}

void DanmakuViddupServiceClientContext::ReConnectWebSocket() {
    LOG(INFO) << "[ws] ReConnectWebSocket";
    try_heart_times_ = 0;
    recive_heart_ = false;
    is_CheckHeartbeat_ = false;
    is_Heartbeat_ = false;
    
    ConnectWebSocket();
}

void DanmakuViddupServiceClientContext::SendAuthBuffer(client* c, websocketpp::connection_hdl hdl) {
    LOG(INFO) << "[ws] SendAuthBuffer called with hdl: " << hdl.lock().get();
    bilibili::broadcast::v1::AuthReq auth;
    auth.set_last_msg_id(last_message_id_);
    auth.set_guid(base::Int64ToString(uid_));
    auth.set_conn_id(base::GenerateGUID());
    bilibili::broadcast::v1::BroadcastFrame broadcast_frame;
    broadcast_frame.set_target_path("/bilibili.broadcast.v1.Broadcast/Auth");
    broadcast_frame.mutable_body()->PackFrom(auth);
    std::string frame_data = broadcast_frame.SerializeAsString();
    c->send(hdl, frame_data, websocketpp::frame::opcode::binary);
}

void DanmakuViddupServiceClientContext::SendEnterRoomBuffer(client* c, websocketpp::connection_hdl hdl) {
    LOG(INFO) << "[ws] SendEnterRoomBuffer called with hdl: " << hdl.lock().get();
    bilibili::broadcast::v1::BroadcastFrame broadcast_frame;
    bilibili::broadcast::v1::RoomReq room_request;

    string room_id = "bstar://live/";
    room_id.append(base::Int64ToString(room_id_));
    room_request.set_id(room_id);
    //room_request.set_id(base::Int64ToString(room_id_));
    bilibili::broadcast::v1::RoomJoinEvent room_join;
    room_request.mutable_join()->CopyFrom(room_join);
    broadcast_frame.set_target_path("/bilibili.broadcast.v1.BroadcastRoom/Enter");
    broadcast_frame.mutable_body()->PackFrom(room_request);
    std::string frame_data = broadcast_frame.SerializeAsString();
    c->send(hdl, frame_data, websocketpp::frame::opcode::binary);
}

void DanmakuViddupServiceClientContext::SendHeartBeat(websocketpp::connection_hdl hdl) {
    LOG(INFO) << "[ws] SendHeartBeat called with hdl: " << hdl.lock().get();
    bilibili::broadcast::v1::BroadcastFrame broadcast_frame;
    broadcast_frame.set_target_path("/bilibili.broadcast.v1.Broadcast/Heartbeat");
    bilibili::broadcast::v1::HeartbeatReq heart_req;
    broadcast_frame.mutable_body()->PackFrom(heart_req);
    std::string frame_data = broadcast_frame.SerializeAsString();
    if (con_status_ == WebSocketConnectStatus::Open) {
        websocket_client_.send(hdl, frame_data, websocketpp::frame::opcode::binary);
        try_heart_times_++;
    }
}

context_ptr DanmakuViddupServiceClientContext::OnTlsInit(client* c, websocketpp::connection_hdl hdl) {
    LOG(INFO) << "[ws] OnTlsInit called with hdl: " << hdl.lock().get();
    con_status_ = WebSocketConnectStatus::TlsInit;

    // establishes a SSL connection
    context_ptr ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
            boost::asio::ssl::context::no_sslv2 |
            boost::asio::ssl::context::no_sslv3 |
            boost::asio::ssl::context::single_dh_use);
    } catch (std::exception& e) {
        LOG(WARNING) << "[ws] error in context pointer: " << e.what();
    }

    return ctx;
}

// This message handler will be invoked once for each incoming message. It
// prints the message and then sends a copy of the message back to the server.
void DanmakuViddupServiceClientContext::OnMessage(client* c, websocketpp::connection_hdl hdl, message_ptr msg) {
#ifdef _DEBUG
    char* print_payload = FormatMessagePayload(msg);
    LOG(INFO) << "[ws] OnMessage called with hdl: " << hdl.lock().get() << 
        " and message: " << print_payload;
    delete[] print_payload;
    print_payload = nullptr;
#else
    LOG(INFO) << "[ws] OnMessage called with hdl: " << hdl.lock().get() <<
        " and message: " << msg->get_payload();
#endif

    bilibili::broadcast::v1::BroadcastFrame broadcast_frame;
    bool res = broadcast_frame.ParseFromArray(msg->get_payload().c_str(), msg->get_payload().size());
    if (res) {
        ProcessReceviceMsg(broadcast_frame);
        if (broadcast_frame.mutable_options())
            last_message_id_ = broadcast_frame.mutable_options()->message_id();
    }
}

void DanmakuViddupServiceClientContext::OnOpen(client* c, websocketpp::connection_hdl hdl) {
    LOG(INFO) << "[ws] OnOpen called with hdl: " << hdl.lock().get();
    con_status_ = WebSocketConnectStatus::Open;
    is_Heartbeat_ = true;

    SendAuthBuffer(c, hdl);
    SendEnterRoomBuffer(c, hdl);
}

void DanmakuViddupServiceClientContext::OnClose(client* c, websocketpp::connection_hdl hdl) {
    LOG(INFO) << "[ws] OnClose called with hdl: " << hdl.lock().get();
    con_status_ = WebSocketConnectStatus::Closed;
    is_Heartbeat_ = false;

    if (!isStopping) {
        // 关闭时，先检测下是否是重新连接，是的话说明未检测到心跳包，关闭当前连接然后重新连接
        if (is_reconnect_) {
            LOG(INFO) << "[ws] OnClose close reason: reconnect";
            is_reconnect_ = false;
            ConnectWebSocket();
        } else {
            client::connection_ptr con = c->get_con_from_hdl(hdl);
            LOG(INFO) << "[ws] OnClose close code: " << con->get_remote_close_code() << " ("
                << websocketpp::close::status::get_string(con->get_remote_close_code())
                << "), close reason: " << con->get_remote_close_reason();

            Sleep(1000);
            ReConnectWebSocket();
        }
    }
}

void DanmakuViddupServiceClientContext::OnFail(client* c, websocketpp::connection_hdl hdl) {
    LOG(INFO) << "[ws] OnFail called with hdl: " << hdl.lock().get();
    con_status_ = WebSocketConnectStatus::Failed;
    is_Heartbeat_ = false;

    if (!isStopping) {
        Sleep(1000);
        ReConnectWebSocket();
    }
}

void DanmakuViddupServiceClientContext::OnInterrupt(client* c, websocketpp::connection_hdl hdl) {
    LOG(INFO) << "[ws] OnInterrupt called with hdl: " << hdl.lock().get();
    con_status_ = WebSocketConnectStatus::Interrupt;
    is_Heartbeat_ = false;

    if (!isStopping) {
        Sleep(1000);
        ReConnectWebSocket();
    }
}

void DanmakuViddupServiceClientContext::OnHeartBeat(websocketpp::connection_hdl hdl) {
    LOG(INFO) << "[ws] OnHeartBeat called with hdl: " << hdl.lock().get();
    recive_heart_ = false;
    SendHeartBeat(hdl);
    Sleep(500);

    // 发送完立刻去验证是否收到心跳
    is_CheckHeartbeat_ = true;
    OnCheckHeartBeat(hdl);
}

void DanmakuViddupServiceClientContext::OnCheckHeartBeat(websocketpp::connection_hdl hdl) {
    LOG(INFO) << "[ws] OnCheckHeartBeat called with hdl: " << hdl.lock().get();
    if (recive_heart_) {
        is_CheckHeartbeat_ = false;
        try_heart_times_ = 0;
    } else {
        if (try_heart_times_ >= kTryHeartTimesMax) {
            DisConnectWebSocket();
        } else {
            SendHeartBeat(hdl);
            try_heart_times_++;
        }
    }
}

void DanmakuViddupServiceClientContext::ProcessReceviceMsg(const bilibili::broadcast::v1::BroadcastFrame& broadcast_frame) {
    LOG(INFO) << "[ws] ProcessReceviceMsg";
    if (broadcast_frame.body().Is<bilibili::broadcast::v1::RoomResp>()) {
        LOG(INFO) << "[ws] ProcessReceviceMsg RoomResp";
        bilibili::broadcast::v1::RoomResp  room_response;
        if (broadcast_frame.body().UnpackTo(&room_response)) {
            if (room_response.msg().body().Is<bilibili::broadcast::message::intl::DmEventReply>()) {
                bilibili::broadcast::message::intl::DmEventReply dm_reply;
                if (room_response.msg().body().UnpackTo(&dm_reply))
                    ProcessDMReply(dm_reply);
            }
        }
    } else if (broadcast_frame.body().Is<bilibili::broadcast::v1::HeartbeatResp>()) {
        LOG(INFO) << "[ws] ProcessReceviceMsg HeartbeatResp";
        recive_heart_ = true;
        try_heart_times_ = 0;
    }
}

void DanmakuViddupServiceClientContext::ProcessDMReply(const bilibili::broadcast::message::intl::DmEventReply& reply) {
    LOG(INFO) << "[ws] ProcessDMReply";
    base::DictionaryValue* dict = new base::DictionaryValue();
    base::ListValue* list = new base::ListValue();
    std::vector<ChatData> chat_data_list;
    auto dm_items = reply.items();
    for (auto item : dm_items) {
        //if (item.has_author()) {
            ChatData data = TransferProtoData(item);
            if (item.cmd() != bilibili::broadcast::message::intl::DmType::PopupMsg)
            {
                if (data.userName.empty() && data.content.empty())
                {
                    continue;
                }
            }

            base::DictionaryValue* value = new base::DictionaryValue();
            value->SetInteger("type", data.type);
            value->SetInteger("id", data.id);
            value->SetString("content", data.content);
            value->SetString("user_name", data.userName);
            value->SetBoolean("is_admin", data.isAdmin);
            value->SetString("gif_file_url", data.gifFileUrl);
            value->SetString("gift_name", data.giftName);
            value->SetString("combo_string", data.comboString);
            value->SetInteger("treasure_iD", data.treasureID);
            value->SetString("notice", data.notice);

            value->SetString("popup_title", data.popupTitle);
            value->SetString("popup_content", data.popupContent);
            value->SetInteger("duration", data.duration);

            list->Append(value);
        //}
    }

    if (list->empty())
    {
        return;
    }
    

    dict->SetString("cmd", "DANMU_MSG");
    dict->Set("info", list);
    std::string json_content;
    dict->GetAsString(&json_content);
    RefDictionary ref_dict = RefCountedDictionary::TakeDictionary(
        reinterpret_cast<base::DictionaryValue*>(dict));

    // 将chat_data_list转成json传给上层，减少业务依赖
    if (!msg_notify_.is_null()) {
        msg_notify_.Run(ref_dict, json_content);
    }
}

ChatData DanmakuViddupServiceClientContext::TransferProtoData(const bilibili::broadcast::message::intl::DmItem& item)
{
    ChatData res;
    res.type = ChatData::System;
    res.userName = item.author().name();

    int id = static_cast<ChatData::UserIdentity>(item.author().rank());
    if (id >= 1 && id <= 3)
        res.id = static_cast<ChatData::UserIdentity>(id);

    res.isAdmin = false;

    switch (item.cmd())
    {
    case bilibili::broadcast::message::intl::DmType::DefaultType:
        res.type = ChatData::User;
        break;
    case bilibili::broadcast::message::intl::DmType::TreasureSticker:
        res.type = ChatData::Gift;
        break;
    case bilibili::broadcast::message::intl::DmType::MuteMsg:
        res.type = ChatData::MuteMsg;
        break;
    case bilibili::broadcast::message::intl::DmType::WarnMsg:
        res.type = ChatData::WarnMs;
        break;
    case bilibili::broadcast::message::intl::DmType::PopupMsg:
        res.type = ChatData::PopupMsg;
        break;
    case bilibili::broadcast::message::intl::DmType::BlockMsg:
        res.type = ChatData::BlockMsg;
        break;
    default:
        break;
    }

    //断播后显示
    res.notice = item.block_info().notice();
    res.end_time = item.block_info().end_time();

    //弹窗信息
    res.duration = item.popup_info().duration();
    auto popup_info_list = item.popup_info().popup_info_list();
    for (auto popupInfo : popup_info_list)
    {
        if (popupInfo.positon() == bilibili::broadcast::message::intl::PopupItemPosition::PopupItemPositionTitle)
        {
            res.popupTitle = popupInfo.content();
        }
        else if (popupInfo.positon() == bilibili::broadcast::message::intl::PopupItemPosition::PopupItemPositionContent)
        {
            res.popupContent = popupInfo.content();
        }
    }


    //if (item.cmd() != bilibili::broadcast::message::intl::DmType::DefaultType)
    //    res.type = ChatData::System;

    //check adminid
    auto message_list = item.messagelist();

    for (auto message : message_list) {
        if (message.type() == bilibili::broadcast::message::intl::MessageItemType::Icon) {
            if (message.subtype() == bilibili::broadcast::message::intl::MessageItemSubType::DefaultSubType) {
                if (message.iconname() == "bstar_live_admin_1")
                    res.isAdmin = true;
            }
        }

        res.content = message.content();
    }

    if (item.cmd() == bilibili::broadcast::message::intl::DmType::TreasureSticker)
    {
        //res.type = ChatData::Gift;
        for (auto message : message_list)
        {
            if (message.subtype() == bilibili::broadcast::message::intl::MessageItemSubType::GiftNum) {
                res.comboString = message.content();
            } else if (message.subtype() == bilibili::broadcast::message::intl::MessageItemSubType::Gift) {
                res.giftName = message.content();
            } else if (message.subtype() == bilibili::broadcast::message::intl::MessageItemSubType::GiftIcon) {
                res.gifFileUrl = message.iconurl();
                res.treasureID = message.treasure_id();
            }

            LOG(INFO) << "item type:" << message.type() 
                << "item subtype: " << message.subtype()
                << " icon name: " << message.iconname()
                << "icon url:" << message.iconurl()
                << " git content: " << message.content()
                << " treasure id: " << message.treasure_id();
        }
    }

    return res;
}

// 海外礼物弹幕广播msg中存在\n、\0等字符导致无法打印日志，先采用下面过滤格式化方法
// 注信息不全仅供参考，仅在debug模式下使用
char* DanmakuViddupServiceClientContext::FormatMessagePayload(message_ptr msg) {
    int size = msg->get_payload().size();
    int index = 0;
    char* data = new char[size];
    for (int i = 0; i < size; i++) {
        BYTE ch = msg->get_payload()[i];
        if (ch != '\n' && ch != '\0') {
            data[index++] = ch;
        }
    }

    data[index] = '\0';
    return data;
}

void DanmakuViddupServiceClientContext::CloseConnections() {
    LOG(INFO) << "[ws] CloseConnections status:: " << GetConnectStatus(con_status_);
    if (con_status_ == WebSocketConnectStatus::Open) {  // Only close open connections
        LOG(INFO) << "[ws] CloseConnections closing with hdl: " << con_current_;
        websocketpp::lib::error_code ec;
        websocket_client_.close(con_current_, websocketpp::close::status::going_away, "", ec);
        if (ec) {
            LOG(WARNING) << "[ws] CloseConnections error closing with hdl: " << con_current_
                << ", message: " << ec.message();
        }
    }
}

std::string DanmakuViddupServiceClientContext::GetConnectStatus(WebSocketConnectStatus status) {
    std::string str_status;
    switch (status) {
    case WebSocketConnectStatus::Empty:
        str_status = "Empty";
        break;
    case WebSocketConnectStatus::Connecting:
        str_status = "Connecting";
        break;
    case WebSocketConnectStatus::TlsInit:
        str_status = "TlsInit";
        break;
    case WebSocketConnectStatus::Open:
        str_status = "Open";
        break;
    case WebSocketConnectStatus::Closed:
        str_status = "Closed";
        break;
    case WebSocketConnectStatus::Failed:
        str_status = "Failed";
        break;
    case WebSocketConnectStatus::Interrupt:
        str_status = "Interrupt";
        break;
    default:
        break;
    }

    return str_status;
}

}//namespace internal