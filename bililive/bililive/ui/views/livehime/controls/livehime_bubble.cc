#include "livehime_bubble.h"

namespace livehime
{
    BililiveBubbleView* ShowBubble(views::View* anchor, views::BubbleBorder::Arrow arrow,
        const base::string16 &tooltip, bool use_focusless, const gfx::Font& font/* = ftPrimary*/)
    {
        BililiveBubbleView *bubble = new BililiveBubbleView(anchor, arrow, tooltip);
        bubble->set_use_focusless(use_focusless);
        bubble->set_text_color(clrTextPrimary);
        bubble->SetFont(font);

        views::Widget* widget = views::BubbleDelegateView::CreateBubble(bubble);
        widget->Show();

        return bubble;
    }

    BililiveBubbleView* ShowBubble(views::View* anchor, views::BubbleBorder::Arrow arrow,
        views::View* content_view)
    {
        BililiveBubbleView *bubble = new BililiveBubbleView(anchor, arrow, content_view);
        bubble->set_close_on_deactivate(false);

        views::Widget* widget = views::BubbleDelegateView::CreateBubble(bubble);
        widget->Show();

        return bubble;
    }
}
