#include "bililive_round_bubble.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "ui/views/painter.h"

void BililiveRoundBubbleView::ShowWindow(views::View* anchor_view, views::View* contents_view, 
	RoundBubbleType bubble_type/* = RoundBubbleType::Type_RectangleWhite*/)
{
	views::Widget* widget = new views::Widget();
	BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(widget);
	BililiveRoundBubbleView* view = new BililiveRoundBubbleView(contents_view,bubble_type);
	native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_NONE);
	native_widget->set_insure_show_corner(views::InsureShowCorner::ISC_BOTTOMRIGHT);
	views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW_FRAMELESS);
	params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;
	params.native_widget = native_widget;
	params.remove_standard_frame = true;
	params.remove_taskbar_icon = true;
	params.keep_on_top = true;
	params.parent = nullptr;
	ShowWidget(view, widget, params, true, false, views::InsureShowCorner::ISC_NONE);
	if (anchor_view)
	{
		gfx::Rect show_area = anchor_view->GetBoundsInScreen();
		gfx::Size view_size = view->GetPreferredSize();

		widget->SetBounds(gfx::Rect(
			show_area.x() + show_area.width() / 2 - view_size.width() / 2,
			show_area.y() - view_size.height() + view->GetPopupYPositionOffset(),
			view_size.width(), view_size.height()));
	}
}

BililiveRoundBubbleView::BililiveRoundBubbleView(views::View* contents_view, RoundBubbleType bubble_type):
	contents_view_(contents_view),bubble_type_(bubble_type)
{
	if (bubble_type_ == RoundBubbleType::Type_RectangleWhite)
	{
		int image_width = 363;
		int image_height = 126;
		int image_contents_width = 319;
		int image_contents_height = 78;
		float ratio_contents_width = (float)image_contents_width / (float)image_width;
		float ratio_contents_height = (float)image_contents_height / (float)image_height;
		float ratio_left = 22.0f / (float)image_width;
		float ratio_top = 16.0f / (float)image_height;
		layout_info_.contents_view_size = contents_view_ ? contents_view_->GetPreferredSize() :
			GetSizeByDPIScale(gfx::Size(323, 82));
		layout_info_.view_size.set_width((float)layout_info_.contents_view_size.width() / ratio_contents_width);
		layout_info_.view_size.set_height((float)layout_info_.contents_view_size.height() / ratio_contents_height);
		layout_info_.left_margin = (float)layout_info_.view_size.width() * ratio_left;// GetLengthByDPIScale(22);
		layout_info_.top_margin = (float)layout_info_.view_size.height() * ratio_top;// GetLengthByDPIScale(16);
		//layout_info_.right_margin = 33;//GetLengthByDPIScale(22);
		//layout_info_.bottom_margin = 50;// GetLengthByDPIScale(33);
		layout_info_.popup_y_position_offset = layout_info_.view_size.height() - layout_info_.top_margin -
			layout_info_.contents_view_size.height() - GetLengthByDPIScale(10);//GetLengthByDPIScale(20);
	}
	else if (bubble_type_ == RoundBubbleType::Type_SquareWhite || bubble_type_ == RoundBubbleType::Type_SquareBlack)
	{
		layout_info_.left_margin = GetLengthByDPIScale(20);
		layout_info_.top_margin = GetLengthByDPIScale(20);
		layout_info_.contents_view_size.set_width(GetLengthByDPIScale(92));
		layout_info_.contents_view_size.set_height(GetLengthByDPIScale(70));
		layout_info_.popup_y_position_offset = GetLengthByDPIScale(10);
	}
}

BililiveRoundBubbleView::~BililiveRoundBubbleView()
{

}

void BililiveRoundBubbleView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
	if (details.child == this)
	{
		if (details.is_add)
		{
			InitViews();
		}
	}
}

void BililiveRoundBubbleView::InitViews()
{
	set_background(views::Background::CreateSolidBackground(SkColorSetARGB(0, 0, 0, 0)));
	if (contents_view_)
	{
		this->AddChildView(contents_view_);
	}
}

gfx::Size BililiveRoundBubbleView::GetPreferredSize()
{
	return layout_info_.view_size;
}

scoped_ptr<views::Painter> g_shadow_border_painter;
void BililiveRoundBubbleView::OnPaintBackground(gfx::Canvas* canvas)
{
	__super::OnPaintBackground(canvas);

	const gfx::Rect& rect = GetContentsBounds();
	int resource_id = IDR_LIVEHIME_ROUND_ARROW_BG_RECTANGLE_WHITE;
	if (bubble_type_ == RoundBubbleType::Type_SquareWhite)
	{
		resource_id = IDR_LIVEHIME_ROUND_ARROW_BG_SQUARE_WHITE;
	}else if (bubble_type_ == RoundBubbleType::Type_SquareBlack)
	{
		resource_id = IDR_LIVEHIME_ROUND_ARROW_BG_SQUARE_BLACK;
	}
	//´øÈý½ÇµÄÔ²½Ç±³¾°Í¼Æ¬
	gfx::ImageSkia* shadow_image = GetImageSkiaNamed(resource_id);
	if (shadow_image)
	{
		canvas->DrawImageInt(*shadow_image, 0, 0, shadow_image->width(), shadow_image->height(),
			0, 0, rect.width(), rect.height(), true);
	}
}

void BililiveRoundBubbleView::Layout()
{
	if (contents_view_)
	{
		contents_view_->SetBounds(layout_info_.left_margin, layout_info_.top_margin,
			layout_info_.contents_view_size.width(), layout_info_.contents_view_size.height());
	}	
}

void BililiveRoundBubbleView::OnWidgetActivationChanged(views::Widget* widget, bool active)
{
	if (!active && GetWidget())
	{
		GetWidget()->Close();
		return;
	}
}

