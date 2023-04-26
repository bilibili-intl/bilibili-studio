#ifndef BILILIVE_BILILIVE_UI_VIEWS_TABAREA_TABAREA_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_TABAREA_TABAREA_VIEW_H

#include "ui/views/view.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/widget/widget_observer.h"

#include "bililive/bililive/livehime/live_model/live_model_controller.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"

class BililiveLabel;
class BililiveImageButton;
class TabAreaTabStripView;
class TabAreaScenesView;
class BililiveBubbleView;
class LivehimeTitleLabel;
class LivehimeSwitchModeButton;

class TabAreaView
    : public views::View
    , views::ButtonListener
    , LivehimeLiveRoomObserver
    , LiveModelControllerObserver
    , views::WidgetObserver
    , base::NotificationObserver
{
public:
    TabAreaView();
    virtual ~TabAreaView();

    // 获取中间区域，所谓中间区域，即：
    // 侧栏上部“上上下下移动场景项按钮的顶边”到侧栏下部“历史弹幕页中功能弹窗按钮的顶边”直接的区域

    void ShowOperateView(const std::string& name,bool show);

    gfx::Size GetPreferredSize() override;

    void OpenChatRoomView();

    void CloseChatRoomView();

protected:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) override;

    void OnPaintBackground(gfx::Canvas* canvas) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // WidgetObserver
    void OnWidgetClosing(views::Widget* widget) override;

    // NotificationObserver
    void Observe(int type,
        const base::NotificationSource& source,
        const base::NotificationDetails& details) override;

    // LivehimeLiveRoomObserver
    void OnEnterIntoThirdPartyStreamingMode() override;
    void OnLeaveThirdPartyStreamingMode() override;

    // LiveModelControllerObserver
    void OnLiveLayoutModelChanged(bool user_invoke) override;

private:
    void InitViews();
    void ShowModelButtonGuideBubble();

private:
    base::NotificationRegistrar notifation_register_;

    BililiveLabel* model_label_ = nullptr;
    //BililiveImageButton* model_button_ = nullptr;
    TabAreaScenesView* scenes_view_ = nullptr;
    //LivehimeTitleLabel* direction_label_ = nullptr;
    LivehimeSwitchModeButton* switch_button_ = nullptr;


    BililiveBubbleView* model_button_guide_bubble_ = nullptr;
    views::View* mode_view_ = nullptr;

    base::WeakPtrFactory<TabAreaView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(TabAreaView);

};

#endif