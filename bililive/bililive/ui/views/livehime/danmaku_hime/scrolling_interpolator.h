#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_SCROLLING_INTERPOLATION_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_SCROLLING_INTERPOLATION_H_


class ScrollingInterpolator {
public:
    enum InterpolatorType {
        SIMPLE,
        PARABOLIC,
        CUBIC_BEZIER
    };

    virtual ~ScrollingInterpolator() = default;

    virtual InterpolatorType GetType() = 0;
    virtual int GetScrollingSpan(int time_interval, int total_distance, float scale) = 0;
};


// ʹ���˶�ѧ��ʽ 2ah=v^2-v0^2 ���㵱ǰ�ٶȣ��ٳ�֡�������ÿ֡�������ķ�����
// Ч���Ϻã���Ҫ����һ��������
class ParabolicInterpolator : public ScrollingInterpolator {
public:
    InterpolatorType GetType() override { return PARABOLIC; }
    int GetScrollingSpan(int time_interval, int total_distance, float scale) override;
};

// ʹ�����α��������ߵļ��㷨��
// ֱ��ʹ�� Android ϵͳ�� Scroller ����и�������֮��ʽ��
// ���α��������ߵĲ���Ϊ��
// P0=(0, 0) P1=(0.175, 0.5) P2=(0.35, 1) P3=(1, 1)
// �����ø�URL��http://cubic-bezier.com/#.175,.5,.35,1 Ԥ�������ߡ�
// Ч���ܺã����������ϴ�
class CubicBezierInterpolator : public ScrollingInterpolator {
public:
    CubicBezierInterpolator();

    InterpolatorType GetType() override { return CUBIC_BEZIER; }
    int GetScrollingSpan(int time_interval, int total_distance, float scale) override;

private:
    float inflexion_;
    float friction_factor_;
    float deceleration_factor_;
    float gravity_acceleration_;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_SCROLLING_INTERPOLATION_H_