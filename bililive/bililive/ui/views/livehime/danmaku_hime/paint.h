#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_PAINT_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_PAINT_H_

#include <vector>

#include "d2d_manager.h"


class Paint {
public:
    using Stops = std::vector<std::pair<D2D1_COLOR_F, float>>;

    enum Style {
        STROKE,
        FILL,
    };

    explicit Paint(const base::win::ScopedComPtr<ID2D1RenderTarget>& rt);

    void SetStyle(Style s);
    void SetColor(const D2D1_COLOR_F& color);
    void SetOpacity(float opacity);
    void SetStrokeWidth(float width);
    void SetGradient(
        const Stops& stops, const D2D1_POINT_2F& start, const D2D1_POINT_2F& end);
    void SetGradientEnabled(bool enabled);

    Style GetStyle() const;
    float GetStrokeWidth() const;
    bool IsGradientEnabled() const;
    ID2D1Brush* GetBrush() const;

private:
    Style style_;
    float stroke_width_;
    bool is_gradient_enabled_;

    base::win::ScopedComPtr<ID2D1RenderTarget> rt_;
    base::win::ScopedComPtr<ID2D1SolidColorBrush> solid_brush_;
    base::win::ScopedComPtr<ID2D1LinearGradientBrush> lin_grad_brush_;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_PAINT_H_