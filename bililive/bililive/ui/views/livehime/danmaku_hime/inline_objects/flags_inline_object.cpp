#include "bililive/bililive/ui/views/livehime/danmaku_hime/inline_objects/flags_inline_object.h"

#include "bililive/bililive/livehime/gift_image/image_fetcher.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmakuhime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/paint.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/sculptor.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/text_layout.h"


FlagsInlineObject::FlagsInlineObject(const Flags& flags)
    : ref_count_(1),
      baseline_(0),
      line_height_(0),
      desired_height_(0),
      font_size_(14.f),
      opacity_(1.f)
{
    for (const auto& pair : flags) {
        FlagInfo info;
        info.df = pair.first;
        if (pair.first == DanmakuFlags::VIP) {
            info.raw_buf = std::make_shared<std::string>(
                ImageFetcher::GetInstance()->FetchAsset(ImageFetcher::MONTHLY_LORD));
        } else if (pair.first == DanmakuFlags::SVIP) {
            info.raw_buf = std::make_shared<std::string>(
                ImageFetcher::GetInstance()->FetchAsset(ImageFetcher::YEARLY_LORD));
        } else {
            auto layout = new TextLayout();
            layout->Init(pair.second, 0);
            layout->SetWrap(false);

            info.color = D2D1::ColorF(D2D1::ColorF::Yellow);
            info.layout.reset(layout);
        }

        if (info.raw_buf && !info.raw_buf->empty()) {
            info.wic_bmp = D2DManager::GetInstance()->DecodeImageFromData(*info.raw_buf.get());
        }
        user_flags_.push_back(std::move(info));
    }
}

void FlagsInlineObject::SetOpacity(float opacity) {
    opacity_ = opacity;
}

void FlagsInlineObject::SetFontSize(float size) {
    for (auto& f : user_flags_) {
        if (f.df != DanmakuFlags::VIP &&
            f.df != DanmakuFlags::SVIP)
        {
            f.layout->SetFontSize(size);
        }
    }
}

void FlagsInlineObject::SetMetrics(float desired_height, float line_height, float baseline) {
    baseline_ = baseline;
    line_height_ = line_height;
    desired_height_ = desired_height;
}

bool FlagsInlineObject::SetRenderTarget(
    base::win::ScopedComPtr<ID2D1RenderTarget> rt, bool hw_acc)
{
    DCHECK(rt != nullptr);
    rt_ = rt;
    for (auto& f : user_flags_) {
        if (f.df == DanmakuFlags::VIP || f.df == DanmakuFlags::SVIP) {
            if (!f.wic_bmp) {
                continue;
            }

            if (hw_acc != hw_acc_rendered_) {
                f.d2d_bmp.Release();
                hw_acc_rendered_ = hw_acc;
            }

            if (!f.d2d_bmp && rt_) {
                D2D1_BITMAP_PROPERTIES properties
                    = D2D1::BitmapProperties(
                        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

                HRESULT hr = rt->CreateBitmapFromWicBitmap(f.wic_bmp.get(), properties, f.d2d_bmp.Receive());
                if (FAILED(hr)) {
                    DCHECK(false);
                    LOG(WARNING) << "Failed to create bitmap from wic bitmap: " << hr;
                }
            }
        }
    }

    return true;
}

void FlagsInlineObject::SetFlagsColor(const FlagColorSetter& f) {
    for (auto& info : user_flags_) {
        if (info.df != DanmakuFlags::VIP &&
            info.df != DanmakuFlags::SVIP)
        {
            f(info.df, info.color);
        }
    }
}

void FlagsInlineObject::ReleaseBitmaps() {
    for (auto& f : user_flags_) {
        if (f.df == DanmakuFlags::VIP || f.df == DanmakuFlags::SVIP) {
            f.d2d_bmp.Release();
        }
    }
}

void FlagsInlineObject::DrawUserFlags(
    float left, float top, float height)
{
    if (!rt_) {
        return;
    }

    Sculptor sculptor(rt_);
    sculptor.GetPaint()->SetOpacity(opacity_);

    int flag_rect_offx = left;

    for (auto& f : user_flags_) {
        int flag_rect_width = 0;
        if (f.df == DanmakuFlags::VIP || f.df == DanmakuFlags::SVIP) {
            if (f.d2d_bmp) {
                auto size = f.d2d_bmp->GetSize();
                if (size.height > 0) {
                    flag_rect_width = float(desired_height_) / size.height * size.width;
                    float offy = (height - desired_height_) / 2.f + top;
                    sculptor.DrawBitmap(
                        f.d2d_bmp.get(),
                        flag_rect_offx, offy,
                        flag_rect_width, desired_height_);
                }
            }
        } else {
            int flag_text_width = f.layout->GetTextWidth();
            int flag_text_height = f.layout->GetTextHeight();
            flag_rect_width = flag_text_width + dmkhime::kUserFlagPaddingHori * 2;

            float flag_text_offx = dmkhime::kUserFlagPaddingHori + flag_rect_offx;
            float flag_text_offy = (height - flag_text_height) / 2.f + top;

            auto flag_rect = D2D1::RectF(
                flag_rect_offx, top,
                flag_rect_offx + flag_rect_width, top + height);

            // ±³¾°É«
            auto bg_color = f.color;
            bg_color.a *= 0.5f;
            sculptor.GetPaint()->SetColor(bg_color);
            sculptor.GetPaint()->SetStyle(Paint::FILL);
            sculptor.DrawRoundRect(flag_rect, 2);

            // ±ß¿òÉ«
            sculptor.GetPaint()->SetColor(f.color);
            sculptor.GetPaint()->SetStyle(Paint::STROKE);
            sculptor.DrawRoundRect(flag_rect, 2);

            sculptor.GetPaint()->SetStyle(Paint::FILL);
            sculptor.GetPaint()->SetColor(D2D1::ColorF(D2D1::ColorF::White));
            sculptor.DrawTextLayout(*f.layout, flag_text_offx, flag_text_offy);
        }
        flag_rect_offx += flag_rect_width + dmkhime::kUserFlagMarginHori;
    }
}

STDMETHODIMP FlagsInlineObject::Draw(
    void* clientDrawingContext,
    IDWriteTextRenderer* renderer,
    FLOAT originX,
    FLOAT originY,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect) {

    DrawUserFlags(originX, originY, desired_height_);
    return S_OK;
}

STDMETHODIMP FlagsInlineObject::GetMetrics(
    DWRITE_INLINE_OBJECT_METRICS* metrics) {

    if (metrics == nullptr) {
        return E_INVALIDARG;
    }

    int total_width = 0;
    for (auto& f : user_flags_) {
        int flag_rect_width = 0;
        if (f.df == DanmakuFlags::VIP || f.df == DanmakuFlags::SVIP) {
            UINT width, height;
            if (f.wic_bmp &&
                SUCCEEDED(f.wic_bmp->GetSize(&width, &height)) &&
                height > 0)
            {
                flag_rect_width = float(desired_height_) / height*width;
            }
        } else {
            int flag_text_width = f.layout->GetTextWidth();
            flag_rect_width = flag_text_width + dmkhime::kUserFlagPaddingHori * 2;
        }
        total_width += flag_rect_width + dmkhime::kUserFlagMarginHori;
    }

    metrics->width = total_width;
    metrics->height = desired_height_;
    metrics->baseline = baseline_ + (desired_height_ - line_height_) / 2;
    metrics->supportsSideways = FALSE;

    return S_OK;
}

STDMETHODIMP FlagsInlineObject::GetOverhangMetrics(
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

STDMETHODIMP FlagsInlineObject::GetBreakConditions(
    DWRITE_BREAK_CONDITION* breakConditionBefore,
    DWRITE_BREAK_CONDITION* breakConditionAfter) {

    if (breakConditionBefore == nullptr
        || breakConditionAfter == nullptr) {
        return E_INVALIDARG;
    }

    *breakConditionBefore = DWRITE_BREAK_CONDITION_NEUTRAL;
    *breakConditionAfter = DWRITE_BREAK_CONDITION_NEUTRAL;

    return S_OK;
}


STDMETHODIMP_(ULONG) FlagsInlineObject::AddRef() {
    return InterlockedIncrement(&ref_count_);
}

STDMETHODIMP_(ULONG) FlagsInlineObject::Release() {
    auto new_count = InterlockedDecrement(&ref_count_);
    if (new_count == 0) {
        delete this;
    }

    return new_count;
}

STDMETHODIMP FlagsInlineObject::QueryInterface(
    IID const& riid, void** ppvObject) {

    if (ppvObject == nullptr) {
        return E_POINTER;
    }

    if (__uuidof(FlagsInlineObject) == riid) {
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