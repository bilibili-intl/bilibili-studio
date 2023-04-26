#include "stretch_control_view.h"
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

void PopupLeftToolView::ShowForm(views::View* anchor_view, views::View* content_view, views::View* detect_view)
{
	if (!g_widget_single_instance_)
	{
		views::Widget* widget = new views::Widget();
		widget->set_frame_type(views::Widget::FRAME_TYPE_FORCE_CUSTOM);
		views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
		params.remove_standard_frame = true;
		auto popup_view = new PopupLeftToolView(content_view);
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

void PopupLeftToolView::HideForm()
{
	if (g_widget_single_instance_)
	{
		g_widget_single_instance_->Hide();
	}
}

void PopupLeftToolView::CloseForm()
{
	if (g_widget_single_instance_)
	{
		g_widget_single_instance_->Close();
		g_widget_single_instance_ = nullptr;
	}
}

views::Widget* PopupLeftToolView::GetWidgetSingleInstance()
{
	return g_widget_single_instance_;
}

PopupLeftToolView::PopupLeftToolView(views::View* content_view)
{
	set_background(views::Background::CreateSolidBackground(SkColorSetRGB(221, 225, 228)));
	SetLayoutManager(new views::FillLayout());
	AddChildView(content_view);
}

PopupLeftToolView::~PopupLeftToolView()
{

}



StretchControlView::StretchImageButton::StretchImageButton(StretchControlView* parent_view):BililiveImageButton(parent_view), parent_view_(parent_view)
{

}

void StretchControlView::StretchImageButton::StateChanged()
{
	parent_view_->ImageButtonStateChanged(state() == Button::STATE_HOVERED);
}



StretchControlView::StretchControlView(views::View* main_view, views::View* tabarea_view, views::View* tabarea_dummy_view):
	main_view_(main_view), tabarea_view_(tabarea_view), tabarea_dummy_view_(tabarea_dummy_view)
{

}

StretchControlView::~StretchControlView()
{

}

void StretchControlView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitView();
        }
    }
}

void StretchControlView::InitView()
{
	set_background(views::Background::CreateSolidBackground(SkColorSetRGB(221, 225, 228)));//SK_ColorWHITE
	int column_set_index = 0;
	views::GridLayout* layout = new views::GridLayout(this);
	SetLayoutManager(layout);

	auto column_set = layout->AddColumnSet(column_set_index);
	column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

	stretch_button_ = new StretchImageButton(this);
	stretch_button_->SetAllStateImage(GetImageSkiaNamed(IDR_LIVEHIME_V4_MAIN_VIEW_LEFT_TOOL_VIEW_HIDE));

	layout->AddPaddingRow(1.0f, GetLengthByDPIScale(20));
	layout->StartRow(0, column_set_index);
	layout->AddView(stretch_button_);
	layout->AddPaddingRow(1.0f, GetLengthByDPIScale(20));

	can_clicked_timer_.Start(FROM_HERE, base::TimeDelta::FromSeconds(10), this, &StretchControlView::OnStretchButtonCanClicked);
}

void StretchControlView::OnStretchButtonCanClicked()
{
	is_button_can_clicked_ = true;
}

void StretchControlView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
	if (sender == stretch_button_ && is_button_can_clicked_)
	{
		is_stretch_ = !is_stretch_;
		stretch_button_->SetAllStateImage(is_stretch_ ? GetImageSkiaNamed(IDR_LIVEHIME_V4_MAIN_VIEW_LEFT_TOOL_VIEW_HIDE) :
				GetImageSkiaNamed(IDR_LIVEHIME_V4_MAIN_VIEW_LEFT_TOOL_VIEW_SHOW));

		if (main_view_ && tabarea_view_ && tabarea_dummy_view_)
		{
			if (is_stretch_)//主界面显示
			{
				StopDoCheckMouse();
				if (tabarea_view_->parent())
				{
					tabarea_view_->parent()->RemoveChildViewCancelNotifications(tabarea_view_);//从PopupLeftToolView里移除
				}
				tabarea_dummy_view_->AddChildView(tabarea_view_);
				tabarea_dummy_view_->SetVisible(true);
				main_view_->Layout();
				PopupLeftToolView::CloseForm();
			}
			else//主界面隐藏
			{
				tabarea_dummy_view_->RemoveChildViewCancelNotifications(tabarea_view_);
				tabarea_dummy_view_->SetVisible(false);
				PopupLeftToolView::ShowForm(stretch_button_, tabarea_view_, tabarea_dummy_view_);
				PopupLeftToolView::HideForm();
				main_view_->Layout();
			}
		}

		//埋点上报
		base::StringPairs data;
		std::pair<std::string, std::string> pair1("switch_type", is_stretch_ ? "1" : "0");
		data.push_back(pair1);
		livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::LeftToolbarCollapseClick, data);
	}
}

void StretchControlView::ImageButtonStateChanged(bool is_hovered)
{
	if (!is_stretch_ && main_view_ && tabarea_view_ && tabarea_dummy_view_)
	{
		if (is_hovered && PopupLeftToolView::GetWidgetSingleInstance())
		{
			PopupLeftToolView::ShowForm(stretch_button_, tabarea_view_, tabarea_dummy_view_);
			StartDoCheckMouse();
		}
	}
}

void StretchControlView::Quit()
{
	main_view_ = nullptr;
	tabarea_view_ = nullptr;
	tabarea_dummy_view_ = nullptr;
}

void StretchControlView::StartDoCheckMouse()
{
	timer_.Start(FROM_HERE, base::TimeDelta::FromMilliseconds(100),
		this, &StretchControlView::OnCheckMouse);
}

void StretchControlView::StopDoCheckMouse()
{
	if (timer_.IsRunning())
		timer_.Stop();
}

void StretchControlView::OnCheckMouse()
{
	if (PopupLeftToolView::GetWidgetSingleInstance() && PopupLeftToolView::GetWidgetSingleInstance()->IsVisible())
	{
		gfx::Rect rcWidget = PopupLeftToolView::GetWidgetSingleInstance()->GetWindowBoundsInScreen();
		gfx::Rect rcRelate = stretch_button_->GetBoundsInScreen();
		if (rcRelate.right() < rcWidget.x())
		{
			rcRelate.Inset(0, 0, rcRelate.right() - rcWidget.x(),0);
		}
		static const int kExtendDetectThickness = GetLengthByDPIScale(20);
		rcWidget.Inset(-kExtendDetectThickness, -kExtendDetectThickness);
		gfx::Point cursor_pos = gfx::Screen::GetNativeScreen()->GetCursorScreenPoint();
		if (!rcWidget.Contains(cursor_pos) && !rcRelate.Contains(cursor_pos))
		{
			StopDoCheckMouse();
			PopupLeftToolView::HideForm();
		};
	}
}

