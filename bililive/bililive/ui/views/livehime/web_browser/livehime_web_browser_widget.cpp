#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_widget.h"

#include "base/bind.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"

#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/browser/livehime_web_browser_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_list_state_banner.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_datatype.h"
#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/secret/public/event_tracking_service.h"
#include "cef/bililive_browser/public/bililive_browser_ipc_messages.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include <bililive/bililive/livehime/function_control/app_function_controller.h>

namespace
{
    const int kDefWidth = GetLengthByDPIScale(800);
    const int kDefHeight = GetLengthByDPIScale(600);
};

void LivehimeHybridWebBrowserView::ShowWindow(
    views::Widget *parent,
    const std::string& web_url,
    bool show_native_titlebar/* = false*/,
    const string16& title/* = {}*/,
    WebViewPopupType allow_popup/* = WebViewPopupType::NotAllow*/,
    cef_proxy::client_handler_type browser_type/* = cef_proxy::client_handler_type::bilibili*/,
    const std::string& post_data,/* = ""*/
    LivehimeWebBrowserDelegate* parent_web_delegate/* = nullptr*/,
    bool window_stack/* = false*/)
{
    views::Widget* widget = GetCurrentWidget();
    if (widget && !window_stack) {
        if (!widget->IsVisible()) {
            widget->Show();
        }
        return;
    }

    hybrid_ui::LivehimeWebUrlDetails url_ex = hybrid_ui::GetUrlExInfo(web_url);

    widget = new views::Widget();
    BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(widget);
    native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_CLOSE);
    native_widget->SetDisableRedrawWhenDidProcessMsg(true);
    views::Widget::InitParams params;
    params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
    params.native_widget = native_widget;
    params.parent = parent->GetNativeView();

    LivehimeHybridWebBrowserView* instance = new LivehimeHybridWebBrowserView(
        url_ex, show_native_titlebar, allow_popup, browser_type, post_data, parent_web_delegate);
    instance->SetWindowTitle(title);
    DoModalWidget(instance, widget, params);
}

views::Widget* LivehimeHybridWebBrowserView::PopupWindow(
    views::View* anchor_view,
	const std::string& web_url,
	bool show_native_titlebar/* = false*/,
	const string16& title/* = {}*/,
	WebViewPopupType allow_popup/* = WebViewPopupType::NotAllow*/,
	cef_proxy::client_handler_type browser_type/* = cef_proxy::client_handler_type::bilibili*/,
	const std::string& post_data/* = ""*/,
    LivehimeWebBrowserDelegate* parent_web_delegate/* = nullptr*/)
{
	hybrid_ui::LivehimeWebUrlDetails url_ex = hybrid_ui::GetUrlExInfo(web_url);

    views::Widget* widget = new views::Widget();
	views::Widget::InitParams params;
	params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
    params.remove_taskbar_icon = true;

	LivehimeHybridWebBrowserView* instance = new LivehimeHybridWebBrowserView(
		url_ex, show_native_titlebar, allow_popup, browser_type, post_data, parent_web_delegate);
	instance->SetWindowTitle(title);
    instance->SetCloseWhenDeactive(true);
	ShowWidget(instance, widget, params);

    if (anchor_view)
    {
		gfx::Rect show_area = anchor_view->GetBoundsInScreen();
		gfx::Size view_size = instance->GetPreferredSize();

		widget->SetBounds(gfx::Rect(
			show_area.x() + show_area.width() / 2 - view_size.width() / 2,
			show_area.y() - GetLengthByDPIScale(6) - view_size.height(),
			view_size.width(), view_size.height()));
    }
    return widget;
}

LivehimeHybridWebBrowserView::LivehimeHybridWebBrowserView(
    const hybrid_ui::LivehimeWebUrlDetails& url_ex,
    bool show_native_titlebar,
    WebViewPopupType allow_popup,
    cef_proxy::client_handler_type browser_type/* = cef_proxy::client_handler_type::bilibili*/,
    const std::string& post_data/* = ""*/,
    LivehimeWebBrowserDelegate* parent_web_delegate/* = nullptr*/)
    : show_native_titlebar_(show_native_titlebar)
    , allow_popup_(allow_popup)
    , web_url_ex_(url_ex)
    , browser_type_(browser_type)
    , default_pref_size_(kDefWidth, kDefHeight)
    , post_data_(post_data)
    , parent_web_delegate_(parent_web_delegate)
{
    if (show_native_titlebar_) {
        native_close_always_show_ = false;
    } else {
        native_close_always_show_ = (web_url_ex_.ex_close == "1");
    }
}

LivehimeHybridWebBrowserView::~LivehimeHybridWebBrowserView() {
}

void LivehimeHybridWebBrowserView::ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) {
    if (details.child == this) {
        if (details.is_add) {
            InitViews();
        }
    }
}

void LivehimeHybridWebBrowserView::InitViews()
{
    title_label_ = new LivehimeTitleLabel(GetWindowTitle());
    title_label_->SetAutoColorReadabilityEnabled(true);
    title_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);

    close_button_ = new BililiveImageButton(this);
    close_button_->SetImage(views::Button::STATE_NORMAL, GetImageSkiaNamed(IDR_LIVEMAIN_JOINMIC_CLOSE));
    close_button_->SetImage(views::Button::STATE_HOVERED, GetImageSkiaNamed(IDR_LIVEMAIN_JOINMIC_CLOSE_HV));
    close_button_->SetImage(views::Button::STATE_PRESSED, GetImageSkiaNamed(IDR_LIVEMAIN_JOINMIC_CLOSE_HV));
    close_button_->SetTooltipText(GetLocalizedString(IDS_UGC_CLOSE));
    close_button_->set_id(kCloseButtonID);

    state_view_ = new LivehimeListStateBannerView();
    state_view_->SetStateText(
        livehime::ListState::ListState_Loading, GetLocalizedString(IDS_LUCKYGIFT_INFO_STATUS_LOADING));
    state_view_->SetStateText(
        livehime::ListState::ListState_Faild, GetLocalizedString(IDS_LUCKYGIFT_INFO_STATUS_LOADING_FAILED));

    web_browser_view_ = new LivehimeWebBrowserView(
        browser_type_,
        web_url_ex_.url,
        this,
        post_data_);

    AddChildView(title_label_);
    AddChildView(close_button_);
    AddChildView(state_view_);
    AddChildView(web_browser_view_);

    int color = 0;
    if (base::HexStringToInt(web_url_ex_.ex_background, &color))
    {
        color |= 0xFF000000;
        set_background(views::Background::CreateSolidBackground(color));
        state_view_->SetAutoColorReadabilityEnabled(true, color);
    }

    // 网页未加载完成前不管WebView的visible怎么变化都不显示依附其上的原生子窗口
    web_browser_view_->NativeControlIgnoreViewVisibilityChanged(true, false);

    if (show_native_titlebar_) {
        title_label_->SetVisible(false);
        close_button_->SetVisible(false);
    }
}

gfx::Size LivehimeHybridWebBrowserView::GetPreferredSize()
{
    int width = 0;
    int height = 0;

    if (base::StringToInt(web_url_ex_.ex_width, &width) &&
        base::StringToInt(web_url_ex_.ex_height, &height))
    {
        if (native_close_always_show_)
        {
            gfx::Size title_pref_size = title_label_->GetPreferredSize();
            gfx::Size pref_size = close_button_->GetPreferredSize();
            int title_cy = std::max(title_pref_size.height(), pref_size.height());
            height += title_cy;
        }
        return gfx::Size(GetLengthByDPIScale(width), GetLengthByDPIScale(height));
    }
    else
    {
        return default_pref_size_;
    }
}

void LivehimeHybridWebBrowserView::Layout()
{
    gfx::Rect rect = GetContentsBounds();
    LayoutInBounds(rect);
}

void LivehimeHybridWebBrowserView::LayoutInBounds(gfx::Rect rect)
{
    gfx::Size title_pref_size = title_label_->GetPreferredSize();
    gfx::Size pref_size = close_button_->GetPreferredSize();
    int title_cy = std::max(title_pref_size.height(), pref_size.height());
    title_label_->SetBounds(rect.x() + kPaddingColWidthForGroupCtrls,
        rect.y(),
        rect.width() - kPaddingColWidthForGroupCtrls * 2 - pref_size.width(), title_cy);

    close_button_->SetBounds(rect.right() - pref_size.width(),
        rect.y() + (title_cy - pref_size.height()) / 2,
        pref_size.width(), pref_size.height());

    pref_size = state_view_->GetPreferredSize();
    state_view_->SetBounds(rect.x(), close_button_->bounds().bottom(),
        rect.width(), rect.height() - close_button_->bounds().height());

    if (native_close_always_show_)
    {
        rect.Inset(0, close_button_->height(), 0, 0);
    }
    web_browser_view_->SetBoundsRect(rect);
}

views::NonClientFrameView* LivehimeHybridWebBrowserView::CreateNonClientFrameView(views::Widget* widget)
{
    if (show_native_titlebar_) {
        return __super::CreateNonClientFrameView(widget);
    }
    return new BililiveNonTitleBarFrameView(this);
}

int LivehimeHybridWebBrowserView::NonClientHitTest(const gfx::Point &point)
{
    if (GetEventHandlerForPoint(point) == this)
    {
        return HTCAPTION;
    }

    return HTNOWHERE;
}

void LivehimeHybridWebBrowserView::SetWindowTitle(const base::string16& title)
{
    __super::SetWindowTitle(title);

    if (title_label_)
    {
        title_label_->SetText(title);
    }
}

void LivehimeHybridWebBrowserView::OnWidgetDestroyed(views::Widget* widget)
{
    __super::OnWidgetDestroyed(widget);

    if (parent_web_delegate_)
    {
        parent_web_delegate_->OnWebBrowserWidgetDestroyed(widget);
    }
}

void LivehimeHybridWebBrowserView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (sender->id() == kCloseButtonID)
    {
        GetWidget()->Close();
    }
}

// LivehimeWebBrowserDelegate
void LivehimeHybridWebBrowserView::OnWebBrowserCoreInvalid()
{
    state_view_->SetStateText(
        livehime::ListState::ListState_Faild, L"核心组件初始化失败，页面加载失败");
    state_view_->SetState(livehime::ListState::ListState_Faild);
}

void LivehimeHybridWebBrowserView::OnWebBrowserLoadStart(const std::string& url)
{
    webview_loading_start_time_ = base::Time::Now();
    url_loading_maps_[url] = webview_loading_start_time_;
}

std::string LivehimeHybridWebBrowserView::GetPageUrl(const std::string& page_url) {
    //去除测试环境ff域名及参数
    std::string url = page_url;
    size_t suffix_pos = url.find('?');
    if (suffix_pos != std::string::npos) {
        url = url.substr(0, suffix_pos);
    }

    suffix_pos = url.find("://ff-");
    if (suffix_pos != std::string::npos) {
        std::string protocol = url.substr(0, suffix_pos + 3);
        size_t point_pos = url.find('.');
        if (point_pos != std::string::npos) {
            std::string url_value = url.substr(point_pos + 1, url.length() - point_pos - 1);
            url = protocol + url_value;
        }
    }

    size_t ext_pos = url.rfind(".html");
    if (ext_pos != std::string::npos) {
        ext_pos = url.rfind('/');
        if (ext_pos != std::string::npos) {
            url = url.substr(0, ext_pos);
        }
    }
    return url;
}

void LivehimeHybridWebBrowserView::OnWebBrowserLoadEnd(const std::string& url, int http_status_code)
{

    if (HTTP_STATUS_OK == http_status_code)
    {
        web_browser_view_->NativeControlIgnoreViewVisibilityChanged(false, true);


    }
    else
    {
        OnWebBrowserLoadError(url, http_status_code, "");
    }

    if (url.empty() || HTTP_STATUS_OK != http_status_code)
        return;

    int64_t loading_time = 0;
    auto iter = url_loading_maps_.find(url);
    if (iter != url_loading_maps_.end()) {
        loading_time = (base::Time::Now() - iter->second).InMilliseconds();
    }

    if (!AppFunctionController::GetInstance()) {
        return;
    }
}

void LivehimeHybridWebBrowserView::OnWebBrowserLoadError(const std::string& failed_url, int http_status_code, const std::string& error_text)
{
    state_view_->SetState(livehime::ListState::ListState_Faild);
    if (AppFunctionController::GetInstance()){
       
    }
}

WebViewPopupType LivehimeHybridWebBrowserView::OnWebBrowserPopup(const std::string& url, int target_disposition)
{
    return allow_popup_;
}

void LivehimeHybridWebBrowserView::OnWidgetActivationChanged(views::Widget* widget, bool active)
{
	if (close_when_deactive_ && !active && GetWidget())
	{
		GetWidget()->Close();
        return;
	}
    __super::OnWidgetActivationChanged(widget, active);
}

bool LivehimeHybridWebBrowserView::OnWebBrowserMsgReceived(const std::string& msg_name, const cef_proxy::calldata* data)
{
    if (parent_web_delegate_)
    {
        return parent_web_delegate_->OnWebBrowserMsgReceived(msg_name, data);
    }
    return false;
}

void LivehimeHybridWebBrowserView::OnWebBrowserPageArouse(const std::string& module_name, const std::string& param, bool close_current)
{

}


LivehimeAnchorTaskWebBrowserView* g_anchor_task_view_instance = nullptr;
void LivehimeAnchorTaskWebBrowserView::ShowAnchorTaskTopWindow(const std::string& web_url)
{
    if (g_anchor_task_view_instance)
    {
		if (g_anchor_task_view_instance->GetWidget())
		{
            g_anchor_task_view_instance->GetWidget()->Show();
            g_anchor_task_view_instance->GetWidget()->Activate();
		}
    }
    else
    {
		hybrid_ui::LivehimeWebUrlDetails url_ex = hybrid_ui::GetUrlExInfo(web_url);
        if (url_ex.ex_width.empty() || url_ex.ex_height.empty())
        {
            url_ex.ex_width = std::to_string(GetLengthByDPIScale(375));
            url_ex.ex_height = std::to_string(GetLengthByDPIScale(626));
        }
		views::Widget* widget = new views::Widget();
		BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(widget);
		native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_NONE);
		native_widget->SetDisableRedrawWhenDidProcessMsg(true);
		views::Widget::InitParams params(views::Widget::InitParams::TYPE_PANEL);
		params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
		params.native_widget = native_widget;
		params.remove_taskbar_icon = true;
        g_anchor_task_view_instance = new LivehimeAnchorTaskWebBrowserView(url_ex);
		ShowWidget(g_anchor_task_view_instance, widget, params);

    }
}

void LivehimeAnchorTaskWebBrowserView::CloseAnchorTaskTopWindow()
{
    if (g_anchor_task_view_instance)
    {
        g_anchor_task_view_instance->GetWidget()->Close();
    }
}

LivehimeAnchorTaskWebBrowserView::LivehimeAnchorTaskWebBrowserView(const hybrid_ui::LivehimeWebUrlDetails& url_ex):
    LivehimeHybridWebBrowserView(url_ex, false, WebViewPopupType::Native, cef_proxy::client_handler_type::bilibili, "", this)
{

}

LivehimeAnchorTaskWebBrowserView::~LivehimeAnchorTaskWebBrowserView()
{
    g_anchor_task_view_instance = nullptr;
}

void LivehimeAnchorTaskWebBrowserView::OnWebBrowserPageArouse(const std::string& module_name, const std::string& param, bool close_current)
{

}


#include "base/ext/bind_lambda.h"
#include "bililive/bililive/ui/views/controls/bililive_textfield.h"
namespace livehime_test
{
    class TestBrowserNavigateEditView
        : public BililiveWidgetDelegate
        , views::TextfieldController
    {
    public:
        TestBrowserNavigateEditView()
        {
            label_ = new BililiveLabel(L"输入完网址拍回车就行了");
            label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
            textfield_ = new BililiveTextfield();
            textfield_->text_field()->SetController(this);
            AddChildView(label_);
            AddChildView(textfield_);
            base::MessageLoop::current()->PostTask(FROM_HERE,
                base::Bind(&views::Textfield::RequestFocus, base::Unretained(textfield_->text_field())));
        }

    protected:
        views::View *GetContentsView() override { return this; }

        gfx::Size GetPreferredSize() override
        {
            return gfx::Size(GetLengthByDPIScale(660), 36 + label_->GetPreferredSize().height() +
                ((views::View*)textfield_)->GetPreferredSize().height());
        }

        void Layout()
        {
            gfx::Rect bound(GetContentsBounds());

            gfx::Rect label_bound(bound);
            label_bound.Inset(12, 12, 12, 0);
            label_->SetBounds(label_bound.x(), label_bound.y(),
                label_bound.width(), label_->GetPreferredSize().height());
            textfield_->SetBounds(label_bound.x(), label_->bounds().bottom() + 12,
                label_bound.width(), ((views::View*)textfield_)->GetPreferredSize().height());
        }

        bool HandleKeyEvent(views::Textfield* sender, const ui::KeyEvent& key_event) override
        {
            if (key_event.key_code() == ui::VKEY_RETURN)
            {
                GetWidget()->Close();

                views::Widget* widget = views::Widget::GetWidgetForNativeView(::GetWindow(GetWidget()->GetNativeView(), GW_OWNER));
                std::string text = WideToASCII(sender->text());
                base::MessageLoop::current()->PostTask(FROM_HERE,
                    base::BindLambda([widget, text]() {
                    LivehimeHybridWebBrowserView::ShowWindow(widget, text, false, L"", WebViewPopupType::Native,
                        cef_proxy::client_handler_type::biliyun);
                }));

                return true;
            }
            return false;
        }

    private:
        BililiveLabel* label_ = nullptr;
        BililiveTextfield* textfield_ = nullptr;
    };

    void ShowPopupLivehimeBrowser(views::Widget* parent)
    {
        views::Widget *widget = new views::Widget();
        BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(widget);
        native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_CLOSE);
        views::Widget::InitParams params;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.native_widget = native_widget;
        params.parent = parent->GetNativeView();

        TestBrowserNavigateEditView* instance = new TestBrowserNavigateEditView();
        BililiveWidgetDelegate::ShowWidget(instance, widget, params);
    }
}