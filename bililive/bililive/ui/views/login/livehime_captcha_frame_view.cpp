#include "bililive/bililive/ui/views/login/livehime_captcha_frame_view.h"

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/base/win/hwnd_util.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/widget/widget.h"

#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/bililive_frame_view/bililive_frame_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/utils/net_util.h"
#include "bililive/common/bililive_context.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/secret/public/login_info.h"

#include "cef/bililive_browser/public/bililive_browser_ipc_messages.h"

namespace
{
    // 滑块图的宽高比是1.2，页面标题栏46DIP，左右固定边距20DIP，
    // 多次尝试之后得出图的宽度是260的时候字和图看起来都比较舒服一些
    float kCaptchaWebScale = 1.2f;
    int kCaptchaWebBorderThickness = 20;
    int kCaptchaWebTitleHeight = 46;
    int kCaptchaWidth = 260;
    int kCaptchaHeight = kCaptchaWidth / kCaptchaWebScale;

    int kOnlineHelpWidth = 360;// 页面源码看在线帮助最小宽度
    // 由于二次验证页可以跳到在线帮助，在线帮助又有最小尺寸要求，那么二次验证页的宽度定为帮助页宽度，
    // 又因为二次验证页是以一定的宽高比例进行自缩放的，所以二次验证页的高度要另设，以便初始页不出现滚动条
    int kSecondaryVerifyHeight = 468;

    gfx::Size kCaptchaWebSize((kCaptchaWidth + kCaptchaWebBorderThickness * 2),
                              (kCaptchaWebTitleHeight + kCaptchaHeight + kCaptchaWebBorderThickness));

    gfx::Size kSecondaryVerifyWebSize(kOnlineHelpWidth, kSecondaryVerifyHeight);

    LivehimeGeetestCaptchaView* g_captcha_view_instance = nullptr;
}

void LivehimeGeetestCaptchaView::ShowGeetestCaptcha(views::Widget* owner_widget, const std::string& geetest_url,
                                                    VerifyStatus status, LiveHimeCaptchaFrameDelegate* delegate)
{
    DCHECK(!g_captcha_view_instance);

    views::Widget *widget = new views::Widget();
    views::Widget::InitParams params;
    params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
    params.parent = owner_widget->GetNativeView();
    params.native_widget = new BililiveNativeWidgetWin(widget);

    std::string url(geetest_url);
    // 为二次验证的页面设置初始底色
    if (status == VerifyStatus::SecondaryVerify)
    {
        url = bililive::AppendURLQueryParams(geetest_url, base::StringPrintf("pc_ui=%d,%d,F4F4F4,1",
            kSecondaryVerifyWebSize.width(), kSecondaryVerifyWebSize.height()));
    }
    else
    {
        url = bililive::AppendURLQueryParams(geetest_url, base::StringPrintf("pc_ui=%d,%d,FFFFFF,0",
            kCaptchaWebSize.width(), kCaptchaWebSize.height()));
    }

    g_captcha_view_instance = new LivehimeGeetestCaptchaView(status, url, delegate);
    BililiveWidgetDelegate::DoModalWidget(g_captcha_view_instance, widget, params);
}

bool LivehimeGeetestCaptchaView::ShowSecondaryVerify(const secret::SecondaryVerifyInfo& verify_info)
{
    DCHECK(g_captcha_view_instance);
    if (g_captcha_view_instance)
    {
        g_captcha_view_instance->secondary_verify_info_ =
            std::make_unique<secret::SecondaryVerifyInfo>(verify_info);
        // 如果登录态已经设置了就直接让CEF跳到完成/改密页面，否则就等cookie设置完成后再跳转
        if (CefProxyWrapper::GetInstance()->IsTokenCookiesValid())
        {
            g_captcha_view_instance->OnSetTokenCookiesCompleted(true);
        }
        return true;
    }
    return false;
}

void LivehimeGeetestCaptchaView::CloseVerifyWindow()
{
    if (g_captcha_view_instance)
    {
        g_captcha_view_instance->GetWidget()->Close();
    }
}

LivehimeGeetestCaptchaView::LivehimeGeetestCaptchaView(VerifyStatus status, const std::string& url,
                                                       LiveHimeCaptchaFrameDelegate* delegate)
    : LivehimeHybridWebBrowserView(hybrid_ui::GetUrlExInfo(url), false, WebViewPopupType::NotAllow,
    (status == VerifyStatus::Captcha) ?
        cef_proxy::client_handler_type::geetest :
        cef_proxy::client_handler_type::sec_sign_verify)
    , status_(status)
    , delegate_(delegate)
{
    SetShowMaskWndWhenDoModal(true);

    SetWindowTitle(GetLocalizedString(
        (status_ == VerifyStatus::Captcha) ? IDS_LOGIN_CAPTCHA_HINT : IDS_LOGIN_SECURITY_VERIFY));
}

LivehimeGeetestCaptchaView::~LivehimeGeetestCaptchaView()
{
}

void LivehimeGeetestCaptchaView::WindowClosing()
{
    // 用户点的关闭，退回到账号密码输入界面
    if (delegate_)
    {
        delegate_->OnCaptchaFrameClosed(captcha_valid_closed_);
    }
    g_captcha_view_instance = nullptr;
}

views::NonClientFrameView* LivehimeGeetestCaptchaView::CreateNonClientFrameView(views::Widget *widget)
{
    BililiveNonTitleBarFrameView* frame_view = (BililiveNonTitleBarFrameView*)__super::CreateNonClientFrameView(widget);
    frame_view->SetEnableDragMove(false);
    return frame_view;
}

bool LivehimeGeetestCaptchaView::OnWebBrowserMsgReceived(const std::string& msg_name, const cef_proxy::calldata* data)
{
    bool handled = false;
    if (msg_name == ipc_messages::kGeetestVerifyCompleted)
    {
        handled = true;
        // 通知登录主界面验证完毕，让其再次调用登录接口，登录主界面的UI状态不需要切换，即仍然为“登录中”
        if (delegate_)
        {
            // 验证成功，用验证参数填充接口再次发起登录
            secret::CaptchaInfo captcha_info;
            captcha_info.challenge = data->at("challenge").str_;
            captcha_info.validate = data->at("validate").str_;
            captcha_info.seccode = data->at("seccode").str_;

            delegate_->OnCaptchaVerifySuccessed(captcha_info);
        }

        captcha_valid_closed_ = true;
        GetWidget()->Close();
    }
    else if (msg_name == ipc_messages::kGeetestImageCaptchaVerifyCompleted)
    {
        handled = true;
        // 通知登录主界面验证完毕，让其再次调用登录接口，登录主界面的UI状态不需要切换，即仍然为“登录中”
        if (delegate_)
        {
            // 验证成功，用验证参数填充接口再次发起登录
            secret::CaptchaInfo captcha_info;
            captcha_info.captcha = data->at("captcha").str_;

            delegate_->OnCaptchaVerifySuccessed(captcha_info);
        }

        captcha_valid_closed_ = true;
        GetWidget()->Close();
    }
    else if (msg_name == ipc_messages::kGeetestClosed)
    {
        handled = true;
        GetWidget()->Close();
    }
    else if (msg_name == ipc_messages::kSecondaryVerifyValidateLogin)
    {
        handled = true;
        // 标识验证框为验证码校验通过，这样的话在接口未返回前用户就关掉窗口的话
        // 主登录界面也不会导致UI状态和整个登录流程状态不匹配,
        // 登录主界面的UI状态不需要切换，即仍然为“登录中”
        captcha_valid_closed_ = true;

        // 验证成功，用验证参数填充接口再次发起登录
        if (delegate_)
        {
            secret::SecondaryVerifyInfo verify_info;
            verify_info.ticket = data->at("ticket").str_;
            verify_info.target_url = data->at("target_url").str_;
            verify_info.callbackId = data->at("callbackId").str_;

            delegate_->OnSecondaryVerifyValidateLogin(verify_info);
        }
    }
    else if (msg_name == ipc_messages::kSecondaryVerifyCompleted)
    {
        handled = true;
        captcha_valid_closed_ = true;
        GetWidget()->Close();

        if (delegate_)
        {
            delegate_->OnSecondaryVerifySuccessed();
        }
    }
    else if (msg_name == ipc_messages::kLivehimeCefOnTitleChanged)
    {
        handled = true;
        if (data)
        {
            SetWindowTitle(base::UTF8ToWide(data->at("title").str_));
        }
    }
    else if (msg_name == ipc_messages::kChangePasswordCompleted)
    {
        handled = true;
        // 通知主登录框重置登录态
        if (delegate_)
        {
            delegate_->OnChangePasswordSuccessed();
        }
        captcha_valid_closed_ = false;
    }
    else if (msg_name == ipc_messages::kChangePasswordReLogin)
    {
        handled = true;
        // 不需要显示重登页，直接关闭窗口
        captcha_valid_closed_ = false;
        GetWidget()->Close();
    }

    return handled;
}

void LivehimeGeetestCaptchaView::OnSetTokenCookiesCompleted(bool success)
{
    if (secondary_verify_info_)
    {
        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

        data_filed.str_ = secondary_verify_info_->target_url;
        data["target_url"] = data_filed;

        data_filed.str_ = secondary_verify_info_->callbackId;
        data["callbackId"] = data_filed;

        CefProxyWrapper::GetInstance()->ExecuteJSFunctionWithKV(
            *web_browser_view()->browser_bind_data(),
            ipc_messages::kSecondaryVerifyAccessTokenCompleted,
            &data);
    }
}
