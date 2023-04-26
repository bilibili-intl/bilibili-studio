#include "bililive/bililive/livehime/server_broadcast/danmaku_statistics.h"

#include <chrono>

#include "base/logging.h"

#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"


namespace {

    const int kLogIntervalInSeconds = 60 * 30;

}

namespace livehime {

    void DanmakuStatistics::BeginScope() {
        if (time_ == 0) {
            time_ = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        }
    }

    void DanmakuStatistics::EndScope() {
        Log();
    }

    void DanmakuStatistics::Flush() {
        std::ostringstream ss;
        ToString(ss);
        LOG(INFO) << ss.str();
        Clear();
    }

    void DanmakuStatistics::OnDanmaku(int type, Code code) {
        ++danmaku_count_;
        if (code != Code::NONE) {
            ++danmaku_error_count_;
        }

        switch (code) {
        case Code::NONE:
        {
            break;
        }
        case Code::PARSING_JSON_ERROR:
        {
            auto& info = map_[type];
            ++info.parsing_error_count;
            break;
        }
        case Code::REGEX_MATCHING_ERROR:
        {
            auto& info = map_[type];
            ++info.regex_matching_error;
            break;
        }
        default:
            NOTREACHED();
            break;
        }
    }

    void DanmakuStatistics::Log() {
        auto cur_time = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        bool need_log = (cur_time - time_ >= kLogIntervalInSeconds);

        if (need_log) {
            Flush();
            time_ = cur_time;
        }
    }

    void DanmakuStatistics::Clear() {
        danmaku_count_ = 0;
        danmaku_error_count_ = 0;
        map_.clear();
    }

    void DanmakuStatistics::ToString(std::ostringstream& ss) const {
        ss << "[D_STAT] Dc: " << danmaku_count_
            << ", DEc: " << danmaku_error_count_;

        if (!map_.empty()) {
            ss << " ";
        }

        bool elide = false;
        size_t count = 0;
        for (const auto& p : map_) {
            ++count;
            // 最多打印8个记录
            if (count > 8 /*&&
                p.first != DT_SUPERCHAT &&
                p.first != DT_COMBO_END*/)
            {
                elide = true;
                continue;
            }

            ss << "{<" << p.first << ">,"
                << p.second.normal_count << ","
                << p.second.parsing_error_count << ","
                << p.second.regex_matching_error << "}";
        }

        if (elide) {
            ss << "...";
        }
    }

}
