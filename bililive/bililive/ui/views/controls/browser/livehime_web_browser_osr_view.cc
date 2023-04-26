#include "livehime_web_browser_osr_view.h"

#include <shellapi.h>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"

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
#include "base/ext/callable_callback.h"
#include <functional>
#include "base/ext/bind_lambda.h"

namespace
{
    //using osr_onpaint = std::function<void(std::unique_ptr<char> buffer, int width, int height)>;
    typedef base::Callback<void(std::string* buffer, int width, int height)> OnOsrPaint;
    void OnOsrPaintOnBrowserThread(OnOsrPaint cb,
        const void* buffer, int width, int height)
    {
        std::unique_ptr<std::string> data = std::make_unique<std::string>((char*)buffer, width * height);

        base::Closure closure = base::Bind(cb, data.release(), width, height);
        BililiveThread::PostTask(BililiveThread::UI, FROM_HERE, closure);
    }
}


// LivehimePreviewView
LivehimeWebBrowserOsrView::LivehimeWebBrowserOsrView(cef_proxy::client_handler_type browser_type, const std::string& init_url,
    LivehimeWebBrowserDelegate* delegate, const std::string& post_data /*= ""*/)
    : browser_type_(browser_type)
    , url_(init_url)
    , delegate_(delegate)
    , post_data_(post_data)
    , weakptr_factory_(this)
{
}

LivehimeWebBrowserOsrView::~LivehimeWebBrowserOsrView()
{
    UninitViews();
}

bool LivehimeWebBrowserOsrView::Navigate(const std::string &url)
{
    url_ = url;
    return CefProxyWrapper::GetInstance()->LoadUrl(*bind_data_.get(), url_);
}

cef_proxy::browser_bind_data* LivehimeWebBrowserOsrView::browser_bind_data() const
{
    return bind_data_.get();
}

void LivehimeWebBrowserOsrView::NativeControlIgnoreViewVisibilityChanged(bool ignore, bool visible)
{
    native_ctrl_visible_ignore_view_visibility_changed_ = ignore;
    //SetVisible(visible);
}

void LivehimeWebBrowserOsrView::InitViews()
{
    CefProxyWrapper::GetInstance()->AddObserver(this);
    if (CefProxyWrapper::GetInstance()->IsValid())
    {
        base::MessageLoop::current()->PostTask(
            FROM_HERE, base::Bind(&LivehimeWebBrowserOsrView::OnLoadCefProxyCore, weakptr_factory_.GetWeakPtr(), true));
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

void LivehimeWebBrowserOsrView::UninitViews()
{
    weakptr_factory_.InvalidateWeakPtrs();
    if (bind_data_)
    {
        CefProxyWrapper::GetInstance()->CloseBrowser(*bind_data_.get());
        bind_data_.reset();
    }
    CefProxyWrapper::GetInstance()->RemoveObserver(this);
}

void LivehimeWebBrowserOsrView::OnWebBrowserCoreInvalid()
{
    if (delegate_)
    {
        delegate_->OnWebBrowserCoreInvalid();
    }
}

void LivehimeWebBrowserOsrView::OnPaint(std::string* buffer, int width, int height)
{
    std::unique_ptr<std::string> data(buffer);
}

void LivehimeWebBrowserOsrView::OnLoadCefProxyCore(bool succeed)
{
    if (succeed)
    {
        bind_data_.reset(new cef_proxy::browser_bind_data(CefProxyWrapper::GenerateBindDataId(), browser_type_));

        // 打开url之前拼上当前时间戳到queryString中，解决html页面缓存问题
        std::string url = bililive::AppendURLQueryParams(url_, 
            base::StringPrintf("livehime_ts=%lld", base::Time::Now().ToTimeT()));

        auto callback = base::Bind(&LivehimeWebBrowserOsrView::OnPaint,
            weakptr_factory_.GetWeakPtr());

        /*CefProxyWrapper::GetInstance()->CreateOsrBrowser(*bind_data_.get(), gfx::Rect(0,0, 1, 1), 24, callback,
            url, post_data_);*/
        using namespace std::placeholders;
        CefProxyWrapper::GetInstance()->CreateOsrBrowser(*bind_data_.get(), gfx::Rect(0, 0, 1, 1), 24, 
            std::bind(OnOsrPaintOnBrowserThread, callback, _1, _2, _3),
            url, post_data_);
    }
    else
    {
        OnWebBrowserCoreInvalid();
    }
}

void LivehimeWebBrowserOsrView::OnSetTokenCookiesCompleted(bool success)
{
    if (delegate_)
    {
        delegate_->OnSetTokenCookiesCompleted(success);
    }
}

void LivehimeWebBrowserOsrView::OnCefProxyMsgReceived(const cef_proxy::browser_bind_data& bind_data,
    const std::string& msg_name, const cef_proxy::calldata* data)
{
    // 在这里要判断一下这个特定业务模块的通知是不是此webview的
    if (bind_data_ && (*bind_data_ == bind_data))
    {
        if (!handler_details_)
        {
            using namespace std::placeholders;
            handler_details_ = std::make_unique<CefProxyMsgHandleExtraDetails>();
            //handler_details_->widget = GetWidget();
            handler_details_->NativeControlIgnoreViewVisibilityChanged =
                std::bind(&LivehimeWebBrowserOsrView::NativeControlIgnoreViewVisibilityChanged, this, _1, _2);
            handler_details_->OnCefKeyEvent =
                std::bind(&LivehimeWebBrowserOsrView::OnCefKeyEvent, this, _1, _2);
        }
        CefProxyMsgHandle(delegate_, handler_details_.get(), bind_data, msg_name, url_, data);
    }
}

void LivehimeWebBrowserOsrView::OnCefKeyEvent(bool pre_key_event, const cef_proxy::key_event_t& key_event)
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

void LivehimeWebBrowserOsrView::OnEscapeKeyEvent(bool pre_key_event, const cef_proxy::key_event_t& key_event)
{
    if (!pre_key_event)
    {
        if (esc_operation_ == WebViewOperation::WVO_Close)
        {
            //GetWidget()->Close();
        }
    }
}
