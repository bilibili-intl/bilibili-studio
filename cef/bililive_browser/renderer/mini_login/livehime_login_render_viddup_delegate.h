#pragma once

#include "bililive_browser/renderer/livehime_cef_app_render.h"


class LivehimeLoginRenderViddupDelegate : 
    public LivehimeCefAppRender::Delegate,
    public CefV8Handler
{
public:
    LivehimeLoginRenderViddupDelegate();
    virtual ~LivehimeLoginRenderViddupDelegate();

protected:
    // LivehimeCefAppRender::Delegate
    void OnContextCreated(CefRefPtr<LivehimeCefAppRender> app,
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context) override;

    bool OnProcessMessageReceived(CefRefPtr<LivehimeCefAppRender> app,
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message) override;

    // CefV8Handler
    bool Execute(const CefString& name,
        CefRefPtr<CefV8Value> object,
        const CefV8ValueList& arguments,
        CefRefPtr<CefV8Value>& retval,
        CefString& exception) override;

private:
    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(LivehimeLoginRenderViddupDelegate);
    DISALLOW_COPY_AND_ASSIGN(LivehimeLoginRenderViddupDelegate);
};