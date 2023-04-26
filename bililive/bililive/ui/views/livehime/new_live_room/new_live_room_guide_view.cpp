#include "new_live_room_guide_view.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/ext/callable_callback.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/grid_layout.h"

//#include "bililive/bililive/livehime/rank_popup_window/rank_popup_window_controller.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"


using namespace bililive;

namespace
{
    const int kMinItemHeigth = GetLengthByDPIScale(100);

    enum ButtonId
    {
        ID_OK_BUTTON = 1,
        ID_CANCEL_BUTTON
    };

    NewLiveRoomGuideWindow* g_single_instance = nullptr;
}

void NewLiveRoomGuideWindow::ShowLayerWindow(livehime::NotifyUICloseClosure layer_ui_close_callback)
{
    if (!g_single_instance)
    {
        LOG(INFO) << "start show window.";

        g_single_instance = new NewLiveRoomGuideWindow(layer_ui_close_callback);
        if (!GetBililiveProcess()->bililive_obs() || !GetBililiveProcess()->bililive_obs()->obs_view())
        {
            g_single_instance->WindowClosing();
            delete g_single_instance;
            LOG(WARNING) << "try to show ui when app closing.";
            return;
        }

        views::Widget *parent = GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget();

        views::Widget *widget = new views::Widget();
        views::Widget::InitParams params;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.native_widget = new BililiveNativeWidgetWin(widget);
        params.parent = parent->GetNativeView();

        DoModalWidget(g_single_instance, widget, params);
    }
    else
    {
        if (g_single_instance->GetWidget())
        {
            g_single_instance->GetWidget()->Activate();
        }
    }
}

NewLiveRoomGuideWindow::NewLiveRoomGuideWindow(livehime::NotifyUICloseClosure layer_ui_close_callback)
    : layer_ui_close_callback_(layer_ui_close_callback)
    , weakptr_factory_(this)
{
    g_single_instance = this;

    InitViews();
}

NewLiveRoomGuideWindow::~NewLiveRoomGuideWindow()
{
    g_single_instance = nullptr;

}

views::NonClientFrameView* NewLiveRoomGuideWindow::CreateNonClientFrameView(views::Widget *widget)
{
    auto frame_view = new BililiveNonTitleBarFrameView(nullptr);
    frame_view->SetEnableDragMove(true);
    return frame_view;
}

void NewLiveRoomGuideWindow::InitViews()
{
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    auto columnset = layout->AddColumnSet(0);
    columnset->AddPaddingColumn(1.0f, 0);
    columnset->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    columnset->AddPaddingColumn(1.0f, 0);

    columnset = layout->AddColumnSet(1);
    columnset->AddPaddingColumn(0, GetLengthByDPIScale(60));
    columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
    columnset->AddPaddingColumn(0, GetLengthByDPIScale(24));
    columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
    columnset->AddPaddingColumn(0, GetLengthByDPIScale(60));

    svga_img_view_ = new LivehimeSVGAImageView();
    svga_img_view_->setPreferredSize(GetSizeByDPIScale(gfx::Size(196, 228)));
    svga_img_view_->setLoopCount(-1);
    svga_img_view_->setSVGAResourceId(IDR_LIVEMAIN_NEW_ROOM_GUIDE);

    btn_ok_ = new LivehimeActionLabelButton(this, L"马上开启", true);
    btn_ok_->set_id(ID_OK_BUTTON);
    btn_ok_->set_focusable(false);

    btn_cancel_ = new LivehimeActionLabelButton(this, L"再想想", false);
    btn_cancel_->set_id(ID_CANCEL_BUTTON);
    btn_cancel_->set_focusable(false);

    BililiveLabel* title_label = new LivehimeTitleLabel(L"新版直播间全新沉浸体验");
    title_label->SetFont(ftSixteenBold);

    BililiveLabel* label = new LivehimeSmallContentLabel(L"温馨提示：近期开播模式将统一为新版直播间");
    label->SetFont(ftFourteen);

    layout->AddPaddingRow(0, GetLengthByDPIScale(30));
    layout->StartRow(0, 0);
    layout->AddView(svga_img_view_);
 
    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    layout->StartRow(0, 0);
    layout->AddView(title_label);

    layout->AddPaddingRow(0, kPaddingRowHeightForCtrlTips);
    layout->StartRow(0, 0);
    layout->AddView(label);

    layout->AddPaddingRow(1.0f, 0);
    layout->StartRow(0, 1);
    layout->AddView(btn_cancel_);
    layout->AddView(btn_ok_);
    layout->AddPaddingRow(0, GetLengthByDPIScale(12));
}

gfx::Size NewLiveRoomGuideWindow::GetPreferredSize()
{
    return gfx::Size(GetLengthByDPIScale(329), GetLengthByDPIScale(381));
}

void NewLiveRoomGuideWindow::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (sender == btn_ok_) {
        //livehime::RankPopupWindowController::GetInstance()->JoinSlide();
    }

    GetWidget()->Close();
}

void NewLiveRoomGuideWindow::WindowClosing()
{
    LOG(INFO) << "window closing.";

    if (!layer_ui_close_callback_.is_null())
    {
        layer_ui_close_callback_.Run();
    }
}

void NewLiveRoomGuideWindow::OnPaint(gfx::Canvas* canvas)
{
    __super::OnPaint(canvas);

    static gfx::ImageSkia* img = GetImageSkiaNamed(IDR_LIVEHIME_NEW_LIVEROOM_GUIDE_FRAME);
    auto svage_bounds = svga_img_view_->bounds();
    int x = svage_bounds.x() - GetLengthByDPIScale(5);
    int y = svage_bounds.y() - GetLengthByDPIScale(22);
    canvas->DrawImageInt(*img, x, y);
}
