#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_SPREAD_INLINE_OBJECT_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_SPREAD_INLINE_OBJECT_H_

#include "bililive/bililive/ui/views/livehime/danmaku_hime/inline_objects/custom_inline_object.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/text_layout.h"


class DECLSPEC_UUID("BE6BC38B-D557-431D-BCF9-B166507531DB") SpreadInlineObject
    : public CustomInlineObject
{
public:
    SpreadInlineObject(
        const string16& text,
        uint32_t color_start, uint32_t color_end);

    void SetFontSize(float size) override;

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

    HRESULT STDMETHODCALLTYPE QueryInterface(
        IID const& riid, void** ppvObject) override;

private:
    using super = CustomInlineObject;

    float font_size_ = 9.f;
    TextLayout layout_;
    uint32_t color_start_;
    uint32_t color_end_;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_SPREAD_INLINE_OBJECT_H_