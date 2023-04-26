// cef_proxy_dll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "cef_proxy_dll.h"

#include <thread>
#include <functional>
#include <stdlib.h>

#include "util/cef_proxy_util.h"

#include "browser/livehime_cef_app_browser.h"
#include "browser/livehime_cef_client_browser.h"
#include "event_dispatcher/livehime_cef_proxy_events_dispatcher.h"
#include "public/livehime_cef_proxy_constants.h"

#include "bililive_browser/public/bililive_browser_ipc_messages.h"

#include "cef_core/include/cef_waitable_event.h"


extern std::string exe_version;
extern std::string device_name_;
extern std::string device_platform_;
extern std::string buvid3_;


namespace
{
    std::thread manager_thread;

    CefRefPtr<CefWaitableEvent> g_can_safely_cef_shutdown;

	struct SetCookieTaskInfo
	{
		int set_cookie_task_id = -1;
		size_t cookie_count = 0;
		size_t complete_count = 0;
		bool all_success = true;
	};
	std::map<int, SetCookieTaskInfo> g_set_cookie_tasks;

    class CefProxySetCookieCallback : public CefSetCookieCallback
    {
    public:
        CefProxySetCookieCallback(int set_cookie_task_id, const CefString& url, const CefCookie& cookie)
            : set_cookie_task_id_(set_cookie_task_id)
            , url_(url)
            , cookie_(cookie)
        {
        }

        void OnComplete(bool success) override
        {
            if (g_set_cookie_tasks.find(set_cookie_task_id_) == g_set_cookie_tasks.end())
            {
                return;
            }

            SetCookieTaskInfo& task_info = g_set_cookie_tasks.at(set_cookie_task_id_);
            task_info.complete_count++;
            task_info.all_success &= success;

            if (!success)
            {
                char expires[64] = { 0 };
                cef_time_t expires_time;
                cef_time_from_basetime(cookie_.expires, &expires_time);
                sprintf_s(expires, "%4d-%02d-%02d %02d:%02d:%02d",
                    expires_time.year, expires_time.month, expires_time.day_of_month,
                    expires_time.hour, expires_time.minute, expires_time.second);

                LOG(ERROR) << "set cookie item failed.\n\t" <<
                    "url: \"" << url_.ToString() << "\", " <<
                    "name: \"" << CefString(&cookie_.name).ToString() << "\", " <<
                    "value: \"" << CefString(&cookie_.value).ToString() << "\", " <<
                    "domain: \"" << CefString(&cookie_.domain).ToString() << "\", " <<
                    "has_expires: \"" << cookie_.has_expires << "\", " << 
                    "expires: \"" << expires << "\"";
            }

            if (task_info.complete_count == task_info.cookie_count)
            {
                cef_proxy::calldata data;
                cef_proxy::calldata_filed data_filed;

                data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
                data_filed.numeric_union.int_ = set_cookie_task_id_;
                data["task_id"] = data_filed;

                data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_BOOL;
                data_filed.numeric_union.bool_ = task_info.all_success;
                data["all_success"] = data_filed;

                LivehimeCefProxyEventsDispatcher::DispatchCommonEvent(
                    ipc_messages::kLivehimeCefSetCookieTaskCompleted, &data);
            }
        }

    private:
        int set_cookie_task_id_ = 0;
        CefString url_;
        CefCookie cookie_;

        IMPLEMENT_REFCOUNTING(CefProxySetCookieCallback);
        DISALLOW_COPY_AND_ASSIGN(CefProxySetCookieCallback);
    };

    class BrowserTask : public CefTask
    {
    public:
        std::function<void()> task;

        inline BrowserTask(std::function<void()> task_) : task(task_) {}

        void Execute() override { task(); }

        IMPLEMENT_REFCOUNTING(BrowserTask);
    };

}

CEF_PROXY_DLL_API void cef_proxy_init_module(void)
{
    DCHECK(!manager_thread.joinable());

    static volatile std::atomic_bool manager_initialized = false;
    if (!manager_initialized.exchange(true))
    {
        g_can_safely_cef_shutdown = CefWaitableEvent::CreateWaitableEvent(false, true);
        _putenv(("g_can_safely_cef_shutdown=" + std::to_string((uint64)g_can_safely_cef_shutdown.get())).c_str());

        manager_thread = std::thread(internal::BrowserManagerThread);
    }
}

CEF_PROXY_DLL_API void cef_proxy_shutdown_module(void)
{
    cef_proxy::CloseAllBrowsers();

    if (manager_thread.joinable())
    {
        if (g_can_safely_cef_shutdown)
        {
#ifdef _DEBUG
            g_can_safely_cef_shutdown->Wait();
#else
            g_can_safely_cef_shutdown->TimedWait(5000);
#endif
            g_can_safely_cef_shutdown = nullptr;
        }

        if (internal::IsCefInitializeSucceeded())
        {
            // 解决关闭时Check failed: !IsCefShutdown(). Object reference incorrectly held at CefShutdown问题
            // 只有当关闭Browser后收到OnBeforeClose时才是安全的，否则会出现上面的FATAL错误问题
            // 如未完全关闭，暂时会尝试5次每次1s后进行强制退出原有机制，本地测试35个Browser均可正常退出
            for (int i = 0; i < 5; i++)
            {
                if (cef_proxy::CanCefShutdown())
                {
                    break;
                }
                else
                {
                    Sleep(1);
                }
            }

            //DCHECK(cef_proxy::CanCefShutdown());

            for (int i = 0; i < 5; i++)
            {
                if (CefPostTask(TID_UI, CefRefPtr<BrowserTask>(new BrowserTask([]() { CefQuitMessageLoop(); }))))
                {
                    break;
                }
                else
                {
                    Sleep(5);
                }
            }
        }

        manager_thread.join();
    }
}

CEF_PROXY_DLL_API void cef_proxy_set_version(const std::string& version, const std::string& device_name, const std::string& device_platform, const std::string& buvid3)
{
    exe_version = version;
    device_name_ = device_name;
    device_platform_ = device_platform;
    buvid3_ = buvid3;
}

CEF_PROXY_DLL_API bool cef_proxy_set_cookie(int set_cookie_task_id, const std::string& cookies, const std::string& domains)
{
    CefRefPtr<CefCookieManager> cookie_mng = CefCookieManager::GetGlobalManager(nullptr);
    if (cookie_mng)
    {
        // 解析domain
        std::vector<std::string> cef_domains;
        CefRefPtr<CefValue> domains_value = CefParseJSON(domains, JSON_PARSER_RFC);
        if (domains_value && domains_value->IsValid())
        {
            CefRefPtr<CefListValue> domains_list = domains_value->GetList();
            if (domains_list && domains_list->IsValid())
            {
                for (size_t i = 0; i < domains_list->GetSize(); ++i)
                {
                    cef_domains.push_back(domains_list->GetString(i).ToString());
                }
            }
        }

        if (cef_domains.empty())
        {
            LOG(ERROR) << "cookie domains parse failed. \n\t" << domains;
            return false;
        }

        // 解析cookie
        std::vector<CefCookie> cef_cookies;
        CefRefPtr<CefValue> cookies_value = CefParseJSON(cookies, JSON_PARSER_RFC);
        if (cookies_value && cookies_value->IsValid())
        {
            CefRefPtr<CefListValue> cookies_list = cookies_value->GetList();
            if (cookies_list && cookies_list->IsValid())
            {
                for (size_t i = 0; i < cookies_list->GetSize(); ++i)
                {
                    CefRefPtr<CefDictionaryValue> cookie_dct = cookies_list->GetValue(i)->GetDictionary();
                    if (cookie_dct && cookie_dct->IsValid())
                    {
                        CefCookie cookie;

                        //CefString(&cookie.domain) = domain;
                        CefString(&cookie.name) = cookie_dct->GetString("name");
                        CefString(&cookie.value) = cookie_dct->GetString("value");
                        cookie.httponly = cookie_dct->GetInt("http_only");
                        double db_expires = cookie_dct->GetDouble("expires");
                        if (db_expires != 0)
                        {
                            cookie.has_expires = 1;
                            cef_time_t expires_time;
                            cef_time_from_timet(db_expires, &expires_time);
                            cef_time_to_basetime(&expires_time, &cookie.expires);
                        }

                        cef_cookies.push_back(cookie);
                    }
                }
            }
        }

        if (cef_cookies.empty())
        {
            LOG(ERROR) << "cookie items parse failed. \n\t" << cookies;
            return false;
        }

        // 开始写cookie
        bool set_cookies_success = true;
        std::string domain;
        CefString url;
        CefCookie cookie;

        // 记录SetCookie任务
        SetCookieTaskInfo task_info;
        task_info.complete_count = 0;
        task_info.cookie_count = cef_domains.size() * cef_cookies.size();
        task_info.set_cookie_task_id = set_cookie_task_id;
        g_set_cookie_tasks[set_cookie_task_id] = task_info;

        for (size_t i = 0; i < cef_domains.size(); i++)
        {
            domain = cef_domains[i];
            url = "https://" + domain;

            for (size_t j = 0; j < cef_cookies.size(); j++)
            {
                cookie = cef_cookies[j];

                CefString(&cookie.domain) = domain;

                CefRefPtr<CefSetCookieCallback> callback =
                    new CefProxySetCookieCallback(set_cookie_task_id, url, cookie);
                set_cookies_success &= cookie_mng->SetCookie(url, cookie, callback);
            }
        }
        if (!set_cookies_success)
        {
            LOG(ERROR) << "set cookie task post failed.";
        }
        return set_cookies_success;
    }
    return false;
}

CEF_PROXY_DLL_API bool cef_proxy_set_cef_cookie(int set_cookie_task_id, const cef_proxy::cookies& cookies)
{
    CefRefPtr<CefCookieManager> cookie_mng = CefCookieManager::GetGlobalManager(nullptr);
    if (cookie_mng)
    {
        if (cookies.empty())
        {
            LOG(ERROR) << "cef-cookie items empty.";
            return false;
        }

        // 开始写cookie
        bool set_cookies_success = true;

        CefString url("https://.bilibili.tv");
       
        CefCookie cookie;

        // 记录SetCookie任务
        SetCookieTaskInfo task_info;
        task_info.complete_count = 0;
        task_info.cookie_count = cookies.size();
        task_info.set_cookie_task_id = set_cookie_task_id;
        g_set_cookie_tasks[set_cookie_task_id] = task_info;

        for (size_t j = 0; j < cookies.size(); j++)
        {
            const cef_proxy::cookie_t& cef_cookie = cookies.at(j);

            CefString(&cookie.name) = cef_cookie.name;
            CefString(&cookie.value) = cef_cookie.value;
            CefString(&cookie.domain) = cef_cookie.domain;
            CefString(&cookie.path) = cef_cookie.path;
            cookie.secure = cef_cookie.secure;
            cookie.httponly = cef_cookie.httponly;
            cef_time_t creation_time;
            cef_time_from_timet(cef_cookie.creation, &creation_time);
            cef_time_to_basetime(&creation_time, &cookie.creation);
            cef_time_t last_access_time;
            cef_time_from_timet(cef_cookie.last_access, &last_access_time);
            cef_time_to_basetime(&last_access_time, &cookie.last_access);
            cookie.has_expires = cef_cookie.has_expires;
            cef_time_t expires_time;
            cef_time_from_timet(cef_cookie.expires, &expires_time);
            cef_time_to_basetime(&expires_time, &cookie.expires);

            CefRefPtr<CefSetCookieCallback> callback =
                new CefProxySetCookieCallback(set_cookie_task_id, url, cookie);
            set_cookies_success &= cookie_mng->SetCookie(url, cookie, callback);
        }

        if (!set_cookies_success)
        {
            LOG(ERROR) << "set cef-cookie task post failed.";
        }
        return set_cookies_success;
    }
    return false;
}

CEF_PROXY_DLL_API bool cef_proxy_delete_cookie()
{
    bool do_cookies_success = true;
    CefRefPtr<CefCookieManager> cookie_mng = CefCookieManager::GetGlobalManager(nullptr);
    if (cookie_mng)
    {

        CefString urlIntl("https://.bilibili.tv");

        static const std::vector<std::string> kCookieNames{
            "_uuid",
            "buvid3",
            "DedeUserID",
            "DedeUserID__ckMd5",
            "PVID",
            "SESSDATA",
            "bili_jct",
            "sid",
        };

        for (auto& iter : kCookieNames)
        {
            do_cookies_success &= cookie_mng->DeleteCookies(urlIntl, iter, nullptr);
        }
    }
    return do_cookies_success;
}

CEF_PROXY_DLL_API void cef_proxy_register_callback(const cef_proxy::cef_proxy_callback& callbacks)
{
    LivehimeCefProxyEventsDispatcher::RegisterCallbacks(callbacks);
}

CEF_PROXY_DLL_API bool cef_proxy_execute_js_function(const cef_proxy::browser_bind_data& bind_data, const std::string& func_name,
                                                     const cef_proxy::calldata_list* const arguments)
{
    return cef_proxy::ExecuteJSFunction(bind_data, func_name, arguments);
}

CEF_PROXY_DLL_API bool cef_proxy_execute_js_function_with_kv(const cef_proxy::browser_bind_data& bind_data, const std::string& func_name,
                                                             const cef_proxy::calldata* const arguments)
{
    return cef_proxy::ExecuteJSFunctionWithKV(bind_data, func_name, arguments);
}

CEF_PROXY_DLL_API bool cef_proxy_dispatch_js_event(const cef_proxy::browser_bind_data& bind_data,
    const std::string& ipc_msg_name, const cef_proxy::calldata* const arguments)
{
    return cef_proxy::DispatchJsEvent(bind_data, ipc_msg_name, arguments);
}

CEF_PROXY_DLL_API bool cef_proxy_create_popup_browser(const cef_proxy::browser_bind_data& bind_data, HWND par_hwnd, const std::string& url,
                                                      const std::string& post_data /*= ""*/, const std::string& headers /*= ""*/)
{
    return cef_proxy::CreatePopupBrowser(bind_data, par_hwnd, url, post_data, headers);
}

CEF_PROXY_DLL_API bool cef_proxy_create_embedded_browser(const cef_proxy::browser_bind_data& bind_data, HWND par_hwnd, const RECT& rc, const std::string& url,
                                                         const std::string& post_data /*= ""*/, const std::string& headers /*= ""*/)
{
    return cef_proxy::CreateEembeddedBrowser(bind_data, par_hwnd, rc, url, post_data, headers);
}

CEF_PROXY_DLL_API bool cef_proxy_create_osr_browser(const cef_proxy::browser_bind_data& bind_data, const RECT& rc,
    int fps, cef_proxy::cef_proxy_osr_onpaint cb,
    const std::string& url,
    const std::string& post_data /*= ""*/, const std::string& headers /*= ""*/)
{
    return cef_proxy::CreateOsrBrowser(bind_data, rc, fps, cb, url, post_data, headers);
}

CEF_PROXY_DLL_API void cef_proxy_send_mouse_click(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, cef_proxy::MouseButtonType type, bool mouse_up, uint32_t click_count)
{
    cef_proxy::SendMouseClick(bind_data, event, type, mouse_up, click_count);
}

CEF_PROXY_DLL_API void cef_proxy_send_mouse_move(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, bool mouse_leave)
{
    cef_proxy::SendMouseMove(bind_data, event, mouse_leave);
}

CEF_PROXY_DLL_API void cef_proxy_send_mouse_wheel(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, int x_delta, int y_delta)
{
    cef_proxy::SendMouseWheel(bind_data, event, x_delta, y_delta);
}

CEF_PROXY_DLL_API void cef_proxy_send_focus(const cef_proxy::browser_bind_data& bind_data, bool focus)
{
    cef_proxy::SendFocus(bind_data, focus);
}

CEF_PROXY_DLL_API void cef_proxy_send_key_click(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::KeyEvent& event)
{
    cef_proxy::SendKeyClick(bind_data, event);
}

CEF_PROXY_DLL_API void cef_proxy_ime_commit_text(const cef_proxy::browser_bind_data& bind_data,
    const std::wstring& text,
    int from_val,
    int to_val,
    int relative_cursor_pos) {
    cef_proxy::ImeCommitText(bind_data, text, from_val, to_val, relative_cursor_pos);
}

CEF_PROXY_DLL_API void cef_proxy_ime_finish_composing_text(const cef_proxy::browser_bind_data& bind_data, bool keep_selection) {
    cef_proxy::ImeFinishComposingText(bind_data, keep_selection);
}

CEF_PROXY_DLL_API void cef_proxy_ime_cancel_composition(const cef_proxy::browser_bind_data& bind_data) {
    cef_proxy::ImeCancelComposition(bind_data);
}

CEF_PROXY_DLL_API void cef_proxy_close_browser(const cef_proxy::browser_bind_data& bind_data)
{
    cef_proxy::CloseBrowser(bind_data);
}

CEF_PROXY_DLL_API bool cef_proxy_load_url(const cef_proxy::browser_bind_data& bind_data, const std::string& url)
{
    return cef_proxy::LoadUrl(bind_data, url);
}

CEF_PROXY_DLL_API void cef_proxy_resize_browser(const cef_proxy::browser_bind_data& bind_data, const RECT& bounds)
{
    cef_proxy::ResizeBrowser(bind_data, bounds);
}

CEF_PROXY_DLL_API void cef_proxy_repaint_browser(const cef_proxy::browser_bind_data& bind_data)
{
    cef_proxy::RepaintBrowser(bind_data);
}

CEF_PROXY_DLL_API void cef_proxy_update_offline_switch(bool offline_switch)
{
    cef_proxy::UpdateOfflineSwitch(offline_switch);
}

CEF_PROXY_DLL_API void cef_proxy_update_offline_config(const cef_proxy::offline_config* const configs, cef_proxy::offline_config_callback_fn callback)
{
    cef_proxy::UpdateOfflineConfig(configs, callback);
}