#include "stdafx.h"
#include "livehime_login_client_delegate.h"

#include <regex>

#include "bililive_browser/public/bililive_browser_ipc_messages.h"
#include "bililive_browser/public/bililive_browser_js_values.h"

#include "event_dispatcher/livehime_cef_proxy_events_dispatcher.h"

#include "public/livehime_cef_proxy_calldata.h"
#include "public/livehime_cef_proxy_constants.h"
#include "util/cef_proxy_util.h"
#include "bililive_browser/public/bililive_cef_headers.h"


namespace
{
    const char kUrlSplitMiniLogin[] = "/oauth/video-up";

    const std::string kUrlBilibili(R"(www.bilibili.tv)");

    bool g_sms_verify = false;

    std::string GetMiniLoginSplitUrl()
    {
        static std::string url = CefCommandLine::GetGlobalCommandLine()->GetSwitchValue("mini-login-url");
        if (url.empty())
        {
            url = kUrlSplitMiniLogin;
        }
        return url;
    }

    class AuthCookieVisitor : public CefCookieVisitor
    {
    public:
        AuthCookieVisitor(const cef_proxy::browser_bind_data& bind_data, const std::string& login_type)
            : bind_data_(bind_data),
            login_type_(login_type)
        {
        }

        virtual ~AuthCookieVisitor()
        {
            cef_proxy::calldata data;
            cef_proxy::calldata_filed data_filed;

            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
            data_filed.str_ = login_type_;
            data["details"] = data_filed;

            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_COOKIES;
            data_filed.cookies_ = std::move(bilibili_cookies_);
            data["cookie"] = data_filed;

            LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data_, ipc_messages::kMiniLoginCookies, &data);
        }

        virtual bool Visit(const CefCookie& cookie,
            int count,
            int total,
            bool& deleteCookie) override
        {
            static const std::string bilibiliViddup(".bilibili.tv");
            std::string domain = CefString(cookie.domain.str);

            if (domain == bilibiliViddup)
            {
                cef_proxy::cookie_t proxy_cookie;

                proxy_cookie.name = CefString(cookie.name.str);
                proxy_cookie.value = CefString(cookie.value.str);
                proxy_cookie.domain = CefString(cookie.domain.str);
                proxy_cookie.path = CefString(cookie.path.str);
                proxy_cookie.secure = cookie.secure;
                proxy_cookie.httponly = cookie.httponly;
                cef_time_t creation_time;
                cef_time_from_basetime(cookie.creation, &creation_time);
                cef_time_to_timet(&creation_time, &proxy_cookie.creation);
                cef_time_t last_access_time;
                cef_time_from_basetime(cookie.last_access, &last_access_time);
                cef_time_to_timet(&last_access_time, &proxy_cookie.last_access);
                proxy_cookie.has_expires = cookie.has_expires;
                cef_time_t expires_time;
                cef_time_from_basetime(cookie.expires, &expires_time);
                cef_time_to_timet(&expires_time, &proxy_cookie.expires);

                bilibili_cookies_.push_back(proxy_cookie);
            }
            return true;
        }

    private:
        cef_proxy::browser_bind_data bind_data_;
        std::string login_type_;
        cef_proxy::cookies bilibili_cookies_;

        IMPLEMENT_REFCOUNTING(AuthCookieVisitor);
    };
}


LivehimeLoginClientDelegate::LivehimeLoginClientDelegate(const cef_proxy::browser_bind_data& bind_data)
    : LivehimeCefClientBrowser::Delegate(bind_data)
{
}

LivehimeLoginClientDelegate::~LivehimeLoginClientDelegate()
{
}

void LivehimeLoginClientDelegate::OnBrowserCreated(CefRefPtr<CefBrowser> browser)
{
    DCHECK(!browser_);
    browser_ = browser;
}

void LivehimeLoginClientDelegate::OnBrowserClosed(CefRefPtr<CefBrowser> browser)
{
    DCHECK(browser_);
    if (browser->IsSame(browser_))
    {
        browser_ = nullptr;
    }
}

void LivehimeLoginClientDelegate::OnBeforeContextMenu(CefRefPtr<CefFrame> frame,
                                                          CefRefPtr<CefContextMenuParams> params,
                                                          CefRefPtr<CefMenuModel> model)
{
    model->Clear();
}

void LivehimeLoginClientDelegate::OnSetTitle(const CefString& new_title)
{
    CefString title = new_title;

    cef_proxy::calldata data;
    cef_proxy::calldata_filed data_filed;

    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
    data_filed.str_ = new_title.ToString();
    data["title"] = data_filed;

    // 二次验证的窗口是有我们自己的标题栏的，实时更新标题为网页标题
    LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), ipc_messages::kLivehimeCefOnTitleChanged, &data);
}

void LivehimeLoginClientDelegate::OnSetAddress(const CefString& url)
{
    std::string a_str(url.ToString());

    bool vld_complete = false;
    if ((a_str.compare("https://" + kUrlBilibili) == 0) ||
        (a_str.compare("http://" + kUrlBilibili) == 0) ||
        (a_str.compare("https://" + kUrlBilibili + "/") == 0) ||
        (a_str.compare("http://" + kUrlBilibili + "/") == 0))
    {
        vld_complete = true;
    }
    else if (g_sms_verify && (a_str.find(GetMiniLoginSplitUrl()) != std::string::npos))
    {
        g_sms_verify = false;
        vld_complete = true;
    }

    if (vld_complete)
    {
        // 各种二次验证完毕，结果已出，业务层应该校验登录态
        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), ipc_messages::kMiniLoginSecVldResult);

        // 二次验证结束了，但不知道验证结果如何，先把cookie拿出来，交给上层去调接口校验一下
        static std::string login_type(R"({"by":"second sign-in verification","type":"login"})");
        GetCookies(login_type);
    }
    else
    {
        // 不懂是什么case，让上层窗口放大展示
        if (a_str.find(GetMiniLoginSplitUrl()) == std::string::npos)
        {
            cef_proxy::calldata data;
            cef_proxy::calldata_filed data_filed;

            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
            data_filed.str_ = a_str;
            data["url"] = data_filed;

            LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), ipc_messages::kMiniLoginUnexpectedPage, &data);
        }
    }
    
}

void LivehimeLoginClientDelegate::OnLoadStart(CefRefPtr<CefFrame> frame, CefLoadHandler::TransitionType transition_type)
{
    if (frame->IsMain())
    {
        std::string version = GetExecutableVersion();
        std::string device_name = GetDeviceName();
        std::string device_platform = GetDevicePlatform();
        std::string buvid = GetBuvid3_();
        std::string app_key = "aae92bc66f3edfab";

        CefString eval_browser = "window.browser = {" \
                "version: {" \
                    "android: false, BiliApp: true, mobile: false," \
                    "ios: false, iPhone: false, iPad: false, MicroMessenger: false," \
                    "webApp: false, pc_link: true" \
                "}," \
                "language: 'zh-CN'," \
                "pc_link_scene: 'popup',"
                "app_version: '" + version + "',"\
                "appkey: '" + app_key + "',"\
                "device_name: '" + device_name + "',"\
                "device_platform: '" + device_platform + "',"\
                "buvid3: '" + buvid + "',"\
                "rollout: 'support-new-browser'"
            "}";

        CefString url = frame->GetURL();
        frame->ExecuteJavaScript(eval_browser, url, 0);
    }
}

void LivehimeLoginClientDelegate::OnLoadEnd(CefRefPtr<CefFrame> frame, int httpStatusCode)
{
}

void LivehimeLoginClientDelegate::OnLoadError(CefRefPtr<CefFrame> frame,
                                                  CefLoadHandler::ErrorCode errorCode,
                                                  const CefString& errorText,
                                                  const CefString& failedUrl)
{
}

bool LivehimeLoginClientDelegate::OnCertificateError(CefRefPtr<CefBrowser> browser,
                                                         cef_errorcode_t cert_error,
                                                         const CefString& request_url,
                                                         CefRefPtr<CefSSLInfo> ssl_info,
                                                         CefRefPtr<CefRequestCallback> callback)
{
    callback->Continue();
    return true;
}

bool LivehimeLoginClientDelegate::OnProcessMessageReceived(CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
    if (ScheduleCommonRendererIPCMessage(bind_data(), message))
    {
        return true;
    }
    else
    {
        std::string message_name = message->GetName();
        if (message_name == ipc_messages::kMiniLoginSuccess)
        {
            CefRefPtr<CefListValue> args = message->GetArgumentList();

            std::string login_type = args->GetString(0);

            cef_proxy::calldata data;
            cef_proxy::calldata_filed data_filed;

            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
            data_filed.str_ = login_type;
            data["details"] = data_filed;

            // 先通知上层登录成功了，让它改一下UI状态，稍后cookie送上去了它再进行有效性验证
            LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data_, message_name, &data);

            // 二次验证结束了，但不知道验证结果如何，先把cookie拿出来，交给上层去调接口校验一下
            GetCookies(login_type);

            return true;
        }
        else if (message_name == ipc_messages::kMiniLoginCancel)
        {
            LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), message_name);
            return true;
        }
        else if (message_name == ipc_messages::kMiniLoginChangeLoginMode)
        {
            CefRefPtr<CefListValue> args = message->GetArgumentList();

            cef_proxy::calldata data;
            cef_proxy::calldata_filed data_filed;

            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
            data_filed.numeric_union.int_ = args->GetInt(0);
            data["width"] = data_filed;

            data_filed.numeric_union.int_ = args->GetInt(1);
            data["height"] = data_filed;

            LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), message_name, &data);
            return true;
        }
        else if (message_name == ipc_messages::kMiniLoginSecVldResult)
        {
            static std::string login_type(R"({"by":"pw second sign-in verification","type":"login"})");
            cef_proxy::calldata data;
            cef_proxy::calldata_filed data_filed;

            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
            data_filed.str_ = login_type;
            data["details"] = data_filed;

            // 先通知上层二次验证结束了，让它改一下UI状态，稍后cookie送上去了它再进行有效性验证
            LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data_, message_name, &data);

            // 二次验证结束了，但不知道验证结果如何，先把cookie拿出来，交给上层去调接口校验一下
            GetCookies(login_type);

            return true;
        }
    }
    return false;
}

bool LivehimeLoginClientDelegate::DispatchJsEvent(const std::string& ipc_msg_name, const cef_proxy::calldata& arguments)
{
    return false;
}

void LivehimeLoginClientDelegate::GetCookies(const std::string& login_type)
{
    // 二次验证结束了，但不知道验证结果如何，先把cookie拿出来，交给上层去调接口校验一下
    // 获取cookie，把.bilibili.tv这个domain下的所有cookie都读出，读完再通知上层
    CefRefPtr<CefCookieManager> cm = browser_->GetHost()->GetRequestContext()->GetCookieManager(nullptr);
    CefRefPtr<AuthCookieVisitor> visitor = new AuthCookieVisitor(bind_data(), login_type);
    cm->VisitAllCookies(visitor);
}
