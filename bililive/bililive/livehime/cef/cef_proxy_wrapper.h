#ifndef BILILIVE_BILILIVE_LIVEHIME_CEF_PROXY_WRAPPER_H_
#define BILILIVE_BILILIVE_LIVEHIME_CEF_PROXY_WRAPPER_H_

#include "obs_frontend_callbacks_impl.h"

#include "base/observer_list.h"
#include "base/strings/string16.h"
#include "base/memory/weak_ptr.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/waitable_event.h"

#include "cef/cef_proxy_dll/public/livehime_cef_proxy_exports.h"

#include "ui/gfx/rect.h"

extern bool obs_module_inited;// CoreProxyImpl
extern base::WaitableEvent cef_module_loaded_event;// CoreProxyImpl

class BililiveProcessImpl;

class CefProxyObserver
{
public:
    virtual ~CefProxyObserver() = default;

    // 公共通知
    virtual void OnLoadCefProxyCore(bool succeed) {};
    virtual void OnSetTokenCookiesCompleted(bool success) {};
    // 特定模块相关的通知
    virtual void OnCefProxyMsgReceived(const cef_proxy::browser_bind_data& bind_data,
        const std::string& msg_name, const cef_proxy::calldata* data) {};
};

class CefProxyWrapper
{
public:
    static CefProxyWrapper* GetInstance();
    static __int64 GenerateBindDataId();

    void AddObserver(CefProxyObserver* observer) { observer_list_.AddObserver(observer); }
    void RemoveObserver(CefProxyObserver* observer) { observer_list_.RemoveObserver(observer); }

    bool IsValid() const;
    bool IsAlreadyInitFinished() const;

    // 异步加载DLL并初始化cef browser环境
    void LoadCefProxyCore();
    void Shutdown();

    bool CreateBrowser(cef_proxy::browser_bind_data& bind_data, HWND par_hwnd, const gfx::Rect& rect_in_screen, const std::string& url,
        const std::string& post_data = "", const std::string& headers = "");
    bool CreateOsrBrowser(cef_proxy::browser_bind_data& bind_data, const gfx::Rect& rect,
        int fps, cef_proxy::cef_proxy_osr_onpaint cb,
        const std::string& url,
        const std::string& post_data = "", const std::string& headers = "");
    void CloseBrowser(const cef_proxy::browser_bind_data& bind_data);
    void ResizeBrowser(const cef_proxy::browser_bind_data& bind_data, const RECT& bounds);
    void RepaintBrowser(const cef_proxy::browser_bind_data& bind_data);
    bool ExecuteJSFunction(const cef_proxy::browser_bind_data& bind_data, const std::string& func_name,
        const cef_proxy::calldata_list* const arguments = nullptr);
    bool ExecuteJSFunctionWithKV(const cef_proxy::browser_bind_data& bind_data, const std::string& func_name,
        const cef_proxy::calldata* const arguments = nullptr);
    bool DispatchJsEvent(const cef_proxy::browser_bind_data& bind_data,
        const std::string& ipc_msg_name, const cef_proxy::calldata* const arguments = nullptr);
    bool LoadUrl(cef_proxy::browser_bind_data& bind_data, const std::string& url);

    void UpdataWebviewOfflineSwitch(bool offline_switch);
    void UpdataWebviewOfflineConfig(const cef_proxy::offline_config* const configs);

    //发送鼠标键盘事件
    void SendMouseClick(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, cef_proxy::MouseButtonType type, bool mouse_up, uint32_t click_count);
    void SendMouseMove(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, bool mouse_leave);
    void SendMouseWheel(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, int x_delta, int y_delta);
    void SendFocus(const cef_proxy::browser_bind_data& bind_data, bool focus);
    void SendKeyClick(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::KeyEvent& event);

    // 中文输入法事件
    void ImeCommitText(const cef_proxy::browser_bind_data& bind_data,
        const std::wstring& text,
        int from_val,
        int to_val,
        int relative_cursor_pos);

    void ImeFinishComposingText(const cef_proxy::browser_bind_data& bind_data, bool keep_selection);

    void ImeCancelComposition(const cef_proxy::browser_bind_data& bind_data);

    // 设置B站自己的登录态cookie
    void SetTokenCookies();
    void SetMiniLoginCookies(const cef_proxy::cookies& cookies);
    bool IsTokenCookiesValid() const;
    void DeleteAuthCookies();

    // for test
    void TestPopup(HWND hwnd);
    void TestEembedded(HWND hwnd);
    void TestJS(HWND hwnd);

    // worker thread reply
    static void CefProxyInitStatusCallback(bool success, cef_proxy::CefInitResult code);
    static void CefProxyCommonEventCallback(const std::string& msg_name, const cef_proxy::calldata* data);
    static void CefProxyMsgReceivedCallback(const cef_proxy::browser_bind_data& bind_data,
        const std::string& msg_name, const cef_proxy::calldata* data);
    static void WebviewOfflineCallback(const cef_proxy::offline_callback_enevt_t& event, const std::string& msg, const cef_proxy::offline_config_filed& filed);

private:
    CefProxyWrapper();
    ~CefProxyWrapper();

    void FreeCefProxyCore(bool exec_on_worker_thread);
private:
    friend class BililiveProcessImpl;
    friend struct std::default_delete<CefProxyWrapper>;
    bool initialized_ = false;
    bool is_token_cookie_set_success = false;

    ObserverList<CefProxyObserver> observer_list_;

    base::WeakPtrFactory<CefProxyWrapper> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(CefProxyWrapper);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_CEF_PROXY_WRAPPER_H_