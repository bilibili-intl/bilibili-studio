#include "bililive/bililive/ui/views/viddup/main_view/stretch_control_viddup_view.h"

#include "bilibase/basic_types.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "bililive/bililive/ui/views/controls/bililive_bubble.h"
#include "ui/gfx/screen.h"
#include "bililive/secret/public/event_tracking_service.h"

namespace {
	views::Widget* g_widget_single_instance_ = nullptr;
}

void PopupLeftToolViddupView::ShowForm(views::View* anchor_view, views::View* content_view, views::View* detect_view)
{
	if (!g_widget_single_instance_)
	{
		views::Widget* widget = new views::Widget();
		widget->set_frame_type(views::Widget::FRAME_TYPE_FORCE_CUSTOM);
		views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
		params.remove_standard_frame = true;
		auto popup_view = new PopupLeftToolViddupView(content_view);
		params.delegate = popup_view;
		params.remove_taskbar_icon = true;
		params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
		widget->Init(params);
		g_widget_single_instance_ = widget;
	}
	if (g_widget_single_instance_)
	{
		gfx::Rect show_area = anchor_view->GetBoundsInScreen();
		gfx::Size view_size = detect_view->GetBoundsInScreen().size();
		if (view_size.width() < 100)
		{
			view_size.set_width(content_view->GetPreferredSize().width());
		}

		g_widget_single_instance_->SetBounds(gfx::Rect(
			show_area.x() + show_area.width(),
			show_area.y() + show_area.height() / 2 - view_size.height() / 2,
			view_size.width(), view_size.height()));
		g_widget_single_instance_->Show();
		g_widget_single_instance_->Activate();
	}
}

void PopupLeftToolViddupView::HideForm()
{
	if (g_widget_single_instance_)
	{
		g_widget_single_instance_->Hide();
	}
}

void PopupLeftToolViddupView::CloseForm()
{
	if (g_widget_single_instance_)
	{
		g_widget_single_instance_->Close();
		g_widget_single_instance_ = nullptr;
	}
}

views::Widget* PopupLeftToolViddupView::GetWidgetSingleInstance()
{
	return g_widget_single_instance_;
}

PopupLeftToolViddupView::PopupLeftToolViddupView(views::View* content_view)
{
	set_background(views::Background::CreateSolidBackground(GetColor(WindowClient)));
	SetLayoutManager(new views::FillLayout());
	AddChildView(content_view);
}

PopupLeftToolViddupView::~PopupLeftToolViddupView()
{
}

StretchControlViddupView::StretchImageButton::StretchImageButton(StretchControlViddupView* parent_view):BililiveImageButton(parent_view), parent_view_(parent_view)
{
}

void StretchControlViddupView::StretchImageButton::StateChanged()
{
	//parent_view_->ImageButtonStateChanged(state() == Button::STATE_HOVERED);
}

StretchControlViddupView::StretchControlViddupView(views::View* main_view, views::View* tabarea_view, views::View* tabarea_dummy_view):
	main_view_(main_view), tabarea_view_(tabarea_view), tabarea_dummy_view_(tabarea_dummy_view)
{
}

StretchControlViddupView::~StretchControlViddupView()
{
}

void StretchControlViddupView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitView();
        }
    }
}

void StretchControlViddupView::InitView()
{
		set_background(views::Background::CreateSolidBackground(GetColor(WindowClient)));
}

void StretchControlViddupView::OnStretchButtonCanClicked()
{
		is_button_can_clicked_ = true;
}

void StretchControlViddupView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
}

void StretchControlViddupView::ImageButtonStateChanged(bool is_hovered)
{
}

void StretchControlViddupView::Quit()
{
	main_view_ = nullptr;
	tabarea_view_ = nullptr;
	tabarea_dummy_view_ = nullptr;
}

void StretchControlViddupView::StartDoCheckMouse()
{
}

void StretchControlViddupView::StopDoCheckMouse()
{
}

void StretchControlViddupView::OnCheckMouse()
{
}