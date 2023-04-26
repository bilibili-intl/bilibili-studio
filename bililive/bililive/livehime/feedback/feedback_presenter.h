#ifndef BILILIVE_BILILIVE_LIVEHIME_FEEDBACK_FEEDBACK_PRESENTER_H_
#define BILILIVE_BILILIVE_LIVEHIME_FEEDBACK_FEEDBACK_PRESENTER_H_

namespace bililive
{
    using FeedbackCallback = base::Closure;

    struct FeedbackPostInfo
    {
        int p_id = 0; //批次id
        int live_duration = 0; //开播时长
        int popularity_count = 0; //最高人气
        int fans = 0; //粉丝数
        std::string uname;//用户名
        std::string answer;//用户提交的json
    };
}// namespace bililive

#endif  // BILILIVE_BILILIVE_LIVEHIME_FEEDBACK_FEEDBACK_PRESENTER_H_