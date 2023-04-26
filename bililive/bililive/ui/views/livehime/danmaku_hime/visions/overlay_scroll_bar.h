#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISIONS_OVERLAY_SCROLL_BAR_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISIONS_OVERLAY_SCROLL_BAR_H_

#include <functional>

#include "ui/gfx/canvas.h"
#include "ui/views/view.h"
#include "ui/base/animation/slide_animation.h"


class Sculptor;

namespace dmkhime {

class OverlayScrollBar {
public:
    enum Direction {
        TO_TOP,
        TO_BOTTOM,
    };

    explicit OverlayScrollBar();
    ~OverlayScrollBar();

    void Update(int content_height, float percent);

    void RegisterScrollHandler(std::function<void(int, Direction)> h);

    // View
    void SetBounds(int x, int y, int width, int height);
    void SetBounds(const gfx::Rect& bounds);

    void SetDrawingEnabled(bool drawing);

    void OnDraw(Sculptor* s);
    bool OnMousePressed(const ui::MouseEvent& event);
    void OnMouseReleased(const ui::MouseEvent& event);
    void OnMouseCaptureLost();
    bool OnMouseMoved(const ui::MouseEvent& event);
    bool OnMouseDragged(const ui::MouseEvent& event);

    bool IsEmpty() const;
    bool IsAnimating() const;
    bool IsInScrollBar(const gfx::Point& p) const;

private:
    bool MoveScroller(int distance_y, const gfx::Point& loc);

    int content_height_;
    bool is_drawing_ = true;

    gfx::Rect view_bounds_;
    gfx::Rect scrollbar_bounds_;
    gfx::Rect thumb_bounds_;

    bool is_pressed_ = false;
    gfx::Point dragging_prev_;
    gfx::Point dragging_start_;

    bool drag_to_limit_ = false;
    int dragging_dir_ = 0;
    gfx::Point dragging_limit_;

    ui::SlideAnimation anim_;
    std::function<void(int, Direction)> scroll_handler_;

    DISALLOW_COPY_AND_ASSIGN(OverlayScrollBar);
};

}

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISIONS_OVERLAY_SCROLL_BAR_H_