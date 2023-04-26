#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/vision.h"

#include "base/logging.h"


namespace dmkhime {

Vision::Vision()
    : id_(0),
      measured_width_(0),
      measured_height_(0),
      is_measured_(false),
      is_layouted_(false),
      is_drawn_(false) {
}

void Vision::Measure(int width, int height) {
    /*if (is_measured_) {
        return;
    }*/

    OnMeasure(width, height);
    DCHECK(is_measured_) << "You must invoke SetMeasuredSize() in OnMeasure() !!!";
}

void Vision::Layout(int left, int top, int right, int bottom) {
    int new_width = right - left;
    int new_height = bottom - top;
    int old_width = bounds_.width();
    int old_height = bounds_.height();

    bool size_changed = (new_width != old_width || new_height != old_height);
    bool layout_changed = (left != bounds_.x() || top != bounds_.y() ||
            right != bounds_.right() || bottom != bounds_.bottom());

    if (layout_changed) {
        bounds_.SetRect(left, top, right - left, bottom - top);
    }

    if (size_changed) {
        OnSizeChanged(new_width, new_height, old_width, old_height);
    }

    if (layout_changed) {
        OnLayout(left, top, right, bottom);
    }

    is_layouted_ = true;
}

void Vision::Draw(Sculptor* s) {
    if (!is_measured_ || !is_layouted_) {
        NOTREACHED();
        return;
    }

    /*if (is_drawn_) {
        return;
    }*/

    OnDraw(s);

    is_drawn_ = true;
}

void Vision::OffsetLeftAndRight(int dx) {
    bounds_.Offset(dx, 0);
}

void Vision::OffsetTopAndBottom(int dy) {
    bounds_.Offset(0, dy);
}

void Vision::invalidate() {
    if (render_callback_) {
        render_callback_();
    }
}

void Vision::SetMeasuredSize(int width, int height) {
    measured_width_ = width;
    measured_height_ = height;
    is_measured_ = true;
}

void Vision::SetId(int id) {
    id_ = id;
}

void Vision::SetRenderCallback(std::function<void()> c) {
    render_callback_ = c;
}

int Vision::GetId() const {
    return id_;
}

int Vision::GetLeft() const {
    return bounds_.x();
}

int Vision::GetTop() const {
    return bounds_.y();
}

int Vision::GetRight() const {
    return bounds_.right();
}

int Vision::GetBottom() const {
    return bounds_.bottom();
}

gfx::Rect Vision::GetBounds() const {
    return bounds_;
}

gfx::Rect Vision::GetLocalBounds() const {
    return gfx::Rect(0, 0, bounds_.width(), bounds_.height());
}

int Vision::GetWidth() const {
    return bounds_.width();
}

int Vision::GetHeight() const {
    return bounds_.height();
}

int Vision::GetMeasuredWidth() const {
    return measured_width_;
}

int Vision::GetMeasuredHeight() const {
    return measured_height_;
}

}
