#include "stdafx.h"
#include "livehime_cef_client_browser.h"

#include "bililive_browser/public/bililive_browser_ipc_messages.h"

#include "bilibili/livehime_bilibili_client_delegate.h"
#include "biliyun/livehime_biliyun_client_delegate.h"
#include "dev_tools/livehime_dev_tools_client_delegate.h"
#include "event_dispatcher/livehime_cef_proxy_events_dispatcher.h"
#include "mini_login/livehime_login_client_delegate.h"


namespace
{
    class LivehimeCefClientDelegate : public LivehimeCefClientBrowser::Delegate
    {
    public:
        explicit LivehimeCefClientDelegate(const cef_proxy::browser_bind_data& bind_data)
            : LivehimeCefClientBrowser::Delegate(bind_data)
        {
        }
        
        virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) override
        {
            browser_ = browser;
        }

        virtual void OnBrowserClosed(CefRefPtr<CefBrowser> browser) override
        {
            if (browser_)
            {
                DCHECK(browser_->IsSame(browser));
                browser_ = nullptr;
            }
        }
        
        virtual void OnSetTitle(const CefString& title) override
        {
            CefWindowHandle hwnd = browser_->GetHost()->GetWindowHandle();
            ::SetWindowText(hwnd, std::wstring(title).c_str());
        }

        virtual void OnLoadError(CefRefPtr<CefFrame> frame, CefLoadHandler::ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl) override
        {
            // Display a load error message.
            std::stringstream ss;
            ss << "<html><body bgcolor=\"white\">"
                "<h2>Failed to load URL "
                << std::string(failedUrl) << " with error " << std::string(errorText)
                << " (" << errorCode << ").</h2></body></html>";
            //frame->LoadString(ss.str(), failedUrl);  //DISCARD
            //DCHECK(false);
        }

        virtual void OnBeforeContextMenu(CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) override
        {
            /* remove all context menu contributions */
            model->Clear();
        }

        virtual bool OnProcessMessageReceived(CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override
        {
            std::string message_name = message->GetName();
            if (message_name == ipc_messages::kLivehimeCefJsMsgForTest ||
                message_name == ipc_messages::kLivehimeCefOnWindowClose)
            {
                LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), message_name, nullptr);
                return true;
            }
            return false;
        }

    private:
        CefRefPtr<CefBrowser> browser_;

        // Include the default reference counting implementation.
        IMPLEMENT_REFCOUNTING(LivehimeCefClientDelegate);
        DISALLOW_COPY_AND_ASSIGN(LivehimeCefClientDelegate);
    };
}


CefRefPtr<LivehimeCefClientBrowser::Delegate> LivehimeCefClientBrowser::CreateDelegate(const cef_proxy::browser_bind_data &bind_data)
{
    CefRefPtr<LivehimeCefClientBrowser::Delegate> delegate;
    switch (bind_data.type)
    {
    case cef_proxy::client_handler_type::dev_tools:
        delegate = new LivehimeDevToolsClientDelegate(bind_data);
        break;
    case cef_proxy::client_handler_type::bilibili:
        delegate = new LivehimeBilibiliClientDelegate(bind_data);
        break;
    case cef_proxy::client_handler_type::geetest:
        break;
    case cef_proxy::client_handler_type::sec_sign_verify:
        break;
    case cef_proxy::client_handler_type::lucky_gift:
    case cef_proxy::client_handler_type::lucky_gift_award:
        break;
    case cef_proxy::client_handler_type::lottery:
        break;
    case cef_proxy::client_handler_type::biliyun:
        delegate = new LivehimeBiliyunClientDelegate(bind_data);
        break;
    case cef_proxy::client_handler_type::mini_login:
        delegate = new LivehimeLoginClientDelegate(bind_data);
        break;
    default:
        delegate = new LivehimeCefClientDelegate(bind_data);
        break;
    }
    return delegate;
}
