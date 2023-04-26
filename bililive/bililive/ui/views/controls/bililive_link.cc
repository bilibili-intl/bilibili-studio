
#include "bililive_link.h"
#include "bililive/bililive/ui/views/controls/bililive_theme_common.h"

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/gfx/canvas.h"
#include "ui/base/resource/resource_bundle.h"

#include "third_party/skia/include/core/SkPaint.h"

#include "grit/theme_resources.h"



void BililiveLink::SetTextColor(SkColor clr)
{
    SetEnabledColor(clr);
    SetPressedColor(clr);
}

void BililiveLink::PaintText(gfx::Canvas* canvas,
    const string16& text,
    const gfx::Rect& text_bounds,
    int flags)
{
    gfx::Rect text_rect = text_bounds;
    if (vertical_alignment_ == gfx::ALIGN_TOP)
    {
        text_rect.set_origin(gfx::Point(text_bounds.x(), 0));
    }
    else if (vertical_alignment_ == gfx::ALIGN_BOTTOM)
    {
        text_rect.set_origin(gfx::Point(text_bounds.x(), GetLocalBounds().bottom() - text_bounds.height()));
    }
    __super::PaintText(canvas, text, text_rect, flags | gfx::Canvas::NO_SUBPIXEL_RENDERING);
}
