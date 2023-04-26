#include "round_rect_background.h"

#include "ui/gfx/canvas.h"
#include "ui/gfx/skia_util.h"

RoundRectBackground::RoundRectBackground(int radius, int corner, SkColor color) :
    radius_(radius),
    fillColor_(color),
    corner_(static_cast<BackgroundCornerType>(corner))
{
    SetNativeControlColor(color);
}

void RoundRectBackground::Paint(gfx::Canvas* canvas, views::View* view) const
{
    int w = view->width();
    int h = view->height();

    SkPaint paint;

    paint.setStrokeWidth(0);
    paint.setAntiAlias(true);

    paint.setColor(fillColor_);
    paint.setStyle(SkPaint::kFill_Style);
    canvas->DrawRoundRect(gfx::Rect(0, 0, w, h), radius_, paint);

    BackgroundCornerType square_corner = static_cast<BackgroundCornerType>(BackgroundCornerType::ALL - corner_);

    if (square_corner & TOP_LEFT)
    {
        canvas->FillRect(gfx::Rect(0, 0, radius_, radius_), fillColor_);
    }

    if (square_corner & TOP_RIGHT && w >= radius_)
    {
        canvas->FillRect(gfx::Rect(w - radius_, 0, radius_, radius_), fillColor_);
    }

    if (square_corner & BOTTOM_LEFT && h >= radius_)
    {
        canvas->FillRect(gfx::Rect(0, h - radius_, radius_, radius_), fillColor_);
    }

    if (square_corner & BOTTOM_RIGHT && h >= radius_ && w >= radius_)
    {
        canvas->FillRect(gfx::Rect(w - radius_, h - radius_, radius_, radius_), fillColor_);
    }
}
