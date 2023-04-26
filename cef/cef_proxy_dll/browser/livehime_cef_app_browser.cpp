#include "stdafx.h"

#include "livehime_cef_app_browser.h"

#include "bililive_browser/public/bililive_browser_switches.h"

LivehimeCefAppBrowser::LivehimeCefAppBrowser()
{
    AddDelegates(delegates_);
}

LivehimeCefAppBrowser::~LivehimeCefAppBrowser()
{
}

CefRefPtr<CefBrowserProcessHandler> LivehimeCefAppBrowser::GetBrowserProcessHandler()
{
    return this;
}

void LivehimeCefAppBrowser::OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line)
{
    for (auto &iter : delegates_)
    {
        iter->OnBeforeCommandLineProcessing(this, process_type, command_line);
    }
}

void LivehimeCefAppBrowser::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar)
{
    for (auto &iter : delegates_)
    {
        iter->OnRegisterCustomSchemes(this, registrar);
    }
}

void LivehimeCefAppBrowser::OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line)
{
    // �������̵�PID����CEF�ӽ��̣��Ա��ӽ��̼���������״̬ʵ��ͬ���˳�
    command_line->AppendSwitchWithValue(switches::kBililiveBrowserProcessId, std::to_string(::GetCurrentProcessId()));
}

void LivehimeCefAppBrowser::OnContextInitialized()
{
    for (auto &iter : delegates_)
    {
        iter->OnContextInitialized(this);
    }
}
