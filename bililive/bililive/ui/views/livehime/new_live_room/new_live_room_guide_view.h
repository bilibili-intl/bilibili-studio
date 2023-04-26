#pragma once

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"

#include "ui/views/view.h"
#include "ui/views/controls/button/button.h"

#include "bililive/bililive/livehime/notify_ui_control/notify_ui_control_type.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_svga_image_view.h"


class BililiveLabelButton;

class NewLiveRoomGuideWindow
    : BililiveWidgetDelegate,
    views::ButtonListener
{
public:
    static void ShowLayerWindow(livehime::NotifyUICloseClosure layer_ui_close_callback);

protected:
    // WidgetDelegate
    ui::ModalType GetModalType() const override { return ui::MODAL_TYPE_WINDOW; }
    views::NonClientFrameView* CreateNonClientFrameView(views::Widget *widget) override;
    void WindowClosing() override;

    // View
    gfx::Size GetPreferredSize() override;
    void OnPaint(gfx::Canvas* canvas) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

private:
    NewLiveRoomGuideWindow(livehime::NotifyUICloseClosure layer_ui_close_callback);
    ~NewLiveRoomGuideWindow();

    void InitViews();

private:
    LivehimeSVGAImageView* svga_img_view_ = nullptr;
    BililiveLabelButton* btn_ok_ = nullptr;
    BililiveLabelButton* btn_cancel_ = nullptr;
    livehime::NotifyUICloseClosure layer_ui_close_callback_;
    base::WeakPtrFactory<NewLiveRoomGuideWindow> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(NewLiveRoomGuideWindow);
};
