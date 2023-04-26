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
         * ���ø÷���������һ��ͳ����Ϣ��
         * @param type ��Ļ����
         * @param code ��������
         */
        void OnDanmaku(int type, Code code = Code::PARSING_JSON_ERROR);

    private:
        void Log();
        void Clear();
        void ToString(std::ostringstream& ss) const;

        // ĳһ���͵�Ļ��ͳ����Ϣ
        struct Info {
            // �޴���Ļ��
            int64_t normal_count = 0;
            // ��������Ļ��
            int64_t parsing_error_count = 0;
            // ����ƥ�����Ļ��
            int64_t regex_matching_error = 0;
        };

        int64_t time_ = 0;
        // ��Ļ����
        int64_t danmaku_count_ = 0;
        // ��Ļ��������
        int64_t danmaku_error_count_ = 0;
        // �Ե�Ļ������Ϊ��������ϸ��Ϣ
        std::map<int, Info> map_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SERVER_BROADCAST_DANMAKU_STATISTICS_H_