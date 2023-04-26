#include "paint.h"


Paint::Paint(const base::win::ScopedComPtr<ID2D1RenderTarget>& rt)
    : style_(Style::FILL),
      stroke_width_(1.f),
      is_gradient_enabled_(false),
      rt_(rt)
{
    if (rt_) {
        HRESULT hr = rt_->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::White), solid_brush_.Receive());
        if (FAILED(hr)) {
            DCHECK(false);
            LOG(WARNING) << "Failed to create solid brush: " << hr;
        }
    }
}

void Paint::SetStyle(Style s) {
    style_ = s;
}

void Paint::SetColor(const D2D1_COLOR_F& color) {
    if (solid_brush_) {
        solid_brush_->SetColor(color);
    }
}

void Paint::SetOpacity(float opacity) {
    if (solid_brush_) {
        solid_brush_->SetOpacity(opacity);
    }

    if (lin_grad_brush_) {
        lin_grad_brush_->SetOpacity(opacity);
    }
}

void Paint::SetStrokeWidth(float width) {
    stroke_width_ = width;
}

void Paint::SetGradient(
    const Stops& stops, const D2D1_POINT_2F& start, const D2D1_POINT_2F& end)
{
    if (lin_grad_brush_) {
        lin_grad_brush_.Release();
    }

    base::win::ScopedComPtr<ID2D1GradientStopCollection> grad_stops;
    if (rt_ && !stops.empty()) {
        D2D1_GRADIENT_STOP* gs = new D2D1_GRADIENT_STOP[stops.size()];
        for (size_t i = 0; i < stops.size(); ++i) {
            gs[i].color = stops[i].first;
            gs[i].position = stops[i].second;
        }

        HRESULT hr = rt_->CreateGradientStopCollection(
            gs, stops.size(),
            D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP,
            grad_stops.Receive());
        DCHECK(SUCCEEDED(hr));

        delete[] gs;
    }

    if (grad_stops) {
        HRESULT hr = rt_->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    start, end),
                grad_stops,
                lin_grad_brush_.Receive());
        DCHECK(SUCCEEDED(hr));
    }
}

void Paint::SetGradientEnabled(bool enabled) {
    is_gradient_enabled_ = enabled;
}

Paint::Style Paint::GetStyle() const {
    return style_;
}

float Paint::GetStrokeWidth() const {
    return stroke_width_;
}

bool Paint::IsGradientEnabled() const {
    return is_gradient_enabled_ && lin_grad_brush_ != nullptr;
}

ID2D1Brush* Paint::GetBrush() const {
    if (IsGradientEnabled()) {
        return lin_grad_brush_.get();
    }

    return solid_brush_.get();
}