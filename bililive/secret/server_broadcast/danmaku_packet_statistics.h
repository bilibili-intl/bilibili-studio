#ifndef BILILIVE_SECRET_SERVER_BROADCAST_DANMAKU_PACKET_STATISTICS_H_
#define BILILIVE_SECRET_SERVER_BROADCAST_DANMAKU_PACKET_STATISTICS_H_

#include <array>
#include <cstdint>
#include <sstream>


namespace livehime {

    class DanmakuPacketStatistics {
    public:
        // ���հ�ʱ���ܳ��ֵĴ���
        enum class RecvCode {
            NONE = 0,
            READING_PACK_HEADER_FAILED,
            ILLEGAL_PACK_HEADER_SIZE,
            ILLEGAL_COMMAND_TYPE,
            TOO_LARGE_PACK,
            READING_PAYLOAD_FAILED,
            DECOMPRESSION_FAILED,
        };

        // �����ʱ���ܳ��ֵĴ���
        enum class ProcCode {
            NONE = 0,
            AUTH_DATA_ERROR,
            HB_DATA_ERROR,
            MESSAGE_DATA_ERROR,
            ILLEGAL_COMMAND_TYPE
        };

        // �������صĴ�����Ϣ
        struct RecvErrorInfo {
            int level = 0;
            RecvCode code = RecvCode::NONE;
            int64_t param1 = 0;
            int64_t param2 = 0;
            int64_t param3 = 0;
        };

        // �봦����صĴ�����Ϣ
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
         * Scope �ǽ��ա�����һ�����Ĺ��̡�
         * �����¼���ǵ�ǰ���ڴ���� Scope ����Ϣ��
         */
        struct PacketScope {
            // ������ʧ�ܼ���������ǰ Scope����
            // ֻ��¼���µ���� 3 �� Scope ����
            int64_t pre_count = 0;
            std::array<RecvErrorInfo, 3> pre_array;
            // ������ʧ�ܼ���������ǰ Scope����
            // ֻ��¼���µ���� 3 �� Scope ����
            int64_t ppe_count = 0;
            std::array<ProcErrorInfo, 3> ppe_array;

            void ToString(std::ostringstream& ss) const;
        } scope_;

        int64_t time_ = 0;

        // ��������
        int64_t send_hb_count_ = 0;
        // ����ʧ�ܼ���
        int64_t send_hb_failed_count_ = 0;

        // �쳣����
        int64_t exception_count_ = 0;
        // δ֪�¼�����
        int64_t unknown_count_ = 0;

        // ������
        int64_t packet_count_ = 0;
        int64_t packet_auth_count = 0;
        int64_t packet_hb_count_ = 0;
        int64_t packet_dmk_count_ = 0;
        // �ش����
        int64_t lopital_count_ = 0;
        int64_t lopital_dup_count_ = 0;
        // ������ʧ�ܼ���
        int64_t packet_recv_error_count_ = 0;
        // ������ʧ�ܼ���
        int64_t packet_proc_error_count_ = 0;

        // ֻ��¼���µ���� 3 �� Scope ����
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