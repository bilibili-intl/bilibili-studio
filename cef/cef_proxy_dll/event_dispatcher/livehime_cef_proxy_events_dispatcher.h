#pragma once

#include <atomic>

#include "cef_proxy_dll/public/livehime_cef_proxy_exports.h"

class LivehimeCefProxyEventsDispatcher
{
public:
    LivehimeCefProxyEventsDispatcher();
    virtual ~LivehimeCefProxyEventsDispatcher();

    static void RegisterCallbacks(const cef_proxy::cef_proxy_callback& callbacks);

    // ���ض�ҵ��ģ��ҳ���޹صĹ���֪ͨ
    static void DispatchInitStatus(bool success, cef_proxy::CefInitResult code);
    static void DispatchCommonEvent(const std::string& msg_name, const cef_proxy::calldata* data = nullptr);
    static void DispatchShutdownStatus(bool success);

    // ���ض�ҵ��ģ��ҳ����ص�֪ͨ
    static void DispatchEvent(const cef_proxy::browser_bind_data& bind_data, 
        const std::string& msg_name, const cef_proxy::calldata* data = nullptr);

private:
    std::atomic<cef_proxy::cef_proxy_callback> callbacks_;

    DISALLOW_COPY_AND_ASSIGN(LivehimeCefProxyEventsDispatcher);
};