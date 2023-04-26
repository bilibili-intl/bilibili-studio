#include "stdafx.h"
#include "livehime_dev_tools_client_delegate.h"

#include "bililive_browser/public/bililive_browser_ipc_messages.h"
#include "bililive_browser/public/bililive_browser_js_values.h"

#include "event_dispatcher/livehime_cef_proxy_events_dispatcher.h"

#include "public/livehime_cef_proxy_constants.h"

LivehimeDevToolsClientDelegate::LivehimeDevToolsClientDelegate(const cef_proxy::browser_bind_data& bind_data)
	: LivehimeCefClientBrowser::Delegate(bind_data)
{
}

LivehimeDevToolsClientDelegate::~LivehimeDevToolsClientDelegate()
{
}

void LivehimeDevToolsClientDelegate::OnBrowserCreated(CefRefPtr<CefBrowser> browser)
{
	DCHECK(!browser_);
	browser_ = browser;
}

void LivehimeDevToolsClientDelegate::OnBrowserClosing(CefRefPtr<CefBrowser> browser)
{
    // ¹Øµô×Ô¼º

}

void LivehimeDevToolsClientDelegate::OnBrowserClosed(CefRefPtr<CefBrowser> browser)
{
	DCHECK(browser_);
	if (browser->IsSame(browser_))
	{
		browser_ = nullptr;
	}
}

void LivehimeDevToolsClientDelegate::OnBeforeContextMenu(CefRefPtr<CefFrame> frame,
	CefRefPtr<CefContextMenuParams> params,
	CefRefPtr<CefMenuModel> model)
{
	//model->Clear();
}

void LivehimeDevToolsClientDelegate::OnLoadStart(CefRefPtr<CefFrame> frame, CefLoadHandler::TransitionType transition_type)
{
}

void LivehimeDevToolsClientDelegate::OnLoadEnd(CefRefPtr<CefFrame> frame, int httpStatusCode)
{
}

void LivehimeDevToolsClientDelegate::OnLoadError(CefRefPtr<CefFrame> frame,
    CefLoadHandler::ErrorCode errorCode,
    const CefString& errorText,
    const CefString& failedUrl)
{
}

bool LivehimeDevToolsClientDelegate::OnCertificateError(CefRefPtr<CefBrowser> browser,
	cef_errorcode_t cert_error,
	const CefString& request_url,
	CefRefPtr<CefSSLInfo> ssl_info,
	CefRefPtr<CefRequestCallback> callback)
{
	callback->Continue();
	return true;
}
