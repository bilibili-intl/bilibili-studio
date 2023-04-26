#include "refcounted_canvas.h"

RefCountedCanvas::RefCountedCanvas()
{

}

RefCountedCanvas::RefCountedCanvas(const gfx::Size& size,
    ui::ScaleFactor scale_factor,
    bool is_opaque)
    : gfx::Canvas(size, scale_factor, is_opaque)
{

}

RefCountedCanvas::RefCountedCanvas(const gfx::ImageSkiaRep& image_rep, bool is_opaque)
    : gfx::Canvas(image_rep, is_opaque)
{

}