// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_COMPOSITOR_TEST_COMPOSITOR_H_
#define UI_GFX_COMPOSITOR_TEST_COMPOSITOR_H_
#pragma once

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/compiler_specific.h"
#include "compositor.h"

namespace ui {

class TestCompositorDelegate;

// Trivial Compositor implementation that creates Textures of type TestTexture.
class TestCompositor : public ui::Compositor {
 public:
  TestCompositor();
  virtual ~TestCompositor();

  // ui::Compositor:
  virtual ui::Texture* CreateTexture() OVERRIDE;
  virtual void OnNotifyStart(bool clear) OVERRIDE;
  virtual void OnNotifyEnd() OVERRIDE;
  virtual void Blur(const gfx::Rect& bounds) OVERRIDE;

 protected:
  virtual void OnWidgetSizeChanged() OVERRIDE;

 private:
  scoped_ptr<TestCompositorDelegate> owned_delegate_;

  DISALLOW_COPY_AND_ASSIGN(TestCompositor);
};

}  // namespace ui

#endif  // UI_GFX_COMPOSITOR_TEST_COMPOSITOR_H_
