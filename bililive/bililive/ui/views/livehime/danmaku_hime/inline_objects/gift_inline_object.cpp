#include "bililive/bililive/ui/views/livehime/danmaku_hime/inline_objects/gift_inline_object.h"


GiftInlineObject::GiftInlineObject(
    const WICBitmaps& source)
    : ref_count_(1),
      baseline_(0),
      line_height_(0),
      desired_gift_height_(0),
      opacity_(1.f),
      hw_acc_rendered_(false),
      source_(source)
{
    render_ = std::make_unique<WICBitmapsRender>(source);
}

void GiftInlineObject::SetOpacity(float opacity) {
    opacity_ = opacity;
}

void GiftInlineObject::SetMetrics(float desired_gift_height, float line_height, float baseline) {
    baseline_ = baseline;
    line_height_ = line_height;
    desired_gift_height_ = desired_gift_height;
}

bool GiftInlineObject::SetRenderTarget(base::win::ScopedComPtr<ID2D1RenderTarget> rt, bool hw_acc) {
    DCHECK(rt != nullptr);
    if (render_){
        return render_->SetRenderTarget(rt, hw_acc);
    }
    return false;
}

void GiftInlineObject::ReleaseBitmaps() {
    //render_->ReleaseBitmaps();
}

bool GiftInlineObject::IncreaseInterval(int64_t interval)
{
    if (!render_) {
        return false;
    }
    cur_index_ = render_->GetCurrentIndex();
    if (cur_index_ >= source_.frames.size())
    {
        cur_index_ = 0;
    }

    if (cur_index_ < source_.frames.size() &&
        pre_interval_ <= (cur_duaration_ + interval))
    {
        pre_interval_ = source_.frames[cur_index_].frame_interval;
        cur_duaration_ = 0;
        need_update_ = true;
        return true;

    }
    else{
        cur_duaration_ += interval;
    }
    return false;
}


STDMETHODIMP GiftInlineObject::Draw(
    void* clientDrawingContext,
    IDWriteTextRenderer* renderer,
    FLOAT originX,
    FLOAT originY,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect)
{
    if (render_ &&
        source_.width > 0 &&
        source_.height> 0&&
        cur_index_ < source_.frames.size())
    {
        if (need_update_){
            render_->PresentFrame();
            need_update_ = false;
        }
        float gift_scale = desired_gift_height_ / source_.height;
        render_->RenderProcess({ originX, originY+1, static_cast<float>(originX + source_.width* gift_scale), static_cast<float>(originY + desired_gift_height_) });
    }

    return S_OK;
}

STDMETHODIMP GiftInlineObject::GetMetrics(
    DWRITE_INLINE_OBJECT_METRICS* metrics) {

    if (!metrics) {
        return E_INVALIDARG;
    }

    if (!source_.empty()) {
        if (source_.width > 0 && source_.height > 0) {
            float gift_scale = desired_gift_height_ / source_.height;

            metrics->width = source_.width * gift_scale;
            metrics->height = desired_gift_height_;
            metrics->baseline = baseline_
                + (desired_gift_height_ - line_height_) / 2;
            metrics->supportsSideways = FALSE;
        } else {
            DCHECK(false);
            metrics->width = desired_gift_height_;
            metrics->height = desired_gift_height_;
            metrics->baseline = baseline_
                + (desired_gift_height_ - line_height_) / 2;
            metrics->supportsSideways = FALSE;
        }
    } else {
        DCHECK(false);

        metrics->width = 0;
        metrics->height = 0;
        metrics->baseline = baseline_;
        metrics->supportsSideways = FALSE;
    }

    return S_OK;
}

STDMETHODIMP GiftInlineObject::GetOverhangMetrics(
    DWRITE_OVERHANG_METRICS* overhangs) {

    if (overhangs == nullptr) {
        return E_INVALIDARG;
    }

    overhangs->left = 0;
    overhangs->top = 0;
    overhangs->right = 0;
    overhangs->bottom = 0;

    return S_OK;
}

STDMETHODIMP GiftInlineObject::GetBreakConditions(
    DWRITE_BREAK_CONDITION* breakConditionBefore,
    DWRITE_BREAK_CONDITION* breakConditionAfter)
{
    if (breakConditionBefore == nullptr
        || breakConditionAfter == nullptr) {
        return E_INVALIDARG;
    }

    *breakConditionBefore = DWRITE_BREAK_CONDITION_NEUTRAL;
    *breakConditionAfter = DWRITE_BREAK_CONDITION_NEUTRAL;

    return S_OK;
}


STDMETHODIMP_(ULONG) GiftInlineObject::AddRef() {
    return InterlockedIncrement(&ref_count_);
}

STDMETHODIMP_(ULONG) GiftInlineObject::Release() {
    auto new_count = InterlockedDecrement(&ref_count_);
    if (new_count == 0) {
        delete this;
    }

    return new_count;
}

STDMETHODIMP GiftInlineObject::QueryInterface(
    IID const& riid, void** ppvObject)
{
    if (ppvObject == nullptr) {
        return E_POINTER;
    }

    if (__uuidof(GiftInlineObject) == riid) {
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