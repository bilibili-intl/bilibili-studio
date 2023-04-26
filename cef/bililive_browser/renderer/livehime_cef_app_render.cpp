#include "stdafx.h"

#include "livehime_cef_app_render.h"


LivehimeCefAppRender::LivehimeCefAppRender()
{
    // 创建的时候就添加
    AddDelegates(delegates_);
}

LivehimeCefAppRender::~LivehimeCefAppRender()
{
}

CefRefPtr<CefRenderProcessHandler> LivehimeCefAppRender::GetRenderProcessHandler()
{
    return this;
}

void LivehimeCefAppRender::OnBeforeCommandLineProcessing(const CefString &process_type, CefRefPtr<CefCommandLine> command_line)
{
    for (auto &iter : delegates_)
    {
        iter->OnBeforeCommandLineProcessing(this, process_type, command_line);
    }
}

void LivehimeCefAppRender::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar)
{
    for (auto &iter : delegates_)
    {
        iter->OnRegisterCustomSchemes(this, registrar);
    }
}

void LivehimeCefAppRender::OnWebKitInitialized()
{
    for (auto &iter : delegates_)
    {
        iter->OnWebKitInitialized(this);
    }
}

void LivehimeCefAppRender::OnBrowserCreated(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDictionaryValue> extra_info)
{
    for (auto &iter : delegates_)
    {
        iter->OnBrowserCreated(this, browser, extra_info);
    }
}

void LivehimeCefAppRender::OnBrowserDestroyed(CefRefPtr<CefBrowser> browser)
{
    for (auto &iter : delegates_)
    {
        iter->OnBrowserDestroyed(this, browser);
    }
}

void LivehimeCefAppRender::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
    // 注册js函数或值
    for (auto &iter : delegates_)
    {
        iter->OnContextCreated(this, browser, frame, context);
    }
}

void LivehimeCefAppRender::OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
    for (auto &iter : delegates_)
    {
        iter->OnContextReleased(this, browser, frame, context);
    }
}

void LivehimeCefAppRender::OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefDOMNode> node)
{
    for (auto &iter : delegates_)
    {
        iter->OnFocusedNodeChanged(this, browser, frame, node);
    }
}

bool LivehimeCefAppRender::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{
    DCHECK(source_process == PID_BROWSER);

    bool handled = false;
    for (auto iter = delegates_.begin(); iter != delegates_.end() && !handled; ++iter)
    {
        handled = (*iter)->OnProcessMessageReceived(this, browser, frame, source_process, message);
    }
    return handled;
}
