#include "bililive/bililive/ui/views/toolbar/live_auto_replay_dialog.h"

#include "base/strings/stringprintf.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_hotkey_notify_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"

namespace{

    void DonotCarePublishLiveReplayResult(bool valid_response, int code, const std::string& err_msg)
    {
        //不关心是否成功
    }
}

// static
LiveAutoReplayDialog* LiveAutoReplayDialog::instance_ = nullptr;

// static
void LiveAutoReplayDialog::ShowWindow(views::Widget* parent)
{
    views::Widget* widget = new views::Widget();

    views::Widget::InitParams params;
    params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
    params.native_widget = new BililiveNativeWidgetWin(widget);
    params.parent = parent->GetNativeView();

    instance_ = new LiveAutoReplayDialog();
    DoModalWidget(instance_, widget, params);
}

LiveAutoReplayDialog::LiveAutoReplayDialog()
    : BililiveWidgetDelegate(gfx::ImageSkia(),
        ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_STOP_DIALOG_MSGBOX_DF_TITTLE))
{
    InitViews();
}

LiveAutoReplayDialog::~LiveAutoReplayDialog()
{
    instance_ = nullptr;
}

void LiveAutoReplayDialog::InitViews()
{
    auto& rb = ResourceBundle::GetSharedInstance();
    auto layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    auto col = layout->AddColumnSet(0);
    col->AddPaddingColumn(1.0f,0);
    col->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    col->AddPaddingColumn(0, kPaddingColWidthForActionButton);
    col->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    col->AddPaddingColumn(1.0f, 0);

    col = layout->AddColumnSet(1);
    col->AddPaddingColumn(1.0f, 0);
    col->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    col->AddPaddingColumn(1.0f, 0);

    auto label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_STOP_DIALOG_MSGBOX_DF_TEXT));
    label->SetFont(ftFourteen);
    label->SetTextColor(GetColor(LabelTitle));

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightEnds + kPaddingRowHeightForCtrlTips);
    layout->AddView(label);

    ok_button_ = new LivehimeActionLabelButton(this, rb.GetLocalizedString(IDS_STOP_DIALOG_MSGBOX_DF_CONFIRM), true);
    ok_button_->SetIsDefault(true);
    ok_button_->SetSize(gfx::Size(GetLengthByDPIScale(80), GetLengthByDPIScale(30)));
    cancel_button_ = new LivehimeActionLabelButton(this, rb.GetLocalizedString(IDS_STOP_DIALOG_MSGBOX_MORE_THINK), false);
    cancel_button_->SetSize(gfx::Size(GetLengthByDPIScale(119), GetLengthByDPIScale(32)));

    layout->StartRowWithPadding(1.0f, 0, 0, kPaddingColWidthEndsSmall);
    layout->AddView(cancel_button_);
    layout->AddView(ok_button_);
    layout->AddPaddingRow(0, kPaddingColWidthEndsSmall);
}

gfx::Size LiveAutoReplayDialog::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.set_width(GetLengthByDPIScale(270));
    size.set_height(GetLengthByDPIScale(126));
    return size;
}

void LiveAutoReplayDialog::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (sender == ok_button_)
    {
        livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::LiveStopReplayDlgClick, "button_type:1");
    }
    else
    {
        livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::LiveStopReplayDlgClick, "button_type:0");
    }

    GetWidget()->Close();
}
