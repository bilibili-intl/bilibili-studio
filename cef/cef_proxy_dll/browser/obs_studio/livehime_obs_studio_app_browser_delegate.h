#pragma once

#include "browser/livehime_cef_app_browser.h"

class LivehimeOBSStudioBrowserDelegate :
    public LivehimeCefAppBrowser::Delegate
{
public:
    LivehimeOBSStudioBrowserDelegate();
    virtual ~LivehimeOBSStudioBrowserDelegate();

    // LivehimeCefAppBrowser::Delegate
    void OnBeforeCommandLineProcessing(
        CefRefPtr<LivehimeCefAppBrowser> app,
        const CefString& process_type,
        CefRefPtr<CefCommandLine> command_line) override;

    void OnRegisterCustomSchemes(CefRefPtr<LivehimeCefAppBrowser> app,
                                 CefRawPtr<CefSchemeRegistrar> registrar) override;

private:
    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(LivehimeOBSStudioBrowserDelegate);
    DISALLOW_COPY_AND_ASSIGN(LivehimeOBSStudioBrowserDelegate);
};