#include "text_renderer.h"

#include <d2d1_2.h>

#include "bililive/bililive/ui/views/livehime/danmaku_hime/text_drawing_effect.h"


TextRenderer::TextRenderer()
    : ref_count_(1),
      text_color_(D2D1::ColorF(D2D1::ColorF::White)),
      outline_color_(D2D1::ColorF(D2D1::ColorF::White)),
      underline_color_(D2D1::ColorF(D2D1::ColorF::White)),
      strikethrough_color_(D2D1::ColorF(D2D1::ColorF::White)) {
}

TextRenderer::~TextRenderer() {
}

void TextRenderer::SetRenderTarget(
    const base::win::ScopedComPtr<ID2D1RenderTarget>& rt)
{
    if (rt == rt_) {
        return;
    }

    rt_ = rt;

    brush_.Release();
    if (rt_) {
        HRESULT hr = rt_->CreateSolidColorBrush(text_color_, brush_.Receive());
        DCHECK(SUCCEEDED(hr));
    }
}

void TextRenderer::setTextColor(const D2D1_COLOR_F& color) {
    text_color_ = color;
}

void TextRenderer::setOutlineColor(const D2D1_COLOR_F& color) {
    outline_color_ = color;
}

void TextRenderer::setOutlineWidth(float width) {
    outline_width_ = width;
}

void TextRenderer::setUnderlineColor(const D2D1_COLOR_F& color) {
    underline_color_ = color;
}

void TextRenderer::setStrikethroughColor(const D2D1_COLOR_F& color) {
    strikethrough_color_ = color;
}

STDMETHODIMP TextRenderer::DrawGlyphRun(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE measuringMode,
    DWRITE_GLYPH_RUN const* glyphRun,
    DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
    IUnknown* clientDrawingEffect)
{
    if (!glyphRun) {
        return E_INVALIDARG;
    }

    if (!brush_) {
        return S_OK;
    }

    auto geo = D2DManager::GetInstance()->CreatePathGeometry();
    if (!geo) {
        return S_OK;
    }

    base::win::ScopedComPtr<ID2D1GeometrySink> sink;
    HRESULT hr = geo->Open(sink.Receive());
    if (FAILED(hr)) {
        return S_OK;
    }

    hr = glyphRun->fontFace->GetGlyphRunOutline(
        glyphRun->fontEmSize, glyphRun->glyphIndices, glyphRun->glyphAdvances, glyphRun->glyphOffsets,
        glyphRun->glyphCount, glyphRun->isSideways, glyphRun->bidiLevel, sink);
    if (SUCCEEDED(hr)) {
        hr = sink->Close();
        if (FAILED(hr)) {
            return S_OK;
        }
    }

    D2D1::Matrix3x2F const matrix = D2D1::Matrix3x2F(
        1.0f, 0.0f,
        0.0f, 1.0f,
        baselineOriginX, baselineOriginY
    );

    base::win::ScopedComPtr<ID2D1Factory> factory;
    rt_->GetFactory(factory.Receive());

    base::win::ScopedComPtr<ID2D1TransformedGeometry> dst_geo;
    hr = factory->CreateTransformedGeometry(geo, matrix, dst_geo.Receive());
    if (FAILED(hr)) {
        return S_OK;
    }

    brush_->SetColor(text_color_);
    rt_->FillGeometry(dst_geo, brush_);
    brush_->SetColor(outline_color_);
    rt_->DrawGeometry(dst_geo, brush_, outline_width_);

    return S_OK;
}

STDMETHODIMP TextRenderer::DrawUnderline(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_UNDERLINE const* underline,
    IUnknown* clientDrawingEffect)
{
    /*if (!underline) {
        return E_INVALIDARG;
    }

    if (!brush_) {
        return S_OK;
    }

    D2D1_RECT_F rect = D2D1::RectF(
        0 + baselineOriginX,
        underline->offset + baselineOriginY,
        underline->width + baselineOriginX,
        underline->offset + underline->thickness + baselineOriginY);

    if (clientDrawingEffect) {
        base::win::ScopedComPtr<TextDrawingEffect> drawingEffect;
        HRESULT hr = drawingEffect.QueryFrom(clientDrawingEffect);
        DCHECK(SUCCEEDED(hr));

        brush_->SetColor(drawingEffect->underline_color_);
        rt_->FillRectangle(rect, brush_.get());
    } else {
        brush_->SetColor(def_underline_color_);
        rt_->FillRectangle(rect, brush_.get());
    }*/

    return S_OK;
}

STDMETHODIMP TextRenderer::DrawStrikethrough(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_STRIKETHROUGH const* strikethrough,
    IUnknown* clientDrawingEffect)
{
    /*if (!strikethrough) {
        return E_INVALIDARG;
    }

    if (!brush_) {
        return S_OK;
    }

    D2D1_RECT_F rect = D2D1::RectF(
        0 + baselineOriginX,
        strikethrough->offset + baselineOriginY,
        strikethrough->width + baselineOriginX,
        strikethrough->offset + strikethrough->thickness + baselineOriginY);

    if (clientDrawingEffect) {
        base::win::ScopedComPtr<TextDrawingEffect> drawingEffect;
        HRESULT hr = drawingEffect.QueryFrom(clientDrawingEffect);
        DCHECK(SUCCEEDED(hr));

        brush_->SetColor(drawingEffect->strikethrough_color_);
        rt_->FillRectangle(rect, brush_.get());
    } else {
        brush_->SetColor(def_strikethrough_color_);
        rt_->FillRectangle(rect, brush_.get());
    }*/

    return S_OK;
}

STDMETHODIMP TextRenderer::DrawInlineObject(
    void* clientDrawingContext,
    FLOAT originX,
    FLOAT originY,
    IDWriteInlineObject* inlineObject,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect)
{
    return inlineObject->Draw(
        clientDrawingContext,
        this,
        originX, originY,
        isSideways,
        isRightToLeft,
        clientDrawingEffect);
}

STDMETHODIMP TextRenderer::IsPixelSnappingDisabled(
    void* clientDrawingContext, BOOL* isDisabled)
{
    if (!isDisabled) {
        return E_INVALIDARG;
    }

    *isDisabled = FALSE;
    return S_OK;
}

STDMETHODIMP TextRenderer::GetCurrentTransform(
    void* clientDrawingContext, DWRITE_MATRIX* transform)
{
    if (!transform) {
        return E_INVALIDARG;
    }

    if (rt_) {
        rt_->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));
    }

    return S_OK;
}

STDMETHODIMP TextRenderer::GetPixelsPerDip(
    void* clientDrawingContext, FLOAT* pixelsPerDip)
{
    if (!pixelsPerDip) {
        return E_INVALIDARG;
    }

    if (rt_) {
        float dpiX, dpiY;

        rt_->GetDpi(&dpiX, &dpiY);
        *pixelsPerDip = dpiX / 96;
    }

    return S_OK;
}


STDMETHODIMP_(ULONG) TextRenderer::AddRef() {
    return InterlockedIncrement(&ref_count_);
}

STDMETHODIMP_(ULONG) TextRenderer::Release() {
    auto new_count = InterlockedDecrement(&ref_count_);
    if (new_count == 0) {
        delete this;
    }

    return new_count;
}

STDMETHODIMP TextRenderer::QueryInterface(
    IID const& riid, void** ppvObject)
{
    if (ppvObject == nullptr) {
        return E_POINTER;
    }

    if (__uuidof(IDWriteTextRenderer) == riid) {
        *ppvObject = this;
    } else if (__uuidof(IDWritePixelSnapping) == riid) {
        *ppvObject = this;
    } else if (__uuidof(IUnknown) == riid) {
        *ppvObject = this;
    } else {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}