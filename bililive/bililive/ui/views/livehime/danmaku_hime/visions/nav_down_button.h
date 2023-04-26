#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISION_NAV_DOWN_BUTTON_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISION_NAV_DOWN_BUTTON_H_

#include "ui/base/animation/slide_animation.h"

#include "bililive/bililive/ui/views/livehime/danmaku_hime/text_layout.h"


class Sculptor;

namespace dmkhime {

    class NavDownButton {
    public:
        NavDownButton();

        void Show();
        void Hide();

        void SetText(const string16& text);

        bool Hit(const gfx::Point& p) const;
        bool IsAnimating() const;

        void OnDrawOver(Sculptor* s, const gfx::Rect& bounds);

    private:
        TextLayout text_tl_;
        gfx::Rect nav_down_btn_rect_;
        ui::SlideAnimation ind_anime_;
    };

}

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISION_NAV_DOWN_BUTTON_H_