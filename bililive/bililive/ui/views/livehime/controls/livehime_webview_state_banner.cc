#include "livehime_webview_state_banner.h"

#include "base/ext/callable_callback.h"

#include "ui/base/resource/resource_bundle.h"

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

    static std::map<livehime::WebViewState, int> img_map{
        { livehime::WebViewState::StartLoad, IDR_LIVEHIME_V3_LUCKYGIFT_STATUS_LOADING },
        { livehime::WebViewState::LoadFaild, IDR_LIVEHIME_V3_LUCKYGIFT_STATUS_LOAD_FAILED },
    };

    gfx::Size GetMaxStateImageSize()
    {
        static gfx::Size size;
        static bool had_test = false;
        if (!had_test)
        {
            had_test = true;
            for (auto& iter : img_map)
            {
                gfx::ImageSkia *img = GetImageSkiaNamed(iter.second);
                if (img)
                {
                    size.SetToMax(img->size());
                }
            }
        }
        return size;
    }
}

namespace livehime
{
    void DrawWebViewStateImage(gfx::Canvas *canvas, const gfx::Rect &bounds,
                               WebViewState state, const base::string16 &text)
    {
        gfx::ImageSkia *img = ResourceBundle::GetSharedInstance().GetImageSkiaNamed(img_map[state]);
        if (img)
        {
            int total_h = img->height() + kPaddingRowHeightForDiffGroups + ftPrimary.GetHeight();
            int y = bounds.y() + (bounds.height() - total_h) / 2;
            canvas->DrawImageInt(*img,
                                 bounds.x() + (bounds.width() - img->width()) / 2,
                                 y);
            y += img->height() + kPaddingRowHeightForDiffGroups;
            canvas->DrawStringInt(text, ftPrimary, clrLabelDescribe,
                                  bounds.x(), y, bounds.width(), ftPrimary.GetHeight(),
                                  gfx::Canvas::TEXT_ALIGN_CENTER);
        }
    }
}


LivehimeWebViewStateBannerView::LivehimeWebViewStateBannerView()
    : state_(livehime::WebViewState::StartLoad)
    , state_text_width_(0)
{
}

LivehimeWebViewStateBannerView::~LivehimeWebViewStateBannerView()
{
}

void LivehimeWebViewStateBannerView::SetState(livehime::WebViewState state, bool repaint/* = true*/)
{
    state_ = state;
    CalculateCurrentStateStringWidth();
    InvalidateLayout();
    if (repaint)
    {
        SchedulePaint();
    }
}

void LivehimeWebViewStateBannerView::SetStateText(livehime::WebViewState state, const base::string16 &text,
                                                  bool repaint/* = false*/)
{
    state_texts_[state] = text;
    CalculateCurrentStateStringWidth();
    InvalidateLayout();
    if (repaint)
    {
        SchedulePaint();
    }
}

gfx::Size LivehimeWebViewStateBannerView::GetPreferredSize()
{
    gfx::Size size = kStateSize;
    size.SetToMax(GetMaxStateImageSize());
    size.Enlarge(0, kPaddingRowHeightForDiffGroups + ftPrimary.GetHeight());
    size.set_width(std::max(size.width(), state_text_width_));
    return size;
}

void LivehimeWebViewStateBannerView::OnPaintBackground(gfx::Canvas* canvas)
{
    livehime::DrawWebViewStateImage(canvas, GetContentsBounds(), state_, state_texts_[state_]);
}

void LivehimeWebViewStateBannerView::CalculateCurrentStateStringWidth()
{
    if (state_texts_.find(state_) != state_texts_.end())
    {
        state_text_width_ = ftPrimary.GetStringWidth(state_texts_[state_]);
    }
    else
    {
        state_text_width_ = 0;
    }
}
