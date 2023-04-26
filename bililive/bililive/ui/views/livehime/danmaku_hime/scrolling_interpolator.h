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


// 使用运动学公式 2ah=v^2-v0^2 计算当前速度，再乘帧间隔计算每帧滚动量的方法。
// 效果较好，需要计算一个开方。
class ParabolicInterpolator : public ScrollingInterpolator {
public:
    InterpolatorType GetType() override { return PARABOLIC; }
    int GetScrollingSpan(int time_interval, int total_distance, float scale) override;
};

// 使用三次贝塞尔曲线的计算法。
// 直接使用 Android 系统的 Scroller 组件中给出的谜之公式。
// 三次贝塞尔曲线的参数为：
// P0=(0, 0) P1=(0.175, 0.5) P2=(0.35, 1) P3=(1, 1)
// 可以用该URL：http://cubic-bezier.com/#.175,.5,.35,1 预览该曲线。
// 效果很好，但计算量较大。
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