#pragma once

#include "cef_proxy_dll/public/livehime_cef_proxy_exports.h"

#include "cef_proxy_dll/browser/livehime_cef_client_browser.h"


class LivehimeDevToolsClientDelegate : public LivehimeCefClientBrowser::Delegate
{
public:
	explicit LivehimeDevToolsClientDelegate(const cef_proxy::browser_bind_data& bind_data);
	virtual ~LivehimeDevToolsClientDelegate();

	void OnBrowserCreated(CefRefPtr<CefBrowser> browser) override;

    // Called when the browser has been closed.
    void OnBrowserClosing(CefRefPtr<CefBrowser> browser) override;
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

private:
	CefRefPtr<CefBrowser> browser_;

	IMPLEMENT_REFCOUNTING(LivehimeDevToolsClientDelegate);
	DISALLOW_COPY_AND_ASSIGN(LivehimeDevToolsClientDelegate);
};
