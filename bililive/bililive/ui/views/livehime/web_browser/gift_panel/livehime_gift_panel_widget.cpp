#include "bililive/bililive/ui/views/livehime/web_browser/gift_panel/livehime_gift_panel_widget.h"
#include "bililive/bililive/ui/views/livehime/web_browser/bililive_browser_frame_view.h"

#include "base/bind.h"
#include "base/ext/bind_lambda.h"
#include "base/ext/callable_callback.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/icon_util.h"
#include "ui/views/focus/accelerator_handler.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/browser/livehime_web_browser_view.h"
#include "bililive/bililive/ui/views/controls/browser/livehime_web_browser_view_osr.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_list_state_banner.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_datatype.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/secret/public/event_tracking_service.h"
#include <bililive/bililive/livehime/function_control/app_function_controller.h>

#include "cef/bililive_browser/public/bililive_browser_ipc_messages.h"


namespace {
    const int kCommonTitleHigth = GetLengthByDPIScale(30);
    const int kCommonDefWidth = GetLengthByDPIScale(800);
    const int kCommonDefHeight = GetLengthByDPIScale(600);
}

void BililiveGiftPanelBrowserView::ShowWindow(views::Widget* parent,
    const std::string& web_url,
    const string16& title,
    WebViewPopupType allow_popup,
    cef_proxy::client_handler_type browser_type,
    const std::string& post_data,
    LivehimeWebBrowserDelegate* parent_web_delegate,
    bool window_stack)
{
    views::Widget* widget = GetCurrentWidget();
    if (widget && !window_stack) {
        if (!widget->IsVisible()) {
            widget->Show();
        }
        return;
    }
    hybrid_ui::LivehimeWebUrlDetails url_ex = hybrid_ui::GetUrlExInfo(web_url);

    BililiveGiftPanelBrowserView* unite_browser = new BililiveGiftPanelBrowserView(url_ex, allow_popup, title,browser_type, post_data, parent_web_delegate);

    widget = new views::Widget();
    views::Widget::InitParams params;
    BililiveNativeWidgetWin* native_widget = new BililiveNativeWidgetWin(widget);
    params.parent = parent->GetNativeView();
    params.native_widget = native_widget;

    params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;
    params.type = views::Widget::InitParams::TYPE_WINDOW;

    native_widget->SetDisableRedrawWhenDidProcessMsg(true);
    BililiveWidgetDelegate::DoModalWidget(unite_browser, widget, params);

}

BililiveGiftPanelBrowserView::BililiveGiftPanelBrowserView(const hybrid_ui::LivehimeWebUrlDetails& url_ex,
    WebViewPopupType allow_popup,
    const base::string16& title,
    cef_proxy::client_handler_type browser_type,
    const std::string& post_data,
    LivehimeWebBrowserDelegate* parent_web_delegate)
    : post_data_(post_data),
    parent_web_delegate_(parent_web_delegate),
    web_url_ex_(url_ex),
    init_title_(title),
    browser_type_(browser_type),
    allow_popup_(allow_popup),
    weak_wpf_(this),
    cef_type_(CefType::Custom_Title)
{
    InitCefType();
    InitCefRect();
    if (!GiftPanelTitleView::GetViewInstance()) {
        gift_title_ = GiftPanelTitleView::GetView();
        gift_title_->SetDelegate(this, cef_type_ == CefType::Close_Title || cef_type_ == CefType::Custom_Title);
    }
}

BililiveGiftPanelBrowserView::~BililiveGiftPanelBrowserView()
{
}

string16 BililiveGiftPanelBrowserView::GetWindowTitle() const {
    return L"";
}

int BililiveGiftPanelBrowserView::NonClientHitTest(const gfx::Point& point) {
    return HTCAPTION;
}

gfx::Size BililiveGiftPanelBrowserView::GetPreferredSize() {

    int height = browser_height_;
    if (cef_type_ == CefType::Close_Title || cef_type_ == CefType::Custom_Title)
    {
        height += kCommonTitleHigth;
    }
    return gfx::Size(browser_width_, height);

}

void BililiveGiftPanelBrowserView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details)
{
    if (details.is_add && details.child == this) {
        InitView();
    }
}

void BililiveGiftPanelBrowserView::InitView() {
    ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();

    state_view_ = new LivehimeListStateBannerView();
    state_view_->SetStateText(
        livehime::ListState::ListState_Loading, GetLocalizedString(IDS_LUCKYGIFT_INFO_STATUS_LOADING));
    state_view_->SetStateText(
        livehime::ListState::ListState_Faild, GetLocalizedString(IDS_LUCKYGIFT_INFO_STATUS_LOADING_FAILED));
    AddChildView(state_view_);

    set_background(nullptr);
    gfx::Rect rect(0, 0, browser_width_, browser_height_);
    offscreen_browser_view_ = new LivehimeWebBrowserViewOSR(cef_proxy::client_handler_type::bilibili, rect, 60,
        web_url_ex_.url,
        this);
    offscreen_browser_view_->NativeControlIgnoreViewVisibilityChanged(true, false);
    AddChildView(offscreen_browser_view_);

    if (gift_title_) {
        gfx::Rect rect(0, 0, browser_width_, kCommonTitleHigth);
        gift_title_->Pop(rect, GetWidget());

        gift_title_->ChangeBackGround(web_url_ex_.ex_background);
        gift_title_->SetTitleContent(init_title_);
        gift_title_->Pin(true);
        AddChildView(gift_title_);
    }
}

void BililiveGiftPanelBrowserView::Layout() {

    gfx::Rect rect = GetContentsBounds();
    offscreen_browser_view_->SetBoundsRect(rect);

    auto pref_size = state_view_->GetPreferredSize();
    state_view_->SetBounds(rect.x(), kCommonDefHeight,
        rect.width(), rect.height());
}

void BililiveGiftPanelBrowserView::OnButtonPressed(CommonType type)
{
    base::string16 tag_name;
    if (type == CommonType::Button_Close) {
        GetWidget()->Close();
    }
    else if (type == CommonType::Button_Help) {
        tag_name = L"help";
    }
    else if (type == CommonType::Button_Custom) {
        tag_name = L"custom";
    }
    else if (type == CommonType::Button_Set) {
        tag_name = L"set";
    }

    if (!tag_name.empty()) {
        SendTitleBtnJsbridge(tag_name);
    }
}

void BililiveGiftPanelBrowserView::InitCefType()
{
    if (web_url_ex_.ex_close == "0") {
        cef_type_ = CefType::Full_Screen;
    }
    else if (web_url_ex_.ex_close == "1") {
        cef_type_ = CefType::Close_Title;
    }
    else if (web_url_ex_.ex_close == "2") {
        cef_type_ = CefType::Custom_Title;
    }
    else if (web_url_ex_.ex_close == "3") {
        cef_type_ = CefType::Transparent_Title;
    }
    else if (web_url_ex_.ex_close == "4") {
        cef_type_ = CefType::Off_Screen;
    }
}

void BililiveGiftPanelBrowserView::InitCefRect()
{
    int width = 0;
    int height = 0;
    if (base::StringToInt(web_url_ex_.ex_width, &width) &&
        base::StringToInt(web_url_ex_.ex_height, &height))
    {
        browser_width_ = GetLengthByDPIScale(width);
        browser_height_ = GetLengthByDPIScale(height);
    }
    else {
        browser_width_ = kCommonDefWidth;
        browser_height_ = kCommonDefHeight;
    }
}

void BililiveGiftPanelBrowserView::GeneralMsgBoxCallBack(const cef_proxy::browser_bind_data& bind_data, const std::string& ipc_message,
    const CefAlert& cef_alert,const std::pair<base::string16, base::string16>& msg_btn, void* data)
{
    cef_proxy::calldata calldata;
    cef_proxy::calldata_filed data_filed;

    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
    data_filed.numeric_union.int_ = (int)(cef_proxy::callback_type::CALL_BACK_TYPE_SUCCESS_DATA);
    const_cast<cef_proxy::calldata&>(calldata).insert({ cef_proxy::kCallbackTypeField, data_filed });

    data_filed.reset();
    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
    if (msg_btn.first == cef_alert.confirmButton) {
        data_filed.str_ = "confirm";
    }
    else {
        data_filed.str_ = "cancel";
    }
    const_cast<cef_proxy::calldata&>(calldata).insert({ "type", data_filed });

    data_filed.reset();
    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
    data_filed.numeric_union.int_ = cef_alert.successCallbackId;
    const_cast<cef_proxy::calldata&>(calldata).insert({ "successCallbackId", data_filed });

    data_filed.reset();
    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_BOOL;
    data_filed.numeric_union.bool_ = true;
    const_cast<cef_proxy::calldata&>(calldata).insert({ "isSuccessCallback", data_filed });

    CefProxyWrapper::GetInstance()->DispatchJsEvent(bind_data,
                ipc_messages::kBilibiliPageAlert, &calldata);
}

void BililiveGiftPanelBrowserView::OnWebBrowserGeneralEvent(const cef_proxy::browser_bind_data& bind_data,
    const std::string& ipc_message, const cef_proxy::calldata* data)
{
    if (ipc_message == ipc_messages::kBilibiliPageAlert) {

        CefAlert cef_alert;
        if (CefCalbackDataPresenter::GetInstance()->ReadJsbridgeAlert(data, cef_alert)) {

            livehime::GeneralMsgBoxEndDialogSignalHandler handler;
            handler.closure = base::Bind(&BililiveGiftPanelBrowserView::GeneralMsgBoxCallBack, weak_wpf_.GetWeakPtr(), bind_data, ipc_message, cef_alert);

            livehime::ShowGeneralMessageBox(
            GetWidget()->GetNativeView(),
                cef_alert.title,
                cef_alert.msg, &handler, cef_alert.type,
                cef_alert.confirmButton, cef_alert.cancelButton, cef_alert.noRemindKey);
        }
    }
    else if(ipc_message == ipc_messages::kBilibiliPageTitleBar){
        gift_title_->SetVisible(false);
        return;
        CefTitleBar cef_title_bar;
        if (CefCalbackDataPresenter::GetInstance()->ReadJsbridgeTitleBar(data, cef_title_bar)) {
            title_bind_data_ = bind_data;
            cef_title_bar_ = cef_title_bar;
            if (gift_title_ ) {
                gift_title_->SetTitleBar(cef_title_bar);
            }
        }
    }
}

void BililiveGiftPanelBrowserView::SendTitleBtnJsbridge(const base::string16& btn_name)
{
    cef_proxy::calldata calldata;
    cef_proxy::calldata_filed data_filed;

    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
    data_filed.numeric_union.int_ = (int)(cef_proxy::callback_type::CALL_BACK_TYPE_SUCCESS_DATA);
    const_cast<cef_proxy::calldata&>(calldata).insert({ cef_proxy::kCallbackTypeField, data_filed });

    data_filed.reset();
    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_WSTRING;
    data_filed.wstr_ = btn_name;
    const_cast<cef_proxy::calldata&>(calldata).insert({ "tagname", data_filed });

    data_filed.reset();
    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
    data_filed.numeric_union.int_ = cef_title_bar_.successCallbackId;
    const_cast<cef_proxy::calldata&>(calldata).insert({ "successCallbackId", data_filed });

    data_filed.reset();
    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_BOOL;
    data_filed.numeric_union.bool_ = true;
    const_cast<cef_proxy::calldata&>(calldata).insert({ "isSuccessCallback", data_filed });

    if (&title_bind_data_) {
        CefProxyWrapper::GetInstance()->DispatchJsEvent(title_bind_data_,
            ipc_messages::kBilibiliPageTitleBar, &calldata);
    }
}

void BililiveGiftPanelBrowserView::OnWebBrowserCoreInvalid()
{
    state_view_->SetStateText(
        livehime::ListState::ListState_Faild, L"核心组件初始化失败，页面加载失败");
    state_view_->SetState(livehime::ListState::ListState_Faild);
}

void BililiveGiftPanelBrowserView::OnWebBrowserLoadStart(const std::string& url)
{
    webview_loading_start_time_ = base::Time::Now();
    url_loading_maps_[url] = webview_loading_start_time_;
}

void BililiveGiftPanelBrowserView::OnWebBrowserLoadTitle(const base::string16& title)
{
    if (!title.empty() && init_title_.empty()) {
        if (gift_title_) {
            gift_title_->SetTitleContent(title);
        }
    }
}

std::string BililiveGiftPanelBrowserView::GetPageUrl(const std::string& page_url) {
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

void BililiveGiftPanelBrowserView::OnWebBrowserLoadEnd(
    const std::string& url,
    int http_status_code)
{
    if (HTTP_STATUS_OK == http_status_code) {
        if (web_browser_view_) {
            web_browser_view_->NativeControlIgnoreViewVisibilityChanged(false, true);
        }
        if (offscreen_browser_view_) {
            offscreen_browser_view_->NativeControlIgnoreViewVisibilityChanged(false, true);
        }
    }
    else {
        OnWebBrowserLoadError(url, http_status_code,"");
    }

    auto rect = this->GetBoundsInScreen();
    rect.set_height(kCommonTitleHigth);
    if (gift_title_) {
        gift_title_->GetWidget()->Show();
        gift_title_->GetWidget()->SetBounds(rect);
    }

    InvalidateLayout();
    Layout();

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

void BililiveGiftPanelBrowserView::OnWebBrowserLoadError(const std::string& failed_url, int http_status_code, const std::string& error_text)
{
    state_view_->SetState(livehime::ListState::ListState_Faild);
    if (AppFunctionController::GetInstance()) {

    }
}

WebViewPopupType BililiveGiftPanelBrowserView::OnWebBrowserPopup(const std::string& url, int target_disposition)
{
    return allow_popup_;
}

bool BililiveGiftPanelBrowserView::OnWebBrowserMsgReceived(const std::string& msg_name, const cef_proxy::calldata* data)
{
    if (parent_web_delegate_)
    {
        return parent_web_delegate_->OnWebBrowserMsgReceived(msg_name, data);
    }
    return false;
}

bool BililiveGiftPanelBrowserView::OnWebBrowserPreKeyEvent(const cef_proxy::key_event_t& key_event)
{
    return false;
}

bool BililiveGiftPanelBrowserView::OnWebBrowserKeyEvent(const cef_proxy::key_event_t& key_event)
{
    return false;
}

void BililiveGiftPanelBrowserView::OnWidgetDestroyed(views::Widget* widget)
{
    __super::OnWidgetDestroyed(widget);

    if (parent_web_delegate_)
    {
        parent_web_delegate_->OnWebBrowserWidgetDestroyed(widget);
    }
}

views::NonClientFrameView* BililiveGiftPanelBrowserView::CreateNonClientFrameView(views::Widget* widget) {
    /*if (cef_type_ == CefType::Off_Screen) {
        BililiveNonTitleBarFrameView* frame_view = new BililiveNonTitleBarFrameView(nullptr);
        frame_view->SetEnableDragMove(false);
        frame_view->SetBackgroundColor(SK_ColorTRANSPARENT);
        return frame_view;
    }
    else {
        return new BililiveBrowserFrameView(this);
    }*/

    BililiveNonTitleBarFrameView* frame_view = new BililiveNonTitleBarFrameView(nullptr);
    frame_view->SetEnableDragMove(false);
    frame_view->SetBackgroundColor(SK_ColorTRANSPARENT);
    return frame_view;
}