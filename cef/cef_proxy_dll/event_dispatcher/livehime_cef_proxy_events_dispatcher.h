#pragma once

#include <atomic>

#include "cef_proxy_dll/public/livehime_cef_proxy_exports.h"

class LivehimeCefProxyEventsDispatcher
{
public:
    LivehimeCefProxyEventsDispatcher();
    virtual ~LivehimeCefProxyEventsDispatcher();

    static void RegisterCallbacks(const cef_proxy::cef_proxy_callback& callbacks);

    // 与特定业务模块页面无关的公共通知
    static void DispatchInitStatus(bool success, cef_proxy::CefInitResult code);
    static void DispatchCommonEvent(const std::string& msg_name, const cef_proxy::calldata* data = nullptr);
    static void DispatchShutdownStatus(bool success);

    // 与特定业务模块页面相关的通知
    static void DispatchEvent(const cef_proxy::browser_bind_data& bind_data, 
        const std::string& msg_name, const cef_proxy::calldata* data = nullptr);

private:
    std::atomic<cef_proxy::cef_proxy_callback> callbacks_;

    DISALLOW_COPY_AND_ASSIGN(LivehimeCefProxyEventsDispatcher);
};