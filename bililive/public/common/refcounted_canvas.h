#ifndef BILILIVE_PUBLIC_COMMON_REFCOUNTED_CANVAS_H
#define BILILIVE_PUBLIC_COMMON_REFCOUNTED_CANVAS_H


#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

#include "ui/gfx/canvas.h"


class RefCountedCanvas
    : public gfx::Canvas
    , public base::RefCountedThreadSafe<RefCountedCanvas>
{
public:
    // Creates an empty canvas with scale factor of 1x.
    RefCountedCanvas();

    // Creates canvas with provided DIP |size| and |scale_factor|.
    // If this canvas is not opaque, it's explicitly cleared to transparent before
    // being returned.
    RefCountedCanvas(const gfx::Size& size,
        ui::ScaleFactor scale_factor,
        bool is_opaque);

    // Constructs a canvas with the size and the scale factor of the
    // provided |image_rep|, and draws the |image_rep| into it.
    RefCountedCanvas(const gfx::ImageSkiaRep& image_rep, bool is_opaque);

private:

    DISALLOW_COPY_AND_ASSIGN(RefCountedCanvas);
};

typedef scoped_refptr<RefCountedCanvas> RefCanvas;


#endif