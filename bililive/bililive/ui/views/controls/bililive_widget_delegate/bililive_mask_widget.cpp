#include "bililive_mask_widget.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_shadow_frame_view.h"
#include "bililive/bililive/ui/views/uploader/bililive_uploader_type.h"

#include "base/run_loop.h"

#include "ui/base/win/dpi.h"
#include "ui/views/focus/accelerator_handler.h"
#include "ui/views/controls/messagebox/message_box.h"


BililiveMaskWidget* BililiveMaskWidget::ShowMaskWidget(HWND parWnd)
{
    RECT rect;
    ::GetWindowRect(parWnd, &rect);
    BililiveMaskWidget *mask_widget = new BililiveMaskWidget();

    views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
    params.parent = parWnd;
    if (params.parent)
    {
        HWND hPopup = ::GetLastActivePopup(params.parent);
        if (hPopup)
        {
            views::Widget *top_widget = views::Widget::GetWidgetForNativeWindow(hPopup);
            if (top_widget)
            {
                params.parent = hPopup;
            }
        }
    }
    params.remove_standard_frame = true;
    params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;
    params.bounds = gfx::Rect(rect);
    params.can_activate = false;
    params.accept_events = false;
    mask_widget->set_frame_type(views::Widget::FRAME_TYPE_FORCE_CUSTOM);
    mask_widget->Init(params);
    mask_widget->ShowInactive();

    mask_widget->GetRootView()->set_background(views::Background::CreateSolidBackground(SK_ColorGRAY));
    mask_widget->SetOpacity(128);

    return mask_widget;
}

BililiveMaskWidget::~BililiveMaskWidget()
{
}

BililiveMaskWidget::BililiveMaskWidget()
{
}
