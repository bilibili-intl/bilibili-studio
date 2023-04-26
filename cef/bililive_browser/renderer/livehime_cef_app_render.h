#pragma once

#include "shared/livehime_cef_app.h"

#include <set>


// Implement application-level callbacks for the browser process.
class LivehimeCefAppRender
    : public LivehimeCefApp
    , public CefRenderProcessHandler
{
public:
    class Delegate : public virtual CefBaseRefCounted
    {
    public:
        virtual void OnBeforeCommandLineProcessing(
            CefRefPtr<LivehimeCefAppRender> app,
            const CefString &process_type,
            CefRefPtr<CefCommandLine> command_line)
        {
        }

        virtual void OnRegisterCustomSchemes(CefRefPtr<LivehimeCefAppRender> app,
                                             CefRawPtr<CefSchemeRegistrar> registrar)
        {
        }

        virtual void OnWebKitInitialized(CefRefPtr<LivehimeCefAppRender> app)
        {
        }

        virtual void OnBrowserCreated(CefRefPtr<LivehimeCefAppRender> app, CefRefPtr<CefBrowser> browser,
            CefRefPtr<CefDictionaryValue> extra_info)
        {
        }

        virtual void OnBrowserDestroyed(CefRefPtr<LivehimeCefAppRender> app, CefRefPtr<CefBrowser> browser)
        {
        }

        virtual void OnContextCreated(CefRefPtr<LivehimeCefAppRender> app,
                                      CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefV8Context> context)
        {
        }

        virtual void OnContextReleased(CefRefPtr<LivehimeCefAppRender> app, 
                                       CefRefPtr<CefBrowser> browser,
                                       CefRefPtr<CefFrame> frame,
                                       CefRefPtr<CefV8Context> context)
        {
        }

        virtual void OnFocusedNodeChanged(CefRefPtr<LivehimeCefAppRender> app, 
                                          CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame,
                                          CefRefPtr<CefDOMNode> node)
        {
        }

        virtual bool OnProcessMessageReceived(CefRefPtr<LivehimeCefAppRender> app,
                                              CefRefPtr<CefBrowser> browser,
                                              CefRefPtr<CefFrame> frame,
                                              CefProcessId source_process,
                                              CefRefPtr<CefProcessMessage> message)
        {
            return false;
        }
    };

    typedef std::set<CefRefPtr<Delegate>> DelegateSet;

    LivehimeCefAppRender();
    virtual ~LivehimeCefAppRender();

    // CefApp methods:
    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override;
    void OnBeforeCommandLineProcessing(
        const CefString &process_type,
        CefRefPtr<CefCommandLine> command_line) override;
    void OnRegisterCustomSchemes(
        CefRawPtr<CefSchemeRegistrar> registrar) override;

    // CefRenderProcessHandler
    void OnWebKitInitialized() override;
    void OnBrowserCreated(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDictionaryValue> extra_info) override;
    void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) override;
    void OnContextCreated(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                          CefRefPtr<CefV8Context> context) override;
    void OnContextReleased(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           CefRefPtr<CefV8Context> context) override;
    void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              CefRefPtr<CefDOMNode> node) override;
    bool OnProcessMessageReceived(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message) override;

private:
    static void AddDelegates(DelegateSet& delegates);

private:
    DelegateSet delegates_;

    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(LivehimeCefAppRender);
    DISALLOW_COPY_AND_ASSIGN(LivehimeCefAppRender);
};