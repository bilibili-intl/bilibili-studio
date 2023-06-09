#ifndef BILI_CANVAS_DRAWER_H_
#define BILI_CANVAS_DRAWER_H_

#include "ui/gfx/rect.h"
#include "SkColor.h"
#include "SkBitmap.h"

#include <vector>

namespace gfx { class Canvas; class ImageSkia; class Size; };
class SkPaint;
class SkPath;

namespace bililive
{
    gfx::Rect GetRoundRectInnerRect(int w, int h, int radius);
    void DrawRoundRect(gfx::Canvas* canvas, int x, int y, int w, int h, int radius, SkPaint& paint);
    void FillRoundRect(gfx::Canvas* canvas, int x, int y, int w, int h, int radius, SkPaint& paint);

    void DrawCircleRect(gfx::Canvas* canvas, int x, int y, int cx, int cy, SkPaint& paint,
                        bool is_horiz = true, SkPath* ret_path = nullptr);

    gfx::ImageSkia CreateScaleBitmap(const gfx::ImageSkia& sourceImage, const gfx::Size& targetSize, bool fill = false);
    gfx::ImageSkia CreateScaleBitmap(const SkBitmap& sourceBitmap, const gfx::Size& targetSize, bool fill = false);
};

#endif
