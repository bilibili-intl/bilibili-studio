#pragma once

#include "ui/views/animation/bounds_animator.h"
#include "ui/views/view.h"

#include "base/timer/timer.h"

#include "bililive/bililive/ui/views/controls/bililive_label.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/super_chat/markedness_message_theme_common.h"

#include "bililive/public/secret/bililive_secret.h"

class DanmakuHimeFrameView;
class SuperChatNewItemView;

class MarkednessMessageView
    : public views::View
    , public ui::AnimationDelegate
{
public:
    using SuperChatData = secret::LiveStreamingService::MarkednessMessageInfo;

    enum ShowType { //展示形式，左边展示完整或者右边展示完整
        LeftSideComplete = 0,
        RightSideComplete,
    };

    explicit MarkednessMessageView(views::View* parent_view);
    ~MarkednessMessageView();

    void AddNewItem(const SuperChatData& danmaku);

    // views::View:
    void OnBoundsChanged(const gfx::Rect& previous_bounds) override;
    void Layout() override;
    void PaintChildren(gfx::Canvas* canvas) override;
    views::View* GetEventHandlerForPoint(const gfx::Point& point) override;
    bool OnMouseWheel(const ui::MouseWheelEvent& event) override;
    void OnMouseEntered(const ui::MouseEvent& event) override;
    void OnMouseExited(const ui::MouseEvent& event) override;
    bool OnMousePressed(const ui::MouseEvent& event) override;
    void OnMouseReleased(const ui::MouseEvent& event) override;

    void RefreshView();
    void RefreshShadow(int64_t sel_id);
    void CloseDetailsView();
    void SetNoSubpixelRendering(bool no_subpixel_rendering);

private:
    enum class NewItemStatus {
        // 当前没有新增动效
        None,
        // 正在播放进入动画
        In,
        // 暂停1秒
        Paused,
        // 正在播放退出动画
        Out,
    };

    void InitViews();
    void UpdateData();
    void UpdateArrow();
    void ArrowLeftPressed();
    void ArrowRightPressed();

    void DoInAnimation();
    void DoOutAnimation();
    void OnPauseTimer();
    void UpdateWhenAnimation(bool animation_show);

    // public ui::AnimationDelegate
    void AnimationCanceled(const ui::Animation* animation) override {}
    void AnimationEnded(const ui::Animation* animation) override;

    bool left_arrow_visible_ = false;
    bool right_arrow_visible_ = false;

    bool animation_show_ = false;

    views::View* box_view_ = nullptr;
    SuperChatNewItemView* new_item_view_ = nullptr;

    gfx::Rect left_bounds_;
    gfx::Rect right_bounds_;

    gfx::ImageSkia* arrow_left_img_ = nullptr;
    gfx::ImageSkia* arrow_right_img_ = nullptr;

    ShowType show_type_ = ShowType::LeftSideComplete;
    views::View* parent_view_ = nullptr;

    NewItemStatus new_item_status_ = NewItemStatus::None;
    views::BoundsAnimator new_item_animator_;
    std::queue<SuperChatData> new_item_queue_;
    base::OneShotTimer<MarkednessMessageView> timer_;

    DISALLOW_COPY_AND_ASSIGN(MarkednessMessageView);
};