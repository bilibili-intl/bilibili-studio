#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISIONS_VISION_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISIONS_VISION_H_

#include <functional>

#include "ui/gfx/rect.h"


class Sculptor;

namespace dmkhime {

// �������� View������ʵ�ֻ��ƺͼ򵥵Ľ�����
// ���е�һЩ��Ҫ�ص�˳��Ϊ��OnMeasure() -> OnSizeChanged() -> OnLayout() -> OnDraw()
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

    // ������ Vision �Ĵ�С���÷�������� OnMeasure()
    void Measure(int width, int height);
    // ���� Vision ������ָ����λ�ã��÷�������� OnLayout() �� OnSizeChanged()
    void Layout(int left, int top, int right, int bottom);
    // ����
    void Draw(Sculptor* s);

    void OffsetLeftAndRight(int dx);
    void OffsetTopAndBottom(int dy);

    // ���� Vision ��Ҫ�ػ�����ʱ����
    void invalidate();

protected:
    void SetMeasuredSize(int width, int height);

    // �ڸ� Vision ��Ҫ�������Сʱ�����á�
    // {width} �� {height} ��Ϊ�����С�����ڲο���
    // �����ڸ÷����е��� SetMeasuredSize() ��������������
    virtual void OnMeasure(int width, int height) {}

    // �ڸ� Vision ��Ҫ��������ָ��λ��ʱ�����á�
    // �÷����ĵ��������� OnMeasure() �Ĳ��������
    virtual void OnLayout(int left, int top, int right, int bottom) {}

    // �ڸ� Vision ��С�ı�ʱ�����á�
    virtual void OnSizeChanged(int width, int height, int old_w, int old_h) {}

    // �ڸ� Vision ��Ҫ����ʱ�����á�
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