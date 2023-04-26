#include "bililive/bililive/ui/views/livehime/third_party_streaming/tps_break_waitting_view.h"

#include "base/file_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "base/win/registry.h"

#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/common/bililive_context.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/dialog/shell_dialogs.h"
#include "bililive/bililive/ui/views/controls/bililive_floating_scroll_view.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_hotkey_notify_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/public/common/bililive_constants.h"

namespace
{
    enum
    {
        Button_Stop = 1,
        Button_Wait,
    };

    TpsBreakWaittingView* g_single_instance = nullptr;
}


void TpsBreakWaittingView::DoModal(gfx::NativeView parent, const EndDialogSignalHandler* handler)
{
    if (!g_single_instance)
    {
        views::Widget* widget = new views::Widget();

        views::Widget::InitParams params;
        params.parent = parent;

        g_single_instance = new TpsBreakWaittingView();
        DoModalWidget(g_single_instance, widget, params, handler);
    }
    else
    {
        g_single_instance->GetWidget()->Activate();
    }
}

void TpsBreakWaittingView::Close()
{
    // 如果当前不是处于模态顶层不要关，不然引发一系列窗口级联关闭问题
    if (g_single_instance)
    {
        LOG(INFO) << "tps force close waiting form";
        g_single_instance->closed_ = true;
        if (!g_single_instance->IsInModalNow())
        {
            g_single_instance->SetResultCode(-1);
            g_single_instance->GetWidget()->Close();
        }
        else
        {
            // 一秒一次检测，已然处于顶层窗口了就关掉了
            base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
                base::Bind(TpsBreakWaittingView::Close), base::TimeDelta::FromSeconds(1));
        }
    }
}

TpsBreakWaittingView::TpsBreakWaittingView()
    : BililiveWidgetDelegate(gfx::ImageSkia(), GetLocalizedString(IDS_LIVE_ROOM_MSGBOX_TITLE))
    , weakptr_factory_(this)
{
}

TpsBreakWaittingView::~TpsBreakWaittingView()
{
    g_single_instance = nullptr;
}

void TpsBreakWaittingView::WindowClosing()
{
    LivehimeLiveRoomController::GetInstance()->RemoveObserver(this);
}

void TpsBreakWaittingView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();

            LivehimeLiveRoomController::GetInstance()->AddObserver(this);
        }
        else
        {
            LivehimeLiveRoomController::GetInstance()->RemoveObserver(this);
        }
    }
}

void TpsBreakWaittingView::InitViews()
{
    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet *column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForActionButton);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddPaddingColumn(1.0f, 0);

    label_ = new LivehimeContentLabel();
    label_->SetMultiLine(true);

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    layout->StartRow(0, 0);
    layout->AddView(label_);

    stop_button_ = new LivehimeActionLabelButton(this, GetLocalizedString(IDS_TOOLBAR_STOP_LIVE), true);
    stop_button_->set_id(Button_Stop);

    wait_button_ = new LivehimeActionLabelButton(this, L"继续等待", false);
    wait_button_->set_id(Button_Wait);

    layout->AddPaddingRow(0, kPaddingRowHeightForDiffGroups);
    layout->StartRow(0, 1);
    layout->AddView(stop_button_);
    layout->AddView(wait_button_);

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);

    OnThirdPartyStreamingBreakCountDown(60);
}

gfx::Size TpsBreakWaittingView::GetPreferredSize()
{
    const static int label_width = kMessageBoxWidth - kPaddingColWidthEndsSmall * 2;
    int label_height = label_->GetHeightForWidth(label_width);
    int cy = kPaddingRowHeightEnds * 2 + kPaddingRowHeightForDiffGroups +
        LivehimeActionLabelButton::GetButtonSize().height() + label_height;
    return gfx::Size(kMessageBoxWidth, cy);
}

void TpsBreakWaittingView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (sender->id() == Button_Stop)
    {
        if (!closed_)
        {
            SetResultCode(1);
        }
    }
    GetWidget()->Close();
}

void TpsBreakWaittingView::OnThirdPartyStreamingBreakCountDown(int sec)
{
    auto str = base::StringPrintf(L"检测到视频流已断开，您可以在%2d秒内在第三方工具里重新推流，"
        L"超时将自动停止直播并退出第三方推流模式。\n或者是否立刻停止直播？", sec);
    label_->SetText(str);
}
