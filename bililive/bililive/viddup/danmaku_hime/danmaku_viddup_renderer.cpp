#include "danmaku_viddup_renderer.h"

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

namespace
{
    // Special effect color
    const D2D1_COLOR_F title_color = D2D1::ColorF(0xffff00);
    const D2D1_COLOR_F combo_gift_color = D2D1::ColorF(0xffeb2b);
    const D2D1_COLOR_F common_text_color = D2D1::ColorF(0xffffff);

    /**
     * Open/renew Big sailing color
     */
     // -Background color
    const D2D1_COLOR_F clrOpenCaptainBg = D2D1::ColorF(0x6ea5ca, 0.9f);
    const D2D1_COLOR_F clrOpenAdmiralBg = D2D1::ColorF(0x7780fc, 0.9f);
    const D2D1_COLOR_F clrOpenGovernorBg = D2D1::ColorF(0xffb64a, 0.9f);


    // -High light color
    const D2D1_COLOR_F clrOpenCaptainHL = D2D1::ColorF(0xfdff2f);
    const D2D1_COLOR_F clrOpenAdmiralHL = D2D1::ColorF(0xfdff2f);
    const D2D1_COLOR_F clrOpenGovernorHL = D2D1::ColorF(0xa80d0d);

    /**
     * Grand Marine approach color
     */
     // -Background color
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

    // -High light color
    const D2D1_COLOR_F clrCaptainEnterHL = D2D1::ColorF(0xe6ff00);
    const D2D1_COLOR_F clrAdmiralEnterHL = D2D1::ColorF(0xfff100);
    const D2D1_COLOR_F clrGovernoEnterHL = D2D1::ColorF(0xa80d0d);

    // -Plain color
    const D2D1_COLOR_F clrGovernoEnterCommon = D2D1::ColorF(0x240e0e);

    /**
     * Eye-catching message color
     */
    const D2D1_COLOR_F clrSChatPrice = D2D1::ColorF(0x333333);
    const D2D1_COLOR_F clrSChatContent = D2D1::ColorF(0xffffff);

    // TextLayout height coefficient
    const float height_factor = 1.5f;

    // Maximum number of characters displayed in the bullet screen
    const int32 max_display = 500;

    base::win::ScopedComPtr<IWICBitmapSource> GetCircleImageFromBuffer(const std::string& buffer)
    {
        auto img_skia = bililive::MakeSkiaImage(
            reinterpret_cast<const unsigned char*>(buffer.data()), buffer.size());
        img_skia = bililive::CreateCircleImageEx(img_skia);
        if (img_skia.isNull())
        {
            return {};
        }

        auto& rep = img_skia.GetRepresentation(ui::GetScaleFactorFromScale(ui::win::GetDeviceScaleFactor()));
        if (rep.is_null())
        {
            return {};
        }

        auto bitmap = D2DManager::GetInstance()->CreateWICBitmap(
            rep.sk_bitmap().width(), rep.sk_bitmap().height(), rep.sk_bitmap().pixelRef()->pixels());
        if (!bitmap)
        {
            return {};
        }

        base::win::ScopedComPtr<IWICBitmapSource> source;
        HRESULT hr = bitmap.QueryInterface(source.Receive());
        if (FAILED(hr))
        {
            return {};
        }

        return source;
    }

    bool IsIntersect(int s1, int l1, int s2, int l2)
    {
        int s_max = std::max(s1, s2);
        int e_min = std::min(s1 + l1, s2 + l2);
        return s_max < e_min;
    }

    bool ParseHighlightText(string16* text, int* start, int* length)
    {
        auto idx = text->find(L"<%");
        if (idx != string16::npos)
        {
            *start = idx;
            ReplaceSubstringsAfterOffset(text, 0, L"<%", L"");

            idx = text->find(L"%>");
            if (idx != string16::npos)
            {
                *length = idx - *start;
                ReplaceSubstringsAfterOffset(text, 0, L"%>", L"");
                return true;
            }
        }
        else
        {
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
        for (;;)
        {
            i = text->find(L"<^icon^>", i);
            if (i == string16::npos)
            {
                break;
            }
            if (!has_block)
            {
                hit = true;
                break;
            }
            if (!IsIntersect(*block_start, *block_length, i, 8))
            {
                hit = true;
                break;
            }
            i += 8;
        }

        if (hit)
        {
            if (has_img)
            {
                text->replace(i, 8, L" ");
                if (int(i) <= *block_start)
                {
                    *block_start -= 7;
                }
                has_rank = true;
                *pos = int(i);
            }

            size_t j = i;
            for (;;)
            {
                j = text->find(L"<^icon^>", j);
                if (j == string16::npos)
                {
                    break;
                }
                if (IsIntersect(*block_start, *block_length, j, 8))
                {
                    j += 8;
                    continue;
                }

                text->replace(j, 8, L"");
                if (int(j) <= *block_start)
                {
                    *block_start -= 8;
                }
            }
        }

        return has_rank;
    }

    void DrawDebugLines(Sculptor& sculptor, const TextLayout& layout, float offx, float offy)
    {
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

namespace dmkhime
{
    // CommonViddupDmkRenderer
    CommonViddupDmkRenderer::CommonViddupDmkRenderer(const CommonDmkData& d)
        : data_(d),
        username_start_(0),
        bg_color_in_black_(D2D1::ColorF(0, 0)),
        bg_color_in_white_(D2D1::ColorF(0, 0))
    {
    }

    CommonViddupDmkRenderer::~CommonViddupDmkRenderer()
    {
        release_ = true;
 
    }

    void CommonViddupDmkRenderer::OnInit()
    {
        string16 text;

        int admin_img_start = 0;
        int top_img_start = 0;
        if (!data_.user_identity_tag_admin_.empty())
        {
            text.append(L" ");
            top_img_start = 1;
            if (!data_.user_identity_tag_top_.empty())
            {
                text.append(L" ");
                top_img_start = 2;
            }
        }

        if (!data_.user_identity_tag_top_.empty())
        {
            text.append(L" ");
        }

        if (!data_.user_identity_tag_admin_.empty() && !data_.user_identity_tag_top_.empty()
            || !(data_.user_identity_tag_admin_.empty() && data_.user_identity_tag_top_.empty()))
        {
            text.append(L" ");
        }

        ImageFetcher::ImageInfo title_info;
        int title_start = 0;
        bool title_result = false;

        username_start_ = text.length();

        int text_start = 0;
        if (!data_.user_name.empty())
        {
            text.append(data_.user_name);
            text.append(L": ");
            text_start = text.length();
        }

        text.append(data_.plain_text);

        if (text.length() > max_display)
        {
            text = text.substr(0, max_display);
        }

        plain_text_layout_.Init(text, 0);

        if (!data_.user_identity_tag_admin_.empty())
        {
            auto source2 = D2DManager::GetInstance()->DecodeImageFromData(data_.user_identity_tag_admin_, true);
            plain_text_layout_.SetInlineObject(
                source2, admin_img_start, 1, 1, true);
        }

        if (!data_.user_identity_tag_top_.empty())
        {
            auto source3 = D2DManager::GetInstance()->DecodeImageFromData(data_.user_identity_tag_top_, true);
            plain_text_layout_.SetInlineObject(
                source3, top_img_start, 1, 1, true);
        }

        {
            std::vector<std::string> colors;
            base::SplitString(data_.bg_color, ',', &colors);
            if (!colors.empty())
            {
                SkColor bg_color;
                if (bililive::ParseColorString(colors[0], &bg_color))
                {
                    has_bg_color_ = true;
                    bg_color_in_white_ = ConvertColorARGB(bg_color);
                    bg_color_in_black_ = bg_color_in_white_;
                }

                if (colors.size() == 3)
                {
                    if (bililive::ParseColorString(colors[2], &bg_color))
                    {
                        bg_color_in_black_ = ConvertColorARGB(bg_color);
                    }
                }
            }
        }
    }

    void CommonViddupDmkRenderer::OnRefactor(bool show_icon)
    {
        // Do nothing.
    }

    void CommonViddupDmkRenderer::OnMeasure(int parent_width, int base_font_size)
    {
        int max_width = width_ - ScaleI(kDanmakuMarginX * 2);
        if (has_bg_color_)
        {
            max_width -= ScaleI(kDanmakuBgPaddingX * 2);
        }

        plain_text_layout_.SetMaxWidth(max_width);
        plain_text_layout_.SetFontSize(ScaleI(base_font_size));

        if (!is_rendered_ || base_font_size != base_font_size_)
        {
            plain_text_layout_.UpdateInlineObjectMetrics();
        }

        int height = plain_text_layout_.GetTextHeight();

        height_ = height + ScaleI(kDanmakuMarginY * height_factor);

        if (has_bg_color_)
        {
            height_ += ScaleI(kDanmakuBgPaddingY * 2);
        }

        parent_width_ = parent_width;
        base_font_ = base_font_size;
    }

    void CommonViddupDmkRenderer::UpdateWicBitMap(base::win::ScopedComPtr<IWICBitmap> bitmap)
    {

        ResetGifPlay();

        cur_bitmap_ = bitmap.get();
        plain_text_layout_.SetInlineObject(cur_bitmap_, voice_start_, 1, 1);

        is_measured_ = false;
        is_rendered_ = false;
    }

    void CommonViddupDmkRenderer::GifStop()
    {

    }

    bool CommonViddupDmkRenderer::SetGifTick(int64_t interval)
    {
        if (!release_ &&
            plain_text_layout_.SetGiftIncreaseInterval(interval))
        {
            Invalid();
            return true;
        }
        return false;
    }

    void CommonViddupDmkRenderer::ResetGifPlay()
    {

    }

    void CommonViddupDmkRenderer::OnRender(Sculptor& sculptor, int bk_opacity, Theme theme, bool hw_acc)
    {
        bool need_measure = plain_text_layout_.UpdateRenderTarget(
            sculptor.GetRenderTarget(), hw_acc);
        if (need_measure)
        {
            OnMeasure(parent_width_, base_font_size_);
        }

        int margin_x = ScaleI(kDanmakuMarginX);
        int margin_y = ScaleI(kDanmakuMarginY);
        int bg_padding_x = ScaleI(kDanmakuBgPaddingX);
        int bg_padding_y = ScaleI(kDanmakuBgPaddingY);

        gfx::RectF rtClt(0, 0, width_, height_);
        rtClt.Inset(margin_x, margin_y, margin_x, margin_y);

        if (!data_.user_name.empty())
        {
            plain_text_layout_.ClearDrawingEffect();
            auto name_paint = sculptor.CreateNewPaint(viddup_dmk_system);
            plain_text_layout_.SetTextColor(*name_paint, username_start_, data_.user_name.length() + 1);
        }

        sculptor.GetPaint()->SetColor(viddup_dmk_user);
        sculptor.DrawTextLayout(plain_text_layout_, rtClt.x(), rtClt.y());
    }

    void CommonViddupDmkRenderer::OnInvalid()
    {
        plain_text_layout_.InvalidInlineObjects();
    }

    bool CommonViddupDmkRenderer::IsOpacitySensitive() const
    {
        return (data_.guard_type == BARRAGE_CAPTAIN ||
            data_.guard_type == BARRAGE_ADMIRAL ||
            data_.guard_type == BARRAGE_GOVERNOR);
    }

    bool CommonViddupDmkRenderer::HitTest(const gfx::Point& p)
    {
        data_.is_click_voice = false;
        int x = p.x() - ScaleI(kDanmakuMarginX);
        int y = p.y() - ScaleI(kDanmakuMarginY);
        if (data_.guard_type == BARRAGE_ADMIRAL || data_.guard_type == BARRAGE_GOVERNOR)
        {
            x -= ScaleI(kDanmakuBgPaddingX);
            y -= ScaleI(kDanmakuBgPaddingY);
        }

        uint32_t pos;
        if (!plain_text_layout_.HitTest(x, y, &pos))
        {
            return false;
        }

        return false;
    }

    // GiftComboViddupDmkRenderer
    GiftComboViddupDmkRenderer::GiftComboViddupDmkRenderer(const GiftComboDmkData& d)
        : data_(d)
    {
    }

    GiftComboViddupDmkRenderer::~GiftComboViddupDmkRenderer()
    {
        release_ = true;
    }

    bool GiftComboViddupDmkRenderer::SetGifTick(int64_t interval)
    {
        if (!release_ &&
            plain_text_layout_.SetGiftIncreaseInterval(interval))
        {
            Invalid();
            return true;
        }
        return false;
    }

    void GiftComboViddupDmkRenderer::OnInit()
    {
        gift_range_.Reset();
        org_gift_range_.Reset();

        string16 text;

        uname_range_.Set(text.length(), data_.user_name.length());
        text.append(data_.user_name).append(L" ").append(data_.action).append(L" ");

        gift_range_.start = text.length();
        text.append(data_.gift_name);

        if (is_icon_showing_ && !data_.gift_icon_data.empty())
        {
            int gift_str_begin = text.length() + 1;
            int gift_str_length = 1;

            text.append(L" * ");

            if (!data_.gift_combo_string.empty())
            {
                text.append(data_.gift_combo_string);
            }

            gift_range_.length = text.length() - gift_range_.start;

            if (text.length() > max_display)
            {
                text = text.substr(0, max_display);
            }

            plain_text_layout_.Init(text, 0);

            if (is_icon_showing_ && !data_.gift_icon_data.empty())
            {
                auto source = D2DManager::GetInstance()->DecodeGifFromData(data_.gift_icon_data, true);
                plain_text_layout_.SetInlineObject(
                    source, gift_str_begin, gift_str_length, 1.6f);
            }
        }
        else
        {
            if (!data_.gift_combo_string.empty())
            {
                text.append(L" ");
                text.append(data_.gift_combo_string);
            }

            gift_range_.length = text.length() - gift_range_.start;

            if (text.length() > max_display)
            {
                text = text.substr(0, max_display);
            }

            plain_text_layout_.Init(text, 0);
        }

        plain_text_layout_.SetBold(true);
    }

    void GiftComboViddupDmkRenderer::OnRefactor(bool show_icon)
    {
        OnInit();
    }

    void GiftComboViddupDmkRenderer::OnMeasure(int parent_width, int base_font_size)
    {
        int max_width = width_ - ScaleI(kDanmakuMarginX * 2);

        plain_text_layout_.SetMaxWidth(max_width);
        plain_text_layout_.SetFontSize(ScaleI(base_font_size));

        if (!is_rendered_ || base_font_size != base_font_size_)
        {
            plain_text_layout_.UpdateInlineObjectMetrics();
        }

        int height = plain_text_layout_.GetTextHeight();

        height_ = height + ScaleI(kDanmakuMarginY * height_factor);
    }

    void GiftComboViddupDmkRenderer::OnRender(Sculptor& sculptor, int bk_opacity, dmkhime::Theme theme, bool hw_acc)
    {
        bool need_measure = plain_text_layout_.UpdateRenderTarget(
            sculptor.GetRenderTarget(), hw_acc);
        if (need_measure)
        {
            OnMeasure(parent_width_, base_font_size_);
        }

        int text_margin_x = ScaleI(kDanmakuMarginX);
        int text_margin_y = ScaleI(kDanmakuMarginY);

        SkColor color_common = GetThemeColor(theme, ColorType::UI_LABEL);

        gfx::RectF rtClt(0, 0, width_, height_);
        rtClt.Inset(text_margin_x, text_margin_y, text_margin_x, text_margin_y);

        plain_text_layout_.ClearDrawingEffect();

        auto paint = sculptor.CreateNewPaint(viddup_dmk_system);
        plain_text_layout_.SetTextColor(*paint, uname_range_.start, uname_range_.length);

        if (gift_range_.Valid() || org_gift_range_.Valid())
        {
            auto paint2 = sculptor.CreateNewPaint({ 0 });
            paint2->SetColor(viddup_dmk_gift);

            if (gift_range_.Valid())
            {
                plain_text_layout_.SetTextColor(*paint2, gift_range_.start, gift_range_.length);
            }
            if (org_gift_range_.Valid())
            {
                plain_text_layout_.SetTextColor(*paint2, org_gift_range_.start, org_gift_range_.length);
            }
        }
        //sculptor.GetPaint()->SetColor(ConvertColorARGB(color_common));
        sculptor.DrawTextLayout(plain_text_layout_, rtClt.x(), rtClt.y());
    }

    void GiftComboViddupDmkRenderer::OnInvalid()
    {
        plain_text_layout_.InvalidInlineObjects();
    }

    // AnnocViddupDmkRenderer
    AnnocViddupDmkRenderer::AnnocViddupDmkRenderer(const AnnocDmkData& d)
        : data_(d)
    {
    }

    void AnnocViddupDmkRenderer::OnInit()
    {
        string16 text;

        userName = data_.user_name;
        if (!userName.empty())
        {
            text.append(userName);
            text.append(L" ");
            uname_length_ = text.length();
        }
        
        text.append(data_.plain_text);

        //real_text_ = data_.plain_text;
        real_text_ = text;

        if (data_.limit_max_display)
        {
            if (real_text_.length() > max_display)
            {
                real_text_ = real_text_.substr(0, max_display);
            }
        }

        ParseHighlightText(&real_text_, &uname_start_, &uname_length_);

        plain_text_layout_.Init(real_text_, 0);
    }

    void AnnocViddupDmkRenderer::OnMeasure(int parent_width, int base_font_size)
    {
        int max_width = width_ - ScaleI(kDanmakuMarginX * 2);

        plain_text_layout_.SetMaxWidth(max_width);
        plain_text_layout_.SetFontSize(ScaleI(base_font_size));

        int height = plain_text_layout_.GetTextHeight();

        height_ = height + ScaleI(kDanmakuMarginY * height_factor);
    }

    void AnnocViddupDmkRenderer::OnRender(Sculptor& sculptor, int bk_opacity, Theme theme, bool hw_acc)
    {
        bool need_measure = plain_text_layout_.UpdateRenderTarget(
            sculptor.GetRenderTarget(), hw_acc);
        if (need_measure)
        {
            OnMeasure(parent_width_, base_font_size_);
        }

        int text_margin_x = ScaleI(kDanmakuMarginX);
        int text_margin_y = ScaleI(kDanmakuMarginY);

        gfx::RectF rtClt(0, 0, width_, height_);
        rtClt.Inset(text_margin_x, text_margin_y, text_margin_x, text_margin_y);

        sculptor.GetPaint()->SetColor(viddup_dmk_system);
        sculptor.DrawTextLayout(plain_text_layout_, rtClt.x(), rtClt.y());       
    }

    void AnnocViddupDmkRenderer::DrawBackground(Sculptor& sculptor, float x, float y, float right, float bottom, const Paint* painter)
    {
        auto path = D2DManager::GetInstance()->CreatePathGeometry();
        base::win::ScopedComPtr<ID2D1GeometrySink> sink;
        HRESULT hr = path->Open(sink.Receive());
        if (SUCCEEDED(hr))
        {
            float height = bottom - y;
            DCHECK(height > 0.0f);

            sink->BeginFigure(
                D2D1::Point2F(x , y),
                D2D1_FIGURE_BEGIN_FILLED);

            sink->AddLine(D2D1::Point2F(x, bottom));                          

            sink->AddLine(D2D1::Point2F(right,bottom));

            sink->AddLine(D2D1::Point2F(right, y));                           

            sink->AddLine(D2D1::Point2F(x , y));

            sink->EndFigure(D2D1_FIGURE_END_CLOSED);

            hr = sink->Close();
            DCHECK(SUCCEEDED(hr));
        }

        sculptor.DrawGeometry(path.get(), painter);

    }

    // LiveAsistantViddupDmkRenderer
    LiveAsistantViddupDmkRenderer::LiveAsistantViddupDmkRenderer(const LiveAsistantDanmakuRenderData& d)
        : data_(d)
    {
    }

    LiveAsistantViddupDmkRenderer::~LiveAsistantViddupDmkRenderer() {}

    void LiveAsistantViddupDmkRenderer::OnInit()
    {
        content_color_ = viddup_dmk_system;
        content_bg_color_ = D2D1::ColorF(0xff4747, 0.1f);

        base::string16 format_str;

        if (!data_.live_asistant_data.warning_icon.empty())
        {
            format_str.append(L"  ");
            content_color_ = viddup_dmk_warning;
        }

        if (!data_.live_asistant_data.asit_title.empty())
        {
            format_str.append(data_.live_asistant_data.asit_title).append(L"£º");
            title_text_size_ = format_str.size();
        }

        format_str.append(data_.live_asistant_data.asit_msg);
        format_str.append(L" ");
        content_text_size_ = data_.live_asistant_data.asit_msg.size() + 1;

        plain_text_layout_.Init(format_str, 0);

        if (!data_.live_asistant_data.warning_icon.empty())
        {
            auto source = D2DManager::GetInstance()->DecodeImageFromData(data_.live_asistant_data.warning_icon, true);
            plain_text_layout_.SetInlineObject(
                source, 0, 1, 0.8f, true);
        }
    }

    void LiveAsistantViddupDmkRenderer::OnMeasure(int parent_width, int base_font_size)
    {
        int max_width = width_ - ScaleI(kDanmakuMarginX * 2);
        max_width -= ScaleI(kDanmakuBgPaddingX * 2);
        plain_text_layout_.SetMaxWidth(max_width);
        plain_text_layout_.SetFontSize(ScaleI(base_font_size));

        if (!is_rendered_ || base_font_size != base_font_size_)
        {
            plain_text_layout_.UpdateInlineObjectMetrics();
        }

        int height = plain_text_layout_.GetTextHeight();
        height_ = height + ScaleI(kDanmakuMarginY * 2);
        height_ += ScaleI(kDanmakuBgPaddingY * 2);
        parent_width_ = parent_width;
    }

    void LiveAsistantViddupDmkRenderer::OnRender(Sculptor& sculptor, int bk_opacity, Theme theme, bool hw_acc)
    {
        plain_text_layout_.ClearDrawingEffect();

        bool need_measure = plain_text_layout_.UpdateRenderTarget(
            sculptor.GetRenderTarget(), hw_acc);
        if (need_measure)
        {
            OnMeasure(parent_width_, base_font_size_);
        }

        int margin_x = ScaleI(kDanmakuMarginX);
        int margin_y = ScaleI(kDanmakuMarginY);
        int bg_padding_x = ScaleI(kDanmakuBgPaddingX);
        int bg_padding_y = ScaleI(kDanmakuBgPaddingY);
        int bg_round_rect_radius = ScaleI(kliveAsistantBackgroundRadius);
        int btn_margin = ScaleI(kCommonNoticeDanmakuBtnMargin);

        gfx::RectF rtClt(0, 0, width_, height_);
        rtClt.Inset(margin_x, margin_y, margin_x, margin_y);

        //auto alpha = 0.00392157f * bk_opacity;
        auto rect = D2D1::RectF(rtClt.x(), rtClt.y(), rtClt.right(), rtClt.bottom());
        rtClt.Inset(bg_padding_x, bg_padding_y, bg_padding_x, bg_padding_y);

        sculptor.GetPaint()->SetGradientEnabled(false);
        sculptor.GetPaint()->SetColor(content_bg_color_);
        sculptor.DrawRoundRect(rect, bg_round_rect_radius);
        sculptor.GetPaint()->SetGradientEnabled(false);
        sculptor.GetPaint()->SetColor(content_color_);
        sculptor.DrawTextLayout(plain_text_layout_, rtClt.x(), rtClt.y());
    }

    void LiveAsistantViddupDmkRenderer::OnInvalid()
    {
        plain_text_layout_.InvalidInlineObjects();
    }

    bool LiveAsistantViddupDmkRenderer::HitTest(const gfx::Point& p)
    {
        if (!has_button_)
            return false;

        int margin_x = ScaleI(kDanmakuMarginX + kDanmakuBgPaddingX);
        int margin_y = ScaleI(kDanmakuMarginY + kDanmakuBgPaddingY);

        uint32_t pos{};
        if (plain_text_layout_.HitTest(p.x() - margin_x/* - 10*/, p.y() - margin_y, &pos))
        {
            return pos == btn_text_start_pos_;
        }
        else
        {
            return false;
        }
    }
}