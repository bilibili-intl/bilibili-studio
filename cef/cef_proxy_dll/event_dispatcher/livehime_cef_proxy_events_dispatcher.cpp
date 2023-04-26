#include "stdafx.h"
#include "livehime_cef_proxy_events_dispatcher.h"

namespace
{
    LivehimeCefProxyEventsDispatcher dispatcher;
}
void LivehimeCefProxyEventsDispatcher::RegisterCallbacks(const cef_proxy::cef_proxy_callback& callbacks)
{
    static bool reg_once = true;
    DCHECK(reg_once);

    reg_once = false;
    dispatcher.callbacks_ = callbacks;
}

void LivehimeCefProxyEventsDispatcher::DispatchInitStatus(bool success, cef_proxy::CefInitResult code)
{
    cef_proxy::cef_proxy_callback callback = dispatcher.callbacks_;
    if (callback.init_status_fn)
    {
        callback.init_status_fn(success, code);
    }
}

void LivehimeCefProxyEventsDispatcher::DispatchCommonEvent(const std::string& msg_name, const cef_proxy::calldata* data /*= nullptr*/)
{
    cef_proxy::cef_proxy_callback callback = dispatcher.callbacks_;
    if (callback.common_event_fn)
    {
        callback.common_event_fn(msg_name, data);
    }
}

void LivehimeCefProxyEventsDispatcher::DispatchShutdownStatus(bool success)
{
    cef_proxy::cef_proxy_callback callback = dispatcher.callbacks_;
    if (callback.shutdown_status_fn)
    {
        callback.shutdown_status_fn(success);
    }
}

void LivehimeCefProxyEventsDispatcher::DispatchEvent(const cef_proxy::browser_bind_data& bind_data, 
    const std::string& msg_name, const cef_proxy::calldata* data/* = nullptr*/)
{
    cef_proxy::cef_proxy_callback callback = dispatcher.callbacks_;
    if (callback.msg_received_fn)
    {
        callback.msg_received_fn(bind_data, msg_name, data);
    }
}

LivehimeCefProxyEventsDispatcher::LivehimeCefProxyEventsDispatcher()
{
}

LivehimeCefProxyEventsDispatcher::~LivehimeCefProxyEventsDispatcher()
{
}
