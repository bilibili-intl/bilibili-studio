#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/danmaku_list.h"

#include <algorithm>

#include "base/stl_util.h"

#include "ui/base/win/dpi.h"

#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_renderer.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/scrolling_interpolator.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/sculptor.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/danmaku_list_adapter.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/danmaku_vision.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/paint.h"


namespace {

    const int kScrollBarDisplaySecs = 1;

    ui::MouseEvent OffsetMouseEvent(const ui::MouseEvent& e, int dx, int dy) {
        auto ev(e);
        auto point(e.location());
        point.Offset(dx, dy);
        ev.set_location(point);
        return ev;
    }

}

namespace dmkhime {

DanmakuList::DanmakuList(bool can_interactive, float speed_factor)
    : cur_position_(0),
      cur_offset_in_position_(0),
      can_interactive_(can_interactive),
      interpolator_(std::make_unique<CubicBezierInterpolator>())
{
    if (can_interactive_) {
        using namespace std::placeholders;
        scroll_bar_.RegisterScrollHandler(
            std::bind(&DanmakuList::OnScroll, this, _1, _2));
    }

    scrolling_scale_ = ui::GetDPIScale() * speed_factor;
}

DanmakuList::~DanmakuList() {
    if (can_interactive_) {
        scroll_bar_.RegisterScrollHandler(nullptr);
    }
    STLDeleteElements(&recycler_);
}

bool DanmakuList::IsClickable(const gfx::Point& p) const {
    if (!can_interactive_) {
        return false;
    }

    if (scroll_bar_.IsInScrollBar(p)) {
        return false;
    }

    if (nav_down_button_.Hit(p)) {
        return true;
    }

    for (const auto& v : visions_) {
        auto bounds = v->GetBounds();
        if (bounds.Contains(p)) {
            auto dv = static_cast<DanmakuVision*>(v);
            gfx::Point cp(p);
            cp.Offset(-bounds.x(), -bounds.y());
            if (dv->IsClickable(cp)) {
                return true;
            }
        }
    }

    return false;
}

bool DanmakuList::OnMouseWheel(const ui::MouseWheelEvent& event) {
    if (!can_interactive_) {
        return false;
    }

    if (scroll_bar_.IsEmpty()) {
        return false;
    }

    auto dy = (event.y_offset() * 32 / ui::MouseWheelEvent::kWheelDelta) * scrolling_scale_;
    bool is_to_bottom = dy < 0;
    dy = DetermineScroll(dy);

    is_at_bottom_ = is_to_bottom && IsAtBottom(dy);
    if (auto_scroll_to_bottom_ != is_at_bottom_) {
        auto_scroll_to_bottom_ = is_at_bottom_;
        OnAutoScrollStateChanged();
    }
    if (dy != 0) {
        OffsetChildren(0, dy);
        RecordCurPositionAndOffset();
        UpdateOverlayScrollBar();
    } else {
        AwakeScrollBar(true);
    }

    return dy != 0;
}

bool DanmakuList::OnMousePressed(
    const ui::MouseEvent& event, DmkHitType* type, const DmkDataObject** data)
{
    if (!can_interactive_) {
        return false;
    }

    // 滚动条优先
    if (event.IsLeftMouseButton() &&
        scroll_bar_.OnMousePressed(event))
    {
        is_pressed_on_scrollbar_ = true;
        return true;
    }

    // 然后是导航按钮
    if (event.IsLeftMouseButton() &&
        nav_down_button_.Hit(event.location()))
    {
        is_pressed_on_nav_down_ = true;
        return true;
    }

    // 最后是弹幕，根据按下位置来通知相应的 vision
    for (const auto& v : visions_) {
        auto bounds(v->GetBounds());
        if (bounds.Contains(event.location())) {
            return static_cast<DanmakuVision*>(v)->OnMousePressed(
                OffsetMouseEvent(event, -bounds.x(), -bounds.y()), type, data);
        }
    }
    return false;
}

void DanmakuList::OnMouseReleased(
    const ui::MouseEvent& event, DmkHitType* type, const DmkDataObject** data)
{
    if (!can_interactive_) {
        return;
    }

    if (is_pressed_on_scrollbar_) {
        scroll_bar_.OnMouseReleased(event);
        is_pressed_on_scrollbar_ = false;
        AwakeScrollBar(true);
        return;
    }

    if (is_pressed_on_nav_down_) {
        is_pressed_on_nav_down_ = false;
        if (nav_down_button_.Hit(event.location())) {
            ScrollToBottom(false);
        }
        return;
    }

    // 本来应该只通知按下位置所在的 vision，不过追踪比较麻烦，
    // 干脆全通知算了，让 vision 自己去判断
    for (const auto& v : visions_) {
        auto bounds(v->GetBounds());
        if (bounds.Contains(event.location())) {
            static_cast<DanmakuVision*>(v)->OnMouseReleased(
                OffsetMouseEvent(event, -bounds.x(), -bounds.y()), type, data);
            return;
        }
    }
}

void DanmakuList::OnMouseCaptureLost() {
    if (!can_interactive_) {
        return;
    }

    if (is_pressed_on_scrollbar_) {
        AwakeScrollBar(true);
    }

    is_pressed_on_nav_down_ = false;
    is_pressed_on_scrollbar_ = false;
    scroll_bar_.OnMouseCaptureLost();

    // 全通知，理由如上
    for (const auto& v : visions_) {
        static_cast<DanmakuVision*>(v)->OnMouseCaptureLost();
    }
}

void DanmakuList::OnMouseMoved(const ui::MouseEvent& event) {
    if (!can_interactive_) {
        return;
    }

    if (scroll_bar_.OnMouseMoved(event)) {
        AwakeScrollBar(true);
    }
}

bool DanmakuList::OnMouseDragged(const ui::MouseEvent& event) {
    if (!can_interactive_) {
        return false;
    }

    if (scroll_bar_.OnMouseDragged(event)) {
        return true;
    }

    return false;
}

void DanmakuList::SetAdapter(DanmakuListAdapter* adapter) {
    adapter_.reset(adapter);
    FillItems(0, 0);
    UpdateOverlayScrollBar();
}

void DanmakuList::SetRecycleListener(ListItemRecycledListener* listener) {
    recycled_listener_ = listener;
}

void DanmakuList::SetStatusChangeListener(ListStatusChangeListener* listener) {
    s_listener_ = listener;
}

void DanmakuList::SetNavDownText(const string16& text) {
    nav_down_button_.SetText(text);
}

void DanmakuList::NotifyItemAddToBottom() {
    FillBottomSpace(0);
    //RecycleTopItems(0);
}

void DanmakuList::NotifyItemRemoveFromTop(int length) {
    if (length <= 0 || visions_.empty()) {
        return;
    }

    auto first_vision = GetFirstVision();
    if (!first_vision) {
        DCHECK(false);
        return;
    }

    if (first_vision->GetPosition() + 1 > length) {
        /**
         * 要移除的项目并不在当前已具现化的 Vision 的数据索引的范围内，
         * 此时只需要更新当前已具现化的 Vision 的数据索引即可。
         */
        for (auto v : visions_) {
            auto dv = static_cast<DanmakuVision*>(v);
            dv->SetPosition(dv->GetPosition() - length);
        }
        cur_position_ = std::max(cur_position_ - length, 0);
    } else {
        /**
         * 要移除的项目在当前已具现化的 Vision 的数据索引的范围内，此时需要：
         * 1. 回收移除项与当前已具现化的 Vision 的数据索引的重叠部分对应的 Vision；
         * 2. 如果移除之后，当前的第一个 Vision 顶部越过了父 Vision 的顶部边界，需要进行移动。
         */
        int remove_count = length - first_vision->GetPosition();
        int count = std::min<int>(remove_count, visions_.size());

        cur_position_ = std::max(cur_position_ - length, 0);
        cur_offset_in_position_ = 0;

        for (int i = 0; i < static_cast<int>(visions_.size()); ++i) {
            if (i < count) {
                recycler_.push_back(static_cast<DanmakuVision*>(visions_[i]));
            } else {
                auto dv = static_cast<DanmakuVision*>(visions_[i]);
                dv->SetPosition(dv->GetPosition() - length);
            }
        }

        if (count != 0) {
            RemoveVisions(0, count);

            // 填补顶部可能出现的空隙
            first_vision = GetFirstVision();
            if (first_vision) {
                int dy = GetLocalBounds().y() - first_vision->GetTop();
                if (dy < 0) {
                    dy = FillBottomSpace(dy);
                    if (dy != 0) {
                        OffsetChildren(0, dy);
                        RecordCurPositionAndOffset();
                        UpdateOverlayScrollBar();
                    }
                }
            }
        }

        OnAnimationStart();
    }
}

void DanmakuList::NotifyDataSetChanged() {
    RecordCurPositionAndOffset();
    FillItems(cur_position_, cur_offset_in_position_);
    RecordCurPositionAndOffset();
    UpdateOverlayScrollBar();
}

void DanmakuList::ScrollToBottom(bool smooth) {
    if (smooth) {
        OnAnimationStart();
    } else {
        ScrollToPosition(adapter_->GetItemCount(), 0);
        RecordCurPositionAndOffset();
        UpdateOverlayScrollBar();

        is_at_bottom_ = true;
        if (!auto_scroll_to_bottom_) {
            auto_scroll_to_bottom_ = true;
            OnAutoScrollStateChanged();
        }
    }
}

void DanmakuList::ScrollToBottomIfAtBottom(bool smooth) {
    // 如果处于非自动滚动状态的话就直接返回
    if (!auto_scroll_to_bottom_) {
        UpdateOverlayScrollBar();
        return;
    }

    if (smooth || !is_at_bottom_) {
        OnAnimationStart();
    } else {
        ScrollToPosition(adapter_->GetItemCount(), 0);
        RecordCurPositionAndOffset();
        UpdateOverlayScrollBar();

        is_at_bottom_ = true;
        if (!auto_scroll_to_bottom_) {
            auto_scroll_to_bottom_ = true;
            OnAutoScrollStateChanged();
        }
    }
}

bool DanmakuList::IsAnimating() const {
    return nav_down_button_.IsAnimating() || scroll_bar_.IsAnimating();
}

bool DanmakuList::IsAutoScrollToBottom() const {
    return auto_scroll_to_bottom_;
}

void DanmakuList::OnMeasure(int width, int height) {
    SetMeasuredSize(width, height);
}

void DanmakuList::OnSizeChanged(int width, int height, int old_w, int old_h) {
}

void DanmakuList::OnLayout(int left, int top, int right, int bottom) {
    RecordCurPositionAndOffset();
    FillItems(cur_position_, cur_offset_in_position_);
    RecordCurPositionAndOffset();
    if (can_interactive_) {
        scroll_bar_.SetBounds(0, 0, right - left, bottom - top);
    }
    UpdateOverlayScrollBar();
}

void DanmakuList::OnDrawOver(Sculptor* s) {
    if (!can_interactive_) {
        return;
    }

    scroll_bar_.OnDraw(s);
    nav_down_button_.OnDrawOver(s, GetLocalBounds());
}

void DanmakuList::OnScroll(int dy, OverlayScrollBar::Direction dir) {
    if (!can_interactive_) {
        return;
    }

    int distance = 0;
    auto bounds = GetLocalBounds();
    auto first_vision = GetFirstVisibleVision();
    if (first_vision) {
        distance = std::max(0, bounds.y() - first_vision->GetTop());
        for (int i = 0; i < first_vision->GetPosition(); ++i) {
            distance += adapter_->GetItems().at(i)->GetHeight();
        }
    }

    int prev, next;
    ComputeTotalHeight(&prev, &next, nullptr);

    dy = DetermineScroll(dy);

    is_at_bottom_ = (dir == OverlayScrollBar::TO_BOTTOM && IsAtBottom(dy));
    if (auto_scroll_to_bottom_ != is_at_bottom_) {
        auto_scroll_to_bottom_ = is_at_bottom_;
        OnAutoScrollStateChanged();
    }
    if (dy != 0) {
        OffsetChildren(0, dy);
        RecordCurPositionAndOffset();
        UpdateOverlayScrollBar();
        invalidate();
    }

    AwakeScrollBar(false);
}

bool DanmakuList::OnComputeScroll(int interval) {
    if (!auto_scroll_to_bottom_) {
        return false;
    }

    auto distance = GetCurToBottomDistance();
    if (distance > 0) {
        int dy = interpolator_->GetScrollingSpan(interval, -distance, scrolling_scale_);
        dy = DetermineScroll(dy);
        if (dy != 0) {
            OffsetChildren(0, dy);
            RecordCurPositionAndOffset();
            UpdateOverlayScrollBar();
        }
        is_at_bottom_ = false;
        return true;
    }
    is_at_bottom_ = true;
    return false;
}

void DanmakuList::Refresh()
{
    ScrollToPosition(cur_position_, cur_offset_in_position_);
}

void DanmakuList::OnAutoScrollStateChanged() {
    if (!can_interactive_) {
        return;
    }

    if (auto_scroll_to_bottom_) {
        nav_down_button_.Hide();
    } else {
        RecycleBottomItems(0);
        nav_down_button_.Show();
    }
    OnAnimationStart();

    if (s_listener_) {
        s_listener_->OnListAutoScrollStateChanged(auto_scroll_to_bottom_);
    }
}

void DanmakuList::OnAnimationStart() {
    if (s_listener_) {
        s_listener_->OnListAnimationStart();
    }
}

void DanmakuList::OnHidingScrollBar() {
    scroll_bar_.SetDrawingEnabled(false);
    OnAnimationStart();
}

void DanmakuList::FillItems(int pos, int offset) {
    auto bounds = GetLocalBounds();
    if (bounds.IsEmpty() || !adapter_) {
        return;
    }

    int count = adapter_->GetItemCount();
    if (count <= 0) {
        pos = 0;
        offset = 0;
    } else {
        if (pos > count - 1) {
            pos = count - 1;
            offset = 0;
        }
    }

    int cur_height = 0;
    size_t index = 0;
    // 往列表中填 Vision 并绑定数据，直到触底
    for (int i = pos; i < count; ++i, ++index) {
        DanmakuVision* vision;
        if (visions_.size() > index) {
            vision = static_cast<DanmakuVision*>(visions_[index]);
        } else {
            vision = MakeNewDanmakuVision(i);
            AddVision(vision);
        }

        vision->SetPosition(i);
        adapter_->OnBindDanmakuVision(vision, i);
        vision->Measure(bounds.width(), bounds.height());
        int vision_height = vision->GetMeasuredHeight();

        // 调整 offset，防止前几项不可见
        if (i == pos && offset >= vision_height) {
            offset = 0;
        }

        vision->Layout(
            bounds.x(),
            bounds.y() + cur_height - offset,
            bounds.x() + vision->GetMeasuredWidth(),
            bounds.y() + cur_height + vision_height - offset);
        cur_height += vision_height;

        if (cur_height > bounds.height() + offset) {
            ++index;
            break;
        }
    }

    // 回收多余的 Vision
    if (index < visions_.size()) {
        for (size_t i = index; i < visions_.size(); ++i) {
            recycler_.push_back(static_cast<DanmakuVision*>(visions_[i]));
        }
        RemoveVisions(index, visions_.size() - index);
    }

    //防止在列表大小变化时项目超出滑动范围。
    auto last_vision = GetLastVisibleVision();
    auto first_vision = GetFirstVisibleVision();
    if (last_vision && first_vision
        && last_vision->GetPosition() + 1 == adapter_->GetItemCount())
    {
        bool can_scroll = ((first_vision->GetPosition() == 0 && 0 - first_vision->GetTop() > 0) ||
            first_vision->GetPosition() > 0);
        if (can_scroll) {
            int bottom_dy = GetHeight() - last_vision->GetBottom();
            if (bottom_dy > 0) {
                bottom_dy = DetermineScroll(bottom_dy);
                if (bottom_dy != 0) {
                    OffsetChildren(0, bottom_dy);
                }
            }
        }
    }
}

void DanmakuList::ScrollToPosition(int pos, int offset) {
    auto bounds = GetLocalBounds();
    if (bounds.IsEmpty() || !adapter_) {
        return;
    }

    bool to_bottom = false;
    int child_count = adapter_->GetItemCount();
    if (pos + 1 > child_count) {
        if (child_count > 0) {
            pos = child_count - 1;
        } else {
            pos = 0;
        }
        offset = 0;
        to_bottom = true;
    }

    size_t index = 0;
    int total_height = 0;
    int diff = 0;
    bool full_child_reached = false;

    // 直接在指定位置处填 Vision 并绑定数据，因为并非是平滑滚动，
    // 所以效果是一样的。
    for (int i = pos; i < child_count; ++i, ++index) {
        DanmakuVision* vision;
        if (visions_.size() > index) {
            vision = static_cast<DanmakuVision*>(visions_[index]);
        } else {
            vision = MakeNewDanmakuVision(i);
            AddVision(vision);
        }

        vision->SetPosition(i);
        adapter_->OnBindDanmakuVision(vision, i);
        vision->Measure(bounds.width(), bounds.height());
        int vision_height = vision->GetMeasuredHeight();
        vision->Layout(
            bounds.x(), bounds.y() + total_height - offset,
            bounds.x() + vision->GetMeasuredWidth(),
            bounds.y() + total_height + vision_height - offset);

        total_height += vision_height;
        diff = bounds.bottom() - vision->GetBottom();
        if (total_height >= bounds.height() + offset) {
            full_child_reached = true;
            ++index;
            break;
        }
    }

    if (index < visions_.size()) {
        for (size_t i = index; i < visions_.size(); ++i) {
            recycler_.push_back(static_cast<DanmakuVision*>(visions_[i]));
        }
        RemoveVisions(index, visions_.size() - index);
    }

    if (child_count > 0) {
        if ((!full_child_reached && diff > 0) || (to_bottom && diff < 0)) {
            int resDiff = FillTopSpace(diff);
            RecycleBottomItems(resDiff);
            OffsetChildren(0, resDiff);
        }
    }
}

void DanmakuList::SmoothScrollToPosition(int pos, int offset) {
    // 没这需求，先不搞
    //auto bounds = GetLocalBounds();
    //if (bounds.IsEmpty() || !adapter_) {
    //    return;
    //}

    //int child_count = adapter_->GetItemCount();
    //if (child_count == 0) {
    //    return;
    //}

    //if (pos + 1 > child_count) {
    //    pos = child_count - 1;
    //    offset = 0;
    //}

    //// scroll_animator_->Stop();

    //RecordCurPositionAndOffset();

    //int start_pos = cur_position_;
    //int start_pos_offset = cur_offset_in_position_;
    //int terminate_pos = pos;
    //int terminate_pos_offset = offset;
    //bool front = (start_pos <= terminate_pos);
}

DanmakuVision* DanmakuList::MakeNewDanmakuVision(int pos) {
    DanmakuVision* vision;
    if (!recycler_.empty()) {
        vision = recycler_.back();
        recycler_.pop_back();
    } else {
        vision = adapter_->OnCreateDanmakuVision(pos);
    }

    return vision;
}

int DanmakuList::DetermineScroll(int dy) {
    if (dy > 0) {         // 向下滚动
        dy = FillTopSpace(dy);
        RecycleBottomItems(dy);
    } else if (dy < 0) {  // 向上滚动
        dy = FillBottomSpace(dy);
        RecycleTopItems(dy);
    }

    return dy;
}

void DanmakuList::UpdateOverlayScrollBar() {
    if (!can_interactive_) {
        return;
    }

    AwakeScrollBar(true);

    int prev_total_height, next_total_height;
    ComputeTotalHeight(&prev_total_height, &next_total_height, nullptr);

    int total_height = prev_total_height + next_total_height;
    float percent = static_cast<float>(prev_total_height) / (total_height - GetHeight());
    percent = std::max(0.f, percent);
    percent = std::min(1.f, percent);

    scroll_bar_.Update(total_height, percent);
}

void DanmakuList::RecordCurPositionAndOffset() {
    auto vision = GetFirstVisibleVision();
    if (vision) {
        cur_position_ = vision->GetPosition();
        cur_offset_in_position_ = 0 - vision->GetTop();
    } else {
        cur_position_ = 0;
        cur_offset_in_position_ = 0;
    }
}

void DanmakuList::ComputeTotalHeight(int* prev, int* next, int* to_bottom) const {
    auto count = adapter_->GetItemCount();
    if (count == 0) {
        *prev = 0;
        *next = 0;
        if (to_bottom) {
            *to_bottom = 0;
        }
        return;
    }

    int child_count = visions_.size();
    if (child_count == 0) {
        *prev = 0;
        *next = 0;
        if (to_bottom) {
            *to_bottom = 0;
        }
        return;
    }

    // 计算当前已测量的 View 高度的平均值
    int det_height = 0;
    for (int i = 0; i < child_count; ++i) {
        det_height += visions_[i]->GetHeight();
    }
    int avgc_height = (det_height + child_count - 1) / child_count;

    // 计算之前的高度
    auto f_holder = GetFirstVision();
    auto fv_holder = GetFirstVisibleVision();
    if (!fv_holder) {
        *prev = 0;
        *next = 0;
        if (to_bottom) {
            *to_bottom = 0;
        }
        return;
    }

    int i;
    int prev_total_height = cur_offset_in_position_ + f_holder->GetPosition() * avgc_height;
    for (i = 0; i < fv_holder->GetPosition() - f_holder->GetPosition(); ++i) {
        prev_total_height += visions_[i]->GetHeight();
    }

    // 计算之后的高度
    auto l_holder = GetLastVision();
    if (!l_holder) {
        *prev = 0;
        *next = 0;
        if (to_bottom) {
            *to_bottom = 0;
        }
        return;
    }

    int next_total_height = -cur_offset_in_position_ + (count - l_holder->GetPosition() - 1) * avgc_height;
    for (; i <= l_holder->GetPosition() - f_holder->GetPosition(); ++i) {
        next_total_height += visions_[i]->GetHeight();
    }

    if (to_bottom) {
        auto bounds = GetLocalBounds();
        auto lv_vision = GetLastVisibleVision();

        int to_bottom_height = std::max(0, lv_vision->GetBottom() - bounds.bottom())
            + (count - l_holder->GetPosition() - 1) * avgc_height;
        for (i = lv_vision->GetPosition() - f_holder->GetPosition() + 1;
            i <= l_holder->GetPosition() - f_holder->GetPosition(); ++i)
        {
            to_bottom_height += visions_[i]->GetHeight();
        }
        *to_bottom = to_bottom_height;
    }

    *prev = prev_total_height;
    *next = next_total_height;
}

void DanmakuList::AwakeScrollBar(bool preparing_to_sleep) {
    scrollbar_timer_.Stop();
    scroll_bar_.SetDrawingEnabled(true);

    if (preparing_to_sleep) {
        scrollbar_timer_.Start(
            FROM_HERE, base::TimeDelta::FromSeconds(kScrollBarDisplaySecs),
            base::Bind(&DanmakuList::OnHidingScrollBar, base::Unretained(this)));
    }

    OnAnimationStart();
}

bool DanmakuList::IsAtBottom(int dy) const {
    auto last = GetLastVision();
    auto bounds = GetLocalBounds();
    if (last &&
        last->GetPosition() + 1 == adapter_->GetItemCount() &&
        last->GetBottom() + dy <= bounds.bottom())
    {
        return true;
    }
    return false;
}

int DanmakuList::GetCurToBottomDistance() const {
    int prev, next, to_bottom;
    ComputeTotalHeight(&prev, &next, &to_bottom);
    return to_bottom;
}

DanmakuVision* DanmakuList::GetFirstVision() const {
    if (!visions_.empty()) {
        return static_cast<DanmakuVision*>(visions_.front());
    }
    return nullptr;
}

DanmakuVision* DanmakuList::GetLastVision() const {
    if (!visions_.empty()) {
        return static_cast<DanmakuVision*>(visions_.back());
    }
    return nullptr;
}

DanmakuVision* DanmakuList::GetFirstVisibleVision() const {
    for (size_t i = 0; i < visions_.size(); ++i) {
        auto vision = visions_[i];
        if (vision->GetBottom() > 0 && GetHeight() > 0) {
            return static_cast<DanmakuVision*>(vision);
        }
    }

    return nullptr;
}

DanmakuVision* DanmakuList::GetLastVisibleVision() const {
    for (int i = visions_.size(); i > 0; --i) {
        auto vision = visions_[i - 1];
        if (vision->GetTop() < GetHeight() && GetHeight() > 0) {
            return static_cast<DanmakuVision*>(vision);
        }
    }

    return nullptr;
}

int DanmakuList::FillTopSpace(int dy) {
    // 向上滚动（内容向下），dy 为正值
    if (visions_.empty()) {
        return 0;
    }

    auto first_vision = static_cast<DanmakuVision*>(visions_.front());
    auto prev_vision = first_vision;

    auto bounds = GetLocalBounds();
    int pos = first_vision->GetPosition();

    int inc_y = 0;
    int distance_y = first_vision->GetTop() + dy - bounds.y();

    // 因列表子 Vision 下移，上部可能会出现空缺，需要填补
    while (pos > 0 && bounds.y() - prev_vision->GetTop() < dy) {
        --pos;

        auto new_vision = MakeNewDanmakuVision(pos);
        AddVision(new_vision, 0);

        new_vision->SetPosition(pos);
        adapter_->OnBindDanmakuVision(new_vision, pos);
        new_vision->Measure(bounds.width(), bounds.height());
        int vision_height = new_vision->GetMeasuredHeight();
        new_vision->Layout(
            bounds.x(), prev_vision->GetTop() - vision_height,
            bounds.x() + new_vision->GetMeasuredWidth(),
            prev_vision->GetTop());

        prev_vision = new_vision;

        // 先行回收，防止用户快速拖动滚动条时创建出大量的 ViewHolder
        inc_y += vision_height;
        if (inc_y <= distance_y) {
            RecycleBottomItems(inc_y);
        }
    }

    if (bounds.y() - prev_vision->GetTop() >= dy) {
        return dy;
    }

    if (pos == 0) {
        return std::max(bounds.y() - prev_vision->GetTop(), 0);
    }

    return 0;
}

int DanmakuList::FillBottomSpace(int dy) {
    // 向下滚动（内容向上），dy 为负值
    auto last_vision = GetLastVision();
    auto prev_vision = last_vision;

    auto bounds = GetLocalBounds();
    int pos = last_vision ? last_vision->GetPosition() : -1;
    int prev_bottom = prev_vision ? prev_vision->GetBottom() : 0;

    int inc_y = 0;
    int distance_y = (last_vision ? last_vision->GetBottom() : 0) + dy - bounds.bottom();

    // 因列表子 Vision 上移，下部可能会出现空缺，需要填补
    while (pos < adapter_->GetItemCount() - 1 && bounds.bottom() - prev_bottom > dy) {
        ++pos;

        auto new_vision = MakeNewDanmakuVision(pos);
        AddVision(new_vision);

        new_vision->SetPosition(pos);
        adapter_->OnBindDanmakuVision(new_vision, pos);
        new_vision->Measure(bounds.width(), bounds.height());
        int vision_height = new_vision->GetMeasuredHeight();
        new_vision->Layout(
            bounds.x(), prev_bottom,
            bounds.x() + new_vision->GetMeasuredWidth(),
            prev_bottom + vision_height);

        prev_vision = new_vision;
        prev_bottom = prev_vision->GetBottom();

        if (dy != 0) {
            // 先行回收，防止用户快速拖动滚动条时创建出大量的 ViewHolder
            inc_y += vision_height;
            if (inc_y <= -distance_y) {
                RecycleTopItems(-inc_y);
            }
        }
    }

    if (bounds.bottom() - prev_bottom <= dy) {
        return dy;
    }

    if (pos == adapter_->GetItemCount() - 1) {
        auto first = GetFirstVisibleVision();
        if (first && first->GetTop() > bounds.y()) {
            return std::max(bounds.y() - first->GetTop(), dy);
        }
        return std::min(bounds.bottom() - prev_bottom, 0);
    }

    return 0;
}

void DanmakuList::RecycleTopItems(int dy) {
    // 向下滚动（内容向上），dy 为负值
    size_t count = 0;
    auto bounds = GetLocalBounds();

    for (const auto& cur : visions_) {
        if (cur->GetBottom() + dy > bounds.y()) {
            break;
        }
        ++count;
    }

    for (size_t i = 0; i < count; ++i) {
        auto dv = static_cast<DanmakuVision*>(visions_[i]);
        dv->Invalid();
        if (recycled_listener_) {
            recycled_listener_->OnChildRecycled(dv);
        }
        recycler_.push_back(dv);
    }

    if (count != 0) {
        RemoveVisions(0, count);
    }
}

void DanmakuList::RecycleBottomItems(int dy) {
    // 向上滚动（内容向上），dy 为正值
    auto bounds = GetLocalBounds();
    auto target_vision_index = visions_.size();

    for (auto it = visions_.crbegin(); it != visions_.crend(); ++it) {
        if ((*it)->GetTop() + dy < bounds.bottom()) {
            break;
        }
        --target_vision_index;
    }

    for (size_t i = target_vision_index; i < visions_.size(); ++i) {
        auto dv = static_cast<DanmakuVision*>(visions_[i]);
        dv->Invalid();
        if (recycled_listener_) {
            recycled_listener_->OnChildRecycled(dv);
        }
        recycler_.push_back(dv);
    }

    if (target_vision_index != visions_.size()) {
        RemoveVisions(target_vision_index, visions_.size() - target_vision_index);
    }
}

}