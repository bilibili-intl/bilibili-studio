#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_CUSTOM_INLINE_OBJECT_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_CUSTOM_INLINE_OBJECT_H_

#include "bililive/bililive/ui/views/livehime/danmaku_hime/d2d_manager.h"


class DECLSPEC_UUID("BD9F702E-46B1-4CA2-B1FB-37A7AA71A607") CustomInlineObject
    : public IDWriteInlineObject
{
public:
    CustomInlineObject();
    virtual ~CustomInlineObject() = default;

    virtual void SetOpacity(float opacity);
    virtual void SetFontSize(float size);
    virtual void SetMetrics(float desired_image_height, float line_height, float baseline);
    virtual bool SetRenderTarget(base::win::ScopedComPtr<ID2D1RenderTarget> rt, bool hw_acc);
    virtual void ReleaseBitmaps() {}

    STDMETHOD(GetOverhangMetrics)(
        DWRITE_OVERHANG_METRICS* overhangs) override;

    STDMETHOD(GetBreakConditions)(
        DWRITE_BREAK_CONDITION* breakConditionBefore,
        DWRITE_BREAK_CONDITION* breakConditionAfter) override;

    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE QueryInterface(
        IID const& riid, void** ppvObject) override;

protected:
    float baseline_;
    float line_height_;
    float desired_height_;
    float opacity_;

    base::win::ScopedComPtr<ID2D1RenderTarget> rt_;

private:
    ULONG ref_count_;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_INLINE_OBJECTS_CUSTOM_INLINE_OBJECT_H_