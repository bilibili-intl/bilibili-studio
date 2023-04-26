#pragma once

#include <winhttp.h>

#include "base/message_loop/message_loop.h"

#include "ui/views/controls/native_control.h"

#include "bililive/bililive/livehime/cef/cef_proxy_ipc_handler.h"
#include "bililive/bililive/livehime/cef/cef_proxy_wrapper.h"
#include "bililive/bililive/ui/views/controls/browser/livehime_web_browser_delegate.h"

#include "cef/cef_proxy_dll/public/livehime_cef_proxy_exports.h"



class LivehimeWebBrowserOsrView
    : public CefProxyObserver
{
public:
    LivehimeWebBrowserOsrView(cef_proxy::client_handler_type browser_type, const std::string& init_url,
        LivehimeWebBrowserDelegate* delegate, const std::string& post_data = "");
    virtual ~LivehimeWebBrowserOsrView();

    void InitViews();
    void UninitViews();

    bool Navigate(const std::string &url);

    cef_proxy::browser_bind_data* browser_bind_data() const;

    void NativeControlIgnoreViewVisibilityChanged(bool ignore, bool visible);

protected:
    // CefProxyObserver
    void OnLoadCefProxyCore(bool succeed) override;
    void OnSetTokenCookiesCompleted(bool success) override;
    void OnCefProxyMsgReceived(const cef_proxy::browser_bind_data& bind_data,
        const std::string& msg_name, const cef_proxy::calldata* data) override;

protected:
    virtual void OnCefKeyEvent(bool pre_key_event, const cef_proxy::key_event_t& key_event);
    virtual void OnEscapeKeyEvent(bool pre_key_event, const cef_proxy::key_event_t& key_event);

private:
    void OnWebBrowserCoreInvalid();

    void OnPaint(std::string* buffer, int width, int height);

private:
    LivehimeWebBrowserDelegate* delegate_;
    cef_proxy::client_handler_type browser_type_;
    std::string url_;
    std::string post_data_;
    std::unique_ptr<cef_proxy::browser_bind_data> bind_data_;
    std::unique_ptr<CefProxyMsgHandleExtraDetails> handler_details_;

    bool native_ctrl_visible_ignore_view_visibility_changed_ = false; // 原生窗口的可视属性不受View自身visible属性的变更影响

    WebViewOperation esc_operation_ = WebViewOperation::WVO_Close;

    base::WeakPtrFactory<LivehimeWebBrowserOsrView> weakptr_factory_;
    DISALLOW_COPY_AND_ASSIGN(LivehimeWebBrowserOsrView);
};
