#ifndef BILILIVE_SECRET_SERVER_BROADCAST_DANMAKU_PACKET_STATISTICS_H_
#define BILILIVE_SECRET_SERVER_BROADCAST_DANMAKU_PACKET_STATISTICS_H_

#include <array>
#include <cstdint>
#include <sstream>


namespace livehime {

    class DanmakuPacketStatistics {
    public:
        // 接收包时可能出现的错误
        enum class RecvCode {
            NONE = 0,
            READING_PACK_HEADER_FAILED,
            ILLEGAL_PACK_HEADER_SIZE,
            ILLEGAL_COMMAND_TYPE,
            TOO_LARGE_PACK,
            READING_PAYLOAD_FAILED,
            DECOMPRESSION_FAILED,
        };

        // 处理包时可能出现的错误
        enum class ProcCode {
            NONE = 0,
            AUTH_DATA_ERROR,
            HB_DATA_ERROR,
            MESSAGE_DATA_ERROR,
            ILLEGAL_COMMAND_TYPE
        };

        // 与接收相关的错误信息
        struct RecvErrorInfo {
            int level = 0;
            RecvCode code = RecvCode::NONE;
            int64_t param1 = 0;
            int64_t param2 = 0;
            int64_t param3 = 0;
        };

        // 与处理相关的错误信息
        struct ProcErrorInfo {
            ProcCode code = ProcCode::NONE;
            int64_t param1 = 0;
            int64_t param2 = 0;
            int64_t param3 = 0;
        };

        DanmakuPacketStatistics();

        void OnSendHeartBeat(bool succeeded);
        void OnException();
        void OnUnknown(int type);

        void BeginScope();
        void EndScope();
        void Flush();

        void OnLopital(int count, int duplicated);

        void BeginRecvPacket();
        void EndRecvPacket(int64_t count);
        void PushRecvPacketError(
            int level, RecvCode code,
            int64_t param1 = 0, int64_t param2 = 0, int64_t param3 = 0);

        void BeginProcPacket();
        void EndProcPacket();
        void PushProcPacketError(
            ProcCode code,
            int64_t param1 = 0, int64_t param2 = 0, int64_t param3 = 0);
        void OnPacketAuthAdded();
        void OnPacketHBAdded();
        void OnPacketDanmakuAdded();

    private:
        void Log();
        void Clear();
        void ToString(std::ostringstream& ss) const;

        /**
         * Scope 是接收、处理一个包的过程。
         * 这里记录的是当前正在处理的 Scope 的信息。
         */
        struct PacketScope {
            // 包接收失败计数（仅当前 Scope），
            // 只记录最新的最多 3 个 Scope 错误
            int64_t pre_count = 0;
            std::array<RecvErrorInfo, 3> pre_array;
            // 包处理失败计数（仅当前 Scope），
            // 只记录最新的最多 3 个 Scope 错误
            int64_t ppe_count = 0;
            std::array<ProcErrorInfo, 3> ppe_array;

            void ToString(std::ostringstream& ss) const;
        } scope_;

        int64_t time_ = 0;

        // 心跳计数
        int64_t send_hb_count_ = 0;
        // 心跳失败计数
        int64_t send_hb_failed_count_ = 0;

        // 异常计数
        int64_t exception_count_ = 0;
        // 未知事件计数
        int64_t unknown_count_ = 0;

        // 包计数
        int64_t packet_count_ = 0;
        int64_t packet_auth_count = 0;
        int64_t packet_hb_count_ = 0;
        int64_t packet_dmk_count_ = 0;
        // 必达计数
        int64_t lopital_count_ = 0;
        int64_t lopital_dup_count_ = 0;
        // 包接收失败计数
        int64_t packet_recv_error_count_ = 0;
        // 包处理失败计数
        int64_t packet_proc_error_count_ = 0;

        // 只记录最新的最多 3 个 Scope 错误
        int64_t scope_error_count_ = 0;
        std::array<PacketScope, 3> scope_error_array_;
    };


    struct RecvContext {
        int cur_level = 0;
        DanmakuPacketStatistics* statistics = nullptr;

        void PushRecvError(
            DanmakuPacketStatistics::RecvCode code,
            int64_t param1 = 0, int64_t param2 = 0, int64_t param3 = 0)
        {
            statistics->PushRecvPacketError(cur_level, code, param1, param2, param3);
        }
    };

}

#endif  // BILILIVE_SECRET_SERVER_BROADCAST_DANMAKU_PACKET_STATISTICS_H_