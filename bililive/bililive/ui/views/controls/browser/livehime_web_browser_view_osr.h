#pragma once

#include <mutex>

#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_pump_observer.h"
#include "bililive/bililive/ui/views/controls/browser/livehime_web_browser_view.h"
#include "cef/cef_proxy_dll/public/livehime_cef_proxy_define.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/base/win/dpi.h"
#include "SkBitmap.h"

class LivehimeWebBrowserViewOSR :
    public views::View,
    protected CefProxyObserver
    , protected base::MessageLoopForUI::Observer
{
public:
    LivehimeWebBrowserViewOSR(
        cef_proxy::client_handler_type browser_type,
        const gfx::Rect& broswer_rect,
        int fps,
        const std::string& init_url,
        LivehimeWebBrowserDelegate* delegate,
        const std::string& post_data = "");

    virtual ~LivehimeWebBrowserViewOSR();

    void NativeControlIgnoreViewVisibilityChanged(bool ignore, bool visible);

    cef_proxy::browser_bind_data* browser_bind_data() const;

    const std::string& GetURL();
    cef_proxy::client_handler_type GetBrowserType();
    const std::string& GetPostData();
    cef_proxy::browser_bind_data* GetBindData();
    void ResetBindData(__int64 id, cef_proxy::client_handler_type type = cef_proxy::client_handler_type::bilibili);

    void LoadURL(const std::string &url);
    void BrowserResize(const gfx::Rect& rect);

    void SetReceiveKey(bool receive) {
        receive_key_event_ = receive;
    };

protected:
    void OnPaint(gfx::Canvas* canvas) override;

    bool OnMousePressed(const ui::MouseEvent& event) override;
    bool OnMouseDragged(const ui::MouseEvent& event) override;
    void OnMouseReleased(const ui::MouseEvent& event) override;
    void OnMouseCaptureLost() override;
    void OnMouseMoved(const ui::MouseEvent& event) override;
    void OnMouseEntered(const ui::MouseEvent& event) override;
    void OnMouseExited(const ui::MouseEvent& event) override;

    bool OnIMEMessage(const MSG event, LRESULT* result) override;
    bool OnMouseWheel(const ui::MouseWheelEvent& event) override;

    void OnLoadCefProxyCore(bool succeed) override;
    void OnSetTokenCookiesCompleted(bool success) override;
    void OnWebBrowserCoreInvalid();

    void OnCefProxyMsgReceived(const cef_proxy::browser_bind_data& bind_data,
        const std::string& msg_name, const cef_proxy::calldata* data) override;

    void OnCefKeyEvent(bool pre_key_event, const cef_proxy::key_event_t& key_event);
    void OnEscapeKeyEvent(bool pre_key_event, const cef_proxy::key_event_t& key_event);

    // base::MessageLoopForUI::Observer
    base::EventStatus WillProcessEvent(const gfx::NativeEvent& event) override;
    void DidProcessEvent(const gfx::NativeEvent& event) override;
    void EventSpared() override {}

private:
    void InitViews();
    void UninitViews();
    void ClosureFunction();
    void CefPaintCallBackOnWorkThread(base::Closure cb, const void* data, int width, int height);
    void OnKeyPressed(const gfx::NativeEvent& event);

    bool GetString(HIMC imc, WPARAM lparam, int type, std::wstring& result);
    bool GetResult(HWND hwnd, LPARAM lparam, std::wstring& result);

    LivehimeWebBrowserDelegate* delegate_;
    cef_proxy::client_handler_type browser_type_;
    std::string url_;
    std::string post_data_;
    std::unique_ptr<cef_proxy::browser_bind_data> bind_data_;
    std::unique_ptr<CefProxyMsgHandleExtraDetails> handler_details_;

    bool native_ctrl_visible_ignore_view_visibility_changed_ = false; // 原生窗口的可视属性不受View自身visible属性的变更影响

    base::WeakPtrFactory<LivehimeWebBrowserViewOSR> weakptr_factory_;
    gfx::Rect                                       browser_rect_;
    int                                             fps_;
	gfx::ImageSkia                 paint_image;
	std::mutex                     paint_mutex;
    bool is_deleted_ = false;
    bool receive_key_event_ = true;             //是否需要发送键盘事件给h5
};
