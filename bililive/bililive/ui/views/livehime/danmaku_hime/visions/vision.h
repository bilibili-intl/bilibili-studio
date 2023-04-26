#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISIONS_VISION_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISIONS_VISION_H_

#include <functional>

#include "ui/gfx/rect.h"


class Sculptor;

namespace dmkhime {

// 轻量级的 View，用于实现绘制和简单的交互。
// 其中的一些主要回调顺序为：OnMeasure() -> OnSizeChanged() -> OnLayout() -> OnDraw()
class Vision {
public:
    Vision();
    virtual ~Vision() = default;

    void SetId(int id);
    void SetRenderCallback(std::function<void()> c);

    int GetId() const;
    int GetLeft() const;
    int GetTop() const;
    int GetRight() const;
    int GetBottom() const;
    gfx::Rect GetBounds() const;
    gfx::Rect GetLocalBounds() const;
    int GetWidth() const;
    int GetHeight() const;
    int GetMeasuredWidth() const;
    int GetMeasuredHeight() const;

    // 测量该 Vision 的大小，该方法会调用 OnMeasure()
    void Measure(int width, int height);
    // 将该 Vision 放置于指定的位置，该方法会调用 OnLayout() 和 OnSizeChanged()
    void Layout(int left, int top, int right, int bottom);
    // 绘制
    void Draw(Sculptor* s);

    void OffsetLeftAndRight(int dx);
    void OffsetTopAndBottom(int dy);

    // 当该 Vision 想要重绘自身时调用
    void invalidate();

protected:
    void SetMeasuredSize(int width, int height);

    // 在该 Vision 需要决定其大小时被调用。
    // {width} 和 {height} 均为建议大小，用于参考。
    // 必须在该方法中调用 SetMeasuredSize() 来保存测量结果。
    virtual void OnMeasure(int width, int height) {}

    // 在该 Vision 需要被放置于指定位置时被调用。
    // 该方法的调用依赖于 OnMeasure() 的测量结果。
    virtual void OnLayout(int left, int top, int right, int bottom) {}

    // 在该 Vision 大小改变时被调用。
    virtual void OnSizeChanged(int width, int height, int old_w, int old_h) {}

    // 在该 Vision 需要绘制时被调用。
    virtual void OnDraw(Sculptor* s) {}

private:
    int id_;
    gfx::Rect bounds_;

    int measured_width_;
    int measured_height_;

    bool is_measured_;
    bool is_layouted_;
    bool is_drawn_;

    std::function<void()> render_callback_;
};

}

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISIONS_VISION_H_