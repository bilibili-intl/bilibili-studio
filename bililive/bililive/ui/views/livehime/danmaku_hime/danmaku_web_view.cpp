#include "danmaku_web_view.h"

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

#include "cef/bililive_browser/public/bililive_browser_ipc_messages.h"

namespace
{
    const int kDefWidth = GetLengthByDPIScale(670);
    const int kDefHeight = GetLengthByDPIScale(600);
};

int DanmakuWebView::is_web_broswer_created_{ 0 };

void DanmakuWebView::ShowWindow(
    views::Widget* parent,
    const std::string& web_url,
    bool show_native_titlebar,
    const string16& title,
    WebViewPopupType allow_popup/* = WebViewPopupType::NotAllow*/,
    cef_proxy::client_handler_type browser_type/* = cef_proxy::client_handler_type::bilibili*/,
    const std::string& post_data/* = ""*/)
{
    hybrid_ui::LivehimeWebUrlDetails url_ex = hybrid_ui::GetUrlExInfo(web_url);

    views::Widget* widget = new views::Widget();
    BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(widget);
    native_widget->set_escape_operate(BililiveNativeWidgetWin::WO_CLOSE);
    native_widget->SetDisableRedrawWhenDidProcessMsg(true);
    views::Widget::InitParams params;
    params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
    params.native_widget = native_widget;
    if (parent)
    {
        params.parent = parent->GetNativeView();
    }

    DanmakuWebView* instance = new DanmakuWebView(
        url_ex, show_native_titlebar, allow_popup, browser_type, post_data);
    instance->SetWindowTitle(title);
    DoModalWidget(instance, widget, params);
}

bool DanmakuWebView::IsWebCreated()
{
    return is_web_broswer_created_ > 0;
}

DanmakuWebView::DanmakuWebView(
    const hybrid_ui::LivehimeWebUrlDetails& url_ex,
    bool show_native_titlebar,
    WebViewPopupType allow_popup,
    cef_proxy::client_handler_type browser_type/* = cef_proxy::client_handler_type::bilibili*/,
    const std::string& post_data/* = ""*/)
    : show_native_titlebar_(show_native_titlebar)
    , allow_popup_(allow_popup)
    , web_url_ex_(url_ex)
    , browser_type_(browser_type)
    , default_pref_size_(kDefWidth, kDefHeight)
    , post_data_(post_data)
{
    if (show_native_titlebar_) {
        native_close_always_show_ = false;
    }
    else {
        native_close_always_show_ = (web_url_ex_.ex_close == "1");
    }

    DCHECK(is_web_broswer_created_ >= 0);
    is_web_broswer_created_++;
}

DanmakuWebView::~DanmakuWebView() {

    DCHECK(is_web_broswer_created_ > 0);
    if (is_web_broswer_created_ > 0)
    {
        is_web_broswer_created_--;
    }
}

void DanmakuWebView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) {
    if (details.child == this) {
        if (details.is_add) {
            InitViews();
        }
    }
}

void DanmakuWebView::InitViews()
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

gfx::Size DanmakuWebView::GetPreferredSize()
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

void DanmakuWebView::Layout()
{
    gfx::Rect rect = GetContentsBounds();
    LayoutInBounds(rect);
}

void DanmakuWebView::LayoutInBounds(gfx::Rect rect)
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

views::NonClientFrameView* DanmakuWebView::CreateNonClientFrameView(views::Widget* widget)
{
    if (show_native_titlebar_) {
        return __super::CreateNonClientFrameView(widget);
    }
    return new BililiveNonTitleBarFrameView(this);
}

int DanmakuWebView::NonClientHitTest(const gfx::Point& point)
{
    if (GetEventHandlerForPoint(point) == this)
    {
        return HTCAPTION;
    }

    return HTNOWHERE;
}

void DanmakuWebView::SetWindowTitle(const base::string16& title)
{
    __super::SetWindowTitle(title);

    if (title_label_)
    {
        title_label_->SetText(title);
    }
}

void DanmakuWebView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (sender->id() == kCloseButtonID)
    {
        GetWidget()->Close();
    }
}

// LivehimeWebBrowserDelegate
void DanmakuWebView::OnWebBrowserCoreInvalid()
{
    state_view_->SetStateText(
        livehime::ListState::ListState_Faild, L"核心组件初始化失败，页面加载失败");
    state_view_->SetState(livehime::ListState::ListState_Faild);
}

void DanmakuWebView::OnWebBrowserLoadStart(const std::string& url)
{
}

void DanmakuWebView::OnWebBrowserLoadEnd(const std::string& url, int http_status_code)
{
    if (HTTP_STATUS_OK == http_status_code)
    {
        web_browser_view_->NativeControlIgnoreViewVisibilityChanged(false, true);
    }
    else
    {
        OnWebBrowserLoadError(url, http_status_code, "");
    }
}

void DanmakuWebView::OnWebBrowserLoadError(const std::string& failed_url, int http_status_code, const std::string& error_text)
{
    state_view_->SetState(livehime::ListState::ListState_Faild);
}

WebViewPopupType DanmakuWebView::OnWebBrowserPopup(const std::string& url, int target_disposition)
{
    return allow_popup_;
}

