#include "stdafx.h"
#include "livehime_obs_studio_app_browser_delegate.h"

#include "cef_core/include/cef_version.h"


LivehimeOBSStudioBrowserDelegate::LivehimeOBSStudioBrowserDelegate()
{
}

LivehimeOBSStudioBrowserDelegate::~LivehimeOBSStudioBrowserDelegate()
{
}

void LivehimeOBSStudioBrowserDelegate::OnBeforeCommandLineProcessing(CefRefPtr<LivehimeCefAppBrowser> app,
                                                                     const CefString& process_type,
                                                                     CefRefPtr<CefCommandLine> command_line)
{
    bool enableGPU = command_line->HasSwitch("enable-gpu");
    CefString type = command_line->GetSwitchValue("type");

    if (!enableGPU && type.empty())
    {
        command_line->AppendSwitch("disable-gpu");
        command_line->AppendSwitch("disable-gpu-compositing");
    }

    if (command_line->HasSwitch("disable-features"))
    {
        // Don't override existing, as this can break OSR
        std::string disableFeatures =
            command_line->GetSwitchValue("disable-features");
        disableFeatures += ",HardwareMediaKeyHandling"
#ifdef __APPLE__
            ",NetworkService"
#endif
            ;
        command_line->AppendSwitchWithValue("disable-features",
            disableFeatures);
    }
    else
    {
        command_line->AppendSwitchWithValue("disable-features",
            "HardwareMediaKeyHandling"
#ifdef __APPLE__
            ",NetworkService"
#endif
        );
    }

    command_line->AppendSwitchWithValue("autoplay-policy",
        "no-user-gesture-required");
}

void LivehimeOBSStudioBrowserDelegate::OnRegisterCustomSchemes(CefRefPtr<LivehimeCefAppBrowser> app,
                                                               CefRawPtr<CefSchemeRegistrar> registrar)
{
#if CHROME_VERSION_BUILD >= 3683
    registrar->AddCustomScheme("http",
        CEF_SCHEME_OPTION_STANDARD |
        CEF_SCHEME_OPTION_CORS_ENABLED);
#elif CHROME_VERSION_BUILD >= 3029
    registrar->AddCustomScheme("http", true, false, false, false, true,
        false);
#else
    registrar->AddCustomScheme("http", true, false, false, false, true);
#endif
}
