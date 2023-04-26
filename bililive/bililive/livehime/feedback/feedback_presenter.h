#ifndef BILILIVE_BILILIVE_LIVEHIME_FEEDBACK_FEEDBACK_PRESENTER_H_
#define BILILIVE_BILILIVE_LIVEHIME_FEEDBACK_FEEDBACK_PRESENTER_H_

namespace bililive
{
    using FeedbackCallback = base::Closure;

    struct FeedbackPostInfo
    {
        int p_id = 0; //����id
        int live_duration = 0; //����ʱ��
        int popularity_count = 0; //�������
        int fans = 0; //��˿��
        std::string uname;//�û���
        std::string answer;//�û��ύ��json
    };
}// namespace bililive

#endif  // BILILIVE_BILILIVE_LIVEHIME_FEEDBACK_FEEDBACK_PRESENTER_H_