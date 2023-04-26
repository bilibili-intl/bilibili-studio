#ifndef BILILIVE_BILILIVE_LIVEHIME_SERVER_BROADCAST_DANMAKU_STATISTICS_H_
#define BILILIVE_BILILIVE_LIVEHIME_SERVER_BROADCAST_DANMAKU_STATISTICS_H_

#include <map>
#include <sstream>


namespace livehime {

    class DanmakuStatistics {
    public:
        enum class Code {
            NONE = 0,
            PARSING_JSON_ERROR,
            REGEX_MATCHING_ERROR,
        };

        DanmakuStatistics() = default;

        void BeginScope();
        void EndScope();
        void Flush();

        /**
         * 调用该方法来生成一条统计信息。
         * @param type 弹幕类型
         * @param code 错误类型
         */
        void OnDanmaku(int type, Code code = Code::PARSING_JSON_ERROR);

    private:
        void Log();
        void Clear();
        void ToString(std::ostringstream& ss) const;

        // 某一类型弹幕的统计信息
        struct Info {
            // 无错误弹幕数
            int64_t normal_count = 0;
            // 解析错误弹幕数
            int64_t parsing_error_count = 0;
            // 正则匹配错误弹幕数
            int64_t regex_matching_error = 0;
        };

        int64_t time_ = 0;
        // 弹幕总数
        int64_t danmaku_count_ = 0;
        // 弹幕错误总数
        int64_t danmaku_error_count_ = 0;
        // 以弹幕类型作为索引的详细信息
        std::map<int, Info> map_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SERVER_BROADCAST_DANMAKU_STATISTICS_H_