#include "spread_inline_object.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/paint.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/sculptor.h"


SpreadInlineObject::SpreadInlineObject(
    const string16& text,
    uint32_t color_start, uint32_t color_end)
    : color_start_(color_start), color_end_(color_end)
{
    layout_.Init(text, 0);
    layout_.SetWrap(false);
}

void SpreadInlineObject::SetFontSize(float size) {
    font_size_ = size - GetLengthByDPIScale(5);
    layout_.SetFontSize(font_size_);
}

STDMETHODIMP SpreadInlineObject::Draw(
    void* clientDrawingContext,
    IDWriteTextRenderer* renderer,
    FLOAT originX,
    FLOAT originY,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect)
{
    if (!rt_) {
        return S_OK;
    }

    Sculptor sculptor(rt_);
    sculptor.GetPaint()->SetOpacity(opacity_);

    int hori_padding = font_size_ / 2;
    int hori_margin = font_size_ / 2;

    float left = originX + hori_margin;
    float top = originY;

    float text_width = layout_.GetTextWidthFloat();
    float text_height = layout_.GetTextHeightFloat();
    int rect_width = text_width + hori_padding * 2;

    float text_x = hori_padding + left;
    float text_y = top;

    auto flag_rect = D2D1::RectF(
        left, text_y + 1,
        left + rect_width, text_y + text_height + 1);

    {
        Paint::Stops stops;
        stops.push_back({ D2D1::ColorF(color_start_), 0 });
        stops.push_back({ D2D1::ColorF(color_end_), 1 });

        sculptor.GetPaint()->SetGradientEnabled(true);
        sculptor.GetPaint()->SetStyle(Paint::FILL);
        sculptor.GetPaint()->SetGradient(
            stops, D2D1::Point2F(flag_rect.left, 0), D2D1::Point2F(flag_rect.right, 0));
        sculptor.DrawRoundRect(flag_rect, text_height / 2);
        sculptor.GetPaint()->SetGradientEnabled(false);
    }
    sculptor.GetPaint()->SetStyle(Paint::FILL);
    sculptor.GetPaint()->SetColor(D2D1::ColorF(D2D1::ColorF::White));
    sculptor.DrawTextLayout(layout_, text_x, text_y);

    return S_OK;
}

STDMETHODIMP SpreadInlineObject::GetMetrics(
    DWRITE_INLINE_OBJECT_METRICS* metrics)
{
    if (!metrics) {
        return E_INVALIDARG;
    }

    int hori_padding = font_size_ / 2;
    int hori_margin = font_size_ / 2;

    float text_height = layout_.GetTextHeightFloat();
    float rect_width = layout_.GetTextWidthFloat() + hori_padding * 2 + hori_margin;

    metrics->width = rect_width;
    metrics->height = text_height;
    metrics->baseline = baseline_ + (text_height - line_height_) / 2;
    metrics->supportsSideways = FALSE;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE SpreadInlineObject::QueryInterface(
    IID const& riid, void** ppvObject)
{
    HRESULT hr = super::QueryInterface(riid, ppvObject);
    if (hr != E_NOINTERFACE) {
        return hr;
    }

    if (__uuidof(SpreadInlineObject) == riid) {
        *ppvObject = this;
    } else {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}