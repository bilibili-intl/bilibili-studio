#include "colive_splitter_view.h"

#include "ui/gfx/canvas.h"


ColiveSplitterView::ColiveSplitterView(int thickness, SkColor color)
    : thickness_(thickness),
      color_(color)
{
    paint_.setColor(color);
    paint_.setAntiAlias(true);
    paint_.setStrokeWidth(thickness);
}

void ColiveSplitterView::SetColor(SkColor color) {
    color_ = color;
    paint_.setColor(color);
    SchedulePaint();
}

gfx::Size ColiveSplitterView::GetPreferredSize() {
    auto size = super::GetPreferredSize();
    size.set_height(thickness_);
    return size;
}

void ColiveSplitterView::OnPaint(gfx::Canvas* canvas) {
    auto bounds = GetContentsBounds();
    canvas->DrawLine(bounds.origin(), bounds.top_right(), paint_);
}