#pragma once

#include "bililive_browser/shared/livehime_cef_app.h"

#include <set>


// Implement application-level callbacks for the browser process.
class LivehimeCefAppBrowser
    : public LivehimeCefApp
    , public CefBrowserProcessHandler
{
public:
    class Delegate : public virtual CefBaseRefCounted
    {
    public:
        virtual void OnBeforeCommandLineProcessing(
            CefRefPtr<LivehimeCefAppBrowser> app,
            const CefString& process_type,
            CefRefPtr<CefCommandLine> command_line)
        {
        }

        virtual void OnRegisterCustomSchemes(CefRefPtr<LivehimeCefAppBrowser> app,
                                             CefRawPtr<CefSchemeRegistrar> registrar)
        {
        }

        virtual void OnContextInitialized(CefRefPtr<LivehimeCefAppBrowser> app) {}
    };

    typedef std::set<CefRefPtr<Delegate>> DelegateSet;

    LivehimeCefAppBrowser();
    virtual ~LivehimeCefAppBrowser();

    // CefApp methods:
    void OnBeforeCommandLineProcessing(
        const CefString& process_type,
        CefRefPtr<CefCommandLine> command_line) override;
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override;
    void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) override;

    // CefBrowserProcessHandler methods:
    void OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line) override;
    void OnContextInitialized() override;

private:
    // Creates all of the Delegate objects. Implemented by cefclient in
    // client_app_delegates_browser.cc
    static void AddDelegates(DelegateSet& delegates);

private:
    // Set of supported Delegates.
    DelegateSet delegates_;

    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(LivehimeCefAppBrowser);
    DISALLOW_COPY_AND_ASSIGN(LivehimeCefAppBrowser);
};