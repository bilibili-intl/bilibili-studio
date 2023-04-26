#ifndef LIVEHIME_BUBBLE_H_
#define LIVEHIME_BUBBLE_H_

#include "bililive/bililive/ui/views/controls/bililive_bubble.h"
#include "livehime_theme_common.h"

namespace livehime
{
    BililiveBubbleView* ShowBubble(views::View* anchor, views::BubbleBorder::Arrow arrow,
        const base::string16 &tooltip, bool use_focusless = false, const gfx::Font& font = ftPrimary);

    BililiveBubbleView* ShowBubble(views::View* anchor, views::BubbleBorder::Arrow arrow,
        views::View* content_view);
}

#endif  // LIVEHIME_BUBBLE_H_
