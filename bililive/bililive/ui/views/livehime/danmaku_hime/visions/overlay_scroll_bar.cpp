#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/overlay_scroll_bar.h"

#include "ui/base/win/dpi.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/sculptor.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/paint.h"


namespace {

    const int kScrollBarWidth = GetLengthByDPIScale(7);
    const int kScrollBarMinHeight = GetLengthByDPIScale(20);

    const D2D1_COLOR_F clrScrollBar = D2D1::ColorF(0x8696a3, 0.8f);

}

namespace dmkhime {

OverlayScrollBar::OverlayScrollBar()
    : content_height_(0),
      anim_(nullptr)
{
    anim_.SetSlideDuration(450);
    anim_.SetTweenType(ui::Tween::EASE_OUT);
    anim_.Reset(1, true);
}

OverlayScrollBar::~OverlayScrollBar() {}


void OverlayScrollBar::Update(int content_height, float percent) {
    content_height_ = content_height;

    if (content_height_ <= 0 ||
        view_bounds_.IsEmpty() ||
        scrollbar_bounds_.IsEmpty())
    {
        return;
    }

    int view_height = view_bounds_.height();
    if (view_height >= content_height_) {
        thumb_bounds_.SetRect(0, 0, 0, 0);
        return;
    }

    float scale = view_height / static_cast<float>(content_height_);

    int thumb_width = scrollbar_bounds_.width();
    int thumb_height = std::max(static_cast<int>(view_height * scale), kScrollBarMinHeight);
    int thumb_y = percent * (view_height - thumb_height);

    thumb_bounds_ = gfx::Rect(
        view_bounds_.right() - thumb_width,
        thumb_y,
        thumb_width, thumb_height);
}

void OverlayScrollBar::RegisterScrollHandler(std::function<void(int, Direction)> h) {
    scroll_handler_ = h;
}

void OverlayScrollBar::SetBounds(int x, int y, int width, int height) {
    view_bounds_.SetRect(x, y, width, height);
    scrollbar_bounds_.SetRect(
        x + width - kScrollBarWidth, y, kScrollBarWidth, height);
}

void OverlayScrollBar::SetBounds(const gfx::Rect& bounds) {
    view_bounds_ = bounds;
    scrollbar_bounds_.SetRect(
        bounds.right() - kScrollBarWidth,
        bounds.y(),
        kScrollBarWidth, bounds.height());
}

void OverlayScrollBar::SetDrawingEnabled(bool drawing) {
    if (is_drawing_ != drawing) {
        is_drawing_ = drawing;

        anim_.Reset(1, true);
        if (!drawing) {
            anim_.Hide();
        }
    }
}


void OverlayScrollBar::OnDraw(Sculptor* s) {
    if (!thumb_bounds_.IsEmpty()) {
        D2D1_RECT_F rect {
            thumb_bounds_.x(),
            thumb_bounds_.y(),
            thumb_bounds_.right(),
            thumb_bounds_.bottom() };

        auto color = clrScrollBar;
        color.a *= anim_.GetCurrentValue();

        s->GetPaint()->SetColor(color);
        s->DrawRoundRect(rect, kScrollBarWidth / 2.f);
    }
}

bool OverlayScrollBar::OnMousePressed(const ui::MouseEvent& event) {
    if (thumb_bounds_.IsEmpty()) {
        return false;
    }

    auto loc = event.location();
    if (scrollbar_bounds_.Contains(loc)) {
        is_pressed_ = true;

        if (thumb_bounds_.Contains(loc)) {
            // 在滑块上按下鼠标键
            dragging_prev_ = dragging_start_ = loc;
        } else {
            int distance_y = 0;
            if (loc.y() < thumb_bounds_.y()) {
                distance_y = -thumb_bounds_.height();
            } else if (loc.y() >= thumb_bounds_.bottom()) {
                distance_y = thumb_bounds_.height();
            }

            if (distance_y != 0) {
                MoveScroller(distance_y, loc);
            }
        }
        return true;
    }
    return false;
}

void OverlayScrollBar::OnMouseReleased(const ui::MouseEvent& event) {
    is_pressed_ = false;
    drag_to_limit_ = false;
}

void OverlayScrollBar::OnMouseCaptureLost() {
    is_pressed_ = false;
    drag_to_limit_ = false;
}

bool OverlayScrollBar::OnMouseMoved(const ui::MouseEvent& event) {
    auto loc = event.location();
    return scrollbar_bounds_.Contains(loc);
}

bool OverlayScrollBar::OnMouseDragged(const ui::MouseEvent& event) {
    if (!is_pressed_) {
        return false;
    }

    auto loc = event.location();
    int distance_y = loc.y() - dragging_prev_.y();
    if (distance_y != 0) {
        MoveScroller(distance_y, loc);
    }
    dragging_prev_ = loc;
    return true;
}

bool OverlayScrollBar::IsEmpty() const {
    return thumb_bounds_.IsEmpty();
}

bool OverlayScrollBar::IsAnimating() const {
    return anim_.is_animating();
}

bool OverlayScrollBar::IsInScrollBar(const gfx::Point& p) const {
    return scrollbar_bounds_.Contains(p);
}

bool OverlayScrollBar::MoveScroller(int distance_y, const gfx::Point& loc) {
    int view_height = view_bounds_.height();

    int thumb_dy = std::max(0 - thumb_bounds_.y(), distance_y);
    thumb_dy = std::min(view_height - thumb_bounds_.height() - thumb_bounds_.y(), thumb_dy);

    float scale = thumb_dy / static_cast<float>(view_height - thumb_bounds_.height());
    int dy = (content_height_ - view_height) * scale;

    if (dy == 0 && !drag_to_limit_) {
        drag_to_limit_ = true;
        dragging_limit_ = loc;
        dragging_dir_ = (distance_y < 0 ? -1 : 1);
    } else if (dy != 0) {
        if (drag_to_limit_ &&
            (dragging_limit_.y() - loc.y()) * dragging_dir_ < 0)
        {
            return false;
        }
        drag_to_limit_ = false;
    }

    thumb_bounds_.Offset(0, thumb_dy);
    if (scroll_handler_) {
        scroll_handler_(-dy, distance_y > 0 ? TO_BOTTOM : TO_TOP);
    }

    return dy != 0;
}

}