#pragma once

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
//#include <vector>

#include "cef_core/include/cef_client.h"
#include "cef_core/include/cef_command_line.h"
#include "cef_core/include/wrapper/cef_helpers.h"

#include "cef_proxy_dll/public/livehime_cef_proxy_exports.h"


class LivehimeCefClientBrowser
    : public CefClient
    , public CefLifeSpanHandler
    , public CefDisplayHandler
    , public CefLoadHandler
    , public CefContextMenuHandler
    , public CefDragHandler
    , public CefRequestHandler
    , public CefKeyboardHandler
    , public CefResourceRequestHandler
    , public CefRenderHandler
{
    // Constructor & Destructor
public:
    class Delegate : public CefBaseRefCounted
    {
    public:
        explicit Delegate(const cef_proxy::browser_bind_data& bind_data)
            : bind_data_(bind_data)
        {
        }

        virtual bool ExecuteJSFunctionWithKV(const std::string& func_name, const cef_proxy::calldata& arguments)
        {
            NOTREACHED() << "not impl!";
            return false;
        }

        virtual bool DispatchJsEvent(const std::string& ipc_msg_name, const cef_proxy::calldata& arguments)
        {
            NOTREACHED() << "not impl!";
            return false;
        }

        // Called when the browser is created.
        virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) {}

        // Called when the browser is closing.
        virtual void OnBrowserClosing(CefRefPtr<CefBrowser> browser) {}

        // Called when the browser has been closed.
        virtual void OnBrowserClosed(CefRefPtr<CefBrowser> browser) {}

        // Set the window URL address.
        virtual void OnSetAddress(const CefString& url) {}

        // Set the window title.
        virtual void OnSetTitle(const CefString& title) {}

        // ranges from 0.0 to 1.0
        virtual void OnLoadingProgressChange(double progress) {}

        // Set the loading state.
        virtual void OnSetLoadingState(bool isLoading,
            bool canGoBack,
            bool canGoForward)
        {
        }

        virtual void OnLoadStart(CefRefPtr<CefFrame> frame, CefLoadHandler::TransitionType transition_type) {}
        virtual void OnLoadEnd(CefRefPtr<CefFrame> frame, int httpStatusCode) {}
        virtual void OnLoadError(CefRefPtr<CefFrame> frame, CefLoadHandler::ErrorCode errorCode,
            const CefString& errorText, const CefString& failedUrl) {}

        // Called on the UI thread before a context menu is displayed.
        virtual void OnBeforeContextMenu(CefRefPtr<CefFrame> frame,
            CefRefPtr<CefContextMenuParams> params,
            CefRefPtr<CefMenuModel> model)
        {
        }

        virtual bool OnContextMenuCommand(CefRefPtr<CefFrame> frame,
            CefRefPtr<CefContextMenuParams> params,
            int command_id,
            CefContextMenuHandler::EventFlags event_flags)
        {
            return false;
        }

        virtual bool OnProcessMessageReceived(
            CefProcessId source_process,
            CefRefPtr<CefProcessMessage> message)
        {
            return false;
        }

        virtual ReturnValue OnBeforeResourceLoad(
            CefRefPtr<CefBrowser> browser,
            CefRefPtr<CefFrame> frame,
            CefRefPtr<CefRequest> request,
            CefRefPtr<CefRequestCallback> callback) {
            return RV_CONTINUE;
        }

        virtual bool OnResourceResponse(CefRefPtr<CefBrowser> browser,
            CefRefPtr<CefFrame> frame,
            CefRefPtr<CefRequest> request,
            CefRefPtr<CefResponse> response) {
            return false;
        }

        virtual bool OnCertificateError(CefRefPtr<CefBrowser> browser,
            cef_errorcode_t cert_error,
            const CefString& request_url,
            CefRefPtr<CefSSLInfo> ssl_info,
            CefRefPtr<CefRequestCallback> callback)
        {
            return false;
        }

        virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
            CefRequestHandler::TerminationStatus status)
        {}

        virtual bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
            const CefKeyEvent& event,
            CefEventHandle os_event,
            bool* is_keyboard_shortcut)
        {
            return false;
        }

        virtual bool OnKeyEvent(CefRefPtr<CefBrowser> browser,
            const CefKeyEvent& event,
            CefEventHandle os_event)
        {
            return false;
        }

        cef_proxy::browser_bind_data bind_data() const { return bind_data_; }

    protected:
        const cef_proxy::browser_bind_data bind_data_;
    };

    explicit LivehimeCefClientBrowser(const cef_proxy::browser_bind_data& bind_data,
        const std::string& post_data = "");
    explicit LivehimeCefClientBrowser(const cef_proxy::browser_bind_data& bind_data,
        const RECT& osr_rect, int fps, cef_proxy::cef_proxy_osr_onpaint cb,
        const std::string& post_data = "");
    virtual ~LivehimeCefClientBrowser();

    static void UpdateOfflineSwitch(const bool& offline_switch);
    static void UpdateOfflineConfig(const cef_proxy::offline_config& configs, const cef_proxy::offline_config_callback_fn callback);
    static void AddOfflineConfig(const std::string& url, cef_proxy::offline_config_filed filed);
    static bool GetOfflineResource(std::string& url, cef_proxy::offline_config_filed& filed);
    static void RunOfflineCallback(const cef_proxy::offline_callback_enevt_t& event, const std::string& msg, const cef_proxy::offline_config_filed& filed);

    cef_proxy::browser_bind_data bind_data() const { return bind_data_; }
    void Close();
    void Resize(const RECT& bounds);
    void Repaint();
    bool ExecuteJSFunction(const std::string& func_name, const cef_proxy::calldata_list& arguments);
    bool ExecuteJSFunctionWithKV(const std::string& func_name, const cef_proxy::calldata& arguments);
    bool DispatchJsEvent(const std::string& ipc_msg_name, const cef_proxy::calldata& arguments);
    bool LoadUrl(const std::string& url);

    // CefClient methods:
    CefRefPtr<CefDisplayHandler> GetDisplayHandler() override;
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;
    CefRefPtr<CefLoadHandler> GetLoadHandler() override;
    CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override;
    CefRefPtr<CefDragHandler> GetDragHandler() override;
    CefRefPtr<CefRequestHandler> GetRequestHandler() override;
    CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() override;
    CefRefPtr<CefRenderHandler> GetRenderHandler() override;
    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message) override;

    // CefDisplayHandler methods:
    void OnLoadingProgressChange(CefRefPtr<CefBrowser> browser,
        double progress) override;
    void OnTitleChange(CefRefPtr<CefBrowser> browser,
        const CefString& title) override;
    void OnAddressChange(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        const CefString& url) override;
    bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
        cef_log_severity_t level,
        const CefString& message,
        const CefString& source,
        int line) override;

    // CefDragHandler methods:
    bool OnDragEnter(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefDragData> dragData,
        CefDragHandler::DragOperationsMask mask) override;

    // CefLifeSpanHandler methods:
    bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
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
        bool* no_javascript_access) override;
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    bool DoClose(CefRefPtr<CefBrowser> browser) override;
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // CefLoadHandler methods:
    void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
        bool isLoading,
        bool canGoBack,
        bool canGoForward) override;
    void OnLoadStart(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        TransitionType transition_type) override;
    void OnLoadEnd(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        int httpStatusCode) override;
    void OnLoadError(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        ErrorCode errorCode,
        const CefString& errorText,
        const CefString& failedUrl) override;

    /* CefContextMenuHandler */
    void OnBeforeContextMenu(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefContextMenuParams> params,
        CefRefPtr<CefMenuModel> model) override;
    bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefContextMenuParams> params,
        int command_id,
        EventFlags event_flags) override;

    // CefRequestHandler
    bool OnCertificateError(CefRefPtr<CefBrowser> browser,
        cef_errorcode_t cert_error,
        const CefString& request_url,
        CefRefPtr<CefSSLInfo> ssl_info,
        CefRefPtr<CefRequestCallback> callback) override;
    void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
        CefRequestHandler::TerminationStatus status) override;

    CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request,
        bool is_navigation,
        bool is_download,
        const CefString& request_initiator,
        bool& disable_default_handling) override {
        return this;
    }

    // CefResourceRequestHandler
    CefResourceRequestHandler::ReturnValue OnBeforeResourceLoad(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request,
        CefRefPtr<CefRequestCallback> callback) override;
    bool OnResourceResponse(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request,
        CefRefPtr<CefResponse> response) override;
    CefRefPtr<CefResourceHandler> GetResourceHandler(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request) override;

    // CefKeyboardHandler
    bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
        const CefKeyEvent& event,
        CefEventHandle os_event,
        bool* is_keyboard_shortcut);
    bool OnKeyEvent(CefRefPtr<CefBrowser> browser,
        const CefKeyEvent& event,
        CefEventHandle os_event);

    // CefRenderHandler
    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
    void OnPaint(CefRefPtr<CefBrowser> browser,
        PaintElementType type, const RectList& dirtyRects,
        const void* buffer, int width,
        int height) override;
    void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList& dirtyRects,
        void* shared_handle) override;

    CefRefPtr<CefBrowserHost> GetHost();

private:
    static CefRefPtr<Delegate> CreateDelegate(const cef_proxy::browser_bind_data &bind_data);

private:
    CefRefPtr<Delegate> delegate_;
    cef_proxy::browser_bind_data bind_data_;
    std::string post_data_;

    // 暂且直接使用BGRA
    bool osr_mode_ = false;
    bool sharing_available_ = false;
    int osr_fps_ = 0;
    RECT osr_rect_;
    cef_proxy::cef_proxy_osr_onpaint osr_onpaint_cb_;

    // List of existing browser windows. Only accessed on the CEF UI thread.
    CefRefPtr<CefBrowser> browser_;

    // 离线资源映射表Mappings
    static bool offline_switch_;
    static cef_proxy::offline_config offline_configs_;
    static cef_proxy::offline_config_callback_fn offline_callback_;

    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(LivehimeCefClientBrowser);
    DISALLOW_COPY_AND_ASSIGN(LivehimeCefClientBrowser);
};


namespace cef_proxy
{
    bool CreatePopupBrowser(const browser_bind_data& bind_data, HWND par_hwnd, const std::string& url,
        const std::string& post_data = "", const std::string& headers = "");

    bool CreateEembeddedBrowser(const browser_bind_data& bind_data, HWND par_hwnd, const RECT& rect, const std::string& url,
        const std::string& post_data = "", const std::string& headers = "");

    bool CreateOsrBrowser(const browser_bind_data& bind_data, const RECT& rect, int fps, cef_proxy::cef_proxy_osr_onpaint cb,
        const std::string& url,
        const std::string& post_data = "", const std::string& headers = "");

    void ResizeBrowser(const cef_proxy::browser_bind_data& bind_data, const RECT& bounds);

    void RepaintBrowser(const cef_proxy::browser_bind_data& bind_data);

    void CloseBrowser(const browser_bind_data& bind_data);

    void CloseAllBrowsers();

    bool CanCefShutdown();

    void SendMouseClick(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, cef_proxy::MouseButtonType type, bool mouse_up, uint32_t click_count);

    void SendMouseMove(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, bool mouse_leave);

    void SendMouseWheel(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::MouseEvent& event, int x_delta, int y_delta);

    void SendFocus(const cef_proxy::browser_bind_data& bind_data, bool focus);

    void SendKeyClick(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::KeyEvent& event);

    /*void ImeSetComposition(const cef_proxy::browser_bind_data& bind_data,
        const std::wstring& text,
        const std::vector<CefCompositionUnderline>& underlines,
        const CefRange& replacement_range,
        const CefRange& selection_range);*/

    void ImeCommitText(const cef_proxy::browser_bind_data& bind_data,
        const std::wstring& text,
        int from_val,
        int to_val,
        int relative_cursor_pos);

    void ImeFinishComposingText(const cef_proxy::browser_bind_data& bind_data, bool keep_selection);
    void ImeCancelComposition(const cef_proxy::browser_bind_data& bind_data);

    // 传入的是按序的参数列表，根据传入的参数顺序直接调用指定browser执行js，不交由实际的业务对象额外处理
    bool ExecuteJSFunction(const cef_proxy::browser_bind_data& bind_data, const std::string& func_name,
        const cef_proxy::calldata_list* const arguments);

    // 传入key-value形式的参数列表，参数顺序不一定就符合js函数的参数顺序，需要交由实际的业务对象自己提取参数自己构建CefListValue列表
    bool ExecuteJSFunctionWithKV(const cef_proxy::browser_bind_data& bind_data, const std::string& func_name,
        const cef_proxy::calldata* const arguments);

    // 传入key-value形式的参数列表，参数顺序不一定就符合js函数的参数顺序，需要交由实际的业务对象自己提取参数自行构建CefListValue列表
    bool DispatchJsEvent(const cef_proxy::browser_bind_data& bind_data,
        const std::string& ipc_msg_name, const cef_proxy::calldata* const arguments);

    bool LoadUrl(const cef_proxy::browser_bind_data& bind_data, const std::string& url);

    void UpdateOfflineSwitch(bool offline_switch);

    void UpdateOfflineConfig(const cef_proxy::offline_config* const configs, cef_proxy::offline_config_callback_fn callback);
}

CefRefPtr<LivehimeCefClientBrowser> GetCefClient(const cef_proxy::browser_bind_data& bind_data, bool remove = false);
