#include "bililive/bililive/ui/views/livehime/streaming_diagrams/streaming_warning_notify_view.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/screen.h"
#include "ui/views/widget/widget.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/fill_layout.h"

#include "bililive/bililive/ui/views/controls/bililive_floating_scroll_view.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

namespace {
    StreamingWarningNotifyView* g_single_instance = nullptr;
}

void StreamingWarningNotifyView::ShowWindow(const std::wstring& info)
{
    if (!g_single_instance)
    {
        views::Widget *widget = new views::Widget();

        views::Widget::InitParams params;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.native_widget = new BililiveNativeWidgetWin(widget);
        params.can_activate = false;
        g_single_instance = new StreamingWarningNotifyView();
        g_single_instance->info_ = info;
        ShowWidget(g_single_instance, widget, params,false);

        MoveDialog(widget);

        widget->ShowInactive();
    }
    else
    {
        g_single_instance->AddInfo(info);
    }
}


StreamingWarningNotifyView::StreamingWarningNotifyView()
    :weak_ptr_factory_(this)
{}

StreamingWarningNotifyView::~StreamingWarningNotifyView()
{
    g_single_instance = nullptr;
}

void StreamingWarningNotifyView::InitViews()
{
    BililiveViewWithFloatingScrollbar* scroll_view = new BililiveViewWithFloatingScrollbar();
    SetLayoutManager(new views::FillLayout());
    AddChildView(scroll_view->Container());

    ResourceBundle &rb = ResourceBundle::GetSharedInstance();
    views::GridLayout *layout = new views::GridLayout(scroll_view);
    scroll_view->SetLayoutManager(layout);

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::LEADING, 1.0, views::GridLayout::FIXED, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEndsSmall);

    layout->AddPaddingRow(0, kPaddingRowHeightEnds);

    label_ = new LivehimeContentLabel(info_);
    label_->SetVerticalAlignment(gfx::ALIGN_VCENTER);
    label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    label_->SetAllowCharacterBreak(true);
    label_->SetMultiLine(true);

    layout->StartRow(0,0);
    layout->AddView(label_);
}

void StreamingWarningNotifyView::ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) {
    if (details.child == this) {
        if (details.is_add) {
            InitViews();
        }
    }
}

gfx::Size StreamingWarningNotifyView::GetPreferredSize()
{
    return gfx::Size(GetLengthByDPIScale(400), GetLengthByDPIScale(210));
}

string16 StreamingWarningNotifyView::GetWindowTitle() const
{
    return GetLocalizedString(IDS_STREAMING_WARNING_NOTIFY_LAGGING);
}


void StreamingWarningNotifyView::MoveDialog(views::Widget *widget)
{
    gfx::Display disp = gfx::Screen::GetNativeScreen()->GetPrimaryDisplay();
    gfx::Rect work_area = disp.work_area();
    gfx::Rect wnd_rect = widget->GetWindowBoundsInScreen();
    gfx::Point position;
    position.set_x(work_area.right() - wnd_rect.width() - GetLengthByDPIScale(10));
    position.set_y(work_area.bottom() - wnd_rect.height() - GetLengthByDPIScale(10));
    widget->SetBounds(gfx::Rect(position.x(), position.y(), wnd_rect.width(), wnd_rect.height()));
}

void StreamingWarningNotifyView::AddInfo(const std::wstring& info)
{
    info_ += L"\r\n";
    info_ += L"\r\n";

    info_ += info;

    label_->SetText(info_);
    PreferredSizeChanged();

    Layout();
}