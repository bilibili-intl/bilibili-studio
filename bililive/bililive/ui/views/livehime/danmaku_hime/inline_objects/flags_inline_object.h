#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_FLAGS_INLINE_OBJECT_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_FLAGS_INLINE_OBJECT_H_

#include <vector>

#include "bililive/bililive/ui/views/livehime/danmaku_hime/d2d_manager.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_structs.h"


class TextLayout;

class DECLSPEC_UUID("6E4B8C9E-66F7-40D5-813C-57413C3E0233") FlagsInlineObject
    : public IDWriteInlineObject {
public:
    using Flags = std::vector<std::pair<DanmakuFlags, string16>>;
    using FlagColorSetter = std::function<void(DanmakuFlags, D2D1_COLOR_F&)>;

    explicit FlagsInlineObject(const Flags& flags);
    virtual ~FlagsInlineObject() = default;

    void SetOpacity(float opacity);
    void SetFontSize(float size);
    void SetMetrics(float desired_image_height, float line_height, float baseline);
    bool SetRenderTarget(base::win::ScopedComPtr<ID2D1RenderTarget> rt, bool hw_acc);

    void SetFlagsColor(const FlagColorSetter& f);

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
    void FlagsInlineObject::DrawUserFlags(
        float left, float top, float height);

    struct FlagInfo {
        DanmakuFlags df;
        D2D1_COLOR_F color;
        std::shared_ptr<TextLayout> layout;

        std::shared_ptr<std::string> raw_buf;
        base::win::ScopedComPtr<IWICBitmapSource> wic_bmp;
        base::win::ScopedComPtr<ID2D1Bitmap> d2d_bmp;
    };

    ULONG ref_count_;

    float baseline_;
    float line_height_;
    float desired_height_;
    float font_size_;
    float opacity_;
    std::vector<FlagInfo> user_flags_;
    bool hw_acc_rendered_ = false;

    base::win::ScopedComPtr<ID2D1RenderTarget> rt_;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_FLAGS_INLINE_OBJECT_H_