#include "stdafx.h"
#include "livehime_cef_app_browser.h"

#include "browser/obs_studio/livehime_obs_studio_app_browser_delegate.h"

namespace
{
    class LivehimeCefAppBrowserDelegate : public LivehimeCefAppBrowser::Delegate
    {
    public:
        LivehimeCefAppBrowserDelegate() {}

        void OnRegisterCustomSchemes(CefRefPtr<LivehimeCefAppBrowser> app,
                                             CefRawPtr<CefSchemeRegistrar> registrar) override
        {
        }

        void OnBeforeCommandLineProcessing(CefRefPtr<LivehimeCefAppBrowser> app,
            const CefString& process_type,
            CefRefPtr<CefCommandLine> command_line) override
        {
            //command_line->AppendSwitch("enable-system-flash");
            
            // Append Chromium command line parameters if touch events are enabled
            command_line->AppendSwitchWithValue("touch-events", "enabled");

            //command_line->AppendSwitch("disable-request-handling-for-testing");
        }

        void OnContextInitialized(CefRefPtr<LivehimeCefAppBrowser> app) override
        {
        }

    private:
        IMPLEMENT_REFCOUNTING(LivehimeCefAppBrowserDelegate);
        DISALLOW_COPY_AND_ASSIGN(LivehimeCefAppBrowserDelegate);
    };
}


void LivehimeCefAppBrowser::AddDelegates(DelegateSet& delegates)
{
    delegates.insert(new LivehimeCefAppBrowserDelegate());

    // extension
    delegates.insert(new LivehimeOBSStudioBrowserDelegate());
}
