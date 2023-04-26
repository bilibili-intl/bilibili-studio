#include "danmaku_packet_statistics.h"

#include <chrono>

#include "base/logging.h"


namespace {

    const int kLogIntervalInSeconds = 60 * 30;

}

namespace livehime {

    DanmakuPacketStatistics::DanmakuPacketStatistics() {}

    void DanmakuPacketStatistics::OnSendHeartBeat(bool succeeded) {
        ++send_hb_count_;
        if (!succeeded) {
            ++send_hb_failed_count_;
        }
    }

    void DanmakuPacketStatistics::OnException() {
        ++exception_count_;
    }

    void DanmakuPacketStatistics::OnUnknown(int type) {
        ++unknown_count_;
    }

    void DanmakuPacketStatistics::BeginScope() {
        scope_.ppe_count = 0;
        scope_.pre_count = 0;

        if (time_ == 0) {
            time_ = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        }
    }

    void DanmakuPacketStatistics::EndScope() {
        if (scope_.ppe_count != 0 || scope_.pre_count != 0) {
            auto index = scope_error_count_ % scope_error_array_.size();
            scope_error_array_[index] = std::move(scope_);
            ++scope_error_count_;
        }

        Log();
    }

    void DanmakuPacketStatistics::Flush() {
        std::ostringstream ss;
        ToString(ss);
        LOG(INFO) << ss.str();
        Clear();
    }

    void DanmakuPacketStatistics::OnLopital(int count, int duplicated) {
        lopital_count_ += count;
        lopital_dup_count_ += duplicated;
    }

    void DanmakuPacketStatistics::BeginRecvPacket() {
        scope_.pre_count = 0;
    }

    void DanmakuPacketStatistics::EndRecvPacket(int64_t count) {
        packet_count_ += count;
        packet_recv_error_count_ += scope_.pre_count;
    }

    void DanmakuPacketStatistics::PushRecvPacketError(
        int level, RecvCode code,
        int64_t param1, int64_t param2, int64_t param3)
    {
        if (scope_.pre_count >= scope_.pre_array.size()) {
            ++scope_.pre_count;
            return;
        }

        RecvErrorInfo info;
        info.level = level;
        info.code = code;
        info.param1 = param1;
        info.param2 = param2;
        info.param3 = param3;
        scope_.pre_array[scope_.pre_count] = std::move(info);
        ++scope_.pre_count;
    }

    void DanmakuPacketStatistics::BeginProcPacket() {
        scope_.ppe_count = 0;
    }

    void DanmakuPacketStatistics::EndProcPacket() {
        packet_proc_error_count_ += scope_.ppe_count;
    }

    void DanmakuPacketStatistics::PushProcPacketError(
        ProcCode code,
        int64_t param1, int64_t param2, int64_t param3)
    {
        if (scope_.ppe_count >= scope_.ppe_array.size()) {
            ++scope_.ppe_count;
            return;
        }

        ProcErrorInfo info;
        info.code = code;
        info.param1 = param1;
        info.param2 = param2;
        info.param3 = param3;
        scope_.ppe_array[scope_.ppe_count] = std::move(info);
        ++scope_.ppe_count;
    }

    void DanmakuPacketStatistics::OnPacketAuthAdded() {
        ++packet_auth_count;
    }

    void DanmakuPacketStatistics::OnPacketHBAdded() {
        ++packet_hb_count_;
    }

    void DanmakuPacketStatistics::OnPacketDanmakuAdded() {
        ++packet_dmk_count_;
    }

    void DanmakuPacketStatistics::Log() {
        auto cur_time = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        bool need_log = (cur_time - time_ >= kLogIntervalInSeconds);

        if (need_log) {
            Flush();
            time_ = cur_time;
        }
    }

    void DanmakuPacketStatistics::Clear() {
        send_hb_count_ = 0;
        send_hb_failed_count_ = 0;
        exception_count_ = 0;
        unknown_count_ = 0;
        packet_count_ = 0;
        packet_auth_count = 0;
        packet_hb_count_ = 0;
        packet_dmk_count_ = 0;
        packet_recv_error_count_ = 0;
        packet_proc_error_count_ = 0;
        scope_error_count_ = 0;

        lopital_count_ = 0;
        lopital_dup_count_ = 0;
    }

    void DanmakuPacketStatistics::ToString(std::ostringstream& ss) const {
        ss << "[P_STAT] HBc: " << send_hb_count_
            << ", HBFc: " << send_hb_failed_count_
            << ", Ec: " << exception_count_
            << ", Uc: " << unknown_count_
            << ", PKGc: " << packet_count_
            << "(A" << packet_auth_count << "-H" << packet_hb_count_ << "-D" << packet_dmk_count_ << ")"
            << "LOPc: " << lopital_count_ << "-" << lopital_dup_count_
            << ", PREc: " << packet_recv_error_count_
            << ", PPEc: " << packet_proc_error_count_;

        if (scope_error_count_ > 0) {
            ss << " ";
            auto real_count = ((scope_error_count_ - 1) % scope_error_array_.size()) + 1;
            for (decltype(real_count) i = 0; i < real_count; ++i) {
                auto& scope = scope_error_array_[i];
                scope.ToString(ss);
                ss << " ";
            }
        }
    }

    void DanmakuPacketStatistics::PacketScope::ToString(std::ostringstream& ss) const {
        auto count = std::min<int64_t>(pre_count, pre_array.size());
        for (decltype(count) i = 0; i < count; ++i) {
            auto& item = pre_array[i];
            ss << "{REI:<" << item.level << ">," << int(item.code) << ",("
                << item.param1 << "," << item.param2 << "," << item.param3 << ")}";
        }

        count = std::min<int64_t>(ppe_count, ppe_array.size());
        for (decltype(count) i = 0; i < count; ++i) {
            auto& item = ppe_array[i];
            ss << "{PEI:" << int(item.code) << ",("
                << item.param1 << "," << item.param2 << "," << item.param3 << ")}";
        }
    }

}
