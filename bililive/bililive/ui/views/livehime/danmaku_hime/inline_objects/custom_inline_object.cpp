#include "bililive/bililive/ui/views/livehime/danmaku_hime/inline_objects/custom_inline_object.h"


CustomInlineObject::CustomInlineObject()
    : ref_count_(1),
      baseline_(0),
      line_height_(0),
      desired_height_(0),
      opacity_(1.f)
{
}

void CustomInlineObject::SetOpacity(float opacity) {
    opacity_ = opacity;
}

void CustomInlineObject::SetFontSize(float size) {
}

void CustomInlineObject::SetMetrics(float desired_image_height, float line_height, float baseline) {
    baseline_ = baseline;
    line_height_ = line_height;
    desired_height_ = desired_image_height;
}

bool CustomInlineObject::SetRenderTarget(base::win::ScopedComPtr<ID2D1RenderTarget> rt, bool hw_acc) {
    DCHECK(rt != nullptr);

    rt_ = rt;
    return true;
}

STDMETHODIMP CustomInlineObject::GetOverhangMetrics(
    DWRITE_OVERHANG_METRICS* overhangs)
{
    if (overhangs == nullptr) {
        return E_INVALIDARG;
    }

    overhangs->left = 0;
    overhangs->top = 0;
    overhangs->right = 0;
    overhangs->bottom = 0;

    return S_OK;
}

STDMETHODIMP CustomInlineObject::GetBreakConditions(
    DWRITE_BREAK_CONDITION* breakConditionBefore,
    DWRITE_BREAK_CONDITION* breakConditionAfter)
{
    if (breakConditionBefore == nullptr
        || breakConditionAfter == nullptr)
    {
        return E_INVALIDARG;
    }

    *breakConditionBefore = DWRITE_BREAK_CONDITION_NEUTRAL;
    *breakConditionAfter = DWRITE_BREAK_CONDITION_NEUTRAL;

    return S_OK;
}


STDMETHODIMP_(ULONG) CustomInlineObject::AddRef() {
    return InterlockedIncrement(&ref_count_);
}

STDMETHODIMP_(ULONG) CustomInlineObject::Release() {
    auto new_count = InterlockedDecrement(&ref_count_);
    if (new_count == 0) {
        delete this;
    }

    return new_count;
}

STDMETHODIMP CustomInlineObject::QueryInterface(
    IID const& riid, void** ppvObject)
{
    if (ppvObject == nullptr) {
        return E_POINTER;
    }

    if (__uuidof(CustomInlineObject) == riid) {
        *ppvObject = this;
    } else if (__uuidof(IDWriteInlineObject) == riid) {
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