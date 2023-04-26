#include "bililive/bililive/ui/views/viddup/danmaku_hime/danmaku_merged_main_viddup_view.h"

#include "base/strings/utf_string_conversions.h"
#include "base/strings/stringprintf.h"

#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"

#include "bililive/bililive/livehime/danmaku_hime/danmakuhime_data_handler.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/controls/dmkhime_toast_view.h"
#include "bililive/bililive/ui/views/viddup/danmaku_hime/danmaku_interaction_viddup_view.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/secret/public/event_tracking_service.h"
#include "bililive/bililive/ui/views/livehime/settings/settings_widget.h"


const int DanmakuMergedMainViddupView::kDragHandleHeight = GetLengthByDPIScale(2);

class DanmakuMergedIntputTextViewLayout : public views::LayoutManager
{
public:
    DanmakuMergedIntputTextViewLayout()
    {
    }

    virtual ~DanmakuMergedIntputTextViewLayout()
    {
    }

    virtual void Layout(views::View* host) override
    {
        DCHECK(host->child_count() == 2);
        if (host->child_count() != 2)
            return;

        static const int edit_len = GetLengthByDPIScale(199);
        static const int edit_height = GetLengthByDPIScale(32);
        static const int button_y = GetLengthByDPIScale(4);
        static const int button_width = GetLengthByDPIScale(78);
        static const int button_height = GetLengthByDPIScale(26);

        auto edit_view = host->child_at(0);
        auto button_view = host->child_at(1);

        auto bounds = host->GetContentsBounds();

        edit_view->SetBounds(
            bounds.x(), bounds.y(), edit_len, edit_height);
        button_view->SetBounds(
            bounds.x() + edit_len, bounds.y() + button_y, button_width, button_height);
    }

    virtual gfx::Size GetPreferredSize(views::View* host) override
    {
        static const int view_len = GetLengthByDPIScale(280);
        static const int edit_height = GetLengthByDPIScale(34);
        return gfx::Size(view_len, edit_height);
    }

private:
    DISALLOW_COPY_AND_ASSIGN(DanmakuMergedIntputTextViewLayout);
};

DanmakuMergedMainViddupView::DanmakuMergedMainViddupView(DanmakuhimeDataHandler* danmaku_data_handler) :
    danmaku_data_handler_(danmaku_data_handler)
{
    InitViews();
}

DanmakuMergedMainViddupView::~DanmakuMergedMainViddupView()
{
}

void DanmakuMergedMainViddupView::AttachInteractionView(DanmakuInteractionViddupView* interaction_view)
{
    if (interaction_view_ == interaction_view)
        return;

    if (interaction_view)
    {
        auto parent = interaction_view->parent();
        if (parent)
        {
            parent->RemoveChildView(interaction_view);
        }

        AddChildView(interaction_view);
        interaction_view_ = interaction_view;
    }
    else 
    {
        if (interaction_view_) 
        {
            RemoveChildView(interaction_view_);
            interaction_view_ = nullptr;
        }
    }

    Layout();
}

DanmakuInteractionViddupView* DanmakuMergedMainViddupView::GetInteractionView()
{
    return interaction_view_;
}


void DanmakuMergedMainViddupView::Layout()
{
    auto content_bounds = GetContentsBounds();
    int drag_pos1 = GetDragHandlePos(0);
    int drag_pos2 = GetDragHandlePos(1);
    int y{}, height{};

    if (child_count() == 1) { 
        views::View* child_view = nullptr;
        height = content_bounds.height();
/*        if (activity_task_view_){
            child_view = activity_task_view_; 
            height = dmkhime::kActivityTaskViewPopMinHeight;
        }
        else if (gift_view_) {
            child_view = gift_view_;
        }
        else */if (interaction_view_) {
            child_view = interaction_view_;
        }

        child_view->SetBounds(content_bounds.x(), content_bounds.y(), content_bounds.width(), height);
    }   

    if (interaction_view_) 
    {
        interaction_view_->RelocationMessageLoopView();
    }    
}

void DanmakuMergedMainViddupView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    __super::ViewHierarchyChanged(details);

    if (details.child == interaction_view_ && !details.is_add)
    {
        interaction_view_ = nullptr;
    }

}

void DanmakuMergedMainViddupView::OnBoundsChanged(const gfx::Rect& previous_bounds)
{
    static bool first_enter{true};

    if (first_enter) 
    {
        InitDragPos();
        first_enter = false;
    }
}

gfx::NativeCursor DanmakuMergedMainViddupView::GetCursor(const ui::MouseEvent& event)
{
    if (child_count() > 1) 
    {
        return ::LoadCursor(nullptr, IDC_SIZENS);
    }
    else 
    {
        return ::LoadCursor(nullptr, IDC_ARROW);
    }
}

bool DanmakuMergedMainViddupView::OnMousePressed(const ui::MouseEvent& event)
{
    mouse_pressed_ = DragHandleHitTest(event.location());
    if (mouse_pressed_ != MousePressedType::kNone) 
    {
        mouse_hit_pt_ = event.location();
        return true;
    }

    return false;
}

bool DanmakuMergedMainViddupView::OnMouseDragged(const ui::MouseEvent& event)
{
    if (mouse_pressed_ != MousePressedType::kNone)
    {
        auto last_pt = event.location();

        switch (mouse_pressed_)
        {
            case MousePressedType::kDragPos1:
                SetDragHandlePos(0, last_pt.y());
            break;

            case MousePressedType::kDragPos2:
                SetDragHandlePos(1, last_pt.y());
            break;
        }

        Layout();
        
        return true;
    }

    return false;
}

void DanmakuMergedMainViddupView::OnMouseReleased(const ui::MouseEvent& event)
{
    mouse_pressed_ = MousePressedType::kNone;
}

views::View* DanmakuMergedMainViddupView::GetEventHandlerForPoint(const gfx::Point& point)
{
    if (DragHandleHitTest(point) != MousePressedType::kNone) 
    {
        return this;
    }
    else 
    {
        return views::View::GetEventHandlerForPoint(point);
    }
}

void DanmakuMergedMainViddupView::InitDragPos()
{
    SetDragHandlePos(0, dmkhime::kActivityTaskViewPopMinHeight);

    //drag_handle_pos_[0] = ActivityAndTaskView::kMergeMiniHeight;
    auto content_bounds = GetContentsBounds();
    auto height = std::max(content_bounds.height() - dmkhime::kActivityTaskViewPopMinHeight - kDragHandleHeight, 1);

    SetDragHandlePos(1, height/2);

    //drag_handle_pos_[1] = height / 2.0;
}

void DanmakuMergedMainViddupView::InitViews()
{
    set_background(views::Background::CreateSolidBackground(0xE6, 0xE6, 0xE6, 255));
}

int DanmakuMergedMainViddupView::GetDragHandlePos(int index)
{
    if (index < 0 || index > 1) 
    {
        return 0;
    }

    return drag_handle_pos_[index];
}

void DanmakuMergedMainViddupView::SetDragHandlePos(int index, int pos)
{
    if (index < 0 || index > 1)
    {
        return;
    }

    switch (index)
    {
        case 0:
        {
            int new_pos = std::min(std::max(pos, dmkhime::kActivityTaskViewPopMinHeight), dmkhime::kActivityTaskViewPopMinHeight);
            int diff = new_pos - drag_handle_pos_[0];
            drag_handle_pos_[0] = new_pos;
            SetDragHandlePos(1, GetDragHandlePos(1) + diff);
        }
        break;

        case 1:
        {

        }
        break;
    }
}

gfx::Rect DanmakuMergedMainViddupView::GetDragHandleRect(int index)
{
    int pos = GetDragHandlePos(index);
    return gfx::Rect(0, pos - kDragHandleHeight, bounds().width(), 3 * kDragHandleHeight);
}

DanmakuMergedMainViddupView::MousePressedType DanmakuMergedMainViddupView::DragHandleHitTest(const gfx::Point& pt)
{
    if (child_count() >= 2)
    {
        for (int i = 0; i < 2; i++)
        {
            if (GetDragHandleRect(i).Contains(pt))
            {
                return static_cast<MousePressedType>(i);
            }
        }
    }
    else 
    {
        return MousePressedType::kNone;
    }

    return MousePressedType::kNone;
}

void DanmakuMergedMainViddupView::SetRoomId(int64_t id)
{
}
