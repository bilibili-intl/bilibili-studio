#include "bililive/bililive/ui/views/livehime/danmaku_hime/super_chat/markedness_message_view.h"

#include "base/notification/notification_service.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/animation/bounds_animator.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/painter.h"

#include "bililive/bililive/utils/bililive_canvas_drawer.h"

#include "grit/theme_resources.h"


namespace {
    const int kMaxSize = 4;
}

class BoxView : public views::View {
public:
    BoxView() {
        InitView();
    }
    ~BoxView() {}

    void InitView() {
        auto layout = new views::BoxLayout(views::BoxLayout::kHorizontal, kPadding17,
            (kMessageViewHeight - kItemHeight) / 2, kPadding10);
        SetLayoutManager(layout);
    }

    void PaintChildren(gfx::Canvas* canvas) override {

        auto content_bound = GetContentsBounds();

        if (shadow_) {
            // 画背景(蒙层)
            canvas->FillRect(content_bound, SkColorSetARGB(0.6f * 255, 0x00, 0x00, 0x00));
        }
    }

    void SetShadow(bool shadow) {
        shadow_ = shadow;
    }

    void DrawShadow(int64_t id) {
        selected_id_ = id;
        SchedulePaint();
    }

    void CleanShadow() {
        selected_id_ = 0;
        SchedulePaint();
    }

    void UpdateView() {
       
    }

    void SetNoSubpixelRendering(bool no_subpixel_rendering) {

    }

private:
    bool shadow_ = false;
    int64_t selected_id_ = 0;

    std::vector<views::View*> vec_reuse_;

};

MarkednessMessageView::MarkednessMessageView(views::View* parent_view)
    : parent_view_(parent_view),
      new_item_animator_(this) {
    arrow_left_img_ = GetImageSkiaNamed(IDR_LIVEHIME_MELEE_SETTLEMENT_ARROW_LEFT);
    arrow_right_img_ = GetImageSkiaNamed(IDR_LIVEHIME_MELEE_SETTLEMENT_ARROW_RIGHT);

    InitViews();

    new_item_animator_.SetAnimationDuration(350);
    new_item_animator_.set_tween_type(ui::Tween::EASE_IN_OUT);
}

MarkednessMessageView::~MarkednessMessageView() {
}

void MarkednessMessageView::AddNewItem(const SuperChatData& danmaku) {
    // 需要当前已布局好
    auto bounds = GetContentsBounds();
    if (bounds.IsEmpty()) {
        NOTREACHED();
        return;
    }

    if (new_item_status_ != NewItemStatus::None) {
        new_item_queue_.push(danmaku);
        return;
    }

    UpdateWhenAnimation(true);
    DoInAnimation();
}

void MarkednessMessageView::InitViews() {
    box_view_ = new BoxView();


    AddChildView(box_view_);
}

void MarkednessMessageView::PaintChildren(gfx::Canvas* canvas) {
    __super::PaintChildren(canvas);

    if (animation_show_) { return; }

    auto bounds = GetContentsBounds();
    gfx::Rect bounds_left(bounds.x(), bounds.y(), kArrowViewWidth, bounds.height());
    gfx::Rect bounds_right(bounds.x() + bounds.width() - kArrowViewWidth, bounds.y(),
        kArrowViewWidth, bounds.height());

    if (left_arrow_visible_) {
        scoped_ptr<views::Painter> painter(views::Painter::CreateHorizontalGradient(
            SkColorSetARGB(0xAA, 0x00, 0x00, 0x00), SkColorSetARGB(0, 0x00, 0x00, 0x00)));
        views::Painter::PaintPainterAt(canvas, painter.get(), bounds_left);

        if (!arrow_left_img_->isNull()) {
            auto img_x = bounds.x() + kPadding10;
            auto img_y = bounds.y() + (bounds.height() - arrow_left_img_->size().height()) / 2;
            canvas->DrawImageInt(*arrow_left_img_, img_x, img_y);
        }
    }

    if(right_arrow_visible_) {
        scoped_ptr<views::Painter> painter(views::Painter::CreateHorizontalGradient(
            SkColorSetARGB(0, 0x00, 0x00, 0x00), SkColorSetARGB(0xAA, 0x00, 0x00, 0x00)));
        views::Painter::PaintPainterAt(canvas, painter.get(), bounds_right);

        if (!arrow_right_img_->isNull()) {
            auto img_x = bounds.x() + bounds.width() - kPadding10 - arrow_right_img_->size().width();
            auto img_y = bounds.y() + (bounds.height() - arrow_right_img_->size().height()) / 2;
            canvas->DrawImageInt(*arrow_right_img_, img_x, img_y);
        }
    }
}

void MarkednessMessageView::OnBoundsChanged(const gfx::Rect& previous_bounds) {

}

void MarkednessMessageView::Layout() {
    auto bounds = GetContentsBounds();

    auto box_width = kPadding17 * 2 + kPadding10 * 3 + kItemWidth * 4;

    if (show_type_ == ShowType::LeftSideComplete) {
        box_view_->SetBounds(bounds.x(), bounds.y(),
            box_width, kMessageViewHeight);
    }
    else {
        box_view_->SetBounds(bounds.x() + bounds.width() - box_width, bounds.y(),
            box_width, kMessageViewHeight);
    }
}

void MarkednessMessageView::UpdateData() {
    //给itemview中的每个child赋值

}

bool MarkednessMessageView::OnMouseWheel(const ui::MouseWheelEvent& event) {
    if (animation_show_) { return false; }

    bool consumed = false;
    // 不管高精度滚轮
    int off = event.y_offset() / ui::MouseWheelEvent::kWheelDelta;
    if (off < 0 && right_arrow_visible_) {
        ArrowRightPressed();
        consumed = true;
    } else if (off > 0 && left_arrow_visible_) {
        ArrowLeftPressed();
        consumed = true;
    }
    return consumed;
}

void MarkednessMessageView::OnMouseEntered(const ui::MouseEvent& event) {
    if (animation_show_) { return; }

    UpdateArrow();

    __super::OnMouseEntered(event);
}

void MarkednessMessageView::OnMouseExited(const ui::MouseEvent& event) {
    if (animation_show_) { return; }

    if (box_view_->IsMouseHovered()) {
        return;
    }

    left_arrow_visible_ = false;
    right_arrow_visible_ = false;
    SchedulePaint();

    __super::OnMouseExited(event);
}

bool MarkednessMessageView::OnMousePressed(const ui::MouseEvent& event) {
    if (animation_show_) { return false; }

    __super::OnMousePressed(event);

    return true;
}

void MarkednessMessageView::OnMouseReleased(const ui::MouseEvent& event) {
    if (animation_show_) { return; }

    if (event.IsLeftMouseButton()) {
        auto cur_p = event.location();

        auto bounds = GetContentsBounds();
        gfx::Rect left_bounds(bounds.x(), bounds.y(),
            arrow_left_img_->size().width() + kPadding10, bounds.height());
        gfx::Rect right_bounds(bounds.x() + bounds.width() - kPadding10 - arrow_right_img_->size().width(),
            bounds.y(),
            arrow_right_img_->size().width() + kPadding10, bounds.height());

        if (left_bounds.Contains(cur_p) && left_arrow_visible_) {
            ArrowLeftPressed();
        }

        if (right_bounds.Contains(cur_p) && right_arrow_visible_) {
            ArrowRightPressed();
        }
    }

    __super::OnMouseReleased(event);
}

void MarkednessMessageView::UpdateArrow() {
    right_arrow_visible_ = false;
    left_arrow_visible_ = false;

    InvalidateLayout();
    Layout();

    SchedulePaint();
}

views::View* MarkednessMessageView::GetEventHandlerForPoint(const gfx::Point& point) {
    if (animation_show_) {
        return this;
    }

    auto bounds = GetContentsBounds();
    gfx::Rect left_bounds(bounds.x(), bounds.y(),
        arrow_left_img_->size().width() + kPadding10, bounds.height());
    gfx::Rect right_bounds(bounds.x() + bounds.width() - kPadding10 - arrow_right_img_->size().width(),
        bounds.y(),
        arrow_right_img_->size().width() + kPadding10, bounds.height());

    if ((left_bounds.Contains(point) && left_arrow_visible_)
        || (right_bounds.Contains(point)) && right_arrow_visible_) {
        return this;
    }

    bool is_inchild = false;
    for (int i = 0; i < box_view_->child_count(); ++i) {
        auto child_bound = box_view_->child_at(i)->bounds();
        child_bound.set_x(bounds.x() + child_bound.x());
        child_bound.set_y(bounds.y() + child_bound.y());
        if (child_bound.Contains(point)) {
            is_inchild = true;
            break;
        }
    }

    if (!is_inchild) {
        return this;
    }

    return __super::GetEventHandlerForPoint(point);
}

void MarkednessMessageView::ArrowLeftPressed() {
    if (show_type_ == ShowType::RightSideComplete) {
        //左边显示不完全 则只改变显示模式，不改变实际内容
        show_type_ = ShowType::LeftSideComplete;
        InvalidateLayout();
        Layout();
    }
    else {
        
    }

    UpdateArrow();
}

void MarkednessMessageView::ArrowRightPressed() {

    UpdateArrow();
}

void MarkednessMessageView::DoInAnimation() {

}

void MarkednessMessageView::DoOutAnimation() {

}

void MarkednessMessageView::OnPauseTimer() {
    DoOutAnimation();
}

void MarkednessMessageView::AnimationEnded(const ui::Animation* animation) {

}

void MarkednessMessageView::RefreshView() {
    static_cast<BoxView*>(box_view_)->UpdateView();
    UpdateArrow();
    UpdateData();
}

void MarkednessMessageView::RefreshShadow(int64_t sel_id) {
    static_cast<BoxView*>(box_view_)->SetShadow(true);
    static_cast<BoxView*>(box_view_)->DrawShadow(sel_id);
}

void MarkednessMessageView::CloseDetailsView() {
    static_cast<BoxView*>(box_view_)->SetShadow(false);
    static_cast<BoxView*>(box_view_)->CleanShadow();
}

void MarkednessMessageView::UpdateWhenAnimation(bool animation_show) {
    animation_show_ = animation_show;
    static_cast<BoxView*>(box_view_)->SetShadow(animation_show);
    SchedulePaint();
}

void MarkednessMessageView::SetNoSubpixelRendering(bool no_subpixel_rendering) {
    static_cast<BoxView*>(box_view_)->SetNoSubpixelRendering(no_subpixel_rendering);
}