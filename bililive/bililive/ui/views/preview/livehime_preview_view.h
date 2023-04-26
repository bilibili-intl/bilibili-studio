#ifndef BILILIVE_BILILIVE_UI_VIEWS_PREVIEW_PREVIEW_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_PREVIEW_PREVIEW_VIEW_H

#include "ui/views/view.h"
#include "ui/views/widget/widget_delegate.h"

#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/ui/views/preview/livehime_preview_area_utils.h"

class BililiveFloatingView;
class LivehimeOBSPreviewControl;

class LivehimePreviewView
    : public views::View
    , LivehimeLiveRoomObserver
{
    struct FloatingViewInfo
    {
        BililiveFloatingView* floating_view = nullptr;

        int current_seque_id = -1;
    };

public:
    LivehimePreviewView();
    virtual ~LivehimePreviewView();

    bool SetFloatingViewContent(livehime::PreviewFocusLessFloatingWidgetHierarchy hierarchy, 
        views::View* new_content_view, int seque_id);
    void HideFloatingView(livehime::PreviewFocusLessFloatingWidgetHierarchy hierarchy, 
        int seque_id, bool delete_content_view);

    bool SetAcceptEventFloatingViewContent(livehime::PreviewAcceptEventFloatingWidgetHierarchy hierarchy,
        views::View* new_content_view, int seque_id, void* floating_view_delegate = nullptr);
    void HideAcceptEventFloatingView(livehime::PreviewAcceptEventFloatingWidgetHierarchy hierarchy,
        int seque_id, bool delete_content_view);

protected:
    // View
    void ViewHierarchyChanged(
        const ViewHierarchyChangedDetails &details) override;
    void Layout() override;

    // LivehimeLiveRoomObserver
    void OnEnterIntoThirdPartyStreamingMode() override;
    void OnLeaveThirdPartyStreamingMode() override;

private:
    void InitViews();

private:
    LivehimeOBSPreviewControl* obs_preview_control_ = nullptr;
    FloatingViewInfo focus_less_floating_views_[livehime::PFLFWH_Count];
    FloatingViewInfo accept_event_floating_views_[livehime::PAEFWH_Count];

    base::WeakPtrFactory<LivehimePreviewView> weak_ptr_;
    DISALLOW_COPY_AND_ASSIGN(LivehimePreviewView);
};


class PreviewProjectorView
    : public views::WidgetDelegateView
{
public:
    static void ShowForm(gfx::Rect);

    LivehimeOBSPreviewControl *obs_preview_control(){ return obs_preview_control_; }

protected:
    //views::WidgetDelegate
    virtual views::View* GetContentsView() override{ return this; }

    //views::View
    virtual void ViewHierarchyChanged(const View::ViewHierarchyChangedDetails& details) override;

private:
    PreviewProjectorView();
    virtual ~PreviewProjectorView();

    void InitViews();
    void ShowWidget();

private:
    LivehimeOBSPreviewControl *obs_preview_control_;

    DISALLOW_COPY_AND_ASSIGN(PreviewProjectorView);
};

#endif