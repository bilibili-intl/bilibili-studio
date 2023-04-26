#pragma once


#include "ui/views/widget/widget.h"

#include "bililive/bililive/ui/views/controls/browser/livehime_web_browser_delegate.h"


struct CefProxyMsgHandleExtraDetails
{
    views::Widget* widget = nullptr;

    using NativeControlIgnoreViewVisibilityChanged_fn = std::function<void (bool ignore, bool visible)>;
    NativeControlIgnoreViewVisibilityChanged_fn NativeControlIgnoreViewVisibilityChanged = nullptr;

    using OnCefKeyEvent_fn = std::function<void(bool pre_key_event, const cef_proxy::key_event_t& key_event)>;
    OnCefKeyEvent_fn OnCefKeyEvent = nullptr;
};

void CefProxyMsgHandle(LivehimeWebBrowserDelegate* delegate, CefProxyMsgHandleExtraDetails* details,
    const cef_proxy::browser_bind_data& bind_data,
    const std::string& msg_name, const std::string& page_url, const cef_proxy::calldata* data);
