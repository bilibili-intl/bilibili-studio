#include "livehime_web_browser_view.h"

#include <shellapi.h>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/base/win/dpi.h"
#include "ui/base/win/hwnd_util.h"
#include "ui/views/controls/native/native_view_host.h"
#include "ui/views/widget/widget.h"
#include "ui/gfx/canvas.h"

#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_widget.h"
#include "bililive/bililive/utils/net_util.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/public/event_tracking_service.h"

#include "cef/bililive_browser/public/bililive_browser_ipc_messages.h"
#include "cef/bililive_browser/public/bililive_browser_switches.h"

// LivehimePreviewView
LivehimeWebBrowserView::LivehimeWebBrowserView(cef_proxy::client_handler_type browser_type, const std::string& init_url,
    LivehimeWebBrowserDelegate* delegate, const std::string& post_data /*= ""*/)
    : browser_type_(browser_type)
    , url_(init_url)
    , delegate_(delegate)
    , post_data_(post_data)
    , weakptr_factory_(this)
{
}

LivehimeWebBrowserView::~LivehimeWebBrowserView()
{
}

bool LivehimeWebBrowserView::Navigate(const std::string &url)
{
    url_ = url;
    if (bind_data_ && bind_data_.get()) {
        return CefProxyWrapper::GetInstance()->LoadUrl(*bind_data_.get(), url_);
    }
    return false;
}

void LivehimeWebBrowserView::ViewHierarchyChanged(const ViewHierarchyChangedDetails &details)
{
    __super::ViewHierarchyChanged(details);

    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();

            base::MessageLoopForUI::current()->AddObserver(this);
        }
        else
        {
            base::MessageLoopForUI::current()->RemoveObserver(this);

            UninitViews();
        }
    }
}

void LivehimeWebBrowserView::OnBoundsChanged(const gfx::Rect& previous_bounds)
{
    // 上层OnBoundsChanged的时候CEF层同步拿上层的NativeControl的尺寸是旧的尺寸不是新的，
    // 所以在这里用抛任务的方式延迟通知CEF修改尺寸，
    // 提测了，当前先这么搞，过后再换一个优雅的方式
    /*base::MessageLoop::current()->PostTask(FROM_HERE,
        base::Bind(&LivehimeWebBrowserView::ResizeWebCtrlSize, weakptr_factory_.GetWeakPtr()));*/
    ResizeWebCtrlSize();
}

void LivehimeWebBrowserView::VisibilityChanged(View* starting_from, bool is_visible)
{
    if (native_ctrl_visible_ignore_view_visibility_changed_)
    {
        return;
    }

    __super::VisibilityChanged(starting_from, is_visible);
}

void LivehimeWebBrowserView::SchedulePaint()
{
    __super::SchedulePaint();

    ::InvalidateRect(GetNativeControlHWND(), nullptr, false);
    ::UpdateWindow(GetNativeControlHWND());
}

HWND LivehimeWebBrowserView::CreateNativeControl(HWND parent_container)
{
    DWORD dwStyle = WS_VISIBLE | WS_CHILD;
    HWND hWnd = CreateWindowExW(GetAdditionalExStyle(), L"STATIC", L"",
        dwStyle, 0, 0, 0, 0, parent_container, 0, GetModuleHandle(NULL), 0);

    return hWnd;
}

void LivehimeWebBrowserView::OnDestroy()
{
    UninitViews();
}

cef_proxy::browser_bind_data* LivehimeWebBrowserView::browser_bind_data() const
{
    if (bind_data_) {
        return bind_data_.get();
    }
    return nullptr;
}

void LivehimeWebBrowserView::NativeControlIgnoreViewVisibilityChanged(bool ignore, bool visible)
{
    native_ctrl_visible_ignore_view_visibility_changed_ = ignore;
    SetVisible(visible);
}

void LivehimeWebBrowserView::SendMouseClickEvent(const cef_proxy::MouseEvent& event, cef_proxy::MouseButtonType type, bool mouse_up, uint32_t click_count)
{
    if (bind_data_ && bind_data_.get()) {
        CefProxyWrapper::GetInstance()->SendMouseClick(*bind_data_, event, type, mouse_up, click_count);
    }
}

void LivehimeWebBrowserView::SetEscapeOperation(WebViewOperation operation)
{
    esc_operation_ = operation;
}

const std::string& LivehimeWebBrowserView::GetURL()
{
    return url_;
}

cef_proxy::client_handler_type LivehimeWebBrowserView::GetBrowserType()
{
    return browser_type_;
}

const std::string& LivehimeWebBrowserView::GetPostData()
{
    return post_data_;
}

cef_proxy::browser_bind_data* LivehimeWebBrowserView::GetBindData()
{
    if (bind_data_) {
        return bind_data_.get();
    }
    return nullptr;
}

void LivehimeWebBrowserView::ResetBindData(__int64 id, cef_proxy::client_handler_type type)
{
    bind_data_.reset(new cef_proxy::browser_bind_data(id, type));
}

void LivehimeWebBrowserView::InitViews()
{
    CefProxyWrapper::GetInstance()->AddObserver(this);
    if (CefProxyWrapper::GetInstance()->IsValid())
    {
        base::MessageLoop::current()->PostTask(
            FROM_HERE, base::Bind(&LivehimeWebBrowserView::OnLoadCefProxyCore, weakptr_factory_.GetWeakPtr(), true));
    }
    else
    {
        // 如果CEF模块当前无效，且底层是已经完成整个初始化流程了的话，
        // 那么CEF模块在当前程序运行周期就是不可用的了
        if (CefProxyWrapper::GetInstance()->IsAlreadyInitFinished())
        {
            // 直接在控件上就显示WebCore加载失败的提示，方便直观的问题查看
            OnWebBrowserCoreInvalid();
        }
    }
}

void LivehimeWebBrowserView::UninitViews()
{
    if (bind_data_&& bind_data_.get())
    {
        CefProxyWrapper::GetInstance()->CloseBrowser(*bind_data_.get());
    }
    CefProxyWrapper::GetInstance()->RemoveObserver(this);
}

void LivehimeWebBrowserView::ResizeWebCtrlSize()
{
    if (bind_data_&& bind_data_.get())
    {
        CefProxyWrapper::GetInstance()->ResizeBrowser(*bind_data_.get(), GetContentsBounds().ToRECT());
    }
}

void LivehimeWebBrowserView::OnWebBrowserCoreInvalid()
{
    if (delegate_)
    {
        delegate_->OnWebBrowserCoreInvalid();
    }
}

LRESULT LivehimeWebBrowserView::OnReflectMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (uMsg == WM_ERASEBKGND)
    {
        bHandled = TRUE;
        return FALSE;
    }
    return __super::OnReflectMsg(uMsg, wParam, lParam, bHandled);
}

void LivehimeWebBrowserView::OnLoadCefProxyCore(bool succeed)
{
    if (succeed)
    {
        bind_data_.reset(new cef_proxy::browser_bind_data(CefProxyWrapper::GenerateBindDataId(), browser_type_));

        // 打开url之前拼上当前时间戳到queryString中，解决html页面缓存问题
        std::string url;
        if (append_ts_query_)
        {
            url = bililive::AppendURLQueryParams(url_,
                base::StringPrintf("livehime_ts=%lld", base::Time::Now().ToTimeT()));
        }
        else
        {
            url = url_;
        }

        gfx::Rect rect = GetLocalBounds();
        if (CefProxyWrapper::GetInstance()->CreateBrowser(*bind_data_.get(), GetNativeControlHWND(), rect, url, post_data_))
        {
            delegate_->OnPreWebBrowserWindowCreate();
        }
    }
    else
    {
        OnWebBrowserCoreInvalid();
    }
}

void LivehimeWebBrowserView::OnSetTokenCookiesCompleted(bool success)
{
    if (delegate_)
    {
        delegate_->OnSetTokenCookiesCompleted(success);
    }
}

void LivehimeWebBrowserView::OnCefProxyMsgReceived(const cef_proxy::browser_bind_data& bind_data,
    const std::string& msg_name, const cef_proxy::calldata* data)
{
    // 在这里要判断一下这个特定业务模块的通知是不是此webview的
    if (bind_data_ && (*bind_data_ == bind_data))
    {
        if (!handler_details_)
        {
            using namespace std::placeholders;
            handler_details_ = std::make_unique<CefProxyMsgHandleExtraDetails>();
            handler_details_->widget = GetWidget();
            handler_details_->NativeControlIgnoreViewVisibilityChanged =
                std::bind(&LivehimeWebBrowserView::NativeControlIgnoreViewVisibilityChanged, this, _1, _2);
            handler_details_->OnCefKeyEvent =
                std::bind(&LivehimeWebBrowserView::OnCefKeyEvent, this, _1, _2);
        }
        CefProxyMsgHandle(delegate_, handler_details_.get(), bind_data, msg_name, GetURL(), data);
	}
}

base::EventStatus LivehimeWebBrowserView::WillProcessEvent(const gfx::NativeEvent& event)
{
    if (event.message == WM_PAINT)
    {
        if (event.hwnd == GetWidget()->GetNativeWindow() || event.hwnd == GetNativeControlHWND())
        {
            if (bind_data_&& bind_data_.get())
            {
                CefProxyWrapper::GetInstance()->RepaintBrowser(*bind_data_.get());
            }
        }
    }

    return base::EVENT_CONTINUE;
}

void LivehimeWebBrowserView::DidProcessEvent(const gfx::NativeEvent& event)
{
}

void LivehimeWebBrowserView::OnCefKeyEvent(bool pre_key_event, const cef_proxy::key_event_t& key_event)
{
    if (key_event.type == cef_proxy::KEYEVENT_RAWKEYDOWN)
    {
        // 按键按下且不是在页面内的可编辑区域内按下的就走ESC处理流程
        if (key_event.windows_key_code == VK_ESCAPE && !key_event.focus_on_editable_field)
        {
            OnEscapeKeyEvent(pre_key_event, key_event);
        }
    }
}

void LivehimeWebBrowserView::OnEscapeKeyEvent(bool pre_key_event, const cef_proxy::key_event_t& key_event)
{
    if (!pre_key_event)
    {
        if (esc_operation_ == WebViewOperation::WVO_Close)
        {
            GetWidget()->Close();
        }
    }
}
