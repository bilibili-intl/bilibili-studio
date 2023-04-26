#ifndef BILILIVE_SECRET_UI_VIEWS_LOGIN_LIVEHIME_CAPTCHA_FRAME_VIEW_H
#define BILILIVE_SECRET_UI_VIEWS_LOGIN_LIVEHIME_CAPTCHA_FRAME_VIEW_H

#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_widget.h"

namespace secret
{
    struct CaptchaInfo;
    struct SecondaryVerifyInfo;
}

class LiveHimeCaptchaFrameDelegate {
public:
    virtual void OnCaptchaVerifySuccessed(const secret::CaptchaInfo& captcha_info) = 0;
    virtual void OnSecondaryVerifyValidateLogin(const secret::SecondaryVerifyInfo& verify_info) = 0;
    virtual void OnSecondaryVerifySuccessed() = 0;
    virtual void OnCaptchaFrameClosed(bool captcha_verify_succeesed) = 0;
    virtual void OnChangePasswordSuccessed() = 0;
};

class LivehimeGeetestCaptchaView
    : public LivehimeHybridWebBrowserView
{
public:
    enum class VerifyStatus
    {
        Captcha,
        SecondaryVerify,
    };

    static void ShowGeetestCaptcha(views::Widget* owner_widget, const std::string& geetest_url,
                                   VerifyStatus status, LiveHimeCaptchaFrameDelegate* delegate);

    static bool ShowSecondaryVerify(const secret::SecondaryVerifyInfo& verify_info);

    static void CloseVerifyWindow();

protected:
    LivehimeGeetestCaptchaView(VerifyStatus status, const std::string& url, LiveHimeCaptchaFrameDelegate* delegate);
    virtual ~LivehimeGeetestCaptchaView();

    // WidgetDelegate
    views::NonClientFrameView *CreateNonClientFrameView(views::Widget *widget) override;
    void WindowClosing() override;

    // LivehimeWebBrowserDelegate
    void OnSetTokenCookiesCompleted(bool success) override;
    bool OnWebBrowserMsgReceived(const std::string& msg_name, const cef_proxy::calldata* data) override;

private:
    VerifyStatus status_ = VerifyStatus::Captcha;
    LiveHimeCaptchaFrameDelegate* delegate_ = nullptr;
    bool captcha_valid_closed_ = false;
    std::unique_ptr<secret::SecondaryVerifyInfo> secondary_verify_info_;
};

#endif //BILILIVE_SECRET_UI_VIEWS_LOGIN_BILILIVE_LOGIN_CAPTCHA_VIEW_H