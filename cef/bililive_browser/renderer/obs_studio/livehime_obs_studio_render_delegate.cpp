#include "stdafx.h"
#include "livehime_obs_studio_render_delegate.h"


LivehimeOBSStudioRenderDelegate::LivehimeOBSStudioRenderDelegate()
{
    obs_browser_page_browserapp_ = new BrowserApp();
}

LivehimeOBSStudioRenderDelegate::~LivehimeOBSStudioRenderDelegate()
{
}

void LivehimeOBSStudioRenderDelegate::OnBeforeCommandLineProcessing(CefRefPtr<LivehimeCefAppRender> app,
                                                                    const CefString &process_type,
                                                                    CefRefPtr<CefCommandLine> command_line)
{
    obs_browser_page_browserapp_->OnBeforeCommandLineProcessing(process_type, command_line);
}

void LivehimeOBSStudioRenderDelegate::OnRegisterCustomSchemes(CefRefPtr<LivehimeCefAppRender> app,
                                                              CefRawPtr<CefSchemeRegistrar> registrar)
{
    obs_browser_page_browserapp_->OnRegisterCustomSchemes(registrar);
}

void LivehimeOBSStudioRenderDelegate::OnContextCreated(CefRefPtr<LivehimeCefAppRender> app,
                                                     CefRefPtr<CefBrowser> browser,
                                                     CefRefPtr<CefFrame> frame,
                                                     CefRefPtr<CefV8Context> context)
{
    obs_browser_page_browserapp_->OnContextCreated(browser, frame, context);
}

bool LivehimeOBSStudioRenderDelegate::OnProcessMessageReceived(CefRefPtr<LivehimeCefAppRender> app,
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{
    return obs_browser_page_browserapp_->OnProcessMessageReceived(browser, frame, source_process, message);
}

bool LivehimeOBSStudioRenderDelegate::Execute(const CefString &name,
                                              CefRefPtr<CefV8Value> object,
                                              const CefV8ValueList &arguments,
                                              CefRefPtr<CefV8Value> &retval,
                                              CefString &exception)
{
    return obs_browser_page_browserapp_->Execute(name, object, arguments, retval, exception);
}
