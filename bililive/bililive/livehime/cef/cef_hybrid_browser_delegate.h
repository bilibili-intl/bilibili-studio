#pragma once

#include <vector>

#include "base/strings/string16.h"

#include "bililive/bililive/ui/views/controls/browser/livehime_web_browser_delegate.h"


class CefHybridBrowserDelegate : public LivehimeWebBrowserDelegate {
public:
    CefHybridBrowserDelegate();
    ~CefHybridBrowserDelegate();

    static CefHybridBrowserDelegate* GetInstance();

    void DelayDispatchJsEvent(const cef_proxy::browser_bind_data& bind_data,
        const std::string& ipc_msg_name, const cef_proxy::calldata arguments);

    // �����ر��¼�ʱ��kBilibiliPageForward bridge�ر�ʱ��Ҫ�ص�
    void OnWebBrowserWidgetDestroyed(views::Widget* widget) override;

private:
    cef_proxy::browser_bind_data bind_data_;
    std::string ipc_msg_name_;
    cef_proxy::calldata arguments_;
};