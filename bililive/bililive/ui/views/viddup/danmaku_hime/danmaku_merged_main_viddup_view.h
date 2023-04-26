#ifndef BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_DANMAKU_HIME_DANMAKU_MERGED_MAIN_VIEW
#define BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_DANMAKU_HIME_DANMAKU_MERGED_MAIN_VIEW

#include "ui/views/view.h"

#include "bililive/bililive/livehime/danmaku_hime/danmaku_hime_pref_service.h"
#include "bililive/bililive/ui/views/controls/browser/livehime_web_browser_view_osr.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_structs.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"

class DanmakuInteractionViddupView;
class DanmakuhimeDataHandler;
class MarkednessMessageView;
class LivehimeNativeEditView;

class DanmakuMergedMainViddupView:
    public views::View
{
public:
    DanmakuMergedMainViddupView(DanmakuhimeDataHandler* danmaku_data_handler);
    ~DanmakuMergedMainViddupView();

    void AttachInteractionView(DanmakuInteractionViddupView* interaction_view);

    DanmakuInteractionViddupView* GetInteractionView();

    //void OnHERAudience(int64_t count);

    void Layout() override;
    void SetRoomId(int64_t id);

protected:
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    void OnBoundsChanged(const gfx::Rect& previous_bounds) override;
    gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override;
    bool OnMousePressed(const ui::MouseEvent& event) override;
    bool OnMouseDragged(const ui::MouseEvent& event) override;
    void OnMouseReleased(const ui::MouseEvent& event) override;
    views::View* GetEventHandlerForPoint(const gfx::Point& point) override;

private:
    enum class MousePressedType { kNone = -1, kDragPos1 = 0, kDragPos2 = 1 };

    void InitDragPos();
    void InitViews();

    int GetDragHandlePos(int index);
    void SetDragHandlePos(int index, int pos);
    gfx::Rect GetDragHandleRect(int index);
    MousePressedType DragHandleHitTest(const gfx::Point& pt);

    static const int                    kDragHandleHeight;

    DanmakuhimeDataHandler*             danmaku_data_handler_ = nullptr;

    MousePressedType                    mouse_pressed_ = MousePressedType::kNone;
    gfx::Point                          mouse_hit_pt_;

    DanmakuInteractionViddupView*       interaction_view_ = nullptr;
    int                                 drag_handle_pos_[2] = {};
};

#endif  //BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_DANMAKU_HIME_DANMAKU_MERGED_MAIN_VIEW

