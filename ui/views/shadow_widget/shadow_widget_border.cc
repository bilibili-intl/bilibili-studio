#include "ui/views/shadow_widget/shadow_widget_border.h"

#include <algorithm>

#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "grit/ui_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/skia_util.h"
#include "ui/views/painter.h"

namespace views
{
  namespace internal
  {
    struct ShadowWidgetBorderImages
    {
      ShadowWidgetBorderImages(const int border_image_ids[],
                   int border_interior_thickness,
                   int corner_radius);

      scoped_ptr<Painter> border_painter;

      int border_thickness;
      int border_interior_thickness;

      int corner_radius;
    };

    ShadowWidgetBorderImages::ShadowWidgetBorderImages(const int border_image_ids[],
                               int border_interior_thickness,
                               int corner_radius)
      : border_painter(Painter::CreateImageGridPainter(border_image_ids)),
        border_thickness(0),
        border_interior_thickness(border_interior_thickness),
        corner_radius(corner_radius)
    {
      ResourceBundle &rb = ResourceBundle::GetSharedInstance();
      border_thickness = rb.GetImageSkiaNamed(border_image_ids[0])->width();
    }
  }

  namespace
  {

    const int kStroke = 1;

    // Macros to define arrays of IDR constants used with CreateImageGridPainter.
#define IMAGE_BORDER(x) { x ## _TOP_LEFT,    x ## _TOP,    x ## _TOP_RIGHT, \
                          x ## _LEFT,        0,            x ## _RIGHT, \
                          x ## _BOTTOM_LEFT, x ## _BOTTOM, x ## _BOTTOM_RIGHT, }
#define IMAGE_BORDER_ACRONYM(x) { x ## _TL, x ## _T, x ## _TR, \
                                  x ## _L,  0,       x ## _R, \
                                  x ## _BL, x ## _B, x ## _BR, }

    // Bubble border and arrow image resource ids.
    const int kShadowImages[] = IMAGE_BORDER_ACRONYM(IDR_BUBBLE_SHADOW);
    const int kShadowArrows[] = { 0, 0, 0, 0 };
    const int kNoShadowImages[] = IMAGE_BORDER_ACRONYM(IDR_BUBBLE);
    const int kNoShadowArrows[] = { IDR_BUBBLE_L_ARROW, IDR_BUBBLE_T_ARROW,
                                    IDR_BUBBLE_R_ARROW, IDR_BUBBLE_B_ARROW,
                                  };
    const int kBigShadowImages[] = IMAGE_BORDER(IDR_WINDOW_BUBBLE_SHADOW_BIG);
    const int kSmallShadowImages[] = IMAGE_BORDER(IDR_WINDOW_BUBBLE_SHADOW_SMALL);


    using internal::ShadowWidgetBorderImages;

    ShadowWidgetBorderImages *GetBorderImages(ShadowWidgetBorder::Shadow shadow)
    {
      static ShadowWidgetBorderImages *kBorderImages[ShadowWidgetBorder::SHADOW_COUNT] = { NULL };

      CHECK_LT(shadow, ShadowWidgetBorder::SHADOW_COUNT);
      struct ShadowWidgetBorderImages *&set = kBorderImages[shadow];
      if (set)
      {
        return set;
      }

      switch (shadow)
      {
      case ShadowWidgetBorder::SHADOW:
        set = new ShadowWidgetBorderImages(kShadowImages, 0, 3);
        break;
      case ShadowWidgetBorder::NO_SHADOW:
      case ShadowWidgetBorder::NO_SHADOW_OPAQUE_BORDER:
        set = new ShadowWidgetBorderImages(kNoShadowImages, 6, 4);
        break;
      case ShadowWidgetBorder::BIG_SHADOW:
        set = new ShadowWidgetBorderImages(kBigShadowImages, 23, 2);
        break;
      case ShadowWidgetBorder::SMALL_SHADOW:
        set = new ShadowWidgetBorderImages(kSmallShadowImages, 5, 2);
        break;
      case ShadowWidgetBorder::SHADOW_COUNT:
        NOTREACHED();
        break;
      }

      return set;
    }
  }  // namespace

  ShadowWidgetBorder::ShadowWidgetBorder(Shadow shadow, SkColor color)
    : shadow_(shadow),
      background_color_(color)
  {
    DCHECK(shadow < SHADOW_COUNT);
    images_ = GetBorderImages(shadow);
  }

  ShadowWidgetBorder::~ShadowWidgetBorder() {}

  gfx::Rect ShadowWidgetBorder::GetBounds(const gfx::Rect &content_rect) const
  {
    gfx::Size size(content_rect.size());
    const gfx::Insets insets = GetInsets();
    size.Enlarge(insets.width(), insets.height());

    return gfx::Rect(content_rect.x() - insets.left(),
      content_rect.y() - insets.top(), size.width(), size.height());
  }

  int ShadowWidgetBorder::GetBorderThickness() const
  {
    return images_->border_thickness - images_->border_interior_thickness;
  }

  int ShadowWidgetBorder::GetBorderCornerRadius() const
  {
    return images_->corner_radius;
  }

  gfx::Insets ShadowWidgetBorder::GetInsets() const
  {
    const int inset = GetBorderThickness();
    return gfx::Insets(inset, inset, inset, inset);
  }

  void ShadowWidgetBorder::Paint(const views::View &view, gfx::Canvas *canvas)
  {
    gfx::Rect bounds(view.GetContentsBounds());
    bounds.Inset(-GetBorderThickness(), -GetBorderThickness());

    Painter::PaintPainterAt(canvas, images_->border_painter.get(), bounds);
  }

  void ShadowWidgetBackground::Paint(gfx::Canvas *canvas, views::View *view) const
  {
    if (border_->shadow() == ShadowWidgetBorder::NO_SHADOW_OPAQUE_BORDER)
    {
      canvas->DrawColor(border_->background_color());
    }

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);
    paint.setColor(border_->background_color());
    SkPath path;
    gfx::Rect bounds(view->GetLocalBounds());
    bounds.Inset(border_->GetInsets());

    if (border_->shadow() == ShadowWidgetBorder::SHADOW)
    {
      bounds.Inset(-10, -10);
    }

    SkScalar radius = SkIntToScalar(border_->GetBorderCornerRadius());
    path.addRoundRect(gfx::RectToSkRect(bounds), radius, radius);
    canvas->DrawPath(path, paint);
  }

}  // namespace views