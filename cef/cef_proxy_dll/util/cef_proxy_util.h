#pragma once

#include "public/livehime_cef_proxy_calldata.h"

std::wstring MakeAbsoluteFilePath(const std::wstring& input);

std::string GetExecutableVersion();

std::string GetDeviceName();

std::string GetDevicePlatform();

std::string GetBuvid3_();

// 由browser进程使用的用于处理renderer进程发过来的通用ipc消息
bool ScheduleCommonRendererIPCMessage(const cef_proxy::browser_bind_data& bind_data,
    CefRefPtr<CefProcessMessage> message);

// 用于将browser进程要发到renderer进程的消息打包成CefProcessMessage发出去
bool ScheduleCommonBrowserIPCMessage(CefRefPtr<CefBrowser> browser,
    const std::string& ipc_msg_name, const cef_proxy::calldata& arguments);