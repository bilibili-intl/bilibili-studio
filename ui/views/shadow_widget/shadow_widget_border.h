#ifndef UI_VIEWS_SHADOW_WIDGET_SHADOW_BORDER_H
#define UI_VIEWS_SHADOW_WIDGET_SHADOW_BORDER_H

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/views/background.h"
#include "ui/views/border.h"

namespace gfx
{
  class ImageSkia;
}

namespace views
{
  namespace internal
  {
    struct ShadowWidgetBorderImages;
  }

  class VIEWS_EXPORT ShadowWidgetBorder : public Border
  {
  public:
    enum Shadow
    {
      SHADOW    = 0,
      NO_SHADOW,
      NO_SHADOW_OPAQUE_BORDER,
      BIG_SHADOW,
      SMALL_SHADOW,
      SHADOW_COUNT,
    };

    ShadowWidgetBorder(Shadow shadow, SkColor color);
    virtual ~ShadowWidgetBorder();

    static int GetCornerRadius()
    {
      return 4;
    }

    Shadow shadow() const
    {
      return shadow_;
    }

    void set_background_color(SkColor color)
    {
      background_color_ = color;
    }
    SkColor background_color() const
    {
      return background_color_;
    }

    virtual gfx::Rect GetBounds(const gfx::Rect &content_rect) const;

    int GetBorderThickness() const;

    int GetBorderCornerRadius() const;

    // Overridden from Border:
    virtual gfx::Insets GetInsets() const OVERRIDE;
    virtual void Paint(const View &view, gfx::Canvas *canvas) OVERRIDE;

  private:
    Shadow shadow_;
    internal::ShadowWidgetBorderImages *images_;
    SkColor background_color_;

    DISALLOW_COPY_AND_ASSIGN(ShadowWidgetBorder);
  };

  class VIEWS_EXPORT ShadowWidgetBackground : public Background
  {
  public:
    explicit ShadowWidgetBackground(ShadowWidgetBorder *border) : border_(border) {}

    // Overridden from Background:
    virtual void Paint(gfx::Canvas *canvas, View *view) const OVERRIDE;

  private:
    ShadowWidgetBorder *border_;

    DISALLOW_COPY_AND_ASSIGN(ShadowWidgetBackground);
  };

}  // namespace views

#endif  // UI_VIEWS_SHADOW_WIDGET_SHADOW_BORDER_H