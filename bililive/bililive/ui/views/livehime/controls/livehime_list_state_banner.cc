#include "livehime_list_state_banner.h"

#include "base/ext/callable_callback.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/color_utils.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/public/danmaku_hime_service.h"

#include "grit/theme_resources.h"

namespace
{
    const gfx::Size kStateSize(GetLengthByDPIScale(200), GetLengthByDPIScale(120));

    static std::map<livehime::ListState, int> kStateImageMap =
    {
        { livehime::ListState::ListState_Loading, IDR_LIVEHIME_V3_LUCKYGIFT_STATUS_LOADING },
        { livehime::ListState::ListState_Faild, IDR_LIVEHIME_V3_LUCKYGIFT_STATUS_LOAD_FAILED },
        { livehime::ListState::ListState_Empty, IDR_LIVEHIME_V3_LUCKYGIFT_STATUS_NULL_RECORD },
        { livehime::ListState::ListState_Success, IDR_LIVEHIME_V3_LUCKYGIFT_STATUS_LOADING }
    };

    static std::map<livehime::ListState, int> kStateTextMap =
    {
        { livehime::ListState::ListState_Loading, IDS_LUCKYGIFT_INFO_STATUS_LOADING },
        { livehime::ListState::ListState_Faild, IDS_LUCKYGIFT_INFO_STATUS_LOADING_FAILED },
        { livehime::ListState::ListState_Empty, IDS_LUCKYGIFT_INFO_STATUS_NO_RECORD },
        { livehime::ListState::ListState_Success, IDS_LUCKYGIFT_INFO_STATUS_LOADING }
    };

    void DrawStateDetails(
        gfx::Canvas *canvas, const gfx::Rect &bounds,
        gfx::ImageSkia* image, const base::string16 &text, const gfx::Font& text_font,SkColor clr, bool no_subpixel = false,
        int offset_x = 0)
    {
        int text_cx = bounds.width();
        int text_cy = 0;
        gfx::Canvas::SizeStringInt(text, ftPrimary, &text_cx, &text_cy, 0,
            gfx::Canvas::TEXT_ALIGN_CENTER | gfx::Canvas::MULTI_LINE | gfx::Canvas::CHARACTER_BREAK |
            (no_subpixel ? gfx::Canvas::NO_SUBPIXEL_RENDERING : 0));

        int total_h = kPaddingRowHeightForCtrlTips + text_cy;
        if (image)
        {
            total_h += image->height();
        }
        int y = bounds.y() + (bounds.height() - total_h) / 2;
        if (image)
        {
            canvas->DrawImageInt(*image,
                bounds.x() + (bounds.width() - image->width()) / 2 - offset_x,
                y);
        }
        if (image)
        {
            y += image->height() + kPaddingRowHeightForCtrlTips;
        }
        canvas->DrawStringInt(text, text_font, clr,
            bounds.x() + (bounds.width() - text_cx) / 2 - offset_x, y, text_cx, text_cy,
            gfx::Canvas::TEXT_ALIGN_CENTER | gfx::Canvas::MULTI_LINE | gfx::Canvas::CHARACTER_BREAK |
            (no_subpixel ? gfx::Canvas::NO_SUBPIXEL_RENDERING : 0));
    }
}

namespace livehime
{
    void DrawListStateImage(gfx::Canvas *canvas, const gfx::Rect &bounds,
        ListState state, const base::string16 &text, SkColor clr/* = clrLabelDescribe*/, int offset_x)
    {
        int resource_id = kStateImageMap[state];
        if (-1 != resource_id)
        {
            gfx::ImageSkia *img = ResourceBundle::GetSharedInstance().GetImageSkiaNamed(resource_id);
            DrawStateDetails(canvas, bounds, img, text, ftPrimary,clr, offset_x);
        }
    }
}


LivehimeListStateBannerView::LivehimeListStateBannerView(int offset_x)
    : state_(livehime::ListState::ListState_Loading)
    , offset_x_(offset_x)
{
}

LivehimeListStateBannerView::~LivehimeListStateBannerView()
{
}

void LivehimeListStateBannerView::SetState(livehime::ListState state, bool repaint/* = true*/)
{
    if (state != state_)
    {
        state_ = state;
        InvalidateLayout();
        if (repaint)
        {
            SchedulePaint();
        }
    }
}

void LivehimeListStateBannerView::SetStateText(livehime::ListState state, const base::string16 &text)
{
    state_details_[state].text = text;
    InvalidateLayout();
}

void LivehimeListStateBannerView::SetStateImage(livehime::ListState state, const gfx::ImageSkia* image)
{
    state_details_[state].img = *image;
    InvalidateLayout();
}

void LivehimeListStateBannerView::SetNoSubpixelRender(bool no_subpixel) {
    no_subpixel_render_ = no_subpixel;
}

void LivehimeListStateBannerView::SetStateTextColor(livehime::ListState state, SkColor clr)
{
    state_details_[state].clr = clr;
}

void LivehimeListStateBannerView::SetTextColor(SkColor clr)
{
    for (int state = (int)livehime::ListState::ListState_Loading;
        state < (int)livehime::ListState::ListState_Success;
        ++state)
    {
        SetStateTextColor((livehime::ListState)state, clr);
    }
}

void LivehimeListStateBannerView::SetAutoColorReadabilityEnabled(bool enabled, SkColor background_color)
{
    auto_color_readability_ = enabled;
    background_color_ = background_color;
}

gfx::Size LivehimeListStateBannerView::GetPreferredSize()
{
    int img_cx = kStateSize.width();
    gfx::ImageSkia *image = state_details_[state_].img.isNull() ?
        GetImageSkiaNamed(kStateImageMap[state_]) : &state_details_[state_].img;
    if (image)
    {
        img_cx = std::max(img_cx, image->width());
    }

    gfx::Size size;
    size.set_width(img_cx * 2);
    size.set_height(GetHeightForWidth(size.width()));
    return size;
}

int LivehimeListStateBannerView::GetHeightForWidth(int w)
{
    int img_cy = kStateSize.height();
    gfx::ImageSkia *image = state_details_[state_].img.isNull() ?
        GetImageSkiaNamed(kStateImageMap[state_]) : &state_details_[state_].img;
    if (image)
    {
        img_cy = std::max(img_cy, image->height());
    }

    int text_cx = w;
    int text_cy = 0;
    gfx::Canvas::SizeStringInt(state_details_[state_].text, ftPrimary, &text_cx, &text_cy, 0,
        gfx::Canvas::TEXT_ALIGN_CENTER | gfx::Canvas::MULTI_LINE | gfx::Canvas::CHARACTER_BREAK);

    return img_cy + kPaddingRowHeightForCtrlTips + text_cy;
}

void LivehimeListStateBannerView::OnPaintBackground(gfx::Canvas* canvas)
{
    SkColor text_clr = auto_color_readability_ ?
        color_utils::GetReadableColor(state_details_[state_].clr, background_color_) :
        state_details_[state_].clr;
    base::string16 text = state_details_[state_].text.empty() ?
         GetLocalizedString(kStateTextMap[state_]) : state_details_[state_].text;
    gfx::ImageSkia* image = state_details_[state_].img.isNull() ? GetImageSkiaNamed(kStateImageMap[state_]) : &state_details_[state_].img;
    if (state_ == livehime::ListState::ListState_Empty && no_data_empty_image_)
    {
        image = nullptr;
    }
    DrawStateDetails(canvas, GetContentsBounds(), image, text, text_font_, text_clr, no_subpixel_render_ ,offset_x_);
}

bool LivehimeListStateBannerView::HitTestRect(const gfx::Rect& rect) const
{
    return false;
}
