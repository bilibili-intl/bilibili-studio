#pragma once

#include "public/livehime_cef_proxy_calldata.h"

std::wstring MakeAbsoluteFilePath(const std::wstring& input);

std::string GetExecutableVersion();

std::string GetDeviceName();

std::string GetDevicePlatform();

std::string GetBuvid3_();

// ��browser����ʹ�õ����ڴ���renderer���̷�������ͨ��ipc��Ϣ
bool ScheduleCommonRendererIPCMessage(const cef_proxy::browser_bind_data& bind_data,
    CefRefPtr<CefProcessMessage> message);

// ���ڽ�browser����Ҫ����renderer���̵���Ϣ�����CefProcessMessage����ȥ
bool ScheduleCommonBrowserIPCMessage(CefRefPtr<CefBrowser> browser,
    const std::string& ipc_msg_name, const cef_proxy::calldata& arguments);