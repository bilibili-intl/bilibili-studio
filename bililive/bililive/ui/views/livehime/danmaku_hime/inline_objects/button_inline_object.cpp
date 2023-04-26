#include "button_inline_object.h"
#include <limits>
#include <cmath>


ButtonInlineObject::ButtonInlineObject()
{
}

ButtonInlineObject::~ButtonInlineObject()
{

}

void ButtonInlineObject::SetButtonText(const base::string16& text)
{
    text_ = text;
    text_layout_ = D2DManager::GetInstance()->CreateTextLayout(text, std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
}

void ButtonInlineObject::SetButtonMargin(float left, float right)
{
    left_margin_ = left;
    right_margin_ = right;
}

void ButtonInlineObject::SetBold(bool bold)
{
    if (bold)
    {
        text_layout_->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, { 0, static_cast<UINT32>(text_.size()) });
    }
    else
    {
        text_layout_->SetFontWeight(DWRITE_FONT_WEIGHT_NORMAL, { 0,static_cast<UINT32>(text_.size()) });
    }
}

void ButtonInlineObject::SetFontSize(float size)
{
    text_layout_->SetFontSize(size, { 0, static_cast<UINT32>(text_.size()) });
    //文字每大一号，文字行高百分比增加0.0769230760264182，以14字号为准计算缩放
    scalar_ = 1.0f + ((size - 14.0f) * 0.0769230760264182);
    scalar_ = std::max(scalar_, 0.0f);
}

void ButtonInlineObject::SetFontColor(D2D1_COLOR_F font_color)
{
    font_color_ = font_color;
}

void ButtonInlineObject::SetBackgroundColor(D2D1_COLOR_F background_color)
{
    background_color_ = background_color;
}

void ButtonInlineObject::SetAssistantGuide(bool guide)
{
    guide_ = guide;
}

bool ButtonInlineObject::SetRenderTarget(base::win::ScopedComPtr<ID2D1RenderTarget> rt, bool hw_acc)
{
    if (!__super::SetRenderTarget(rt, hw_acc))
    {
        return false;
    }

    DCHECK(rt.get());
    if (!rt.get())
    {
        return false;
    }

    HRESULT hr{};
    if (!font_brush_)
    {
        hr = rt_->CreateSolidColorBrush(font_color_, font_brush_.Receive());
        if (FAILED(hr))
        {
            DCHECK(false);
            LOG(WARNING) << "Failed to create solid brush: " << hr;
            return false;
        }
    }

    if (!background_brush_)
    {
        hr = rt_->CreateSolidColorBrush(background_color_, background_brush_.Receive());
        if (FAILED(hr))
        {
            DCHECK(false);
            LOG(WARNING) << "Failed to create solid brush: " << hr;
            return false;
        }

    }

    return true;
}

STDMETHODIMP_(HRESULT __stdcall) ButtonInlineObject::Draw(void* clientDrawingContext, IDWriteTextRenderer* renderer, FLOAT originX, FLOAT originY, BOOL isSideways, BOOL isRightToLeft, IUnknown* clientDrawingEffect)
{
    if (!rt_ || !text_layout_ || !font_brush_ || !background_brush_)
    {
        return E_FAIL;
    }

    background_brush_->SetColor(background_color_);

    DWRITE_TEXT_METRICS text_metrics{};
    text_layout_->GetMetrics(&text_metrics);

    D2D1_ROUNDED_RECT round_rect{};
    round_rect.rect = { originX, originY, originX + text_metrics.width + left_margin_ * scalar_ + right_margin_ * scalar_, originY + std::ceil(text_metrics.height)};
    round_rect.radiusX = text_metrics.height * 0.5;
    round_rect.radiusY = text_metrics.height * 0.5;
    rt_->FillRoundedRectangle(round_rect, background_brush_);

    font_brush_->SetColor(font_color_);
    rt_->DrawTextLayout({ originX + left_margin_ * scalar_, originY }, text_layout_, font_brush_);

    if (guide_) {
        FLOAT offset_x = originX + left_margin_ * scalar_ + text_metrics.width + (right_margin_ * scalar_) / 3;
        FLOAT offset_x_end = originX + left_margin_ * scalar_ + text_metrics.width + (right_margin_ * scalar_)*2 / 3;

        FLOAT offset_y = originY + (round_rect.rect.bottom - round_rect.rect.top) / 2 - ScaleI(5);

        rt_->DrawLine(D2D1::Point2F(offset_x, originY+ ScaleI(5)), D2D1::Point2F(offset_x_end, offset_y + ScaleI(5)), font_brush_);
        rt_->DrawLine(D2D1::Point2F(offset_x_end, offset_y + ScaleI(5)), D2D1::Point2F(offset_x, round_rect.rect.bottom - ScaleI(5)), font_brush_);
    }

    return S_OK;
}

STDMETHODIMP_(HRESULT __stdcall) ButtonInlineObject::GetMetrics(DWRITE_INLINE_OBJECT_METRICS* metrics)
{
    DWRITE_TEXT_METRICS text_metrics{};
    text_layout_->GetMetrics(&text_metrics);

    DWRITE_LINE_METRICS line_metrics{};
    UINT32 line_count{};
    text_layout_->GetLineMetrics(&line_metrics, 1, &line_count);

    metrics->baseline = line_metrics.baseline;
    metrics->width = text_metrics.width + left_margin_ * scalar_ + right_margin_ * scalar_;
    metrics->height = text_metrics.height;

    return S_OK;
}

STDMETHODIMP_(HRESULT __stdcall) ButtonInlineObject::GetOverhangMetrics(DWRITE_OVERHANG_METRICS* overhangs)
{
    return text_layout_->GetOverhangMetrics(overhangs);
}
