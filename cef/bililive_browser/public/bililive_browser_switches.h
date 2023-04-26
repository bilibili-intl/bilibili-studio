// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

// Defines all of the command line switches used by cefclient.

#pragma once

namespace switches
{
    const char kMultiThreadedMessageLoop[] = "multi-threaded-message-loop";
    const char kExternalMessagePump[] = "external-message-pump";
    const char kCachePath[] = "cache-path";
    const char kUrl[] = "url";
    const char kOffScreenRenderingEnabled[] = "off-screen-rendering-enabled";
    const char kOffScreenFrameRate[] = "off-screen-frame-rate";
    const char kTransparentPaintingEnabled[] = "transparent-painting-enabled";
    const char kShowUpdateRect[] = "show-update-rect";
    const char kMouseCursorChangeDisabled[] = "mouse-cursor-change-disabled";
    const char kRequestContextPerBrowser[] = "request-context-per-browser";
    const char kRequestContextSharedCache[] = "request-context-shared-cache";
    const char kRequestContextBlockCookies[] = "request-context-block-cookies";
    const char kBackgroundColor[] = "background-color";
    const char kEnableGPU[] = "enable-gpu";
    const char kFilterURL[] = "filter-url";
    const char kUseViews[] = "use-views";
    const char kHideFrame[] = "hide-frame";
    const char kHideControls[] = "hide-controls";
    const char kAlwaysOnTop[] = "always-on-top";
    const char kHideTopMenu[] = "hide-top-menu";
    const char kWidevineCdmPath[] = "widevine-cdm-path";
    const char kSslClientCertificate[] = "ssl-client-certificate";
    const char kCRLSetsPath[] = "crl-sets-path";
    const char kLoadExtension[] = "load-extension";

    // bililive
    const char kBililiveBrowserProcessId[] = "bililive-browser-process-id";
    const char kBililiveBrowserEnableDevTools[] = "bililive-enable-dev-tools";

}  // namespace switches

