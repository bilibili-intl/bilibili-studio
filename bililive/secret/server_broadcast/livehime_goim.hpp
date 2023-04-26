#ifndef BILILIVE_SECRET_SERVER_BROADCAST_LIVEHIME_GOIM_PACKET_H_
#define BILILIVE_SECRET_SERVER_BROADCAST_LIVEHIME_GOIM_PACKET_H_

#include <algorithm>
#include <cassert>
#include <chrono>
#include <unordered_map>
#include <vector>

#include <WinSock2.h>
#include <ws2tcpip.h>

#include "base/logging.h"
#include "base/strings/stringprintf.h"

#include "bililive/secret/server_broadcast/danmaku_packet_statistics.h"
#include "bililive/secret/utils/compressions.h"

#define BEFORE_RECONNECT_WAITTIME 1000
#define SOCKETTIMEOUT_THRESHOLD 10
#define MAX_ALLOWED_SERVER_LIST_SIZE 16

#pragma comment(lib, "ws2_32.lib")


namespace livehime {

#pragma region(方便使用的socket函数)
    struct EndPointV4V6
    {
        EndPointV4V6()
        {
            ZeroMemory(&v4_addr, sizeof(v4_addr));
            ZeroMemory(&v6_addr, sizeof(v6_addr));
        }

        int family = AF_UNSPEC;
        struct sockaddr_in v4_addr;
        struct sockaddr_in6 v6_addr;
        size_t ai_addrlen = 0;
        std::string friendly_address;

        static std::vector<EndPointV4V6> ResolveTCP(const char* host, short int port)
        {
            char szPort[16] = { 0 };
            sprintf_s(szPort, sizeof(szPort) / sizeof(*szPort), "%u", static_cast<unsigned int>(port));

            struct addrinfo* addrResult = nullptr;
            struct addrinfo hints = { 0 };
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            if (getaddrinfo(host, szPort, &hints, &addrResult) != 0)
                return{};
            if (addrResult == nullptr)
                return{};

            std::vector<EndPointV4V6> result;
            for (struct addrinfo* ai = addrResult; ai != nullptr; ai = ai->ai_next)
            {
                assert(ai->ai_protocol == IPPROTO_TCP);

                if ((ai->ai_family == AF_INET) || (ai->ai_family == AF_INET6))
                {
                    char ntop_ip[64] = { 0 };
                    char ntop_port[16] = { 0 };
                    if (0 == getnameinfo(ai->ai_addr, ai->ai_addrlen,
                        ntop_ip, sizeof(ntop_ip), ntop_port, sizeof(ntop_port),
                        NI_NUMERICHOST | NI_NUMERICSERV))
                    {
                        EndPointV4V6 ep;
                        ep.family = ai->ai_family;
                        ep.ai_addrlen = ai->ai_addrlen;
                        switch (ai->ai_family)
                        {
                        case AF_INET:
                            memcpy(&ep.v4_addr, ai->ai_addr, ai->ai_addrlen);
                            ep.friendly_address = base::StringPrintf("%s:%s", ntop_ip, ntop_port);
                            break;
                        case AF_INET6:
                            memcpy(&ep.v6_addr, ai->ai_addr, ai->ai_addrlen);
                            ep.friendly_address = base::StringPrintf("[%s]:%s", ntop_ip, ntop_port);
                            break;
                        default:
                            break;
                        }
                        result.emplace_back(ep);
                    }
                } else
                {
                    NOTREACHED() << "unsupport family!";
                }
            }

            freeaddrinfo(addrResult);

            return result;
        }

        unsigned short getPort() const
        {
            DCHECK(family != AF_UNSPEC);

            return (family == AF_INET) ? ntohs(v4_addr.sin_port) : ntohs(v6_addr.sin6_port);
        }

        std::string toString() const
        {
            DCHECK(family != AF_UNSPEC);

            return friendly_address;
        }

        bool operator==(const EndPointV4V6& rhs) const
        {
            return (family == rhs.family) &&
                (v4_addr.sin_addr.s_addr == rhs.v4_addr.sin_addr.s_addr) &&
                (v4_addr.sin_port == rhs.v4_addr.sin_port) &&
                (0 == memcmp(&v6_addr.sin6_addr, &rhs.v6_addr.sin6_addr, sizeof(IN6_ADDR))) &&
                (v6_addr.sin6_port == rhs.v6_addr.sin6_port);
        }
    };

    class TCPClient
    {
    public:
        TCPClient(const TCPClient&) = delete;
        TCPClient(TCPClient&&) = delete;
        TCPClient& operator = (const TCPClient&) = delete;

        TCPClient();
        ~TCPClient();

        static const int WAIT_SEND = 1;
        static const int WAIT_RECV = 2;
        static const int WAIT_EXCEPTION = 3;

        enum {
            WAIT_RESULT_TIMEOUT = -1,
            WAIT_RESULT_EXCEPTION = -2
        };
        // >0: index
        static int WaitAny(TCPClient* tcpClients, size_t tcpClientsCount, int flag, double timeout);

        // >0: succeed. <0: error. =0: timeout
        int Wait(int flag, double timeout);

        //wait for send to check if connected
        bool Connect(const EndPointV4V6& ep);

        int RecvFixed(char* buf, int buflen, double timeout, int* error_code);
        int SendFixed(const char* buf, int buflen, double timeout);

        void Swap(TCPClient& rhs);
        void Close();

    private:
        SOCKET s_{ INVALID_SOCKET };
    };

    // >=0: index
    // -1: timeout
    // -2: error
    static int WaitAnySockets(SOCKET* sockets, size_t socketsCount, int flag, double timeout)
    {
        FD_SET fdset;
        FD_ZERO(&fdset);
        SOCKET maxfd = 0;
        for (size_t i = 0; i < socketsCount; ++i)
        {
            if (sockets[i] != SOCKET_ERROR)
            {
                FD_SET(sockets[i], &fdset);
                if (sockets[i] > maxfd)
                    maxfd = sockets[i];
            }
        }
        struct timeval timeoutval;
        timeoutval.tv_sec = static_cast<int>(timeout);
        timeoutval.tv_usec = static_cast<int>((timeout - timeoutval.tv_sec) * 1000000);

        FD_SET empty_rdset;
        FD_ZERO(&empty_rdset);
        FD_SET empty_wrset;
        FD_ZERO(&empty_wrset);
        FD_SET dummyset;
        FD_ZERO(&dummyset);

        FD_SET *rdset = &empty_rdset, *wrset = &empty_wrset, *exset = &empty_wrset, *chkset = &dummyset;

        if (flag == TCPClient::WAIT_SEND)
        {
            wrset = chkset = &fdset;
        } else if (flag == TCPClient::WAIT_RECV)
        {
            rdset = chkset = &fdset;
        } else if (flag == TCPClient::WAIT_EXCEPTION)
        {
            exset = chkset = &fdset;
        }

        int selectRet = select(maxfd + 1, rdset, wrset, exset, &timeoutval);

        if (selectRet > 0)
        {
            for (size_t i = 0; i < socketsCount; ++i)
            {
                if (FD_ISSET(sockets[i], chkset))
                {
                    return i;
                }
            }

            return TCPClient::WAIT_RESULT_EXCEPTION;
        } else if (selectRet == 0)
        {
            return TCPClient::WAIT_RESULT_TIMEOUT;
        } else
        {
            return TCPClient::WAIT_RESULT_EXCEPTION;
        }
    }

    inline TCPClient::TCPClient()
    {
    }

    inline TCPClient::~TCPClient()
    {
        Close();
    }

    inline void TCPClient::Swap(TCPClient& rhs)
    {
        std::swap(s_, rhs.s_);
    }

    inline int TCPClient::Wait(int flag, double timeout)
    {
        return WaitAny(this, 1, flag, timeout);
    }

    inline int TCPClient::WaitAny(TCPClient* tcpClients, size_t tcpClientsCount, int flag, double timeout)
    {
        if (tcpClientsCount == 0)
            return WAIT_RESULT_EXCEPTION;

        std::vector<SOCKET> tmp;
        tmp.reserve(tcpClientsCount);
        for (size_t i = 0; i < tcpClientsCount; ++i)
            tmp.emplace_back(tcpClients[i].s_);
        return WaitAnySockets(tmp.data(), tmp.size(), flag, timeout);
    }

    inline bool TCPClient::Connect(const EndPointV4V6& ep)
    {
        DCHECK(INVALID_SOCKET == s_);

        if ((s_ = socket(ep.family, SOCK_STREAM, IPPROTO_TCP)) == SOCKET_ERROR)
            return false;

        u_long ioctlParam = 1;
        int ret = ioctlsocket(s_, FIONBIO, &ioctlParam);
        assert(ret == 0);

        SOCKADDR* addr = (ep.family == AF_INET) ? (SOCKADDR*)&ep.v4_addr : (SOCKADDR*)&ep.v6_addr;
        if (connect(s_, addr, ep.ai_addrlen) == 0)
            return true;

        if (WSAGetLastError() == WSAEWOULDBLOCK)
            return true;

        return false;
    }

    inline int TCPClient::RecvFixed(
        char* buf, int buflen, double timeout, int* error_code)
    {
        using namespace std::chrono;

        if (s_ == SOCKET_ERROR)
            return -1;

        int pos = 0;
        while (pos < buflen)
        {
            if (Wait(WAIT_RECV, timeout) == 0)
            {
                int readbyte = recv(s_, &buf[pos], buflen - pos, 0);
                if (readbyte > 0)
                    pos += readbyte;
                else if (readbyte == 0) //服务器关闭了连接
                    break;
                else if (readbyte < 0)//坏了
                {
                    *error_code = WSAGetLastError();
                    return -1;
                }
            } else
            {
                return pos;
            }
        }
        return pos;
    }

    inline int TCPClient::SendFixed(const char* buf, int buflen, double timeout)
    {
        using namespace std::chrono;

        if (s_ == SOCKET_ERROR)
            return -1;

        int pos = 0;
        while (pos < buflen)
        {
            if (Wait(WAIT_SEND, timeout) == 0)
            {
                int r = send(s_, &buf[pos], buflen - pos, 0);
                if (r <= 0)
                    return -1; // error, return the bytes sent
                else
                    pos += r;
            } else
                return -1;
        }
        return pos;
    }

    inline void TCPClient::Close()
    {
        if (s_ != INVALID_SOCKET)
        {
            shutdown(s_, SD_BOTH);
            closesocket(s_);

            s_ = INVALID_SOCKET;
        }
    }

#pragma endregion

#pragma region(用vector模拟socket接收)
    class VectorStream
    {
        const std::vector<char>& buf_;
        int offset_;
    public:
        explicit VectorStream(const std::vector<char>& buf)
            : buf_(buf)
            , offset_(0)
        {
        }

        int RecvFixed(char* buf, int buflen, double timeout, int* error_code)
        {
            int leftSize = buf_.size() - offset_;
            if (leftSize >= buflen)
            {
                memcpy(buf, buf_.data() + offset_, buflen);
                offset_ += buflen;
                return buflen;
            }
            *error_code = leftSize;
            return -1;
        }
    };
#pragma endregion

#pragma region(解压缩过程)
    inline bool DecompressData(
        uint16_t protocol_version, const char* in_str, size_t in_len, std::vector<char>& out_data, int& error_code)
    {
        switch (protocol_version)
        {
        case 2:
        {
            error_code = secret::DecompressDataZlib(in_str, in_len, out_data);
            return error_code == Z_OK;
        }

        case 3:
        {
            error_code = secret::DecompressDataBrotli(in_str, in_len, out_data);
            return error_code == BROTLI_DECODER_RESULT_SUCCESS;
        }

        default:
            assert(0);
            return false;
        }
    }
#pragma endregion


    class GoimPacket
    {
    public:
        using RecvCode = DanmakuPacketStatistics::RecvCode;
        using RecvErrorInfo = DanmakuPacketStatistics::RecvErrorInfo;

        enum {
            REQ_HEARTBEAT = 2,
            RESP_HEARTBEAT = 3,
            RESP_MESSAGE = 5,
            REQ_AUTH = 7,
            RESP_AUTH = 8
        };

        class PacketHeader
        {
        private:
            std::uint32_t packet_length;
            std::uint16_t header_length;
            std::uint16_t protocol_version;
            std::uint32_t command;
            std::uint32_t seq;

        public:
            PacketHeader()
            {
                std::fill_n(reinterpret_cast<char*>(this), sizeof(*this), 0);
            }

            std::uint32_t get_packet_length() const { return ntohl(packet_length); }
            void set_packet_length(uint32_t val) { packet_length = htonl(val); }

            std::uint16_t get_header_length() const { return ntohs(header_length); }
            void set_header_length(std::uint16_t val) { header_length = htons(val); }

            std::uint16_t get_protocol_version() const { return ntohs(protocol_version); }
            void set_protocol_version(std::uint16_t val) { protocol_version = htons(val); }

            std::uint32_t get_command() const { return ntohl(command); }
            void set_command(std::uint32_t val) { command = htonl(val); }

            std::uint32_t get_seq() const { return ntohl(seq); }
            void set_seq(std::uint32_t val) { seq = htonl(val); }

            std::uint32_t get_payload_length() const
            {
                return get_packet_length() - get_header_length();
            }
        };

        struct NOT_FOR_SEND {};

        explicit GoimPacket(NOT_FOR_SEND)
        {
            InitHeader();
        }

        GoimPacket(int seq, int command)
        {
            InitHeader();

            header().set_seq(seq);
            header().set_command(command);
        }

        GoimPacket(const GoimPacket& rhs)
        {
            *this = rhs;
        }

        GoimPacket(GoimPacket&& rhs) noexcept {
            *this = std::move(rhs);
        }

        GoimPacket& operator=(const GoimPacket& rhs)
        {
            if (&rhs != this)
                m_data = rhs.m_data;
            return *this;
        }

        GoimPacket& operator=(GoimPacket&& rhs) noexcept {
            if (&rhs != this)
                m_data = std::move(rhs.m_data);
            return *this;
        }

        bool SetPayload(const char* jsonData)
        {
            if (!jsonData)
                return false;

            int dataLen = strlen(jsonData);
            return SetPayload(jsonData, dataLen);
        }

        bool SetPayload(const void* data, int dataLen)
        {
            if (data == nullptr && dataLen != 0)
                return false;
            char* dstData = GetDataPtrForWrite(dataLen);
            const char* srcData = static_cast<const char*>(data);
            std::copy(srcData, srcData + dataLen, dstData);
            return true;
        }

        const void* GetPayload(int* dataLen) const
        {
            *dataLen = header().get_payload_length();
            if (*dataLen == 0) {
                return nullptr;
            }
            return &m_data[header().get_header_length()];
        }

        const char* GetData(int* dataLen) const
        {
            *dataLen = m_data.size();
            return &m_data[0];
        }

        const PacketHeader& GetHeader() const
        {
            return header();
        }

        template<class ReadableStream>
        static std::vector<GoimPacket> ReceivePacket(ReadableStream& s, RecvContext* context);

    private:
        void InitHeader()
        {
            m_data.resize(sizeof(PacketHeader));
            header().set_packet_length(sizeof(PacketHeader));
            header().set_header_length(sizeof(PacketHeader));
            header().set_protocol_version(1);
        }

        char* GetDataPtrForWrite(uint32_t dataLen)
        {
            int totalLen = header().get_header_length() + dataLen;
            header().set_packet_length(totalLen);
            m_data.resize(totalLen);
            return &m_data[header().get_header_length()];
        }

    private:
        PacketHeader& header()
        {
            return *reinterpret_cast<PacketHeader*>(&m_data[0]);
        }

        const PacketHeader& header() const
        {
            return *reinterpret_cast<const PacketHeader*>(&m_data[0]);
        }

        std::vector<char> m_data;
    };

    //接收packet
    template<class ReadableStream>
    std::vector<GoimPacket> GoimPacket::ReceivePacket(ReadableStream& s, RecvContext* context)
    {
        GoimPacket packet(GoimPacket::NOT_FOR_SEND{});

        int error_code = 0;
        //接收头
        int readByte = s.RecvFixed(
            reinterpret_cast<char*>(&packet.header()), sizeof(PacketHeader),
            SOCKETTIMEOUT_THRESHOLD, &error_code);

        //检查数据是否正确
        if (sizeof(PacketHeader) != readByte) {
            context->PushRecvError(
                RecvCode::READING_PACK_HEADER_FAILED, readByte, error_code);
            return { };
        }

        if (packet.header().get_header_length() != 16) {
            context->PushRecvError(
                RecvCode::ILLEGAL_PACK_HEADER_SIZE, packet.header().get_header_length());
            return { };
        }

        switch (packet.header().get_command())
        {
        case GoimPacket::RESP_AUTH:
        case GoimPacket::RESP_HEARTBEAT:
        case GoimPacket::RESP_MESSAGE:
            break;
        default:
            context->PushRecvError(
                RecvCode::ILLEGAL_COMMAND_TYPE, packet.header().get_command());
            return { std::move(packet) };
        }

        //会有10M以上的包？出问题了吧
        if (packet.header().get_payload_length() > 10485760)
        {
            //当作是网络错误
            context->PushRecvError(
                RecvCode::TOO_LARGE_PACK, packet.header().get_payload_length());
            return { };
        }

        int payload_length = packet.header().get_payload_length();
        if (payload_length != 0)
        {
            error_code = 0;
            char* pdata = packet.GetDataPtrForWrite(payload_length);
            int recvLen = s.RecvFixed(pdata, payload_length, SOCKETTIMEOUT_THRESHOLD, &error_code);
            if (recvLen != payload_length) {//接收不完整的情况
                context->PushRecvError(
                    RecvCode::READING_PAYLOAD_FAILED, payload_length, recvLen, error_code);
                return { };
            }
        }

        //到这里都没抛异常那是没问题了

        //检查并处理压缩过的包
        uint16_t protocol_version = packet.header().get_protocol_version();
        if (protocol_version == 2 || protocol_version == 3)
        {
            int compressed_data_len;
            const void* compressed_data = packet.GetPayload(&compressed_data_len);
            std::vector<char> decompressed_data;
            int error_code(0);
            bool d_ret = DecompressData(
                protocol_version,
                static_cast<const char*>(compressed_data), compressed_data_len, decompressed_data, error_code);
            if (d_ret != true) {
                context->PushRecvError(
                    RecvCode::DECOMPRESSION_FAILED, error_code, protocol_version);
                return {};
            }

            RecvContext sub_context;
            sub_context.cur_level = context->cur_level + 1;
            sub_context.statistics = context->statistics;

            std::vector<GoimPacket> result;
            VectorStream vs(decompressed_data);
            for (;;) {
                std::vector<GoimPacket> pkts = ReceivePacket(vs, &sub_context);
                if (pkts.empty()) {
                    break;
                }

                for (auto& x : pkts) {
                    result.emplace_back(std::move(x));
                }
            }

            return result;
        }

        return { packet };
    }

    class GoimClient
    {
        int64_t lastHeartbeatTime_;
        int seq_;

    public:
        GoimClient()
            : lastHeartbeatTime_{ 0 }
            , seq_{ 1 }
        {
        }

        ~GoimClient()
        {
        }

        bool ForceClose()
        {
            c_.Close();
            return true;
        }

        bool Connect(
            const std::vector<std::pair<std::string, unsigned short>>& serverList,
            std::unordered_map<std::string, int>& connected_history)
        {
            //解析服务器域名
            std::vector<EndPointV4V6> goim_server_list;

            for (auto& server : serverList)
            {
                auto ep_list = EndPointV4V6::ResolveTCP(server.first.c_str(), server.second);
                for (auto& ep : ep_list)
                {
                    goim_server_list.push_back(ep);
                }
            }
            // 去重
            goim_server_list.erase(std::unique(goim_server_list.begin(), goim_server_list.end()), goim_server_list.end());

            //解析失败的情况
            if (goim_server_list.empty())
            {
                LOG(ERROR) << "[GoimClient]Connect: not resolved any host names.";
                return false;
            }

            LOG(INFO) << "[GoimClient]Connect: get " << goim_server_list.size() << " server addresses.";

            auto get_candidating_score = [&](const EndPointV4V6& endpoint) {
                auto x = connected_history[endpoint.toString()];
                return x + (endpoint.getPort() == 80 ? 1 : 0);
            };

            //找出使用次数最少的服务器是几次
            auto least_score_server = std::min_element(goim_server_list.begin(), goim_server_list.end(),
                [&](const EndPointV4V6& lhs, const EndPointV4V6& rhs)
            {
                return get_candidating_score(lhs) < get_candidating_score(rhs);
            }
            );
            int least_score = get_candidating_score(*least_score_server);

            //找出并列的
            std::vector<EndPointV4V6> to_connect_list;
            std::copy_if(goim_server_list.begin(), goim_server_list.end(), std::back_inserter(to_connect_list),
                [&](const EndPointV4V6& x) {
                return get_candidating_score(x) == least_score;
            });

            LOG(INFO) << "[GoimClient]Connect: will try " << to_connect_list.size() << " of them. (min tried: " << least_score << ")";

            //连接到最快的那个服务器
            int tryConnectCount = std::min<int>(to_connect_list.size(), MAX_ALLOWED_SERVER_LIST_SIZE);

            std::vector<TCPClient> clients(tryConnectCount);
            for (int i = 0; i < tryConnectCount; ++i)
            {
                if (!clients[i].Connect(to_connect_list[i]))
                {
                    LOG(INFO) << "[GoimClient]Connect: failed to connect to " << to_connect_list[i].toString() << ".";
                }
            }

            int ind = TCPClient::WaitAny(clients.data(), clients.size(), TCPClient::WAIT_SEND, SOCKETTIMEOUT_THRESHOLD);
            if (ind < 0)
            {
                //统计连接次数
                for (int i = 0; i < tryConnectCount; ++i)
                {
                    ++connected_history[to_connect_list[i].toString()];
                }
                LOG(ERROR) << "[GoimClient]Connect: failed to connect to any server.";
                return false;
            }

            LOG(INFO) << "[GoimClient]Connect: connected to " << to_connect_list[ind].toString() << ".";

            //统计连接次数
            ++connected_history[to_connect_list[ind].toString()];

            c_.Swap(clients[ind]);
            return true;
        }

        bool SendLogin(int64_t uid, int64_t roomid, const std::string& token)
        {
            GoimPacket loginPacket(seq_++, GoimPacket::REQ_AUTH);
            std::stringstream loginPayload;
            loginPayload
                << "{\"roomid\":" << roomid
                << ",\"uid\":" << uid
                << ",\"protover\":3"
                << ",\"platform\":\"pc_link\""
                << ",\"key\":" << "\"" << token << "\""
                << "}";

            loginPacket.SetPayload(loginPayload.str().c_str());

            int loginDataLen;
            const char* loginData = loginPacket.GetData(&loginDataLen);

            if (c_.SendFixed(loginData, loginDataLen, SOCKETTIMEOUT_THRESHOLD) != loginDataLen)
            {
                LOG(ERROR) << "[GoimClient]SendLogin: fail to send login request.";
                return false;
            }

            return true;
        }

        bool SendHeartBeat()
        {
            GoimPacket heartBeatPacket(seq_++, GoimPacket::REQ_HEARTBEAT);
            int heartBeatPacketLen;
            const char* heartBeatPacketData
                = heartBeatPacket.GetData(&heartBeatPacketLen);

            if (c_.SendFixed(heartBeatPacketData, heartBeatPacketLen, SOCKETTIMEOUT_THRESHOLD) != heartBeatPacketLen)
            {
                LOG(ERROR) << "[GoimClient]SendLogin: fail to send heartbeat.";
                return false;
            }

            return true;
        }

        std::vector<GoimPacket> RecvPacket(RecvContext* context)
        {
            return GoimPacket::ReceivePacket(c_, context);
        }

        enum class Event
        {
            EV_SHOULD_HEARTBEAT,
            EV_PACKET_INCOMING,
            EV_EXCEPTION
        };

        Event WaitEvent()
        {
            Event result;
            for (;;)
            {
                int64_t currentTime = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count();
                if (currentTime - lastHeartbeatTime_ > 20)
                {
                    lastHeartbeatTime_ = currentTime;
                    result = Event::EV_SHOULD_HEARTBEAT;
                    break;
                }

                int waitRecvResult = c_.Wait(TCPClient::WAIT_RECV, 5);
                if (waitRecvResult == 0)
                {
                    result = Event::EV_PACKET_INCOMING;
                    break;
                } else if (waitRecvResult == TCPClient::WAIT_RESULT_EXCEPTION)
                {
                    result = Event::EV_EXCEPTION;
                    break;
                }

                if (c_.Wait(TCPClient::WAIT_EXCEPTION, 0) != TCPClient::WAIT_RESULT_TIMEOUT)
                {
                    result = Event::EV_EXCEPTION;
                    break;
                }
            }

            if (result != Event::EV_EXCEPTION)
                if (c_.Wait(TCPClient::WAIT_EXCEPTION, 0) != TCPClient::WAIT_RESULT_TIMEOUT)
                    return Event::EV_EXCEPTION;

            return result;
        }

    private:
        TCPClient c_;
    };

}

#endif  // BILILIVE_SECRET_SERVER_BROADCAST_LIVEHIME_GOIM_PACKET_H_