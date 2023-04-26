#include "bililive/bililive/ui/views/livehime/settings/join_slide_tips_view.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"

#include "ui/views/layout/grid_layout.h"

namespace
{
    JoinSlideTipsView* g_single_instance = nullptr;

    enum ButtonId
    {
        ID_OK_BUTTON = 1,
        ID_CANCEL_BUTTON
    };

}   // namespace


void JoinSlideTipsView::ShowForm(views::Widget* parent,
    const EndDialogSignalHandler* handler)
{
    if (!g_single_instance)
    {
        views::Widget* widget = new views::Widget();
        views::Widget::InitParams params;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.native_widget = new BililiveNativeWidgetWin(widget);
        params.delegate = g_single_instance;
        if (parent)
        {
            params.parent = parent->GetNativeWindow();
        }

        g_single_instance = new JoinSlideTipsView();
        BililiveWidgetDelegate::DoModalWidget(g_single_instance, widget, params, handler);
    }
    else
    {
        if (g_single_instance->GetWidget())
        {
            g_single_instance->GetWidget()->Activate();
        }
    }
}

// BililiveLivePartitionMainView
JoinSlideTipsView::JoinSlideTipsView()
    : BililiveWidgetDelegate(gfx::ImageSkia(), L"沉浸式体验")
{
}

JoinSlideTipsView::~JoinSlideTipsView()
{
    g_single_instance = nullptr;
}

void JoinSlideTipsView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
    if (details.is_add && details.child == this)
    {
        views::GridLayout* layout = new views::GridLayout(this);
        SetLayoutManager(layout);

        views::ColumnSet* column_set = layout->AddColumnSet(0);
        column_set->AddPaddingColumn(1.0f, 0);
        column_set->AddColumn(views::GridLayout::CENTER, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(1.0f, 0);

        column_set = layout->AddColumnSet(1);
        column_set->AddPaddingColumn(1.0f, 0);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForActionButton);
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(1.0f, 0);

        BililiveLabel* label1 = new LivehimeSmallContentLabel(L"温馨提示：该选项将在近期下线，");
        label1->SetFont(ftFourteen);

        BililiveLabel* label2 = new LivehimeSmallContentLabel(L"后续开播模式统一为新版直播间");
        label2->SetFont(ftFourteen);


        btn_ok_ = new LivehimeActionLabelButton(this, L"确定", false);
        btn_ok_->set_id(ID_OK_BUTTON);
        btn_ok_->set_focusable(false);
        
        btn_cancel_ = new LivehimeActionLabelButton(this, L"取消", true);
        btn_cancel_->set_id(ID_CANCEL_BUTTON);
        btn_cancel_->set_focusable(false);

        layout->AddPaddingRow(0, kPaddingRowHeightEnds);
        layout->StartRow(0, 0);
        layout->AddView(label1);
        
        layout->StartRow(0, 0);
        layout->AddView(label2);

        layout->AddPaddingRow(1.0f, 0);
        layout->StartRow(0, 1);
        layout->AddView(btn_ok_);
        layout->AddView(btn_cancel_);

        layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    }
}

void JoinSlideTipsView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    switch (sender->id())
    {
    case ID_OK_BUTTON:
        SetResultCode(IDOK);
        GetWidget()->Close();
        break;
    case ID_CANCEL_BUTTON:
        SetResultCode(IDCANCEL);
        GetWidget()->Close();
        break;
    default:
        break;
    }
}

gfx::Size JoinSlideTipsView::GetPreferredSize()
{
    return gfx::Size(GetLengthByDPIScale(399), GetLengthByDPIScale(130));
}

void JoinSlideTipsView::WindowClosing()
{
    if (!GetResultCode()) {
        SetResultCode(IDCANCEL);
    }
}