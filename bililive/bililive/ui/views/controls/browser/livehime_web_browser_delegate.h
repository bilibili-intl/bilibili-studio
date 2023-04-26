#pragma once

#include <winhttp.h>

#include "base/message_loop/message_loop.h"

#include "ui/views/controls/native_control.h"

#include "bililive/bililive/livehime/cef/cef_proxy_wrapper.h"

#include "cef/cef_proxy_dll/public/livehime_cef_proxy_exports.h"


enum class WebViewOperation
{
    WVO_None,
    WVO_Close,
    //WVO_Hide,
};

enum class WebViewPopupType
{
    NotAllow,
    Native,
    System
};

class LivehimeWebBrowserDelegate
{
public:
    virtual ~LivehimeWebBrowserDelegate() = default;

    virtual void OnWebBrowserCoreInvalid() {}

    virtual void OnSetTokenCookiesCompleted(bool success) {}

    virtual void OnPreWebBrowserWindowCreate() {}
    virtual void OnWebBrowserLoadStart(const std::string& url) {}
    virtual void OnWebBrowserLoadEnd(const std::string& url, int http_status_code) {}
    virtual void OnWebBrowserLoadError(const std::string& failed_url, int http_status_code,
        const std::string& error_text) {}
    virtual void OnWebBrowserLoadTitle(const base::string16& title) {}

    virtual void OnWebJionTalkTopic(const cef_proxy::browser_bind_data& bind_data,const std::string& topic_id,const cef_proxy::calldata* data) {}
    //互动游戏中心 监听事件
    virtual void OnInteractivePlayCenterEvent(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::calldata* data) {}
    //virtual void OnWebBrowserWindowClose() {}
    // 按键事件的处理，没有特殊逻辑需要中止webview自己的按键处理流程的就直接返回false即可
    virtual bool OnWebBrowserPreKeyEvent(const cef_proxy::key_event_t& key_event) { return false; }
    virtual bool OnWebBrowserKeyEvent(const cef_proxy::key_event_t& key_event) { return false; }
    virtual bool OnWebBrowserMsgReceived(const std::string& msg_name, const cef_proxy::calldata* data) {
        return false;
    }

    // 三端统一通用性jsbridge
    virtual void OnWebBrowserGeneralEvent(const cef_proxy::browser_bind_data& bind_data, const std::string& ipc_message, const cef_proxy::calldata* data) {}

    virtual void OnWebBrowserPageArouse(const std::string& module_name, const std::string& param, bool close_current) {}
    // 处理H5页面的弹窗请求，不做处理的话（return false）就弹出模态框显示，不希望页面能弹窗的直接默认返回true（handled)即可
    virtual WebViewPopupType OnWebBrowserPopup(const std::string& url, int target_disposition) { return WebViewPopupType::NotAllow; }
    // 弹出具体页面的内容信息窗
    virtual void OnPopupWebviewDetails() {}

    // 弹窗关闭事件时，kBilibiliPageForward bridge关闭时需要回调
    virtual void OnWebBrowserWidgetDestroyed(views::Widget* widget) {}
};

