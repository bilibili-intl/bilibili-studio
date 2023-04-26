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
    //������Ϸ���� �����¼�
    virtual void OnInteractivePlayCenterEvent(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::calldata* data) {}
    //virtual void OnWebBrowserWindowClose() {}
    // �����¼��Ĵ���û�������߼���Ҫ��ֹwebview�Լ��İ����������̵ľ�ֱ�ӷ���false����
    virtual bool OnWebBrowserPreKeyEvent(const cef_proxy::key_event_t& key_event) { return false; }
    virtual bool OnWebBrowserKeyEvent(const cef_proxy::key_event_t& key_event) { return false; }
    virtual bool OnWebBrowserMsgReceived(const std::string& msg_name, const cef_proxy::calldata* data) {
        return false;
    }

    // ����ͳһͨ����jsbridge
    virtual void OnWebBrowserGeneralEvent(const cef_proxy::browser_bind_data& bind_data, const std::string& ipc_message, const cef_proxy::calldata* data) {}

    virtual void OnWebBrowserPageArouse(const std::string& module_name, const std::string& param, bool close_current) {}
    // ����H5ҳ��ĵ������󣬲�������Ļ���return false���͵���ģ̬����ʾ����ϣ��ҳ���ܵ�����ֱ��Ĭ�Ϸ���true��handled)����
    virtual WebViewPopupType OnWebBrowserPopup(const std::string& url, int target_disposition) { return WebViewPopupType::NotAllow; }
    // ��������ҳ���������Ϣ��
    virtual void OnPopupWebviewDetails() {}

    // �����ر��¼�ʱ��kBilibiliPageForward bridge�ر�ʱ��Ҫ�ص�
    virtual void OnWebBrowserWidgetDestroyed(views::Widget* widget) {}
};

