#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_MEDAL_INLINE_OBJECT_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_MEDAL_INLINE_OBJECT_H_

#include "bililive/bililive/ui/views/livehime/danmaku_hime/d2d_manager.h"


class TextLayout;

class DECLSPEC_UUID("1F52BC45-859E-4439-9D54-7B2C2EA090B2") MedalInlineObject
    : public IDWriteInlineObject {
public:
    MedalInlineObject(
        const string16& name, int64_t uid, int level, int guard_level,
        bool has_border, const D2D1_COLOR_F& border_color,
        const D2D1_COLOR_F& start_color, const D2D1_COLOR_F& end_color);
    virtual ~MedalInlineObject() = default;

    void SetOpacity(float opacity);
    void SetFontSize(float size);
    void SetMetrics(float desired_image_height, float line_height, float baseline);
    bool SetRenderTarget(
        base::win::ScopedComPtr<ID2D1RenderTarget> rt, bool hw_acc);

    void ReleaseBitmaps();

    STDMETHOD(Draw)(
        void* clientDrawingContext,
        IDWriteTextRenderer* renderer,
        FLOAT originX,
        FLOAT originY,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect) override;

    STDMETHOD(GetMetrics)(
        DWRITE_INLINE_OBJECT_METRICS* metrics) override;

    STDMETHOD(GetOverhangMetrics)(
        DWRITE_OVERHANG_METRICS* overhangs) override;

    STDMETHOD(GetBreakConditions)(
        DWRITE_BREAK_CONDITION* breakConditionBefore,
        DWRITE_BREAK_CONDITION* breakConditionAfter) override;

    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE QueryInterface(
        IID const& riid, void** ppvObject) override;

private:
    void DrawUserMedal(float left, float top, float height);

    ULONG ref_count_ = 0;
    int64_t uid_ = 0;
    bool using_honor_ = false;
    int level_ = 0;
    int guard_level_ = 0;

    float baseline_ = 0.0f;
    float line_height_ = 0.0f;
    float desired_height_ = 0.0f;
    float opacity_ = 0.0f;
    float font_up_space_ = 0.0f;
    float actul_font_height_ = 0.0f;

    bool has_border_ = true;
    D2D1_COLOR_F border_color_ = {1.0f, 0.0f, 0.0f, 1.0f};
    D2D1_COLOR_F start_color_ = {};
    D2D1_COLOR_F end_color_ = {};
    D2D1_COLOR_F level_color_ = {};
    std::pair<std::unique_ptr<TextLayout>, std::unique_ptr<TextLayout>> user_medal_;

    bool hw_acc_rendered_ = false;
    std::string raw_icon_buffer_;
    base::win::ScopedComPtr<ID2D1Bitmap> bitmap_;
    base::win::ScopedComPtr<IWICBitmapSource> source_;

    std::string crown_raw_icon_buffer_;
    base::win::ScopedComPtr<ID2D1Bitmap> crown_bitmap_;
    base::win::ScopedComPtr<IWICBitmapSource> crowwn_source_;

    base::win::ScopedComPtr<ID2D1RenderTarget> rt_;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_MEDAL_INLINE_OBJECT_H_