#include "bililive/bililive/ui/views/login/livehime_login_message_box.h"

#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/views/layout/grid_layout.h"

namespace
{
    enum CtrlID
    {
        Radio_Record = 1,
        Radio_Directory,

        Button_Sure,
        Button_Cancel,
    };
}

void LivehimeLoginDeleteAccountView::DoModal(const secret::LoginInfo& account_info, 
    gfx::NativeView par, const EndDialogSignalHandler* handler)
{
    views::Widget *widget = new views::Widget();
    BililiveNativeWidgetWin *native_widget = new BililiveNativeWidgetWin(widget);
    native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_CLOSE);
    views::Widget::InitParams params;
    params.native_widget = native_widget;
    params.parent = par;
    params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
    BililiveWidgetDelegate::DoModalWidget(
        new LivehimeLoginDeleteAccountView(account_info), widget, params, handler);
}

LivehimeLoginDeleteAccountView::LivehimeLoginDeleteAccountView(const secret::LoginInfo& account_info)
    : BililiveWidgetDelegate(gfx::ImageSkia(), L"删除账号")
    , account_info_(account_info)
{
}

LivehimeLoginDeleteAccountView::~LivehimeLoginDeleteAccountView()
{}

void LivehimeLoginDeleteAccountView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitView();
        }
    }
}

gfx::Size LivehimeLoginDeleteAccountView::GetPreferredSize()
{
    static int cx = GetLengthByDPIScale(370);
    return gfx::Size(cx, GetHeightForWidth(cx));
}

void LivehimeLoginDeleteAccountView::InitView()
{
    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet *column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForActionButton);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);

    LivehimeTitleLabel *title_label = new LivehimeTitleLabel(
        base::StringPrintf(L"删除账号：%ls(%lld)", base::UTF8ToUTF16(account_info_.name).c_str(), account_info_.mid));
    title_label->SetHorizontalAlignment(gfx::ALIGN_LEFT);

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    layout->StartRow(0, 0);
    layout->AddView(title_label);

    radio_button_groupid_ = LivehimeRadioButton::new_groupid();

    just_record_radio_button_ = new LivehimeRadioButton(L"从历史登录列表中删除此账号", radio_button_groupid_);
    just_record_radio_button_->SetChecked(true);
    just_record_radio_button_->set_id(Radio_Record);

    all_info_radio_botton_ = new LivehimeRadioButton(L"删除此账号的所有记录文件", radio_button_groupid_);
    all_info_radio_botton_->set_id(Radio_Directory);

    LivehimeTipLabel* tip_label = new LivehimeTipLabel(L"此操作会删除该账号的所有记录文件(包括场景配置、软件设置历史弹幕等)");
    tip_label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    tip_label->SetMultiLine(true);
    tip_label->SetAllowCharacterBreak(true);

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(just_record_radio_button_);

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(all_info_radio_botton_);

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(tip_label, 1, 1, views::GridLayout::FILL, views::GridLayout::FILL, 0, 0);

    ok_button_ = new LivehimeActionLabelButton(this, GetLocalizedString(IDS_SURE), true);
    ok_button_->SetIsDefault(true);
    ok_button_->set_id(Button_Sure);

    cancel_button_ = new LivehimeActionLabelButton(this, GetLocalizedString(IDS_CANCEL), false);
    cancel_button_->set_id(Button_Cancel);

    layout->AddPaddingRow(0, kPaddingRowHeightForDiffGroups);
    layout->StartRow(0, 1);
    layout->AddView(ok_button_);
    layout->AddView(cancel_button_);

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
}

void LivehimeLoginDeleteAccountView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    switch (sender->id())
    {
    case Button_Sure:
        {
            int ret = just_record_radio_button_->checked() ? OptionType::DelRecord : OptionType::DelDirectory;
            SetResultCode(ret);
            GetWidget()->Close();
        }
        break;
    case Button_Cancel:
        {
            SetResultCode(OptionType::Cancel);
            GetWidget()->Close();
        }
        break;
    default:
        break;
    }
}