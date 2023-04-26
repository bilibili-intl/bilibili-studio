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
    // ����ͼ�Ŀ�߱���1.2��ҳ�������46DIP�����ҹ̶��߾�20DIP��
    // ��γ���֮��ó�ͼ�Ŀ����260��ʱ���ֺ�ͼ���������Ƚ����һЩ
    float kCaptchaWebScale = 1.2f;
    int kCaptchaWebBorderThickness = 20;
    int kCaptchaWebTitleHeight = 46;
    int kCaptchaWidth = 260;
    int kCaptchaHeight = kCaptchaWidth / kCaptchaWebScale;

    int kOnlineHelpWidth = 360;// ҳ��Դ�뿴���߰�����С���
    // ���ڶ�����֤ҳ�����������߰��������߰���������С�ߴ�Ҫ����ô������֤ҳ�Ŀ�ȶ�Ϊ����ҳ��ȣ�
    // ����Ϊ������֤ҳ����һ���Ŀ�߱������������ŵģ����Զ�����֤ҳ�ĸ߶�Ҫ���裬�Ա��ʼҳ�����ֹ�����
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
    // Ϊ������֤��ҳ�����ó�ʼ��ɫ
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
        // �����¼̬�Ѿ������˾�ֱ����CEF�������/����ҳ�棬����͵�cookie������ɺ�����ת
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
    // �û���Ĺرգ��˻ص��˺������������
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
        // ֪ͨ��¼��������֤��ϣ������ٴε��õ�¼�ӿڣ���¼�������UI״̬����Ҫ�л�������ȻΪ����¼�С�
        if (delegate_)
        {
            // ��֤�ɹ�������֤�������ӿ��ٴη����¼
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
        // ֪ͨ��¼��������֤��ϣ������ٴε��õ�¼�ӿڣ���¼�������UI״̬����Ҫ�л�������ȻΪ����¼�С�
        if (delegate_)
        {
            // ��֤�ɹ�������֤�������ӿ��ٴη����¼
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
        // ��ʶ��֤��Ϊ��֤��У��ͨ���������Ļ��ڽӿ�δ����ǰ�û��͹ص����ڵĻ�
        // ����¼����Ҳ���ᵼ��UI״̬��������¼����״̬��ƥ��,
        // ��¼�������UI״̬����Ҫ�л�������ȻΪ����¼�С�
        captcha_valid_closed_ = true;

        // ��֤�ɹ�������֤�������ӿ��ٴη����¼
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
        // ֪ͨ����¼�����õ�¼̬
        if (delegate_)
        {
            delegate_->OnChangePasswordSuccessed();
        }
        captcha_valid_closed_ = false;
    }
    else if (msg_name == ipc_messages::kChangePasswordReLogin)
    {
        handled = true;
        // ����Ҫ��ʾ�ص�ҳ��ֱ�ӹرմ���
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
