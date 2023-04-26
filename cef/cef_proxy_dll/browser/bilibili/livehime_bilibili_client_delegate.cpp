#include "stdafx.h"
#include "livehime_bilibili_client_delegate.h"

#include "bililive_browser/public/bililive_browser_ipc_messages.h"
#include "bililive_browser/public/bililive_browser_js_values.h"

#include "event_dispatcher/livehime_cef_proxy_events_dispatcher.h"

#include "public/livehime_cef_proxy_constants.h"
#include "util/cef_proxy_util.h"

LivehimeBilibiliClientDelegate::LivehimeBilibiliClientDelegate(const cef_proxy::browser_bind_data& bind_data)
    : LivehimeCefClientBrowser::Delegate(bind_data)
{
}

LivehimeBilibiliClientDelegate::~LivehimeBilibiliClientDelegate()
{
}

void LivehimeBilibiliClientDelegate::OnBrowserCreated(CefRefPtr<CefBrowser> browser)
{
    DCHECK(!browser_);
    browser_ = browser;
}

void LivehimeBilibiliClientDelegate::OnBrowserClosed(CefRefPtr<CefBrowser> browser)
{
    DCHECK(browser_);
    if (browser->IsSame(browser_))
    {
        browser_ = nullptr;
    }
}

void LivehimeBilibiliClientDelegate::OnBeforeContextMenu(CefRefPtr<CefFrame> frame, 
                                                          CefRefPtr<CefContextMenuParams> params,
                                                          CefRefPtr<CefMenuModel> model)
{
    model->Clear();
}

void LivehimeBilibiliClientDelegate::OnLoadStart(CefRefPtr<CefFrame> frame, CefLoadHandler::TransitionType transition_type)
{
    if (frame->IsMain())
    {
        CefString eval_biliInject = "window.biliInject = window.biliInject || {" \
                "postMessage: function (string) {" \
                    "try {" \
                        "window.livehime_bilibili.postMessage(string)" \
                    "} catch (error) {" \
                        "console.warn('Your browser does not support livehime_bilibili.postMessage')" \
                    "}" \
                "}" \
            "}";

        std::string version = GetExecutableVersion();

        CefString eval_browser = "window.browser = {" \
                "version: {" \
                    "android: false, BiliApp: true, mobile: false," \
                    "ios: false, iPhone: false, iPad: false, MicroMessenger: false," \
                    "webApp: false, pc_link: true" \
                "}," \
                "language: 'zh-CN'," \
                "pc_link_scene: 'popup',"
                "app_version: '" + version + "',"\
                "rollout: 'support-new-browser'"
            "}";


        CefString url = frame->GetURL();
        frame->ExecuteJavaScript(eval_biliInject, url, 0);
        frame->ExecuteJavaScript(eval_browser, url, 0);
    }
}

void LivehimeBilibiliClientDelegate::OnLoadEnd(CefRefPtr<CefFrame> frame, int httpStatusCode)
{
}

void LivehimeBilibiliClientDelegate::OnLoadError(CefRefPtr<CefFrame> frame, 
                                                  CefLoadHandler::ErrorCode errorCode, 
                                                  const CefString& errorText, 
                                                  const CefString& failedUrl)
{
}

bool LivehimeBilibiliClientDelegate::OnCertificateError(CefRefPtr<CefBrowser> browser, 
                                                         cef_errorcode_t cert_error, 
                                                         const CefString& request_url, 
                                                         CefRefPtr<CefSSLInfo> ssl_info, 
                                                         CefRefPtr<CefRequestCallback> callback)
{
    callback->Continue();
    return true;
}

bool LivehimeBilibiliClientDelegate::OnProcessMessageReceived(CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
    if (ScheduleCommonRendererIPCMessage(bind_data(), message))
    {
        return true;
    }
    else
    {
        // 处理业务特有的消息
        //////////////////////////////////////////////////////////////////////////
    }
    return false;
}

bool LivehimeBilibiliClientDelegate::DispatchJsEvent(const std::string& ipc_msg_name, const cef_proxy::calldata& arguments)
{
    if (ScheduleCommonBrowserIPCMessage(browser_, ipc_msg_name, arguments))
    {
        return true;
    }
    else
    {
        // 处理业务特有的消息
        //////////////////////////////////////////////////////////////////////////
    }
    return false;
}
