#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_LIVEHIME_WEB_BROWSER_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_LIVEHIME_WEB_BROWSER_VIEW_H

#include <winhttp.h>

#include "base/message_loop/message_loop.h"

#include "ui/views/controls/native_control.h"

#include "bililive/bililive/livehime/cef/cef_proxy_ipc_handler.h"
#include "bililive/bililive/livehime/cef/cef_proxy_wrapper.h"
#include "bililive/bililive/ui/views/controls/browser/livehime_web_browser_delegate.h"

#include "cef/cef_proxy_dll/public/livehime_cef_proxy_exports.h"


class LivehimeWebBrowserView
    : public views::NativeControl
    , protected CefProxyObserver
    , protected base::MessageLoopForUI::Observer
{
public:
    LivehimeWebBrowserView(cef_proxy::client_handler_type browser_type, const std::string& init_url,
                           LivehimeWebBrowserDelegate* delegate, const std::string& post_data = "");
    virtual ~LivehimeWebBrowserView();

    bool Navigate(const std::string &url);
    bool ReloadUrl();

    cef_proxy::browser_bind_data* browser_bind_data() const;

    void NativeControlIgnoreViewVisibilityChanged(bool ignore, bool visible);
    void SendMouseClickEvent(const cef_proxy::MouseEvent& event, cef_proxy::MouseButtonType type = cef_proxy::MouseButtonType::MBT_LEFT, bool mouse_up = true, uint32_t click_count = 1);

    void SetEscapeOperation(WebViewOperation operation);
    WebViewOperation escape_operation() const { return esc_operation_; }

    const std::string& GetURL();
    cef_proxy::client_handler_type GetBrowserType();
    const std::string& GetPostData();
    cef_proxy::browser_bind_data* GetBindData();
    void ResetBindData(__int64 id, cef_proxy::client_handler_type type = cef_proxy::client_handler_type::unspecified);

    void set_auto_append_ts(bool append_ts)
    {
        append_ts_query_ = append_ts;
    }

    // View
    void SchedulePaint() override;
protected:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) override;
    void OnBoundsChanged(const gfx::Rect& previous_bounds) override;
    void VisibilityChanged(View* starting_from, bool is_visible) override;

    // NativeControl
    HWND CreateNativeControl(HWND parent_container) override;
    void OnDestroy() override;
    LRESULT OnNotify(int w_param, LPNMHDR l_param) override { return 0; }
    LRESULT OnCommand(UINT code, int id, HWND source) override { return 0; }
    LRESULT OnReflectMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;

    // CefProxyObserver
    void OnLoadCefProxyCore(bool succeed) override;
    void OnSetTokenCookiesCompleted(bool success) override;
    void OnCefProxyMsgReceived(const cef_proxy::browser_bind_data& bind_data,
        const std::string& msg_name, const cef_proxy::calldata* data) override;

    // base::MessageLoopForUI::Observer
    base::EventStatus WillProcessEvent(const gfx::NativeEvent& event) override;
    void DidProcessEvent(const gfx::NativeEvent& event) override;
    void EventSpared() override {}

protected:
    virtual void OnCefKeyEvent(bool pre_key_event, const cef_proxy::key_event_t& key_event);
    virtual void OnEscapeKeyEvent(bool pre_key_event, const cef_proxy::key_event_t& key_event);

private:
    void InitViews();
    void UninitViews();
    void ResizeWebCtrlSize();
    void OnWebBrowserCoreInvalid();

private:
    LivehimeWebBrowserDelegate* delegate_;
    cef_proxy::client_handler_type browser_type_;
    std::string url_;
    std::string post_data_;
    std::unique_ptr<cef_proxy::browser_bind_data> bind_data_;
    std::unique_ptr<CefProxyMsgHandleExtraDetails> handler_details_;
    bool append_ts_query_ = true;

    bool native_ctrl_visible_ignore_view_visibility_changed_ = false; // 原生窗口的可视属性不受View自身visible属性的变更影响

    WebViewOperation esc_operation_ = WebViewOperation::WVO_Close;

    base::WeakPtrFactory<LivehimeWebBrowserView> weakptr_factory_;
    DISALLOW_COPY_AND_ASSIGN(LivehimeWebBrowserView);
};

#endif