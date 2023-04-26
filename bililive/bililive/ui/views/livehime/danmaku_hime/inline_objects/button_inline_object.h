#pragma once

#include "bililive/bililive/ui/views/livehime/danmaku_hime/inline_objects/custom_inline_object.h"

class DECLSPEC_UUID("BD9F702E-46B1-4CA2-B1FB-37A7AA71A607") ButtonInlineObject :
    public CustomInlineObject
{
public:
    ButtonInlineObject();
    virtual ~ButtonInlineObject();

    void SetButtonText(const base::string16& text);
    void SetButtonMargin(float left, float right);
    void SetBold(bool bold);
    void SetFontColor(D2D1_COLOR_F font_color);
    void SetBackgroundColor(D2D1_COLOR_F font_color);
    void SetAssistantGuide(bool guide);

    void SetFontSize(float size) override;
    bool SetRenderTarget(base::win::ScopedComPtr<ID2D1RenderTarget> rt, bool hw_acc) override;

    STDMETHOD(Draw)(
        void* clientDrawingContext,
        IDWriteTextRenderer * renderer,
        FLOAT originX,
        FLOAT originY,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown * clientDrawingEffect) override;

    STDMETHOD(GetMetrics)(
        DWRITE_INLINE_OBJECT_METRICS * metrics) override;

    STDMETHOD(GetOverhangMetrics)(
        DWRITE_OVERHANG_METRICS * overhangs) override;

private:
    int ScaleI(int x) const { return int(x * sx_); }

private:
    float                                               left_margin_ = 0.0f;
    float                                               right_margin_ = 0.0f;
    bool                                                bold_ = false;
    bool                                                guide_ = false;
    float                                               sx_ = 1.f;
    D2D1_COLOR_F                                        font_color_ = {};
    D2D1_COLOR_F                                        background_color_ = {};
    float                                               scalar_ = 1.0f;
    base::string16                                      text_;
    base::win::ScopedComPtr<IDWriteTextLayout>          text_layout_;
    base::win::ScopedComPtr<ID2D1SolidColorBrush>       font_brush_;
    base::win::ScopedComPtr<ID2D1SolidColorBrush>       background_brush_;
};
