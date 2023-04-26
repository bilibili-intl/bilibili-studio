// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "test_suite.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/base/ui_base_paths.h"
#include "ui/gfx/gfx_paths.h"
//#include "ui/gfx/gl/gl_implementation.h"

CompositorTestSuite::CompositorTestSuite(int argc, char** argv)
    : TestSuite(argc, argv) {}

void CompositorTestSuite::Initialize() {
//#if defined(OS_LINUX)
//  gfx::InitializeGLBindings(gfx::kGLImplementationOSMesaGL);
//#endif
  base::TestSuite::Initialize();

  gfx::RegisterPathProvider();
  ui::RegisterPathProvider();

  // Force unittests to run using en-US so if we test against string
  // output, it'll pass regardless of the system language.
  ui::ResourceBundle::InitSharedInstanceWithLocale("en-US", NULL);
}

void CompositorTestSuite::Shutdown() {
  ui::ResourceBundle::CleanupSharedInstance();

  base::TestSuite::Shutdown();
}
