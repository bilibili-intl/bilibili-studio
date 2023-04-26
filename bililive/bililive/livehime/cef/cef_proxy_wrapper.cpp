#include "bililive/bililive/livehime/cef/cef_proxy_wrapper.h"

#include "base/base64.h"
#include "base/ext/bind_lambda.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/stringprintf.h"

#include "bililive/common/bililive_context.h"
#include "bililive/common/bililive_logging.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/public/account_info.h"


#include "cef/bililive_browser/public/bililive_browser_ipc_messages.h"
#include "bililive/bililive/utils/fast_forward_url_convert.h"

#define TYPEDEF_CEF_PROXY_EXPORTS_FUNC(func_name)                                    \
    cef_proxy::##func_name##_fn func_name = nullptr;

#define GET_PROC_ADDRESS_FROM_CEF_PROXY(func_name)                                   \
    {                                                                                \
        func_name = reinterpret_cast<cef_proxy::##func_name##_fn>(                   \
            ::GetProcAddress(g_cef_proxy_dll_instance, #func_name));                 \
        if (func_name == nullptr)                                                    \
        {                                                                            \
            all_func_load_succeed = false;                                           \
            PLOG(ERROR) << "load cef_proxy proc faild! -> " << #func_name;        \
        }                                                                            \
    }


extern obs_frontend_callbacks *InitializeOBSFrontendAPIInterface();

namespace
{
    CefProxyWrapper* g_single_instance = nullptr;

    enum class PendingCefProxyTaskCmd
    {
        SetTokenCookies,
        SetMiniLoginCookies,

        Count,
    };

    struct PendingCefProxyTask
    {
        PendingCefProxyTaskCmd cmd;

        base::Closure closure;

        bool operator == (const PendingCefProxyTask& rhs) const
        {
            return (cmd == rhs.cmd);
        }
    };

    bool is_loading_cef_proxy_module_now = false;   // 当前是否处于异步初始化cef流程中
    bool already_try_to_init_cef_proxy_module = false;  // 是否已经尝试初始化过cef模块了；初始化失败的话，过后就不再尝试了
    HMODULE g_cef_proxy_dll_instance = nullptr;
    std::vector<PendingCefProxyTask> g_pending_tasks;// cef-proxy未初始化完成前积累的待执行任务列表

    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_init_module);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_shutdown_module);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_set_version);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_set_cookie);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_set_cef_cookie);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_delete_cookie);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_register_callback);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_execute_js_function);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_execute_js_function_with_kv);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_dispatch_js_event);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_load_url);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_create_popup_browser);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_create_embedded_browser);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_create_osr_browser);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_close_browser);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_resize_browser);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_repaint_browser);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_send_mouse_click);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_send_mouse_move);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_send_mouse_wheel);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_send_focus);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_send_key_click);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_ime_commit_text);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_ime_finish_composing_text);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_ime_cancel_composition);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_update_offline_switch);
    TYPEDEF_CEF_PROXY_EXPORTS_FUNC(cef_proxy_update_offline_config);

    // cef-proxy callback [-------------------------------
    void CefProxyInitStatusOnWorkerThread(bool success, cef_proxy::CefInitResult code)
    {
        LOG(INFO) << "cef initialize finished, result = " << success << ", code = " << (int)code;

        // cef init成功了直接将信号置信，OBS模块可以放心加载了；
        // 如果init失败了，要等cef模块被彻底释放了
        if (success)
        {
            cef_module_loaded_event.Signal();
        }

        BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
            base::Bind(CefProxyWrapper::CefProxyInitStatusCallback, success, code));
    }

    void CefProxyCommonEventOnWorkerThread(const std::string& msg_name, const cef_proxy::calldata* data)
    {
        std::unique_ptr<cef_proxy::calldata> data_ptr;
        if (data)
        {
            data_ptr = std::make_unique<cef_proxy::calldata>(*data);
        }
        BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
            base::Bind(CefProxyWrapper::CefProxyCommonEventCallback, msg_name, data_ptr.release()));
    }

    void CefProxyMsgReceivedOnWorkerThread(const cef_proxy::browser_bind_data& bind_data,
        const std::string& msg_name, const cef_proxy::calldata* data)
    {
        std::unique_ptr<cef_proxy::calldata> data_ptr;
        if (data)
        {
            data_ptr = std::make_unique<cef_proxy::calldata>(*data);
        }
        BililiveThread::PostTask(BililiveThread::UI, FROM_HERE,
            base::Bind(CefProxyWrapper::CefProxyMsgReceivedCallback, bind_data, msg_name, data_ptr.release()));
    }

    void CefProxyShutdownStatusOnWorkerThread(bool success)
    {
        LOG(WARNING) << "cef shutdown finished, result = " << success;
    }
    // ] ---------------------------------

    bool LoadCefProxyLibrary()
    {
        DCHECK(!g_cef_proxy_dll_instance);

        if (!g_cef_proxy_dll_instance)
        {
            auto dll_path = BililiveContext::Current()->GetMainDirectory();
            dll_path = dll_path.Append(L"plugins");
            dll_path = dll_path.Append(bililive::kBililiveCefProxyDll);
            g_cef_proxy_dll_instance = ::LoadLibraryExW(dll_path.value().c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
            if (!g_cef_proxy_dll_instance)
            {
                PLOG(ERROR) << "faild to load cef_proxy library!";
            }
        }

        return !!g_cef_proxy_dll_instance;
    }

    bool GetCefProxyProcAddress()
    {
        DCHECK(g_cef_proxy_dll_instance);

        bool all_func_load_succeed = false;

        if (g_cef_proxy_dll_instance)
        {
            all_func_load_succeed = true;

            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_init_module);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_shutdown_module);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_set_version);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_set_cookie);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_set_cef_cookie);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_delete_cookie);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_register_callback);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_execute_js_function);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_execute_js_function_with_kv);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_dispatch_js_event);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_load_url);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_create_popup_browser);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_create_embedded_browser);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_create_osr_browser);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_send_mouse_click);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_send_mouse_move);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_send_mouse_wheel);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_send_focus);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_send_key_click);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_ime_commit_text);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_ime_finish_composing_text);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_ime_cancel_composition);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_close_browser);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_resize_browser);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_repaint_browser);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_update_offline_switch);
            GET_PROC_ADDRESS_FROM_CEF_PROXY(cef_proxy_update_offline_config);
        }

        return all_func_load_succeed;
    }

    void RegisterCefProxyCallback()
    {
        if (g_cef_proxy_dll_instance)
        {
            cef_proxy::cef_proxy_callback callbacks;
            callbacks.init_status_fn = CefProxyInitStatusOnWorkerThread;
            callbacks.common_event_fn = CefProxyCommonEventOnWorkerThread;
            callbacks.msg_received_fn = CefProxyMsgReceivedOnWorkerThread;
            callbacks.shutdown_status_fn = CefProxyShutdownStatusOnWorkerThread;

            cef_proxy_register_callback(callbacks);

            // 向obs_core注册供native程序和支持obs-studio相关功能的web JavaScript进行数据交互的回调
            obs_frontend_set_callbacks_internal(InitializeOBSFrontendAPIInterface());
        }
    }

    void InitCefBrowserContext()
    {
        if (g_cef_proxy_dll_instance)
        {
            cef_proxy_init_module();
        }
    }

    void LoadAndGetProcAddressOnWorkerThread()
    {
        DCHECK(!BililiveThread::CurrentlyOn(BililiveThread::UI));

        if (g_cef_proxy_dll_instance)
        {
            return;
        }

        LOG(INFO) << "start cef initialize.";

        bool ret = false;
        cef_proxy::CefInitResult init_type = cef_proxy::CefInitResult::ModuleMissing;

        __try
        {
            ret = LoadCefProxyLibrary();
            if (!ret)
            {
                init_type = cef_proxy::CefInitResult::ModuleMissing;
                NOTREACHED();
                __leave;
            }

            ret = GetCefProxyProcAddress();
            if (!ret)
            {
                init_type = cef_proxy::CefInitResult::ModuleInvalid;
                NOTREACHED();
                __leave;
            }

            // 先注册cef-proxy回调，可以得到cef-init结果
            RegisterCefProxyCallback();

            // 异步初始化CEF模块
            InitCefBrowserContext();
        }
        __finally
        {
            if (!ret)
            {
                // 加载dll或是获取函数地址这个阶段就失败了就在上层自己通知一下模块初始化失败
                CefProxyInitStatusOnWorkerThread(false, init_type);
            }
        }
    }

    void ShutdownCefProxy()
    {
        if (g_cef_proxy_dll_instance && cef_proxy_shutdown_module)
        {
            cef_proxy_shutdown_module();
        }
    }

    void FreeCefProxyLibraryOnWorkerThread()
    {
        if (g_cef_proxy_dll_instance)
        {
            LOG(INFO) << "cef proxy FreeLibrary.";

            // 我们当前的OBS模块是不调用shutdown的，在进程结束的时候由系统清理其资源，
            // OBS和我们自己的cef-proxy共用同一cef模块，那么我们清理cef-proxy的时候就
            // 不要彻底释放其dll了，只单纯把cef的主线程退掉终止其主流程即可，不要清理
            // 其资源，否则容易发生进程结束时cef先于obs卸载，然后obs的线程还在引用cef
            // 资源导致崩溃的问题。所以判断如果obs曾经加载过那就不要释放cef模块
            if (!obs_module_inited)
            {
                ::FreeLibrary(g_cef_proxy_dll_instance);
                g_cef_proxy_dll_instance = nullptr;

                LOG(INFO) << "cef proxy freed.";
            }
            else
            {
                LOG(INFO) << "cef proxy can't be freed after obs inited.";
            }
        }

        // 不管这个释放操作到底有没有实际释放CEF模块，这里都置信允许OBS加载
        cef_module_loaded_event.Signal();
    }

}

CefProxyWrapper* CefProxyWrapper::GetInstance()
{
    return g_single_instance;
}

__int64 CefProxyWrapper::GenerateBindDataId()
{
    static __int64 increment_bind_data_id = 0;
    return increment_bind_data_id++;
}

CefProxyWrapper::CefProxyWrapper()
    : weakptr_factory_(this)
{
    g_single_instance = this;
}

CefProxyWrapper::~CefProxyWrapper()
{
    FreeCefProxyCore(false);
    g_single_instance = nullptr;
}

bool CefProxyWrapper::IsValid() const
{
    return (!!g_cef_proxy_dll_instance && initialized_);
}

bool CefProxyWrapper::IsAlreadyInitFinished() const
{
    return already_try_to_init_cef_proxy_module && !is_loading_cef_proxy_module_now;
}

void CefProxyWrapper::LoadCefProxyCore()
{
    if (already_try_to_init_cef_proxy_module)
    {
        return;
    }

    already_try_to_init_cef_proxy_module = true;
    is_loading_cef_proxy_module_now = true;

    // 加载dll很耗时，放到file线程去做
    LOG(INFO) << "cef proxy loading.";
    BililiveThread::PostTask(BililiveThread::FILE, FROM_HERE, base::Bind(LoadAndGetProcAddressOnWorkerThread));
}

void CefProxyWrapper::FreeCefProxyCore(bool exec_on_worker_thread)
{
    Shutdown();

    // 加载是file线程加载的，释放也放到file线程去释放
    LOG(INFO) << "cef proxy freeing.";
    if (exec_on_worker_thread)
    {
        BililiveThread::PostTask(BililiveThread::FILE, FROM_HERE, base::Bind(FreeCefProxyLibraryOnWorkerThread));
    }
    else
    {
        FreeCefProxyLibraryOnWorkerThread();
    }
}

void CefProxyWrapper::Shutdown()
{
    initialized_ = false;
    ShutdownCefProxy();
}

void OfflineCallbackTest(const int& event, const std::string& msg, const cef_proxy::offline_config_filed& filed)
{
    // 这里需要抛到UI线程中，进行埋点上报等非阻塞操作
    if (!BililiveThread::CurrentlyOn(BililiveThread::UI)) {
        BililiveThread::PostTask(BililiveThread::UI, FROM_HERE, base::Bind(OfflineCallbackTest, event, msg, filed));
        return;
    }

    LOG(INFO) << "OfflineCallbackTest, url=" << filed.url;
    LOG(INFO) << "OfflineCallbackTest, url=" << msg;

    switch (event) {
    case cef_proxy::EVENT_OFFLINE_INTERCEPT_FAILED:
        // 离线webview资源拦截失败，可以在这里根据需要进行埋点上传
        break;
    default:
        break;
    }
}

void CefProxyWrapper::CefProxyInitStatusCallback(bool success, cef_proxy::CefInitResult code)
{
    is_loading_cef_proxy_module_now = false;

    GetInstance()->initialized_ = success;
    FOR_EACH_OBSERVER(CefProxyObserver, GetInstance()->observer_list_, OnLoadCefProxyCore(success));

    // cef-proxy未初始化完成前积累的待执行任务列表去重
    if (success)
    {
        // 将exe的版本号传下去，以便cef层进行js注入时把直播姬当前build号带上
        std::string version = BililiveContext::Current()->GetExecutableVersionAsASCII();
        std::string device_name = BililiveContext::Current()->GetDevicename();
        std::string device_platform = BililiveContext::Current()->GetDevicePlatform();
        std::string buvid3 = GetBililiveProcess()->secret_core()->network_info().buvid();

        cef_proxy_set_version(version, device_name, device_platform, buvid3);
        g_pending_tasks.erase(std::unique(g_pending_tasks.begin(), g_pending_tasks.end()), g_pending_tasks.end());
        for (auto& task : g_pending_tasks)
        {
            base::MessageLoop::current()->PostTask(FROM_HERE, task.closure);
        }
        g_pending_tasks.clear();
    }
    else
    {
        switch (code)
        {
        case cef_proxy::CefInitResult::ModuleMissing:
            LOG(ERROR) << "cef_proxy module missing!";
            break;
        case cef_proxy::CefInitResult::ModuleInvalid:
            LOG(ERROR) << "cef_proxy module invalid!";
            break;
        case cef_proxy::CefInitResult::CefInitializeFailed:
            LOG(ERROR) << "init cef_proxy module failed!";
            break;
        case cef_proxy::CefInitResult::CefInitializeCrash:
            LOG(ERROR) << "init cef_proxy module crashed!";
            break;
        default:
            NOTREACHED();
            break;
        }

        GetInstance()->FreeCefProxyCore(true);

        // 埋个点，要把日志也上传，但这个时候按正常逻辑是还没有登录态的，BFS上传接口是无法成功调用的，
        // 那么在这里直接把日志数据追加到埋点URL中
        std::string log = bililive::GetCurrentLogText();
        std::string cef_log = bililive::GetCurrentCEFLogText();
        log.append("\n" + std::string(bililive::kLogBoundary) + "\n");
        log.append(cef_log);
        base::Base64Encode(log, &cef_log);

        std::string event_msg = base::StringPrintf("type:0;code:%d;log:%s", (int)code, cef_log.c_str());
        GetBililiveProcess()->secret_core()->event_tracking_service()->ReportLivehimeBehaviorEvent(
            secret::LivehimeBehaviorEvent::LivehimeCefError, 0, event_msg
        ).Call();
    }
}

void CefProxyWrapper::CefProxyCommonEventCallback(const std::string& msg_name, const cef_proxy::calldata* data)
{
    std::unique_ptr<cef_proxy::calldata> data_ptr(const_cast<cef_proxy::calldata*>(data));

    if (msg_name == ipc_messages::kLivehimeCefSetCookieTaskCompleted)
    {
        int task_id = data->at("task_id").numeric_union.int_;
        bool all_success = data->at("all_success").numeric_union.bool_;

        GetInstance()->is_token_cookie_set_success = all_success;

        if (all_success)
        {
            LOG(INFO) << "cef proxy set_cookie successed. " << task_id;
        }
        else
        {
            NOTREACHED();
            LOG(ERROR) << "cef proxy set_cookie failed. " << task_id;
        }
        FOR_EACH_OBSERVER(CefProxyObserver, GetInstance()->observer_list_, OnSetTokenCookiesCompleted(all_success));

        if (!all_success)
        {
            // 再重试注入
            base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
                base::Bind(&CefProxyWrapper::SetTokenCookies, CefProxyWrapper::GetInstance()->weakptr_factory_.GetWeakPtr()),
                    base::TimeDelta::FromSeconds(5));
        }
    }
    else if (msg_name == ipc_messages::kLivehimeCefJsMsgForTest)// for test
    {
        CefProxyWrapper::GetInstance()->ExecuteJSFunction(*((cef_proxy::browser_bind_data*)data), "JsFuncForNativeInvoke");
    }
}

void CefProxyWrapper::CefProxyMsgReceivedCallback(const cef_proxy::browser_bind_data& bind_data,
    const std::string& msg_name, const cef_proxy::calldata* data)
{
    std::unique_ptr<cef_proxy::calldata> data_ptr(const_cast<cef_proxy::calldata*>(data));

    FOR_EACH_OBSERVER(CefProxyObserver, GetInstance()->observer_list_,
        OnCefProxyMsgReceived(bind_data, msg_name, data_ptr.get()));
}

void WebviewOfflineActionReport(
    const int& event,
    const std::string& msg,
    const cef_proxy::offline_config_filed& filed) {

}

void CefProxyWrapper::WebviewOfflineCallback(
                        const cef_proxy::offline_callback_enevt_t& event,
                        const std::string& msg,
                        const cef_proxy::offline_config_filed& filed) {

    // 这里需要抛到UI线程中，进行埋点上报等非阻塞操作
    if (!BililiveThread::CurrentlyOn(BililiveThread::UI)) {
        BililiveThread::PostTask(BililiveThread::UI, FROM_HERE, base::Bind(WebviewOfflineActionReport, event, msg, filed));
        return;
    }
}

void CefProxyWrapper::TestPopup(HWND hwnd)
{
    if (IsValid())
    {
        cef_proxy::browser_bind_data bind_data(CefProxyWrapper::GenerateBindDataId());
        cef_proxy_create_popup_browser(bind_data, hwnd, "www.bilibili.tv", "", "");
    }
}

void CefProxyWrapper::TestEembedded(HWND hwnd)
{
    if (IsValid())
    {
        gfx::Rect rect(0, 0, 400, 175);
        cef_proxy::browser_bind_data bind_data(CefProxyWrapper::GenerateBindDataId());
        cef_proxy_create_embedded_browser(bind_data, hwnd, rect.ToRECT(), "www.bilibili.tv", "", "");
    }
}

void CefProxyWrapper::TestJS(HWND hwnd)
{
    auto test_html_path = BililiveContext::Current()->GetMainDirectory();
    test_html_path = test_html_path.AppendASCII("browser_for_test.html");
    gfx::Rect rect(0, 0, 400, 175);
    cef_proxy::browser_bind_data bind_data(CefProxyWrapper::GenerateBindDataId());
    CefProxyWrapper::GetInstance()->CreateBrowser(
        bind_data, hwnd, rect, test_html_path.AsUTF8Unsafe());
}

bool CefProxyWrapper::CreateBrowser(cef_proxy::browser_bind_data& bind_data, HWND par_hwnd, const gfx::Rect& rect_in_screen, const std::string& url,
                                    const std::string& post_data /*= ""*/, const std::string& headers /*= ""*/)
{
    DCHECK(IsValid());

    if (IsValid())
    {
        return cef_proxy_create_embedded_browser(bind_data, par_hwnd, rect_in_screen.ToRECT(), bililive::FastForwardChangeEnv(url), post_data, headers);
    }
    return false;
}

bool CefProxyWrapper::CreateOsrBrowser(cef_proxy::browser_bind_data& bind_data, const gfx::Rect& rect, int fps,
    cef_proxy::cef_proxy_osr_onpaint cb,
    const std::string& url,
    const std::string& post_data /*= ""*/, const std::string& headers /*= ""*/)
{
    DCHECK(IsValid());

    if (IsValid())
    {
        return cef_proxy_create_osr_browser(bind_data, rect.ToRECT(), fps, cb, bililive::FastForwardChangeEnv(url), post_data, headers);
    }
    return false;
}

void CefProxyWrapper::CloseBrowser(const cef_proxy::browser_bind_data& bind_data)
{
    DCHECK(IsValid());

    if (IsValid())
    {
        cef_proxy_close_browser(bind_data);
    }
}

void CefProxyWrapper::ResizeBrowser(const cef_proxy::browser_bind_data& bind_data, const RECT& bounds)
{
    DCHECK(IsValid());

    if (IsValid())
    {
        cef_proxy_resize_browser(bind_data, bounds);
    }
}

void CefProxyWrapper::RepaintBrowser(const cef_proxy::browser_bind_data& bind_data)
{
    DCHECK(IsValid());

    if (IsValid())
    {
        cef_proxy_repaint_browser(bind_data);
    }
}

bool CefProxyWrapper::ExecuteJSFunction(const cef_proxy::browser_bind_data& bind_data, const std::string& func_name,
                                        const cef_proxy::calldata_list* const arguments/* = nullptr*/)
{
    DCHECK(IsValid());

    if (IsValid())
    {
        return cef_proxy_execute_js_function(bind_data, func_name, arguments);
    }
    return false;
}

bool CefProxyWrapper::ExecuteJSFunctionWithKV(const cef_proxy::browser_bind_data& bind_data, const std::string& func_name,
                                              const cef_proxy::calldata* const arguments /*= nullptr*/)
{
    DCHECK(IsValid());

    if (IsValid())
    {
        return cef_proxy_execute_js_function_with_kv(bind_data, func_name, arguments);
    }
    return false;
}

bool CefProxyWrapper::DispatchJsEvent(const cef_proxy::browser_bind_data& bind_data,
    const std::string& ipc_msg_name, const cef_proxy::calldata* const arguments /*= nullptr*/)
{
    DCHECK(IsValid());

    if (IsValid())
    {
        return cef_proxy_dispatch_js_event(bind_data, ipc_msg_name, arguments);
    }
    return false;
}

bool CefProxyWrapper::LoadUrl(cef_proxy::browser_bind_data& bind_data, const std::string& url)
{
    DCHECK(IsValid());

    if (IsValid())
    {
        return cef_proxy_load_url(bind_data, bililive::FastForwardChangeEnv(url));
    }
    return false;
}

void CefProxyWrapper::UpdataWebviewOfflineSwitch(bool offline_switch)
{
    DCHECK(IsValid());

    if (IsValid())
    {
        cef_proxy_update_offline_switch(offline_switch);
    }
}

void CefProxyWrapper::UpdataWebviewOfflineConfig(const cef_proxy::offline_config* const configs) {

    DCHECK(IsValid());

    if (IsValid())
    {
        cef_proxy_update_offline_config(configs, WebviewOfflineCallback);
    }
}

void CefProxyWrapper::SendMouseClick(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, cef_proxy::MouseButtonType type, bool mouse_up, uint32_t click_count)
{
    DCHECK(IsValid());

    if (IsValid())
    {
        cef_proxy_send_mouse_click(bind_data, event, type, mouse_up, click_count);
    }
}

void CefProxyWrapper::SendMouseMove(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, bool mouse_leave)
{
    DCHECK(IsValid());

    if (IsValid())
    {
        cef_proxy_send_mouse_move(bind_data, event, mouse_leave);
    }
}

void CefProxyWrapper::SendMouseWheel(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, int x_delta, int y_delta)
{
    DCHECK(IsValid());

    if (IsValid())
    {
        cef_proxy_send_mouse_wheel(bind_data, event, x_delta, y_delta);
    }
}

void CefProxyWrapper::SendFocus(const cef_proxy::browser_bind_data& bind_data, bool focus)
{
    DCHECK(IsValid());

    if (IsValid())
    {
        cef_proxy_send_focus(bind_data, focus);
    }
}

void CefProxyWrapper::SendKeyClick(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::KeyEvent& event)
{
    DCHECK(IsValid());

    if (IsValid())
    {
        cef_proxy_send_key_click(bind_data, event);
    }
}

void CefProxyWrapper::ImeCommitText(const cef_proxy::browser_bind_data& bind_data,
    const std::wstring& text,
    int from_val,
    int to_val,
    int relative_cursor_pos) {
    DCHECK(IsValid());

    if (IsValid())
    {
        cef_proxy_ime_commit_text(bind_data, text, from_val, to_val, relative_cursor_pos);
    }
}

void CefProxyWrapper::ImeFinishComposingText(const cef_proxy::browser_bind_data& bind_data, bool keep_selection) {
    DCHECK(IsValid());

    if (IsValid())
    {
        cef_proxy_ime_finish_composing_text(bind_data, keep_selection);
    }
}

void CefProxyWrapper::ImeCancelComposition(const cef_proxy::browser_bind_data& bind_data) {
    DCHECK(IsValid());

    if (IsValid())
    {
        cef_proxy_ime_cancel_composition(bind_data);
    }
}

void CefProxyWrapper::SetTokenCookies()
{
    if (IsValid())
    {
        int task_id = GenerateBindDataId();

        bool ret = cef_proxy_set_cookie(task_id,
            GetBililiveProcess()->secret_core()->account_info().cookies(),
            GetBililiveProcess()->secret_core()->account_info().domains());

        if (ret)
        {
            LOG(INFO) << "cef proxy post set_cookie successed. " << task_id;
        }
        else
        {
            NOTREACHED();
            LOG(ERROR) << "cef proxy post set_cookie failed." << task_id;

            FOR_EACH_OBSERVER(CefProxyObserver, GetInstance()->observer_list_, OnSetTokenCookiesCompleted(false));
        }

        if (!ret)
        {
            // 再重试注入
            base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
                base::Bind(&CefProxyWrapper::SetTokenCookies, weakptr_factory_.GetWeakPtr()),
                base::TimeDelta::FromSeconds(5));
        }
    }
    else
    {
        if (is_loading_cef_proxy_module_now)
        {
            auto iter = std::find_if(g_pending_tasks.begin(), g_pending_tasks.end(),
                [](const PendingCefProxyTask& task)->bool
            {
                return (task.cmd == PendingCefProxyTaskCmd::SetTokenCookies);
            });
            if (iter != g_pending_tasks.end())
            {
                LOG(INFO) << "cef proxy try set cookie more than once when cef loading.";
                return;
            }

            PendingCefProxyTask task;
            task.cmd = PendingCefProxyTaskCmd::SetTokenCookies;
            task.closure = base::Bind(&CefProxyWrapper::SetTokenCookies, weakptr_factory_.GetWeakPtr());
            g_pending_tasks.push_back(task);

            LOG(INFO) << "cef proxy try set cookie when cef loading.";
        }
    }
}

void CefProxyWrapper::SetMiniLoginCookies(const cef_proxy::cookies& cookies)
{
    if (IsValid())
    {
        int task_id = GenerateBindDataId();

        bool ret = cef_proxy_set_cef_cookie(task_id,
            cookies);

        if (ret)
        {
            LOG(INFO) << "cef proxy post set_cef_cookie successed. " << task_id;
        }
        else
        {
            NOTREACHED();
            LOG(ERROR) << "cef proxy post set_cef_cookie failed." << task_id;

            FOR_EACH_OBSERVER(CefProxyObserver, GetInstance()->observer_list_, OnSetTokenCookiesCompleted(false));
        }

        if (!ret)
        {
            // 再重试注入
            base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
                base::Bind(&CefProxyWrapper::SetMiniLoginCookies, weakptr_factory_.GetWeakPtr(), cookies),
                base::TimeDelta::FromSeconds(5));
        }
    }
    else
    {
        if (is_loading_cef_proxy_module_now)
        {
            auto iter = std::find_if(g_pending_tasks.begin(), g_pending_tasks.end(),
                [](const PendingCefProxyTask& task)->bool
                {
                    return (task.cmd == PendingCefProxyTaskCmd::SetMiniLoginCookies);
                });
            if (iter != g_pending_tasks.end())
            {
                LOG(INFO) << "cef proxy try set_cef_cookie more than once when cef loading.";
                return;
            }

            PendingCefProxyTask task;
            task.cmd = PendingCefProxyTaskCmd::SetMiniLoginCookies;
            task.closure = base::Bind(&CefProxyWrapper::SetMiniLoginCookies, weakptr_factory_.GetWeakPtr(), cookies);
            g_pending_tasks.push_back(task);

            LOG(INFO) << "cef proxy try set_cef_cookie when cef loading.";
        }
    }
}

bool CefProxyWrapper::IsTokenCookiesValid() const
{
    return is_token_cookie_set_success;
}

void CefProxyWrapper::DeleteAuthCookies()
{
    DCHECK(IsValid());

    if (IsValid())
    {
        cef_proxy_delete_cookie();
    }
}
