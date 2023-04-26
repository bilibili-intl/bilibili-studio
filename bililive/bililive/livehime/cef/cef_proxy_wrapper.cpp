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

    bool is_loading_cef_proxy_module_now = false;   // ��ǰ�Ƿ����첽��ʼ��cef������
    bool already_try_to_init_cef_proxy_module = false;  // �Ƿ��Ѿ����Գ�ʼ����cefģ���ˣ���ʼ��ʧ�ܵĻ�������Ͳ��ٳ�����
    HMODULE g_cef_proxy_dll_instance = nullptr;
    std::vector<PendingCefProxyTask> g_pending_tasks;// cef-proxyδ��ʼ�����ǰ���۵Ĵ�ִ�������б�

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

        // cef init�ɹ���ֱ�ӽ��ź����ţ�OBSģ����Է��ļ����ˣ�
        // ���initʧ���ˣ�Ҫ��cefģ�鱻�����ͷ���
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

            // ��obs_coreע�ṩnative�����֧��obs-studio��ع��ܵ�web JavaScript�������ݽ����Ļص�
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

            // ��ע��cef-proxy�ص������Եõ�cef-init���
            RegisterCefProxyCallback();

            // �첽��ʼ��CEFģ��
            InitCefBrowserContext();
        }
        __finally
        {
            if (!ret)
            {
                // ����dll���ǻ�ȡ������ַ����׶ξ�ʧ���˾����ϲ��Լ�֪ͨһ��ģ���ʼ��ʧ��
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

            // ���ǵ�ǰ��OBSģ���ǲ�����shutdown�ģ��ڽ��̽�����ʱ����ϵͳ��������Դ��
            // OBS�������Լ���cef-proxy����ͬһcefģ�飬��ô��������cef-proxy��ʱ���
            // ��Ҫ�����ͷ���dll�ˣ�ֻ������cef�����߳��˵���ֹ�������̼��ɣ���Ҫ����
            // ����Դ���������׷������̽���ʱcef����obsж�أ�Ȼ��obs���̻߳�������cef
            // ��Դ���±��������⡣�����ж����obs�������ع��ǾͲ�Ҫ�ͷ�cefģ��
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

        // ��������ͷŲ���������û��ʵ���ͷ�CEFģ�飬���ﶼ��������OBS����
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

    // ����dll�ܺ�ʱ���ŵ�file�߳�ȥ��
    LOG(INFO) << "cef proxy loading.";
    BililiveThread::PostTask(BililiveThread::FILE, FROM_HERE, base::Bind(LoadAndGetProcAddressOnWorkerThread));
}

void CefProxyWrapper::FreeCefProxyCore(bool exec_on_worker_thread)
{
    Shutdown();

    // ������file�̼߳��صģ��ͷ�Ҳ�ŵ�file�߳�ȥ�ͷ�
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
    // ������Ҫ�׵�UI�߳��У���������ϱ��ȷ���������
    if (!BililiveThread::CurrentlyOn(BililiveThread::UI)) {
        BililiveThread::PostTask(BililiveThread::UI, FROM_HERE, base::Bind(OfflineCallbackTest, event, msg, filed));
        return;
    }

    LOG(INFO) << "OfflineCallbackTest, url=" << filed.url;
    LOG(INFO) << "OfflineCallbackTest, url=" << msg;

    switch (event) {
    case cef_proxy::EVENT_OFFLINE_INTERCEPT_FAILED:
        // ����webview��Դ����ʧ�ܣ����������������Ҫ��������ϴ�
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

    // cef-proxyδ��ʼ�����ǰ���۵Ĵ�ִ�������б�ȥ��
    if (success)
    {
        // ��exe�İ汾�Ŵ���ȥ���Ա�cef�����jsע��ʱ��ֱ������ǰbuild�Ŵ���
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

        // ����㣬Ҫ����־Ҳ�ϴ��������ʱ�������߼��ǻ�û�е�¼̬�ģ�BFS�ϴ��ӿ����޷��ɹ����õģ�
        // ��ô������ֱ�Ӱ���־����׷�ӵ����URL��
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
            // ������ע��
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

    // ������Ҫ�׵�UI�߳��У���������ϱ��ȷ���������
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
            // ������ע��
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
            // ������ע��
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
