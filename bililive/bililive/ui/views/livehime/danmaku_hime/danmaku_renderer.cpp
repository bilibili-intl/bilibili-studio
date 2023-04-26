#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_renderer.h"

#include <regex>
#include <iomanip>

#include "base/memory/ref_counted_memory.h"
#include "base/notification/notification_service.h"
#include "base/notification/notification_details.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "bilibase/basic_types.h"

#include "ui/base/win/dpi.h"
#include "ui/base/range/range.h"

#include "bililive/bililive/ui/views/livehime/danmaku_hime/paint.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/renderer/danmaku_renderer_utils.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/inline_objects/spread_inline_object.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/bililive/utils/convert_util.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "SkColor.h"


namespace {

    // 特效颜色
    const D2D1_COLOR_F title_color = D2D1::ColorF(0xffff00);
    const D2D1_COLOR_F combo_gift_color = D2D1::ColorF(0xffeb2b);
    const D2D1_COLOR_F common_text_color = D2D1::ColorF(0xffffff);

    /**
     * 开通/续费大航海色
     */
    // -背景色
    const D2D1_COLOR_F clrOpenCaptainBg = D2D1::ColorF(0x6ea5ca, 0.9f);
    const D2D1_COLOR_F clrOpenAdmiralBg = D2D1::ColorF(0x7780fc, 0.9f);
    const D2D1_COLOR_F clrOpenGovernorBg = D2D1::ColorF(0xffb64a, 0.9f);

    // -高亮色
    const D2D1_COLOR_F clrOpenCaptainHL = D2D1::ColorF(0xfdff2f);
    const D2D1_COLOR_F clrOpenAdmiralHL = D2D1::ColorF(0xfdff2f);
    const D2D1_COLOR_F clrOpenGovernorHL = D2D1::ColorF(0xa80d0d);

    /**
     * 大航海进场色
     */
     // -背景色
    const D2D1_COLOR_F clrCaptainEnterStart = D2D1::ColorF(0x2895c6, 0.95f);
    const D2D1_COLOR_F clrCaptainEnterEnd = D2D1::ColorF(0x92b5e0, 0.95f);

    const D2D1_COLOR_F clrAdmiralEnterStart = D2D1::ColorF(0x724291, 0.95f);
    const D2D1_COLOR_F clrAdmiralEnterMid = D2D1::ColorF(0xa17eff, 0.95f);
    const D2D1_COLOR_F clrAdmiralEnterEnd = D2D1::ColorF(0xbd5fff, 0.95f);

    const D2D1_COLOR_F clrHEREnterStart = D2D1::ColorF(0xD2C4FF, 0.95f);
    const D2D1_COLOR_F clrHEREnterMid = D2D1::ColorF(0xA688FF, 0.95f);
    const D2D1_COLOR_F clrHEREnterEnd = D2D1::ColorF(0xD3C5FF, 0.95f);

    const D2D1_COLOR_F clrGovernorEnterStart = D2D1::ColorF(0xffa247, 0.95f);
    const D2D1_COLOR_F clrGovernorEnterEnd = D2D1::ColorF(0xfff4a1, 0.95f);

    // -高亮色
    const D2D1_COLOR_F clrCaptainEnterHL = D2D1::ColorF(0xe6ff00);
    const D2D1_COLOR_F clrAdmiralEnterHL = D2D1::ColorF(0xfff100);
    const D2D1_COLOR_F clrGovernoEnterHL = D2D1::ColorF(0xa80d0d);

    // -普通色
    const D2D1_COLOR_F clrGovernoEnterCommon = D2D1::ColorF(0x240e0e);

    /**
     * 醒目留言色
     */
    const D2D1_COLOR_F clrSChatPrice = D2D1::ColorF(0x333333);
    const D2D1_COLOR_F clrSChatContent = D2D1::ColorF(0xffffff);


    base::win::ScopedComPtr<IWICBitmapSource> GetCircleImageFromBuffer(const std::string& buffer) {
        auto img_skia = bililive::MakeSkiaImage(
            reinterpret_cast<const unsigned char*>(buffer.data()), buffer.size());
        img_skia = bililive::CreateCircleImageEx(img_skia);
        if (img_skia.isNull()) {
            return {};
        }

        auto& rep = img_skia.GetRepresentation(ui::GetScaleFactorFromScale(ui::win::GetDeviceScaleFactor()));
        if (rep.is_null()) {
            return {};
        }

        auto bitmap = D2DManager::GetInstance()->CreateWICBitmap(
            rep.sk_bitmap().width(), rep.sk_bitmap().height(), rep.sk_bitmap().pixelRef()->pixels());
        if (!bitmap) {
            return {};
        }

        base::win::ScopedComPtr<IWICBitmapSource> source;
        HRESULT hr = bitmap.QueryInterface(source.Receive());
        if (FAILED(hr)) {
            return {};
        }
        return source;
    }
    bool IsIntersect(int s1, int l1, int s2, int l2) {
        int s_max = std::max(s1, s2);
        int e_min = std::min(s1 + l1, s2 + l2);
        return s_max < e_min;
    }

    bool ParseHighlightText(string16* text, int* start, int* length) {
        auto idx = text->find(L"<%");
        if (idx != string16::npos) {
            *start = idx;
            ReplaceSubstringsAfterOffset(text, 0, L"<%", L"");

            idx = text->find(L"%>");
            if (idx != string16::npos) {
                *length = idx - *start;
                ReplaceSubstringsAfterOffset(text, 0, L"%>", L"");
                return true;
            }
        } else {
            ReplaceSubstringsAfterOffset(text, 0, L"%>", L"");
        }
        return false;
    }

    bool ParseIconTokens(
        string16* text, bool has_img,
        bool has_block, int* block_start, int* block_length, int* pos)
    {
        size_t i = 0;
        bool hit = false;
        bool has_rank = false;
        for (;;) {
            i = text->find(L"<^icon^>", i);
            if (i == string16::npos) {
                break;
            }
            if (!has_block) {
                hit = true;
                break;
            }
            if (!IsIntersect(*block_start, *block_length, i, 8)) {
                hit = true;
                break;
            }
            i += 8;
        }

        if (hit) {
            if (has_img) {
                text->replace(i, 8, L" ");
                if (int(i) <= *block_start) {
                    *block_start -= 7;
                }
                has_rank = true;
                *pos = int(i);
            }

            size_t j = i;
            for (;;) {
                j = text->find(L"<^icon^>", j);
                if (j == string16::npos) {
                    break;
                }
                if (IsIntersect(*block_start, *block_length, j, 8)) {
                    j += 8;
                    continue;
                }

                text->replace(j, 8, L"");
                if (int(j) <= *block_start) {
                    *block_start -= 8;
                }
            }
        }
        return has_rank;
    }

    void DrawDebugLines(Sculptor& sculptor, const TextLayout& layout, float offx, float offy) {
        sculptor.GetPaint()->SetColor(dmkhime::ConvertColor(SK_ColorYELLOW, 0.5f));
        sculptor.DrawLine(
            D2D1::Point2F(offx, offy),
            D2D1::Point2F(offx + 50, offy));

        float height = 0;
        float baseline = 0;
        layout.GetLineHeight(&height, &baseline);

        sculptor.GetPaint()->SetColor(dmkhime::ConvertColor(SK_ColorWHITE, 0.5f));
        sculptor.DrawLine(
            D2D1::Point2F(offx, offy + baseline),
            D2D1::Point2F(offx + 50, offy + baseline));
        sculptor.GetPaint()->SetColor(dmkhime::ConvertColor(SK_ColorGREEN, 0.5f));
        sculptor.DrawLine(
            D2D1::Point2F(offx, offy + height),
            D2D1::Point2F(offx + 50, offy + height));
    }

}


namespace dmkhime {

// DanmakuRenderer
DanmakuRenderer::DanmakuRenderer()
    : width_(0),
      height_(0),
      is_rendered_(false),
      rendered_opacity_(0),
      rendered_theme_(dmkhime::Theme::SORCERESS),
      is_hw_acc_rendered_(false),
      is_measured_(false),
      is_icon_showing_(true),
      base_font_size_(14) {
}

void DanmakuRenderer::Init(int width, bool show_icon) {
    width_ = width;
    parent_width_ = width;
    is_icon_showing_ = show_icon;

    sx_ = D2DManager::GetInstance()->GetDpiX() / 96.f;

    OnInit();
}

void DanmakuRenderer::Show() {
    OnShow();
}

void DanmakuRenderer::Refactor(bool show_icon) {
    if (is_icon_showing_ == show_icon && !force_refactor_) {
        return;
    }
    is_icon_showing_ = show_icon;

    OnRefactor(show_icon);

    is_measured_ = false;
    is_rendered_ = false;
    force_refactor_ = false;
}

void DanmakuRenderer::Measure(int parent_width) {
    Measure(parent_width, base_font_size_);
}

void DanmakuRenderer::Measure(int parent_width, int base_font_size) {
    if (is_measured_) {
        if ((IsAutoWidth() || width_ == parent_width) &&
            (!IsFontSizeSensitive() || base_font_size_ == base_font_size))
        {
            return;
        }
    }
    width_ = parent_width;
    parent_width_ = parent_width;

    OnMeasure(parent_width_, base_font_size);

    is_measured_ = true;
    is_rendered_ = false;
    base_font_size_ = base_font_size;
}

void DanmakuRenderer::Render(int bk_opacity, dmkhime::Theme theme, bool hw_acc) {
    if (is_rendered_) {
        if ((!IsThemeSensitive() || theme == rendered_theme_) &&
            is_hw_acc_rendered_ == hw_acc &&
            (!IsOpacitySensitive() || bk_opacity == rendered_opacity_))
        {
            return;
        }
    }

    Sculptor sculptor(width_, height_, false, hw_acc);
    if (sculptor.IsFailed()) {
        is_rendered_ = false;
        rendered_theme_ = theme;
        rendered_opacity_ = bk_opacity;
        is_hw_acc_rendered_ = hw_acc;
        return;
    }

    sculptor.Begin();

    OnRender(sculptor, bk_opacity, theme, hw_acc);

    if (sculptor.End()) {
        d2d_bmp_ = sculptor.ExtractBitmap();

        is_rendered_ = true;
        rendered_theme_ = theme;
        rendered_opacity_ = bk_opacity;
        is_hw_acc_rendered_ = hw_acc;
    } else {
        d2d_bmp_.Release();
        is_rendered_ = false;
        rendered_theme_ = theme;
        rendered_opacity_ = bk_opacity;
        is_hw_acc_rendered_ = hw_acc;
    }
}

void DanmakuRenderer::Invalid() {
    is_measured_ = false;
    is_rendered_ = false;
    d2d_bmp_.Release();

    OnInvalid();
}

base::win::ScopedComPtr<ID2D1Bitmap> DanmakuRenderer::GetBitmap() const {
    return d2d_bmp_;
}

}