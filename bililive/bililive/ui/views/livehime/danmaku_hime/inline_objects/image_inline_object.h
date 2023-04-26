#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_IMAGE_INLINE_OBJECT_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_IMAGE_INLINE_OBJECT_H_

#include "bililive/bililive/ui/views/livehime/danmaku_hime/d2d_manager.h"


class DECLSPEC_UUID("4EA8FCE7-6FAA-4994-A852-9531BD0FC086") ImageInlineObject
    : public IDWriteInlineObject {
public:
    //is_cache: 是否开启渲染缓存,适用对象生命周期长并且有大量重复对象存在(eg:弹幕姬普通消息)
    //          其他情况还是不走缓存的情况好
    explicit ImageInlineObject(
        base::win::ScopedComPtr<IWICBitmapSource> source,
        bool is_cache = false);
    virtual ~ImageInlineObject() = default;

    void SetOpacity(float opacity);
    void SetMetrics(float desired_image_height, float line_height, float baseline);
    bool SetRenderTarget(base::win::ScopedComPtr<ID2D1RenderTarget> rt, bool hw_acc);
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
    ULONG ref_count_;

    float baseline_;
    float line_height_;
    float desired_image_height_;
    float opacity_;

    bool hw_acc_rendered_;
    bool is_cache_ = false;
    base::win::ScopedComPtr<ID2D1Bitmap> bitmap_;
    base::win::ScopedComPtr<ID2D1RenderTarget> rt_;
    base::win::ScopedComPtr<IWICBitmapSource> source_;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_IMAGE_INLINE_OBJECT_H_