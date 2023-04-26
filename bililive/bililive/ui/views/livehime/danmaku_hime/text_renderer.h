#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_TEXT_RENDERER_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_TEXT_RENDERER_H_

#include "base/win/scoped_comptr.h"

#include "bililive/bililive/ui/views/livehime/danmaku_hime/d2d_manager.h"


class DECLSPEC_UUID("B132CD67-7380-47B1-97CC-88E48FC48ED0") TextRenderer : public IDWriteTextRenderer {
public:
    TextRenderer();
    virtual ~TextRenderer();

    void SetRenderTarget(const base::win::ScopedComPtr<ID2D1RenderTarget>& rt);

    void setTextColor(const D2D1_COLOR_F& color);
    void setOutlineColor(const D2D1_COLOR_F& color);
    void setOutlineWidth(float width);
    void setUnderlineColor(const D2D1_COLOR_F& color);
    void setStrikethroughColor(const D2D1_COLOR_F& color);

    STDMETHOD(DrawGlyphRun)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        DWRITE_GLYPH_RUN const* glyphRun,
        DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        IUnknown* clientDrawingEffect) override;

    STDMETHOD(DrawUnderline)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_UNDERLINE const* underline,
        IUnknown* clientDrawingEffect) override;

    STDMETHOD(DrawStrikethrough)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_STRIKETHROUGH const* strikethrough,
        IUnknown* clientDrawingEffect) override;

    STDMETHOD(DrawInlineObject)(
        void* clientDrawingContext,
        FLOAT originX,
        FLOAT originY,
        IDWriteInlineObject* inlineObject,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect) override;

    STDMETHOD(IsPixelSnappingDisabled)(
        void* clientDrawingContext,
        BOOL* isDisabled) override;

    STDMETHOD(GetCurrentTransform)(
        void* clientDrawingContext,
        DWRITE_MATRIX* transform) override;

    STDMETHOD(GetPixelsPerDip)(
        void* clientDrawingContext,
        FLOAT* pixelsPerDip) override;

    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE QueryInterface(
        IID const& riid, void** ppvObject) override;

private:
    ULONG ref_count_;

    D2D1_COLOR_F text_color_;
    D2D1_COLOR_F outline_color_;
    float outline_width_ = 1;
    D2D1_COLOR_F underline_color_;
    D2D1_COLOR_F strikethrough_color_;

    base::win::ScopedComPtr<ID2D1RenderTarget> rt_;
    base::win::ScopedComPtr<ID2D1SolidColorBrush> brush_;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_TEXT_RENDERER_H_