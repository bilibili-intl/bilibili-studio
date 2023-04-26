#pragma once

#include "cef_proxy_dll/public/livehime_cef_proxy_exports.h"

#include "cef_proxy_dll/browser/livehime_cef_client_browser.h"


class LivehimeBilibiliClientDelegate : public LivehimeCefClientBrowser::Delegate
{
public:
    explicit LivehimeBilibiliClientDelegate(const cef_proxy::browser_bind_data& bind_data);
    virtual ~LivehimeBilibiliClientDelegate();

    void OnBrowserCreated(CefRefPtr<CefBrowser> browser) override;

    // Called when the browser has been closed.
    void OnBrowserClosed(CefRefPtr<CefBrowser> browser) override;

    // Called on the UI thread before a context menu is displayed.
    void OnBeforeContextMenu(CefRefPtr<CefFrame> frame,
                             CefRefPtr<CefContextMenuParams> params,
                             CefRefPtr<CefMenuModel> model) override;

    void OnLoadStart(CefRefPtr<CefFrame> frame, CefLoadHandler::TransitionType transition_type) override;
    void OnLoadEnd(CefRefPtr<CefFrame> frame, int httpStatusCode) override;
    void OnLoadError(CefRefPtr<CefFrame> frame, CefLoadHandler::ErrorCode errorCode,
                     const CefString& errorText, const CefString& failedUrl) override;
    bool OnCertificateError(CefRefPtr<CefBrowser> browser,
                            cef_errorcode_t cert_error,
                            const CefString& request_url,
                            CefRefPtr<CefSSLInfo> ssl_info,
                            CefRefPtr<CefRequestCallback> callback) override;
    bool OnProcessMessageReceived(CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;

    bool DispatchJsEvent(const std::string& ipc_msg_name, const cef_proxy::calldata& arguments) override;

private:
    CefRefPtr<CefBrowser> browser_;

    IMPLEMENT_REFCOUNTING(LivehimeBilibiliClientDelegate);
    DISALLOW_COPY_AND_ASSIGN(LivehimeBilibiliClientDelegate);
};