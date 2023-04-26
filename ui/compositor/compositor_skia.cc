// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "compositor.h"

namespace ui{
  class SkiaCompositor;

  class SkiaTexture : public ui::Texture {
  public:
    SkiaTexture(SkiaCompositor* compositor)
      : compositor_(compositor)
    {

    }

    virtual ~SkiaTexture()
    {

    }

    // ui::Texture
    virtual void SetCanvas(const SkCanvas& canvas,
      const gfx::Point& origin,
      const gfx::Size& overall_size) OVERRIDE
    {
      view_size_ = overall_size;
      //const SkBitmap& bitmap = canvas.getDevice()->accessBitmap(false);
      //bounds_of_last_paint_.SetRect(
      //  origin.x(), origin.y(), bitmap.width(), bitmap.height());
    }

    virtual void Draw(const ui::TextureDrawParams& params,
      const gfx::Rect& clip_bounds) OVERRIDE
    {
      if (params.vertically_flipped)
        NOTIMPLEMENTED();

      //      compositor_->UpdatePerspective(params.transform, view_size_);
    }

  private:
    // Size of the corresponding View.
    gfx::Size view_size_;

    scoped_refptr<SkiaCompositor> compositor_;

    DISALLOW_COPY_AND_ASSIGN(SkiaTexture);
  };


  class SkiaCompositor : public ui::Compositor {
  public:
    SkiaCompositor(CompositorDelegate* delegate,
      gfx::AcceleratedWidget widget,
      const gfx::Size& size)
      : ui::Compositor(delegate, size)
    {

    }

    virtual ~SkiaCompositor()
    {

    }

    // ui::Compositor:
    virtual ui::Texture* CreateTexture() OVERRIDE
    {
       return new SkiaTexture(this);
    }

    virtual void OnNotifyStart(bool clear) OVERRIDE
    {

    }

    virtual void OnNotifyEnd() OVERRIDE
    {

    }

    virtual void Blur(const gfx::Rect& bounds) OVERRIDE
    {

    }

  protected:
    virtual void OnWidgetSizeChanged() OVERRIDE
    {

    }

  private:

    DISALLOW_COPY_AND_ASSIGN(SkiaCompositor);
  };



}  // namespace ui
