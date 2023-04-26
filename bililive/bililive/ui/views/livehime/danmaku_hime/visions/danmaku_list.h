#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISION_DANMAKU_LIST_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISION_DANMAKU_LIST_H_

#include <memory>

#include "base/timer/timer.h"

#include "ui/base/events/event.h"

#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/vision_group.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/nav_down_button.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/overlay_scroll_bar.h"


class ScrollingInterpolator;

namespace dmkhime {

enum class DmkHitType;
class DanmakuVision;
class DanmakuListAdapter;
class OverlayScrollBar;
class DmkDataObject;


class ListItemRecycledListener {
public:
    virtual ~ListItemRecycledListener() = default;

    virtual void OnChildRecycled(DanmakuVision* vision) = 0;
};

class ListStatusChangeListener {
public:
    virtual ~ListStatusChangeListener() = default;

    virtual void OnListAnimationStart() = 0;
    virtual void OnListAutoScrollStateChanged(bool auto_scroll) = 0;
};


// 线性列表布局，支持回收。
// 列表数据放在 ListAdapter 中，由 Notify* 相关方法通知列表数据更改
class DanmakuList : public VisionGroup {
public:
    explicit DanmakuList(bool can_interactive = true, float speed_factor = 1.f);
    ~DanmakuList();

    bool IsClickable(const gfx::Point& p) const;
    bool OnMouseWheel(const ui::MouseWheelEvent& event);
    bool OnMousePressed(
        const ui::MouseEvent& event, DmkHitType* type, const DmkDataObject** data);
    void OnMouseReleased(
        const ui::MouseEvent& event, DmkHitType* type, const DmkDataObject** data);
    void OnMouseCaptureLost();
    void OnMouseMoved(const ui::MouseEvent& event);
    bool OnMouseDragged(const ui::MouseEvent& event);

    void SetAdapter(DanmakuListAdapter* adapter);
    void SetRecycleListener(ListItemRecycledListener* listener);
    void SetStatusChangeListener(ListStatusChangeListener* listener);

    void SetNavDownText(const string16& text);

    /**
     * 通知列表：有新数据项添加至数据集尾部。
     * 如果最后一个子 Vision 距离底部有空隙，一些与新添加数据项对应的
     * 子 Vision 会被加入，直到空隙填满。填满后将什么都不做。
     */
    void NotifyItemAddToBottom();

    /**
     * 通知列表：有数据项从数据集顶部移除。
     * 将更新子 Vision 的索引。如果存在与被移除索引对应的子 Vision，将移除对应 Vision。
     * 如果移除后顶部出现空隙，整体将上移，并填补上移后底部的空隙（如果有的话）。
     */
    void NotifyItemRemoveFromTop(int length);

    /**
     * 通知列表：数据集发生了未知改变。
     * 将从当前位置重新布局对应数据索引的子 Vision。
     */
    void NotifyDataSetChanged();

    /**
     * 移动至最后一项，使其完全显示。
     */
    void ScrollToBottom(bool smooth = true);
    void ScrollToBottomIfAtBottom(bool smooth = true);

    bool IsAnimating() const;
    bool IsAutoScrollToBottom() const;

    bool OnComputeScroll(int interval);

    void Refresh();

    void UpdateOverlayScrollBar();

protected:
    // VisionGroup
    void OnMeasure(int width, int height) override;
    void OnSizeChanged(int width, int height, int old_w, int old_h) override;
    void OnLayout(int left, int top, int right, int bottom) override;
    void OnDrawOver(Sculptor* s) override;

private:
    void OnScroll(int dy, OverlayScrollBar::Direction dir);
    void OnAutoScrollStateChanged();
    void OnAnimationStart();
    void OnHidingScrollBar();

    void FillItems(int pos, int offset);
    void ScrollToPosition(int pos, int offset = 0);
    void SmoothScrollToPosition(int pos, int offset = 0);
    DanmakuVision* MakeNewDanmakuVision(int pos);

    int DetermineScroll(int dy);
    void RecordCurPositionAndOffset();
    void ComputeTotalHeight(int* prev, int* next, int* to_bottom) const;

    void AwakeScrollBar(bool preparing_to_sleep);

    bool IsAtBottom(int dy) const;

    /**
     * 估计从当前位置到底部的距离。
     * 如果所有项高度完全相同，则估计的值是准确的。
     */
    int GetCurToBottomDistance() const;
    DanmakuVision* GetFirstVision() const;
    DanmakuVision* GetLastVision() const;
    DanmakuVision* GetFirstVisibleVision() const;
    DanmakuVision* GetLastVisibleVision() const;

    /**
     * 假定将内容（即子 Vision）整体下移 dy。
     * 顶部可能会出现空隙，将使用对应的数据索引具现化出 Vision 来填补（如果有）。
     * 如果在假定下移 dy 的过程中触顶，将返回实际可移动的距离，否则返回 dy。
     */
    int FillTopSpace(int dy);

    /**
     * 假定将内容（即子 Vision）整体上移 dy。
     * 底部可能会出现空隙，将使用对应的数据索引具现化出 Vision 来填补（如果有）。
     * 如果在假定上移 dy 的过程中触底，将返回实际可移动的距离，否则返回 dy。
     */
    int FillBottomSpace(int dy);

    /**
     * 假定将内容（即子 Vision）整体上移 dy 时，
     * 回收完全超出顶部的 Vision。
     */
    void RecycleTopItems(int dy);

    /**
     * 假定将内容（即子 Vision）整体下移 dy 时，
     * 回收完全超出底部的 Vision。
     */
    void RecycleBottomItems(int dy);

    int cur_position_;
    int cur_offset_in_position_;

    float scrolling_scale_ = 1;
    bool is_at_bottom_ = false;
    bool auto_scroll_to_bottom_ = true;
    ListStatusChangeListener* s_listener_ = nullptr;

    bool can_interactive_;
    bool is_pressed_on_nav_down_ = false;
    bool is_pressed_on_scrollbar_ = false;

    NavDownButton nav_down_button_;
    OverlayScrollBar scroll_bar_;
    std::vector<DanmakuVision*> recycler_;
    std::unique_ptr<DanmakuListAdapter> adapter_;

    base::OneShotTimer<DanmakuList> scrollbar_timer_;
    ListItemRecycledListener* recycled_listener_ = nullptr;
    std::unique_ptr<ScrollingInterpolator> interpolator_;
};

}

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISION_DANMAKU_LIST_H_