#include "bililive/bililive/ui/views/livehime/feedback/livehime_feedback_web_view.h"

#include "base/bind.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"

#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/browser/livehime_web_browser_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_list_state_banner.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_datatype.h"
#include "bililive/bililive/utils/net_util.h"

#include "cef/bililive_browser/public/bililive_browser_ipc_messages.h"

namespace
{
    LivehimeFeedbackWebView* g_single_instance = nullptr;
    LivehimeFeedbackWebView* g_single_popup_instance = nullptr;
}

void LivehimeFeedbackWebView::ShowWindow(views::Widget *parent, const std::string& web_url)
{
    ShowWindow(parent, web_url, false, true);
}

void LivehimeFeedbackWebView::ShowWindow(views::Widget *parent, const std::string& web_url,
    bool can_resize, bool can_popup)
{
    LivehimeFeedbackWebView* single_instance = can_popup ? g_single_instance : g_single_popup_instance;
    if (single_instance)
    {
        single_instance->GetWidget()->Activate();
        return;
    }

    hybrid_ui::LivehimeWebUrlDetails url_ex = hybrid_ui::GetUrlExInfo(web_url);

    views::Widget *widget = new views::Widget();
    BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(widget);
    native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_CLOSE);
    views::Widget::InitParams params;
    params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
    params.native_widget = native_widget;
    params.parent = parent->GetNativeView();

    LivehimeFeedbackWebView* instance = new LivehimeFeedbackWebView(url_ex, can_popup);
    instance->EnableResize(can_resize);
    ShowWidget(instance, widget, params);
}

LivehimeFeedbackWebView::LivehimeFeedbackWebView(const hybrid_ui::LivehimeWebUrlDetails& url_ex, bool allow_popup)
    : LivehimeHybridWebBrowserView(url_ex, false, allow_popup ? WebViewPopupType::Native : WebViewPopupType::NotAllow)
{
    // 反馈主弹窗允许弹二级窗，二级窗是拿来显示反馈中的截图的
    if (allow_popup)
    {
        g_single_instance = this;
    }
    else
    {
        g_single_popup_instance = this;
    }
}

LivehimeFeedbackWebView::~LivehimeFeedbackWebView()
{
    if (allow_popup())
    {
        g_single_instance = nullptr;
    }
    else
    {
        g_single_popup_instance = nullptr;
    }
}

views::NonClientFrameView* LivehimeFeedbackWebView::CreateNonClientFrameView(views::Widget* widget)
{
    BililiveNonTitleBarFrameView* frame_view = (BililiveNonTitleBarFrameView*)__super::CreateNonClientFrameView(widget);
    frame_view->SetEnableDragSize(CanResize());
    return frame_view;
}

WebViewPopupType LivehimeFeedbackWebView::OnWebBrowserPopup(const std::string& url, int target_disposition)
{
    // 这个URL应该是反馈中的图片的的URL
    if (allow_popup())
    {
        std::string img = bililive::AppendURLQueryParams(url, "pc_ui=800,600,FF0E0E0E,1");
        LivehimeFeedbackWebView::ShowWindow(GetWidget(), img, true, false);
    }

    return WebViewPopupType::NotAllow;
}

