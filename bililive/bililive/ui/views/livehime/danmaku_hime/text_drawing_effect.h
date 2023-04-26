#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_TEXT_DRAWING_EFFECT_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_TEXT_DRAWING_EFFECT_H_

#include <Unknwn.h>

#include "bililive/bililive/ui/views/livehime/danmaku_hime/d2d_manager.h"


class DECLSPEC_UUID("95C5353B-09B0-49B5-B2A6-729CB5F09446") TextDrawingEffect
    : public IUnknown {
public:
    TextDrawingEffect();
    virtual ~TextDrawingEffect();

    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid, void** ppvObject) override;

    D2D1_COLOR_F text_color_;
    D2D1_COLOR_F underline_color_;
    D2D1_COLOR_F strikethrough_color_;

private:
    ULONG ref_count_;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_TEXT_DRAWING_EFFECT_H_