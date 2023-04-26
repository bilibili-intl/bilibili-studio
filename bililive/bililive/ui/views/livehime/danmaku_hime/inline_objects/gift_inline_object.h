#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_GIFT_INLINE_OBJECT_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_GIFT_INLINE_OBJECT_H_

#include "bililive/bililive/ui/views/livehime/danmaku_hime/d2d_manager.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/wic_bitmaps_read.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/wic_bitmaps_render.h"


class DECLSPEC_UUID("971E4E77-7470-3211-0D5D-A79DC6BB1B12") GiftInlineObject
    : public IDWriteInlineObject {
public:
    explicit GiftInlineObject(const WICBitmaps& source);
    virtual ~GiftInlineObject() = default;

    void SetOpacity(float opacity);
    void SetMetrics(float desired_gift_height, float line_height, float baseline);
    bool SetRenderTarget(base::win::ScopedComPtr<ID2D1RenderTarget> rt, bool hw_acc);
    void ReleaseBitmaps();
    bool IncreaseInterval(int64_t interval);

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
    float desired_gift_height_;
    float opacity_;

    int64_t cur_duaration_ = 0;
    int64_t pre_interval_ = 0;
    size_t cur_index_ = 0;
    bool need_update_ = false;

    bool hw_acc_rendered_;
    WICBitmaps         source_;
    std::unique_ptr<WICBitmapsRender>   render_;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_IMAGE_INLINE_OBJECT_H_