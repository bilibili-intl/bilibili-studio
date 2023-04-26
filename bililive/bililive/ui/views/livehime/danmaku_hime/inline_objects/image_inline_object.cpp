#include "bililive/bililive/ui/views/livehime/danmaku_hime/inline_objects/image_inline_object.h"
#include<vector>
using BitmapCacheType = std::pair< base::win::ScopedComPtr<IWICBitmapSource>, base::win::ScopedComPtr<ID2D1Bitmap>>;
std::vector<BitmapCacheType> g_d2dbitmap_cache;

ImageInlineObject::ImageInlineObject(
    base::win::ScopedComPtr<IWICBitmapSource> source,
    bool is_cache)
    : ref_count_(1),
      baseline_(0),
      line_height_(0),
      desired_image_height_(0),
      opacity_(1.f),
      hw_acc_rendered_(false),
      source_(source),
      is_cache_(is_cache){
}

void ImageInlineObject::SetOpacity(float opacity) {
    opacity_ = opacity;
}

void ImageInlineObject::SetMetrics(float desired_image_height, float line_height, float baseline) {
    baseline_ = baseline;
    line_height_ = line_height;
    desired_image_height_ = desired_image_height;
}

bool ImageInlineObject::SetRenderTarget(base::win::ScopedComPtr<ID2D1RenderTarget> rt, bool hw_acc) {
    DCHECK(rt != nullptr);
    rt_ = rt;

    if (!source_) {
        return false;
    }

    if (hw_acc != hw_acc_rendered_) {
        bitmap_.Release();
        hw_acc_rendered_ = hw_acc;
    }

    if (!bitmap_ && rt_) {
        if (is_cache_) {
            auto itr = std::find_if(g_d2dbitmap_cache.begin(), g_d2dbitmap_cache.end(),
                [this](const BitmapCacheType& element) {
                    if (source_) {
                        return source_ == element.first;
                    }
                    return false;
                });
            if (itr != g_d2dbitmap_cache.end()) {
                bitmap_ = itr->second;
            }
            else {
                D2D1_BITMAP_PROPERTIES properties
                    = D2D1::BitmapProperties(
                        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

                HRESULT hr = rt_->CreateBitmapFromWicBitmap(source_.get(), properties, bitmap_.Receive());
                if (FAILED(hr)) {
                    LOG(WARNING) << "Failed to create bitmap from wic bitmap: " << hr;
                    return false;
                }
                g_d2dbitmap_cache.push_back({ source_ ,bitmap_ });
            }
        }
        else {
            D2D1_BITMAP_PROPERTIES properties
                = D2D1::BitmapProperties(
                    D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

            HRESULT hr = rt_->CreateBitmapFromWicBitmap(source_.get(), properties, bitmap_.Receive());
            if (FAILED(hr)) {
                LOG(WARNING) << "Failed to create bitmap from wic bitmap: " << hr;
                return false;
            }
        }
    }
    return true;
}

void ImageInlineObject::ReleaseBitmaps() {
    bitmap_.Release();
}


STDMETHODIMP ImageInlineObject::Draw(
    void* clientDrawingContext,
    IDWriteTextRenderer* renderer,
    FLOAT originX,
    FLOAT originY,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect)
{
    if (bitmap_) {
        auto bmp_size = bitmap_->GetSize();
        float scale = desired_image_height_ / bmp_size.height;

        D2D1_RECT_F dest_rect = {
            originX, originY + 1,
            originX + bmp_size.width * scale, originY + 1 + desired_image_height_ };
        rt_->DrawBitmap(bitmap_.get(), dest_rect, opacity_);

        /*ID2D1SolidColorBrush* b;
        rt_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &b);
        rt_->DrawRectangle(dest_rect, b);
        b->Release();*/
    }

    return S_OK;
}

STDMETHODIMP ImageInlineObject::GetMetrics(
    DWRITE_INLINE_OBJECT_METRICS* metrics) {

    if (!metrics) {
        return E_INVALIDARG;
    }

    if (source_) {
        UINT img_width = 0, img_height = 0;
        HRESULT hr = source_->GetSize(&img_width, &img_height);
        if (SUCCEEDED(hr) && img_width > 0 && img_height > 0) {
            float img_scale = desired_image_height_ / img_height;

            metrics->width = img_width * img_scale;
            metrics->height = desired_image_height_;
            metrics->baseline = baseline_
                + (desired_image_height_ - line_height_) / 2;
            metrics->supportsSideways = FALSE;
        } else {
            DCHECK(false);

            metrics->width = desired_image_height_;
            metrics->height = desired_image_height_;
            metrics->baseline = baseline_
                + (desired_image_height_ - line_height_) / 2;
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

STDMETHODIMP ImageInlineObject::GetOverhangMetrics(
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

STDMETHODIMP ImageInlineObject::GetBreakConditions(
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


STDMETHODIMP_(ULONG) ImageInlineObject::AddRef() {
    return InterlockedIncrement(&ref_count_);
}

STDMETHODIMP_(ULONG) ImageInlineObject::Release() {
    auto new_count = InterlockedDecrement(&ref_count_);
    if (new_count == 0) {
        delete this;
    }

    return new_count;
}

STDMETHODIMP ImageInlineObject::QueryInterface(
    IID const& riid, void** ppvObject)
{
    if (ppvObject == nullptr) {
        return E_POINTER;
    }

    if (__uuidof(ImageInlineObject) == riid) {
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