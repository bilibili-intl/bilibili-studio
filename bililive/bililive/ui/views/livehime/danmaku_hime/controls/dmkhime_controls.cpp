#include "dmkhime_controls.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/bililive/utils/bililive_image_util.h"

#include "grit/theme_resources.h"


namespace {

    string16 FriendlyRank(int64_t rank) {
        if (rank > 9999) {
            return L"9999+";
        }
        return std::to_wstring(rank);
    }

}

namespace dmkhime {

    // ImageOrTextView
    HighEnergyRankView::HighEnergyRankView() {
        img_view_ = new LivehimeImageView();
        img_view_->SetPreferredSize(GetSizeByDPIScale(gfx::Size(24, 16)));

        label_ = new LivehimeContentLabel();
        label_->SetFont(ftTwelve);

        AddChildView(img_view_);
        AddChildView(label_);
    }

    void HighEnergyRankView::SetRank(int64_t rank) {
        if(rank >= 1 && rank <= 3) {
            if (rank == 1) {
                img_view_->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_HE_RANK_1));
            } else if (rank == 2) {
                img_view_->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_HE_RANK_2));
            } else {
                img_view_->SetImage(*GetImageSkiaNamed(IDR_LIVEHIME_HE_RANK_3));
            }
            label_->SetText({});
        } else if (rank >= 0) {
            img_view_->SetImage({});
            label_->SetText(FriendlyRank(rank));
        } else if (rank == -1) {
            img_view_->SetImage({});
            label_->SetText(L"¡ª");
        } else if (rank == -2) {
            img_view_->SetImage({});
            label_->SetText(L"³¤Ê±");
        }
    }

    gfx::Size HighEnergyRankView::GetPreferredSize() {
        auto size = label_->GetPreferredSize();
        size.set_width(label_->font().GetStringWidth(L"9999+"));
        size.SetToMax(img_view_->GetPreferredSize());
        return size;
    }

    int HighEnergyRankView::GetHeightForWidth(int w) {
        auto height = img_view_->GetPreferredSize().height();
        return std::max(height, label_->GetHeightForWidth(w));
    }

    void HighEnergyRankView::SetNoSubpixelRender(bool force)
    {
        label_->set_no_subpixel_rendering(force);
    }

    void HighEnergyRankView::Layout() {
        auto bounds = GetContentsBounds();

        auto size = img_view_->GetPreferredSize();
        img_view_->SetBounds(
            bounds.x() + (bounds.width() - size.width()) / 2,
            bounds.y() + (bounds.height() - size.height()) / 2,
            size.width(), size.height());

        size = label_->GetPreferredSize();
        size.set_width(label_->font().GetStringWidth(L"9999+"));
        label_->SetBounds(
            bounds.x() + (bounds.width() - size.width()) / 2,
            bounds.y() + (bounds.height() - size.height()) / 2,
            size.width(), size.height());
    }


    // HighEnergyVerticalScrollBar
    HighEnergyVerticalScrollBar::HighEnergyVerticalScrollBar(ScrollViewWithFloatingScrollbar* bar)
        : FloatingScrollBar(false,true)
    {
        SetScrollView(bar);
    }

    int HighEnergyVerticalScrollBar::GetScrollBarWidth() const {
        static int bar_width = GetLengthByDPIScale(7);
        return bar_width;
    }

    int HighEnergyVerticalScrollBar::GetScrollBarHeight() const {
        return 0;
    }

    int HighEnergyVerticalScrollBar::GetScrollBarVisibleWidth() const {
        return GetScrollBarWidth();
    }

    int HighEnergyVerticalScrollBar::GetScrollBarVisibleHeight() const {
        return 0;
    }

    int HighEnergyVerticalScrollBar::GetScrollBarBorderThickness() const {
        return 0;
    }

    void HighEnergyVerticalScrollBar::OnPaint(gfx::Canvas* canvas) {
        if (visible()) {
            gfx::Rect thumb_valid_rect = scroll_view_->ConvertRectToParent(thumb_->thumb_bounds());

            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setColor(SkColorSetA(0x8696a3, 255 * 0.8f));
            canvas->DrawRoundRect(thumb_valid_rect, thumb_valid_rect.width() / 2.f, paint);
        }
    }


}