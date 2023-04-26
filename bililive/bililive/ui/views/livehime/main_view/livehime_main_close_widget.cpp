#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_close_widget.h"

#include "bililive/bililive/livehime/main_view/livehime_main_close_pref_constants.h"
#include "bililive/bililive/livehime/main_view/livehime_main_close_presenter_impl.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
//#include "bililive/bililive/livehime/melee/melee_ui_presenter.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"

#include "grit/generated_resources.h"

void LivehimeMainCloseView::DoModal(gfx::NativeView par, const EndDialogSignalHandler* handler)
{
    views::Widget *widget = new views::Widget();
    BililiveNativeWidgetWin *native_widget = new BililiveNativeWidgetWin(widget);
    native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_CLOSE);
    views::Widget::InitParams params;
    params.native_widget = native_widget;
    params.parent = par;
    params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
    //return BililiveWidgetDelegate::ShowViewModelOnWidget(new LivehimeMainCloseView(), widget_, params);
    BililiveWidgetDelegate::DoModalWidget(new LivehimeMainCloseView(), widget, params, handler);
}

LivehimeMainCloseView::LivehimeMainCloseView()
    : BililiveWidgetDelegate(gfx::ImageSkia(),
      ui::ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_MAIN_CLOSE_TITLE)),
      min_radio_button_(nullptr),
      exit_radio_botton_(nullptr),
      remember_choice_(nullptr),
      ok_button_(nullptr),
      cancel_button_(nullptr),
      radio_button_groupid_(0),
      presenter_(std::make_unique<LivehimeClosePresenterImpl>())
{
}

LivehimeMainCloseView::~LivehimeMainCloseView()
{}

void LivehimeMainCloseView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitView();
            InitData();
        }
    }
}

gfx::Size LivehimeMainCloseView::GetPreferredSize()
{
    gfx::Size size = views::View::GetPreferredSize();

    size.SetToMax(gfx::Size(358, 0));

    return size;
}

void LivehimeMainCloseView::InitView()
{
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet *column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);

    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    column_set = layout->AddColumnSet(2);
    column_set->AddPaddingColumn(1.0f, 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForActionButton);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(1.0f, 0);

    LivehimeTitleLabel *label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_MAIN_CLOSE_LABEL_TITLE));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    layout->StartRow(0, 0);
    layout->AddView(label);

    ++radio_button_groupid_;

    min_radio_button_ = new LivehimeRadioButton(rb.GetLocalizedString(IDS_MAIN_CLOSE_MIN), radio_button_groupid_);
    exit_radio_botton_ = new LivehimeRadioButton(rb.GetLocalizedString(IDS_MAIN_CLOSE_EXIT), radio_button_groupid_);

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 1);
    layout->AddView(min_radio_button_);
    layout->AddView(exit_radio_botton_);

    remember_choice_ = new LivehimeCheckbox(rb.GetLocalizedString(IDS_MAIN_CLOSE_REMEMBER));
    label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(remember_choice_);

    ok_button_ = new LivehimeActionLabelButton(this, rb.GetLocalizedString(IDS_SURE), true);
    ok_button_->SetIsDefault(true);
    cancel_button_ = new LivehimeActionLabelButton(this, rb.GetLocalizedString(IDS_CANCEL), false);

    layout->AddPaddingRow(0, kPaddingRowHeightForDiffGroups);
    layout->StartRow(0, 2);
    layout->AddView(ok_button_);
    layout->AddView(cancel_button_);

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
}

void LivehimeMainCloseView::InitData()
{
    int exit_style = presenter_->GetExitStyle();

    if (exit_style == prefs::kMinimizeToTray)
    {
        min_radio_button_->SetChecked(true);
    }
    else if (exit_style == prefs::kExitImmediately)
    {
        exit_radio_botton_->SetChecked(true);
    }

    remember_choice_->SetChecked(presenter_->GetIsRemember());
}

void LivehimeMainCloseView::SaveData()
{
    int exit_style = prefs::kMinimizeToTray;
    if (exit_radio_botton_->checked())
    {
        exit_style = prefs::kExitImmediately;
    }

    presenter_->SaveExitStyle(exit_style);

    presenter_->SaveIsRemember(remember_choice_->checked());
}

void LivehimeMainCloseView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (ok_button_ == sender)
    {
        SaveData();
        SetResultCode(IDOK);
        GetWidget()->Close();
    }
    else
    {
        SetResultCode(IDCANCEL);
        GetWidget()->Close();
    }
}