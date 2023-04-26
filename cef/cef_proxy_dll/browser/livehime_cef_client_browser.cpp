#include "stdafx.h"
#include "livehime_cef_client_browser.h"

#include <mutex>
#include <WinInet.h>

#include "bililive_browser/public/bililive_browser_ipc_messages.h"
#include "bililive_browser/public/bililive_browser_switches.h"
#include "bililive_browser/public/bililive_cef_headers.h"

#include "cef_core/include/cef_app.h"
#include "cef_core/include/cef_browser.h"
#include "cef_core/include/base/cef_callback.h"
#include "cef_core/include/wrapper/cef_closure_task.h"

#include "cef_proxy_dll/browser/livehime_cef_resource_handler.h"
#include "cef_proxy_dll/event_dispatcher/livehime_cef_proxy_events_dispatcher.h"
#include "cef_proxy_dll/public/livehime_cef_proxy_constants.h"


    void replace_str(std::string& str, const std::string& before, const std::string& after)
    {
        for (std::string::size_type pos(0); pos != std::string::npos; pos += after.length())
        {
            pos = str.find(before, pos);
            if (pos != std::string::npos)
                str.replace(pos, before.length(), after);
            else
                break;
        }
    }

    std::pair<std::string, std::string> SplitLocalStorageData(const std::string& storagedata)
    {
        std::string split_string = "||";
        auto query_leading_pos = storagedata.find(split_string);
        if (query_leading_pos == std::string::npos)
        {
            return { storagedata, std::string() };
        }

        auto key = storagedata.substr(0, query_leading_pos);
        auto value = storagedata.substr(query_leading_pos + split_string.length());

        return { key, value };
    }

    CefString CreateLocalStorageCefString(const std::string& key, const std::string& value)
    {
        auto r_key = key;
        auto r_value = value;

        auto before_str = "\"";
        auto after_str = "\\\"";
        replace_str(r_key, before_str, after_str);
        replace_str(r_value, before_str, after_str);

        std::string local_storage;
        local_storage = "window.localStorage.setItem";
        local_storage.append("(\"").append(r_key).append("\"").append(",");
        local_storage.append("\"").append(r_value).append("\"").append(")");

        return CefString(local_storage.c_str());
    }

    // 对CEF做操作时很多情况下都需要拿到对应的CefBrowser，但是我们创建CefBrowser时只能采用异步的
    // CefBrowserHost::CreateBrowser，不能采用同步的CefBrowserHost::CreateBrowserSync（必须在browser线程才能调），
    // 所以为了能拿到指定的CefBrowser对象，就从和CefBrowser一对一绑定的LivehimeCefClientBrowser去拿即可
    std::recursive_mutex client_list_lock;
    std::list<CefRefPtr<LivehimeCefClientBrowser>> g_client_list;

    // List of existing browser windows. Only accessed on the CEF UI thread.
    typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
    BrowserList g_browser_list_;

    CefRefPtr<LivehimeCefClientBrowser> GetCefClient(const cef_proxy::browser_bind_data& bind_data, bool remove/* = false*/)
    {
        std::lock_guard<std::recursive_mutex> lock(client_list_lock);
        CefRefPtr<LivehimeCefClientBrowser> client;
        auto iter = std::find_if(g_client_list.begin(), g_client_list.end(),
            [&bind_data](CefRefPtr<LivehimeCefClientBrowser> client)->bool {
            return (client->bind_data() == bind_data);
        });
        if (iter != g_client_list.end())
        {
            client = (*iter);

            if (remove)
            {
                g_client_list.erase(iter);
            }
        }

        return client;
    }

    // 确保上层调用CreateBrowser之后关闭窗口时必须对应调用一次CloseBrowser
    class CheckAllBrowserClosed
    {
    public:
        CheckAllBrowserClosed() = default;

        ~CheckAllBrowserClosed()
        {
            DCHECK(g_client_list.empty()) << "must close browser explicit by call cef_proxy::CloseBrowser";
        }
    };

    CheckAllBrowserClosed g_check_browsers_lifespan;

    static __int64 dev_tools_bind_data_id = -1;

bool LivehimeCefClientBrowser::offline_switch_ = false;

cef_proxy::offline_config LivehimeCefClientBrowser::offline_configs_;
cef_proxy::offline_config_callback_fn LivehimeCefClientBrowser::offline_callback_ = nullptr;

LivehimeCefClientBrowser::LivehimeCefClientBrowser(const cef_proxy::browser_bind_data& bind_data,
    const std::string& post_data/* = ""*/)
    : bind_data_(bind_data)
    , post_data_(post_data)
{
    delegate_ = CreateDelegate(bind_data_);

    g_client_list.push_back(this);
}

LivehimeCefClientBrowser::LivehimeCefClientBrowser(const cef_proxy::browser_bind_data& bind_data,
    const RECT& osr_rect, int fps, cef_proxy::cef_proxy_osr_onpaint cb,
    const std::string& post_data /*= ""*/)
    : LivehimeCefClientBrowser(bind_data, post_data)
{
    osr_mode_ = true;
    osr_rect_ = osr_rect;
    osr_fps_ = fps;
    osr_onpaint_cb_ = cb;
}

LivehimeCefClientBrowser::~LivehimeCefClientBrowser()
{
}

CefRefPtr<CefDisplayHandler> LivehimeCefClientBrowser::GetDisplayHandler()
{
    return this;
}

CefRefPtr<CefLifeSpanHandler> LivehimeCefClientBrowser::GetLifeSpanHandler()
{
    return this;
}

CefRefPtr<CefLoadHandler> LivehimeCefClientBrowser::GetLoadHandler()
{
    return this;
}

CefRefPtr<CefContextMenuHandler> LivehimeCefClientBrowser::GetContextMenuHandler()
{
    return this;
}

CefRefPtr<CefDragHandler> LivehimeCefClientBrowser::GetDragHandler()
{
    return this;
}

CefRefPtr<CefRequestHandler> LivehimeCefClientBrowser::GetRequestHandler()
{
    return this;
}

CefRefPtr<CefKeyboardHandler> LivehimeCefClientBrowser::GetKeyboardHandler()
{
    return this;
}

CefRefPtr<CefRenderHandler> LivehimeCefClientBrowser::GetRenderHandler()
{
    return this;
}

bool LivehimeCefClientBrowser::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{
    CEF_REQUIRE_UI_THREAD();

    if (frame)
    {
        LOG(INFO) << "[CP] webview OnProcessMessageReceived, name= " << message->GetName().ToString() << ", page= " << frame->GetURL().ToString();
    }

    if (delegate_)
    {
        return delegate_->OnProcessMessageReceived(source_process, message);
    }
    return false;
}

void LivehimeCefClientBrowser::OnLoadingProgressChange(CefRefPtr<CefBrowser> browser, double progress)
{
    CEF_REQUIRE_UI_THREAD();

    if (delegate_)
    {
        delegate_->OnLoadingProgressChange(progress);
    }
}

void LivehimeCefClientBrowser::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
    CEF_REQUIRE_UI_THREAD();

    cef_proxy::calldata data;
    cef_proxy::calldata_filed data_filed;

    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
    data_filed.wstr_ = title.ToWString();
    data["title"] = data_filed;

    LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), ipc_messages::kLivehimeCefOnLoadTitle, &data);

    if (delegate_)
    {
        delegate_->OnSetTitle(title);
    }
}

void LivehimeCefClientBrowser::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{
    CEF_REQUIRE_UI_THREAD();

    if (frame->IsMain())
    {
        if (delegate_)
        {
            delegate_->OnSetAddress(url);
        }
    }
}

bool LivehimeCefClientBrowser::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
    cef_log_severity_t level,
    const CefString& message,
    const CefString& source,
    int line)
{
    /*if (level < LOGSEVERITY_INFO)
#ifdef _DEBUG
        return false;
#else
        return true;
#endif

    return false;*/

    // Debug模式下web console信息全打，Release模式下INFO及以上的信息才打
    bool need_log = (level >= LOGSEVERITY_INFO);
    if (level < LOGSEVERITY_INFO)
    {
#ifdef _DEBUG
        need_log = true;
#endif
    }

    if (need_log)
    {
        CefString url;
        if (browser && browser->GetMainFrame())
        {
            url = browser->GetMainFrame()->GetURL();
        }
        switch (level)
        {
        case LOGSEVERITY_DEBUG:
        case LOGSEVERITY_INFO:
            LOG(INFO) << "CONSOLE: \"" << message.ToString() << "\", \n\tsource: \"" << source.ToString() << " (line: " << line << ")\", "
                << "url: \"" << url.ToString() << "\"";
            break;
        case LOGSEVERITY_WARNING:
            LOG(WARNING) << "CONSOLE: \"" << message.ToString() << "\", \n\tsource: \"" << source.ToString() << " (line: " << line << ")\", "
                << "url: \"" << url.ToString() << "\"";
            break;
        case LOGSEVERITY_ERROR:
            LOG(ERROR) << "CONSOLE: \"" << message.ToString() << "\", \n\tsource: \"" << source.ToString() << " (line: " << line << ")\", "
                << "url: \"" << url.ToString() << "\"";
            break;
        case LOGSEVERITY_FATAL:
            LOG(FATAL) << "CONSOLE: \"" << message.ToString() << "\", \n\tsource: \"" << source.ToString() << " (line: " << line << ")\", "
                << "url: \"" << url.ToString() << "\"";
            break;
        default:
            break;
        }
    }

    // 就不用CEF底层打了，我们直接在上层自己打
    return true;
}

bool LivehimeCefClientBrowser::OnDragEnter(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDragData> dragData,
    CefDragHandler::DragOperationsMask mask)
{
    return false;
}

bool LivehimeCefClientBrowser::OnBeforePopup(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    const CefString& target_url,
    const CefString& target_frame_name,
    CefRequestHandler::WindowOpenDisposition target_disposition,
    bool user_gesture,
    const CefPopupFeatures& popupFeatures,
    CefWindowInfo& windowInfo,
    CefRefPtr<CefClient>& client,
    CefBrowserSettings& settings,
    CefRefPtr<CefDictionaryValue>& extra_info,
    bool* no_javascript_access)
{
    CEF_REQUIRE_UI_THREAD();

    cef_proxy::calldata data;
    cef_proxy::calldata_filed data_filed;
    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

    data_filed.str_ = target_url;
    data["target_url"] = data_filed;

    data_filed.reset();
    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
    data_filed.numeric_union.int_ = target_disposition;
    data["target_disposition"] = data_filed;

    LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), ipc_messages::kLivehimeCefOnPopup, &data);

    /* block popups */
    return true;
}

void LivehimeCefClientBrowser::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    if (browser)
    {
        LOG(INFO) << "[CP] webview OnAfterCreated, browser=" << browser;
    }

    browser_ = browser;

    if (delegate_)
    {
        delegate_->OnBrowserCreated(browser);
    }

    // Add to the list of existing browsers.
    g_browser_list_.push_back(browser);
}

bool LivehimeCefClientBrowser::DoClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    if (browser)
    {
        LOG(INFO) << "[CP] webview DoClose, browser=" << browser << ", page=" << browser->GetMainFrame()->GetURL().ToString();
    }

    if (delegate_)
    {
        delegate_->OnBrowserClosing(browser);

        // 开发者工具自己关闭
        if (delegate_->bind_data().type == cef_proxy::client_handler_type::dev_tools)
        {
            return false;
        }
    }

    if (osr_mode_)
    {
        return false;
    }

    // Allow the close. For windowed browsers this will result in the OS close
    // event being sent.
    return true;
}

void LivehimeCefClientBrowser::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    if (browser)
    {
        LOG(INFO) << "[CP] webview OnBeforeClose, browser=" << browser << ", page=" << browser->GetMainFrame()->GetURL().ToString();
    }

    if (delegate_)
    {
        delegate_->OnBrowserClosed(browser);

        // 调试工具是CEF自己的弹窗，不会走我们自己的void CloseBrowser(const browser_bind_data& bind_data)，
        // 所以在这里主动移除LivehimeCefClientBrowser实例
        if (delegate_->bind_data().type == cef_proxy::client_handler_type::dev_tools)
        {
            GetCefClient(bind_data_, true);
        }
    }

    browser_ = nullptr;

    LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), ipc_messages::kLivehimeCefClientDestoryed);

    // Remove from the list of existing browsers.
    BrowserList::iterator bit = g_browser_list_.begin();
    for (; bit != g_browser_list_.end(); ++bit) {
        if ((*bit)->IsSame(browser)) {
            g_browser_list_.erase(bit);
            break;
        }
    }
}

void LivehimeCefClientBrowser::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
    CEF_REQUIRE_UI_THREAD();

    if (isLoading) {
        CefString url("");

        std::string key;
        std::string value;
        std::tie(key, value) = SplitLocalStorageData(post_data_);

        if (key.empty() || value.empty()) {
            return;
        }

        CefString local_storage = CreateLocalStorageCefString(key, value);
        if (browser_ && browser_->GetMainFrame())
        {
            browser_->GetMainFrame()->ExecuteJavaScript(local_storage, url, 0);
        }
    }

    if (delegate_)
    {
        delegate_->OnSetLoadingState(isLoading, canGoBack, canGoForward);
    }
}

void LivehimeCefClientBrowser::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type)
{
    CEF_REQUIRE_UI_THREAD();

    if (frame)
    {
        LOG(INFO) << "[CP] webview OnLoadStart, browser=" << browser << ", page=" << frame->GetURL().ToString();
    }

    cef_proxy::calldata data;
    cef_proxy::calldata_filed data_filed;

    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
    data_filed.str_ = frame->GetURL().ToString();
    data["url"] = data_filed;

    LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), ipc_messages::kLivehimeCefOnLoadStart, &data);

    if (delegate_)
    {
        delegate_->OnLoadStart(frame, transition_type);
    }
}

void LivehimeCefClientBrowser::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    CEF_REQUIRE_UI_THREAD();

    if (frame)
    {
        LOG(INFO) << "[CP] webview OnLoadEnd, browser=" << browser << ", code=" << httpStatusCode << ", page=" << frame->GetURL().ToString();
    }

    std::string url = frame->GetURL().ToString();
    // 加载本地功能页面的返回值是0，不能算错误
    static std::vector<std::string> kLocalPage{
        "about:blank",
    };
    if (kLocalPage.end() != std::find_if(kLocalPage.begin(), kLocalPage.end(), [&](const std::string& l_url)->bool {
            return url == l_url;
        }))
    {
        if (httpStatusCode == 0)
        {
            httpStatusCode = HTTP_STATUS_OK;
        }
    }
    // 加载的是本地HTML文件(file:///)时这个httpStatusCode在成功时为0,
    if (url.length() >= 8)
    {
        if (0 == url.substr(0, 8).compare("file:///"))
        {
            if (httpStatusCode == 0)
            {
                httpStatusCode = HTTP_STATUS_OK;
            }
        }
    }

    cef_proxy::calldata data;
    cef_proxy::calldata_filed data_filed;

    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
    data_filed.str_ = url;
    data["url"] = data_filed;

    data_filed.reset();
    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
    data_filed.numeric_union.int_ = httpStatusCode;
    data["http_status_code"] = data_filed;

    LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), ipc_messages::kLivehimeCefOnLoadEnd, &data);

    if (httpStatusCode != HTTP_STATUS_OK)
    {
        LOG(WARNING) << "[CP] webview load url failed, code=" << httpStatusCode << ", \n\t" <<
            "failedUrl='" << frame->GetURL().ToString() << "'";
    }

    if (delegate_)
    {
        delegate_->OnLoadEnd(frame, httpStatusCode);
    }

    // 加载结束后，上报当前url使用的离线资源还是线上资源，进行埋点上报
    std::string request_url = frame->GetURL().ToString();
    std::string temp_url = request_url;
    size_t suffix_pos = temp_url.find('?');
    if (suffix_pos != std::string::npos) {
        temp_url = temp_url.substr(0, suffix_pos);
    }

    cef_proxy::offline_config_filed filed;
    if (offline_switch_ && (LivehimeCefClientBrowser::GetOfflineResource(temp_url, filed) &&
        filed.offline_status == cef_proxy::OFFLINE_STATUS_OPEN)) {
        LivehimeCefClientBrowser::RunOfflineCallback(cef_proxy::EVENT_WEBVIEW_USE_OFFLINE,
            "using webview offline resources", filed);
    } else {
        filed.url = request_url;
        LivehimeCefClientBrowser::RunOfflineCallback(cef_proxy::EVENT_WEBVIEW_USE_ONLINE,
            "using webview online resources", filed);
    }
}

void LivehimeCefClientBrowser::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
    CEF_REQUIRE_UI_THREAD();

    // Don't display an error for downloaded files.
    /*if (errorCode == ERR_ABORTED)
        return;*/

    cef_proxy::calldata data;
    cef_proxy::calldata_filed data_filed;

    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
    data_filed.str_ = failedUrl.ToString();
    data["url"] = data_filed;

    data_filed.reset();
    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
    data_filed.numeric_union.int_ = errorCode;
    data["error_code"] = data_filed;

    data_filed.reset();
    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
    data_filed.str_ = errorText.ToString();
    data["error_text"] = data_filed;

    LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), ipc_messages::kLivehimeCefOnLoadError, &data);

    LOG(WARNING) << "[CP] webview load url failed, browser=" << browser << ", errorCode=" << errorCode << ", " <<
        "errorText='" << errorText.ToString().c_str() << "', \n\t" <<
        "failedUrl='" << failedUrl.ToString().c_str() << "'";

    if (delegate_)
    {
        delegate_->OnLoadError(frame, errorCode, errorText, failedUrl);
    }
}

void LivehimeCefClientBrowser::OnBeforeContextMenu(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    CefRefPtr<CefMenuModel> model)
{
    CEF_REQUIRE_UI_THREAD();

    if (delegate_)
    {
        delegate_->OnBeforeContextMenu(frame, params, model);
    }
}

bool LivehimeCefClientBrowser::OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    int command_id, EventFlags event_flags)
{
    if (delegate_)
    {
        return delegate_->OnContextMenuCommand(frame, params, command_id, event_flags);
    }
    return false;
}

 CefResourceRequestHandler::ReturnValue LivehimeCefClientBrowser::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
     CefRefPtr<CefFrame> frame,
     CefRefPtr<CefRequest> request,
     CefRefPtr<CefRequestCallback> callback)
{
    // 特定业务模块需要伪装http请求头UA的可以在OnBeforeResourceLoad里操作
    // 注意HTTP头的UA和JS的UA不是一码事
    //CefRequest::HeaderMap headers;
    //request->GetHeaderMap(headers);
    //
    //// UA伪装成安卓
    //auto ua_pairs = headers.equal_range("User-Agent");
    //if (ua_pairs.first != headers.end())
    //{
    //    for (auto& iter = ua_pairs.first; iter != ua_pairs.second; ++iter)
    //    {
    //        iter->second = iter->second.ToString() + " " + cef_proxy::kCefAndroidUserAgentMasquerading;
    //    }
    //}
    //else
    //{
    //    NOTREACHED() << "why UA empty ?";
    //    headers.insert(std::make_pair("User-Agent", cef_proxy::kCefAndroidUserAgent));
    //}
    //
    //request->SetHeaderMap(headers);

    if (delegate_)
    {
        return delegate_->OnBeforeResourceLoad(browser, frame, request, callback);
    }
    return RV_CONTINUE;
}

bool LivehimeCefClientBrowser::OnResourceResponse(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request,
    CefRefPtr<CefResponse> response)
{
    if (delegate_)
    {
        return delegate_->OnResourceResponse(browser, frame, request, response);
    }
    return false;
}

CefRefPtr<CefResourceHandler> LivehimeCefClientBrowser::GetResourceHandler(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request)
{
    std::string request_url = request->GetURL().ToString();

    // 先判断webview离线化总开关，如果关闭状态，直接走线上资源
    if (!offline_switch_) {
        LOG(INFO) << "[OFFLINE] using online resources, url=" << request_url;
        return nullptr;
    }

    //LOG(INFO) << "[OFFLINE] GetResourceHandler url=" << request_url << ", threadID=" << GetCurrentThreadId();

    // 离线包配置中的url没有带参数?*，这里匹配查找时需要先去掉所有参数，如下url所示
    std::string url = request_url;
    size_t suffix_pos = url.find('?');
    if (suffix_pos != std::string::npos) {
        url = url.substr(0, suffix_pos);
    }

    // 先查找，如果之前查找过存在离线资源，则会增加命中率
    bool use_offline_resource = false;
    cef_proxy::offline_config_filed filed;
    if (!LivehimeCefClientBrowser::GetOfflineResource(url, filed)) {
        std::string temp_url = url;

        // 部分html资源url中会携带#/hash参数，也需要去掉，如下url所示
        suffix_pos = temp_url.find("#/");
        if (suffix_pos != std::string::npos) {
            temp_url = temp_url.substr(0, suffix_pos);
        }

        // 测试环境下，需要去除域名前ff-{env}前缀，否则无法识别拦截，如下url所示
        suffix_pos = temp_url.find("://ff-");
        if (suffix_pos != std::string::npos) {
            std::string protocol = temp_url.substr(0, suffix_pos + 3);
            size_t point_pos = temp_url.find('.');
            if (point_pos != std::string::npos) {
                std::string url_value = temp_url.substr(point_pos + 1, temp_url.length() - point_pos - 1);
                temp_url = protocol + url_value;
            }
        }

        if (LivehimeCefClientBrowser::GetOfflineResource(temp_url, filed)) {
            use_offline_resource = true;
        } else {
            // 检测是否为带index.html的pageurl，带的话去掉再查找一次，因为配置里配的可能不带index.html
            size_t ext_pos = temp_url.rfind(".html");
            if (ext_pos != std::string::npos) {
                ext_pos = temp_url.rfind('/');
                if (ext_pos != std::string::npos) {
                    temp_url = temp_url.substr(0, ext_pos);
                    if (LivehimeCefClientBrowser::GetOfflineResource(temp_url, filed)) {
                        use_offline_resource = true;
                    }
                }
            }
        }

        // 如果匹配到了，将真实url添加到配置中，方便下次直接查找
        if (use_offline_resource) {
            LivehimeCefClientBrowser::AddOfflineConfig(url, filed);
        }
    } else {
        use_offline_resource = true;
    }

    // 匹配到本地离线资源，判断所在项目离线开关状态
    if (use_offline_resource) {
        if (filed.offline_status == cef_proxy::OFFLINE_STATUS_CLOSE) {
            LOG(INFO) << "[OFFLINE] webview offline status is closed, url=" << request_url;
            use_offline_resource = false;
        }
    }

    // 匹配到存在本地离线资源，尝试检查资源文件是否存在，因单个文件没有hash检验，所以文件内容被篡改这里是没法检测的
    if (use_offline_resource) {
        std::ifstream file(filed.file_path);
        if (file.good()) {
            LOG(INFO) << "[OFFLINE] using webview offline resources, url=" << request_url;
            return new LivehimeCefResourceHandler(filed.url, filed.file_path, filed.mime_type);
        } else {
            LOG(WARNING) << "[OFFLINE] webview offline resources does not exist, url=" << request_url << ", path=" << filed.file_path;
            LivehimeCefClientBrowser::RunOfflineCallback(cef_proxy::EVENT_OFFLINE_INTERCEPT_FAILED,
                "webview offline resources does not exist, using online resources", filed);
        }
    }

    LOG(INFO) << "[OFFLINE] using online resources, url=" << request_url;
    return nullptr;
}

bool LivehimeCefClientBrowser::OnCertificateError(CefRefPtr<CefBrowser> browser,
    cef_errorcode_t cert_error,
    const CefString& request_url,
    CefRefPtr<CefSSLInfo> ssl_info,
    CefRefPtr<CefRequestCallback> callback)
{
    LOG(WARNING) << "[CP] Invalid SSL Certificate, browser=" << browser << ", code=" << cert_error << ", page='" << request_url.ToString() << "'";

    // SSL错误把信息传给上层，由上层决定是否需要进行一下提示。
    // 平时对接活动时，有时候用没有安装测试环境证书的新电脑在测试环境里进行直播姬内页面展示的时候，
    // 由于证书不受信导致H5页面自身无法成功加载而在UI上产生白屏，这种情况下
    // 直播姬侧又没提醒，页面侧也无明显提示，排查白屏问题就很费劲，所以在此将证书问题抛上去
    cef_proxy::calldata data;
    cef_proxy::calldata_filed data_filed;

    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
    data_filed.str_ = request_url.ToString();
    data["url"] = data_filed;

    if (ssl_info)
    {
        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
        data_filed.numeric_union.int_ = ssl_info->GetCertStatus();
        data["cert_status"] = data_filed;
    }

    LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), ipc_messages::kLivehimeCefOnCertificateError, &data);

    if (delegate_)
    {
        return delegate_->OnCertificateError(browser, cert_error, request_url, ssl_info, callback);
    }

    return false;
}

void LivehimeCefClientBrowser::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, CefRequestHandler::TerminationStatus status)
{
    switch (status)
    {
    case CefRequestHandler::TerminationStatus::TS_ABNORMAL_TERMINATION:
        LOG(WARNING) << "[CP] RenderProcessTerminated -> Non-zero exit status.";
        break;
    case CefRequestHandler::TerminationStatus::TS_PROCESS_WAS_KILLED:
        LOG(WARNING) << "[CP] RenderProcessTerminated -> SIGKILL or task manager kill.";
        break;
    case CefRequestHandler::TerminationStatus::TS_PROCESS_CRASHED:
        LOG(WARNING) << "[CP] RenderProcessTerminated -> Segmentation fault.";
        break;
    case CefRequestHandler::TerminationStatus::TS_PROCESS_OOM:
        LOG(WARNING) << "[CP] RenderProcessTerminated -> Out of memory. Some platforms may use TS_PROCESS_CRASHED instead.";
        break;
    default:
        LOG(WARNING) << "[CP] RenderProcessTerminated -> code=" << status;
        break;
    }

    if (delegate_)
    {
        delegate_->OnRenderProcessTerminated(browser, status);
    }
}

bool LivehimeCefClientBrowser::OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
    const CefKeyEvent& event, CefEventHandle os_event, bool* is_keyboard_shortcut)
{
    bool handle = false;

    if (delegate_)
    {
        handle = delegate_->OnPreKeyEvent(browser, event, os_event, is_keyboard_shortcut);
    }

    if (!handle)
    {
        cef_key_event_t tmp_event(event);

        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;

        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_KEY_EVENT;
        memcpy(&data_filed.key_event_, &tmp_event, sizeof(cef_proxy::key_event_t));
        data["msg"] = data_filed;

        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), ipc_messages::kLivehimeCefOnPreKeyEvent, &data);
    }

    return handle;
}

bool LivehimeCefClientBrowser::OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event)
{
    bool handle = false;

    if (delegate_)
    {
        if (event.type == KEYEVENT_RAWKEYDOWN)
        {
            if (event.windows_key_code == VK_F11)
            {
                if ((event.modifiers & EVENTFLAG_SHIFT_DOWN) && (event.modifiers & EVENTFLAG_CONTROL_DOWN)) {
                    if (delegate_->bind_data().type != cef_proxy::client_handler_type::dev_tools) {
                        if (browser && browser->GetHost() && !browser->GetHost()->HasDevTools()) {
                            CefWindowInfo windowInfo;
                            CefBrowserSettings settings;

                            //windowInfo.SetAsPopup(browser->GetHost()->GetWindowHandle(), "DevTools");
                            windowInfo.SetAsPopup(NULL, "DevTools");

                            cef_proxy::browser_bind_data bd(--dev_tools_bind_data_id, cef_proxy::client_handler_type::dev_tools);
                            CefRefPtr<LivehimeCefClientBrowser> client(new LivehimeCefClientBrowser(bd));
                            browser->GetHost()->ShowDevTools(windowInfo, client, settings, CefPoint());

                            LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), ipc_messages::kLivehimeCefPopupDevTools);
                        }
                    }
                }
            }
        }

        handle = delegate_->OnKeyEvent(browser, event, os_event);
    }

    // 底层没处理的事件再向上层抛送
    if (!handle)
    {
        cef_key_event_t tmp_event(event);

        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;

        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_KEY_EVENT;
        memcpy(&data_filed.key_event_, &tmp_event, sizeof(cef_proxy::key_event_t));
        data["msg"] = data_filed;

        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), ipc_messages::kLivehimeCefOnKeyEvent, &data);
    }

    return handle;
}

void LivehimeCefClientBrowser::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    rect.Set(osr_rect_.left, osr_rect_.top, osr_rect_.right - osr_rect_.left, osr_rect_.bottom - osr_rect_.top);
}

void LivehimeCefClientBrowser::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects,
    const void* buffer, int width, int height)
{
    if (type != PET_VIEW) {
        return;
    }

    if (sharing_available_) {
        return;
    }

    if (osr_onpaint_cb_)
    {
        osr_onpaint_cb_(buffer, width, height);
    }
}

void LivehimeCefClientBrowser::OnAcceleratedPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, void* shared_handle)
{
}

CefRefPtr<CefBrowserHost> LivehimeCefClientBrowser::GetHost()
{
    if (browser_)
    {
        return browser_->GetHost();
    }
    return nullptr;
}

void LivehimeCefClientBrowser::Close()
{
    if (!CefCurrentlyOn(TID_UI))
    {
        // Execute on the UI thread.
        CefPostTask(TID_UI, base::BindOnce(&LivehimeCefClientBrowser::Close, this));
        return;
    }

    LOG(INFO) << "[CP] webview close, browser=" << browser_;

    if (browser_ && browser_->GetHost())
    {
        if (osr_mode_)
        {
            // stop OnPaint notification
            browser_->GetHost()->WasHidden(true);
        }
        browser_->GetHost()->CloseBrowser(true);
    }
}

void LivehimeCefClientBrowser::Resize(const RECT& bounds)
{
    osr_rect_ = bounds;

    if (!CefCurrentlyOn(TID_UI))
    {
        // Execute on the UI thread.
        CefPostTask(TID_UI, base::BindOnce(&LivehimeCefClientBrowser::Resize, this, bounds));
        return;
    }

    DCHECK(browser_);
    if (browser_ && browser_->GetHost())
    {
        HWND browser_hwnd = browser_->GetHost()->GetWindowHandle();
        if (browser_hwnd)
        {
            ::MoveWindow(browser_hwnd, bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top, TRUE);
        }
    }
}

void LivehimeCefClientBrowser::Repaint()
{
    if (!CefCurrentlyOn(TID_UI))
    {
        // Execute on the UI thread.
        CefPostTask(TID_UI, base::BindOnce(&LivehimeCefClientBrowser::Repaint, this));
        return;
    }

    if (browser_ && browser_->GetHost())
    {
        CefWindowHandle cef_hwnd = browser_->GetHost()->GetWindowHandle();

#if _MSC_VER
        /*
          CefBrowserWindow      --browser_->GetHost()->GetWindowHandle();
          -   Chrome_WidgetWin_0
          --      Chrome_RenderWidgetHostHWND
        */

        ::InvalidateRect(cef_hwnd, nullptr, false);
        ::UpdateWindow(cef_hwnd);
#endif
    }
}

bool LivehimeCefClientBrowser::ExecuteJSFunction(const std::string& func_name, const cef_proxy::calldata_list& arguments)
{
    if (!CefCurrentlyOn(TID_UI))
    {
        // Execute on the UI thread.
        return CefPostTask(TID_UI, base::BindOnce(base::IgnoreResult(&LivehimeCefClientBrowser::ExecuteJSFunction), this,
            func_name, arguments));
    }

    DCHECK(browser_);
    if (!browser_)
    {
        return false;
    }

    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kLivehimeCefExexuteJsFunction);
    CefRefPtr<CefListValue> args = msg->GetArgumentList();
    int index = 0;
    args->SetString(index++, func_name);
    for (auto& iter : arguments)
    {
        switch (iter.type)
        {
        case cef_proxy::calldata_type::CALL_DATA_TYPE_NULL:
            args->SetNull(index++);
            break;
        case cef_proxy::calldata_type::CALL_DATA_TYPE_BOOL:
            args->SetBool(index++, iter.numeric_union.bool_);
            break;
        case cef_proxy::calldata_type::CALL_DATA_TYPE_INT:
            args->SetInt(index++, iter.numeric_union.int_);
            break;
        case cef_proxy::calldata_type::CALL_DATA_TYPE_DOUBLE:
            args->SetDouble(index++, iter.numeric_union.double_);
            break;
        case cef_proxy::calldata_type::CALL_DATA_TYPE_STRING:
            args->SetString(index++, iter.str_);
            break;
        case cef_proxy::calldata_type::CALL_DATA_TYPE_BINARY:
            args->SetBinary(index++, CefBinaryValue::Create(iter.str_.data(), iter.str_.length()));
            break;
        case cef_proxy::calldata_type::CALL_DATA_TYPE_WSTRING:
            args->SetString(index++, iter.wstr_);
            break;
        default:
            NOTREACHED() << "unsupport value type!";
        }
    }

    SendBrowserProcessMessage(browser_, PID_RENDERER, msg);
    return true;
}

bool LivehimeCefClientBrowser::ExecuteJSFunctionWithKV(const std::string& func_name, const cef_proxy::calldata& arguments)
{
    if (!CefCurrentlyOn(TID_UI))
    {
        // Execute on the UI thread.
        return CefPostTask(TID_UI, base::BindOnce(base::IgnoreResult(&LivehimeCefClientBrowser::ExecuteJSFunctionWithKV), this,
            func_name, arguments));
    }

    if (delegate_)
    {
        return delegate_->ExecuteJSFunctionWithKV(func_name, arguments);
    }
    return false;
}

bool LivehimeCefClientBrowser::DispatchJsEvent(const std::string& ipc_msg_name, const cef_proxy::calldata& arguments)
{
    if (!CefCurrentlyOn(TID_UI))
    {
        // Execute on the UI thread.
        return CefPostTask(TID_UI, base::BindOnce(base::IgnoreResult(&LivehimeCefClientBrowser::DispatchJsEvent), this,
            ipc_msg_name, arguments));
    }

    if (delegate_)
    {
        return delegate_->DispatchJsEvent(ipc_msg_name, arguments);
    }
    return false;
}

bool LivehimeCefClientBrowser::LoadUrl(const std::string& url)
{
    if (!CefCurrentlyOn(TID_UI))
    {
        // Execute on the UI thread.
        return CefPostTask(TID_UI, base::BindOnce(base::IgnoreResult(&LivehimeCefClientBrowser::LoadUrl), this,
            url));
    }

    DCHECK(browser_);
    if (browser_ && browser_->GetMainFrame())
    {
        browser_->GetMainFrame()->LoadURL(url);
        return true;
    }
    return false;
}

void LivehimeCefClientBrowser::UpdateOfflineSwitch(const bool& offline_switch)
{
    LOG(INFO) << "[OFFLINE] UpdateOfflineSwitch, offline_switch=" << offline_switch;
    offline_switch_ = offline_switch;
}

void LivehimeCefClientBrowser::UpdateOfflineConfig(const cef_proxy::offline_config& configs, const cef_proxy::offline_config_callback_fn callback)
{
    if (!CefCurrentlyOn(TID_IO))
    {
        // Execute on the IO thread.
        CefPostTask(TID_IO, base::BindOnce(base::IgnoreResult(&LivehimeCefClientBrowser::UpdateOfflineConfig), configs, callback));
        return;
    }

    LOG(INFO) << "[OFFLINE] UpdateOfflineConfig, configs size=" << configs.size();
    for (auto &kv : configs) {
        offline_configs_[kv.first] = kv.second;
    }

    if (configs.size() == 0) {
        offline_configs_.erase(offline_configs_.begin(), offline_configs_.end());
    }

    offline_callback_ = callback;
}

void LivehimeCefClientBrowser::AddOfflineConfig(const std::string& url, cef_proxy::offline_config_filed filed)
{
    LOG(INFO) << "[OFFLINE] AddOfflineConfig, url=" << url;
    offline_configs_.insert(std::pair<std::string, cef_proxy::offline_config_filed>(url, filed));
}

bool LivehimeCefClientBrowser::GetOfflineResource(std::string& url, cef_proxy::offline_config_filed& filed)
{
    cef_proxy::offline_config::iterator iter = offline_configs_.find(url);
    if (iter != offline_configs_.end()) {
        filed = iter->second;
        return true;
    }

    return false;
}

void LivehimeCefClientBrowser::RunOfflineCallback(const cef_proxy::offline_callback_enevt_t& event, const std::string& msg, const cef_proxy::offline_config_filed& filed)
{
    if (!CefCurrentlyOn(TID_UI))
    {
        // Execute on the IO thread.
        CefPostTask(TID_UI, base::BindOnce(base::IgnoreResult(&LivehimeCefClientBrowser::RunOfflineCallback), event, msg, filed));
        return;
    }

    if (offline_callback_) {
        offline_callback_(event, msg, filed);
    }
}

namespace cef_proxy
{
    bool CreatePopupBrowser(const browser_bind_data& bind_data, HWND par_hwnd, const std::string& url,
        const std::string& post_data /*= ""*/, const std::string& headers /*= ""*/)
    {
        // SimpleHandler implements browser-level callbacks.
        CefRefPtr<LivehimeCefClientBrowser> client(new LivehimeCefClientBrowser(bind_data));

        // Information used when creating the native window.
        CefWindowInfo window_info;

        // Specify CEF browser settings here.
        CefBrowserSettings browser_settings;

        // On Windows we need to specify certain flags that will be passed to CreateWindowEx().
        window_info.SetAsPopup(par_hwnd, cef_proxy::kCefBrowserTitle);

        // Create the first browser window.
        bool ret = CefBrowserHost::CreateBrowser(window_info, client, url, browser_settings, nullptr, nullptr);
        return ret;
    }

    bool CreateEembeddedBrowser(const browser_bind_data& bind_data, HWND par_hwnd, const RECT& rect, const std::string& url,
        const std::string& post_data/* = ""*/, const std::string& headers/* = ""*/)
    {
        DCHECK(par_hwnd);
        if (!par_hwnd)
        {
            return false;
        }

        LOG(INFO) << "[CP] webview create, id=" << bind_data.id << ", page=" << url;

        // SimpleHandler implements browser-level callbacks.
        CefRefPtr<LivehimeCefClientBrowser> client(new LivehimeCefClientBrowser(bind_data, post_data));

        // Information used when creating the native window.
        CefWindowInfo window_info;

        // Specify CEF browser settings here.
        CefBrowserSettings browser_settings;
        browser_settings.background_color = 0;

        // On Windows we need to specify certain flags that will be passed to CreateWindowEx().
        window_info.SetAsChild(par_hwnd, CefRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top));

        // Create the first browser window.
        bool ret = CefBrowserHost::CreateBrowser(window_info, client, url, browser_settings, nullptr, nullptr);
        return ret;
    }

    bool CreateOsrBrowser(const browser_bind_data& bind_data, const RECT& rect, int fps, cef_proxy::cef_proxy_osr_onpaint cb,
        const std::string& url,
        const std::string& post_data /*= ""*/, const std::string& headers /*= ""*/)
    {
        /*CefRefPtr<BrowserClient> browserClient = new BrowserClient(
            this, hwaccel && tex_sharing_avail, reroute_audio);*/
        CefRefPtr<LivehimeCefClientBrowser> client(new LivehimeCefClientBrowser(bind_data, rect, fps, cb, post_data));

        CefWindowInfo windowInfo;
        windowInfo.bounds.width = rect.right - rect.left;
        windowInfo.bounds.height = rect.bottom - rect.top;
        windowInfo.windowless_rendering_enabled = true;

        //windowInfo.shared_texture_enabled = true;

        CefBrowserSettings cefBrowserSettings;

        cefBrowserSettings.windowless_frame_rate = (fps > 0) ? fps : 30;
        /*if (fps == 0) {
            windowInfo.external_begin_frame_enabled = true;
            cefBrowserSettings.windowless_frame_rate = 0;
        }
        else {
            cefBrowserSettings.windowless_frame_rate = fps;
        }*/

        /*cefBrowser = CefBrowserHost::CreateBrowserSync(
            windowInfo, client, url, cefBrowserSettings,
            CefRefPtr<CefDictionaryValue>(),
            nullptr);*/
        bool ret = CefBrowserHost::CreateBrowser(
            windowInfo, client, url, cefBrowserSettings,
            CefRefPtr<CefDictionaryValue>(),
            nullptr);
        return ret;
    }

    void ResizeBrowser(const browser_bind_data& bind_data, const RECT& bounds)
    {
        auto iter = GetCefClient(bind_data);
        if (iter)
        {
            iter->Resize(bounds);
        }
    }

    void RepaintBrowser(const browser_bind_data& bind_data)
    {
        auto iter = GetCefClient(bind_data);
        if (iter)
        {
            iter->Repaint();
        }
    }

    void CloseBrowser(const browser_bind_data& bind_data)
    {
        auto iter = GetCefClient(bind_data, true);
        if (iter)
        {
            LOG(INFO) << "[CP] webview do close, id=" << bind_data.id;

            iter->Close();
        }
    }

    void CloseAllBrowsers()
    {
        std::lock_guard<std::recursive_mutex> lock(client_list_lock);
        for (auto iter : g_client_list)
        {
            iter->Close();
        }
        g_client_list.clear();
    }

    bool CanCefShutdown()
    {
        return g_browser_list_.empty();
    }

    void SendMouseClick(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, cef_proxy::MouseButtonType type, bool mouse_up, uint32_t click_count)
    {
        auto iter = GetCefClient(bind_data);
        if (iter && iter->GetHost())
        {
            CefMouseEvent cef_event;
            cef_event.x = event.x;
            cef_event.y = event.y;
            cef_event.modifiers = event.modifiers;

            iter->GetHost()->SendMouseClickEvent(cef_event, static_cast<CefBrowserHost::MouseButtonType>(type), mouse_up, click_count);
        }
    }

    void SendMouseMove(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, bool mouse_leave)
    {
        auto iter = GetCefClient(bind_data);
        if (iter && iter->GetHost())
        {
            CefMouseEvent cef_event;
            cef_event.x = event.x;
            cef_event.y = event.y;
            cef_event.modifiers = event.modifiers;

            iter->GetHost()->SendMouseMoveEvent(cef_event, mouse_leave);
        }
    }

    void SendMouseWheel(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, int x_delta, int y_delta)
    {
        // bililive[
        if (x_delta == 0 && y_delta == 0) {
            LOG(INFO) << "[CP]: SendMouseWheel x_delta and y_delta is invalid";
            return;
        }
        // ]bililive

        auto iter = GetCefClient(bind_data);
        if (iter && iter->GetHost())
        {
            CefMouseEvent cef_event;
            cef_event.x = event.x;
            cef_event.y = event.y;
            cef_event.modifiers = event.modifiers;

            iter->GetHost()->SendMouseWheelEvent(cef_event, x_delta, y_delta);
        }
    }

    void SendFocus(const cef_proxy::browser_bind_data& bind_data, bool focus)
    {
        auto iter = GetCefClient(bind_data);
        if (iter && iter->GetHost())
        {
            //iter->GetHost()->SendFocusEvent(focus);  //DISCARD
            iter->GetHost()->SetFocus(focus);
        }
    }

    void SendKeyClick(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::KeyEvent& event)
    {
        auto iter = GetCefClient(bind_data);
        if (iter && iter->GetHost())
        {
            CefKeyEvent cef_event;
            cef_event.type = static_cast<cef_key_event_type_t>(event.type);
            cef_event.modifiers = event.modifiers;
            cef_event.windows_key_code = event.windows_key_code;
            cef_event.native_key_code = event.native_key_code;
            cef_event.is_system_key = event.is_system_key;
            cef_event.character = event.character;
            cef_event.unmodified_character = event.unmodified_character;
            cef_event.focus_on_editable_field = event.focus_on_editable_field;

            iter->GetHost()->SendKeyEvent(cef_event);
        }
    }

    /*void ImeSetComposition(const cef_proxy::browser_bind_data& bind_data,
    const std::wstring& text,
    const std::vector<CefCompositionUnderline>& underlines,
    const CefRange& replacement_range,
    const CefRange& selection_range) {
    }*/

    void ImeCommitText(const cef_proxy::browser_bind_data& bind_data,
        const std::wstring& text,
        int from_val,
        int to_val,
        int relative_cursor_pos) {
        auto iter = GetCefClient(bind_data);
        if (iter && iter->GetHost()) {
            CefString cTextStr;
            cTextStr.FromWString(text);
            iter->GetHost()->ImeCommitText(cTextStr, CefRange(from_val, to_val), relative_cursor_pos);
        }
    }

    void ImeFinishComposingText(const cef_proxy::browser_bind_data& bind_data, bool keep_selection) {
        auto iter = GetCefClient(bind_data);
        if (iter && iter->GetHost()) {
            iter->GetHost()->ImeFinishComposingText(keep_selection);
        }
    }

    void ImeCancelComposition(const cef_proxy::browser_bind_data& bind_data) {
        auto iter = GetCefClient(bind_data);
        if (iter && iter->GetHost()) {
            iter->GetHost()->ImeCancelComposition();
        }
    }

    bool ExecuteJSFunction(const browser_bind_data& bind_data, const std::string& func_name, const calldata_list* const arguments)
    {
        auto iter = GetCefClient(bind_data);
        if (iter)
        {
            return iter->ExecuteJSFunction(func_name, *arguments);
        }
        return false;
    }

    bool ExecuteJSFunctionWithKV(const browser_bind_data& bind_data, const std::string& func_name, const calldata* const arguments)
    {
        auto iter = GetCefClient(bind_data);
        if (iter)
        {
            return iter->ExecuteJSFunctionWithKV(func_name, *arguments);
        }
        return false;
    }

    bool DispatchJsEvent(const browser_bind_data& bind_data, const std::string& ipc_msg_name, const calldata* const arguments)
    {
        auto iter = GetCefClient(bind_data);
        if (iter)
        {
            return iter->DispatchJsEvent(ipc_msg_name, *arguments);
        }
        return false;
    }

    bool LoadUrl(const browser_bind_data& bind_data, const std::string& url)
    {
        auto iter = GetCefClient(bind_data);
        if (iter)
        {
            return iter->LoadUrl(url);
        }
        return false;
    }

    void UpdateOfflineSwitch(bool offline_switch)
    {
        LivehimeCefClientBrowser::UpdateOfflineSwitch(offline_switch);
    }

    void UpdateOfflineConfig(const cef_proxy::offline_config* const configs, cef_proxy::offline_config_callback_fn callback)
    {
        LivehimeCefClientBrowser::UpdateOfflineConfig(*configs, callback);
    }
}
