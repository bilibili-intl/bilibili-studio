#include <cmath>

#include "ui/gfx/skia_util.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/controls/label.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive_roundrect_view.h"

using namespace views;

namespace bililive
{
	RoundRectView::RoundRectView()
		: fillColor_(0), borderColor_(SkColorSetRGB(0, 0, 0)), radius_(6)
	{
	}

	RoundRectView::~RoundRectView()
	{
	}

	void RoundRectView::OnPaint(gfx::Canvas* canvas)
	{
		int w = width();
		int h = height();

		SkPaint paint;

		paint.setStrokeWidth(0);
		paint.setAntiAlias(true);

		if (SkColorGetA(fillColor_) > 0)
		{
			paint.setColor(fillColor_); //±³¾°ÑÕÉ«
			paint.setStyle(SkPaint::kFill_Style);
			canvas->DrawRoundRect(gfx::Rect(0, 0, w, h), radius_, paint);
		}

		paint.setColor(borderColor_); //Ïß¿òÑÕÉ«
		bililive::DrawRoundRect(canvas, 0, 0, w, h, radius_, paint);
	}

	void RoundRectView::SetBorderColor(SkColor borderColor)
	{
		borderColor_ = borderColor;
		SchedulePaint();
	}

	void RoundRectView::SetBackgroundColor(SkColor fillColor)
	{
		fillColor_ = fillColor;
		SchedulePaint();
	}

	void RoundRectView::SetCornerRadius(int radius)
	{
		radius_ = radius;
		SchedulePaint();
	}

	SkColor RoundRectView::GetBorderColor() const
	{
		return borderColor_;
	}

	SkColor RoundRectView::GetBackgroundColor() const
	{
		return fillColor_;
	}

	int RoundRectView::GetCornerRadius() const
	{
		return radius_;
	}

	gfx::Rect RoundRectView::GetMaxRect() const
	{
		return GetRoundRectInnerRect(width(), height(), GetCornerRadius());
	}
};
