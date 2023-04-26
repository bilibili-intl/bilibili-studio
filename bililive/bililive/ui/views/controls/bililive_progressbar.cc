#include "bililive_progressbar.h"

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_number_conversions.h"

#include "ui/gfx/canvas.h"

#include "third_party/skia/include/core/SkPaint.h"

#include "ui/base/resource/resource_bundle.h"
#include "grit/theme_resources.h"
#include "grit/ui_resources.h"

#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"


BililiveProgressBar::BililiveProgressBar()
    : is_set_details_(false)
    , bar_height_(0)
    , bat_wight_(0)
{
    progress_img_ = GetImageSkiaNamed(IDR_LIVEHIME_PROGRESS_MINITV);
}

void BililiveProgressBar::SetBarDetails(int cy, SkColor clrFull, SkColor clrEmpty)
{
    is_set_details_ = true;
    bar_height_ = cy;
    full_color_ = clrFull;
    empty_color_ = clrEmpty;
}

void BililiveProgressBar::SetBarDetails(int cw, int ch, SkColor clrFull, SkColor clrEmpty)
{
    is_set_details_ = true;
    bar_height_ = ch;
    bat_wight_ = cw;
    full_color_ = clrFull;
    empty_color_ = clrEmpty;
}

void BililiveProgressBar::OnPaint(gfx::Canvas* canvas)
{
    if (is_set_details_)
    {
        int cy = std::min(bar_height_, height());
        int y = (height() - cy) / 2 + 1;

        int wh = width();
        int width = bat_wight_ == 0 ? wh : bat_wight_;
        // »­±³¾°
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kFill_Style);
        paint.setFlags(SkPaint::kAntiAlias_Flag);
        paint.setColor(empty_color_);
        bililive::DrawCircleRect(canvas, 0, y, width, cy, paint);

        double capped_value = std::min(
            std::max(current_value_, min_display_value_), max_display_value_);
        double full_value = capped_value / max_display_value_;
        paint.setColor(full_color_);
        bililive::DrawCircleRect(canvas, 0, y, width * full_value, cy, paint);

        if (bat_wight_ != 0) {
            int start_pos = width * full_value - GetLengthByDPIScale(6);
            if (start_pos < 6) {
                start_pos = 0;
            }
            canvas->DrawImageInt(
                *progress_img_,
                start_pos,
                0);

            gfx::Rect text_bounds(width + GetLengthByDPIScale(12), 0, GetLengthByDPIScale(30), GetLengthByDPIScale(16));

            int value = int(full_value * 100);
            std::string str = std::to_string(value);
            str.append("%");
            canvas->DrawStringInt(base::UTF8ToUTF16(str), ftTen, SkColorSetARGB(255, 255, 255, 255), text_bounds);
        }
    }
    else
    {
        __super::OnPaint(canvas);
    }
}
