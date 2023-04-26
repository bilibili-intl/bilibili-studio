#ifndef BILILIVE_SECRET_UI_VIEWS_LOGIN_BILILIVE_LOGIN_VIEW_H_
#define BILILIVE_SECRET_UI_VIEWS_LOGIN_BILILIVE_LOGIN_VIEW_H_

#include "ui/views/controls/button/button.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/label.h"
#include "ui/views/view.h"

#include "bililive/bililive/login/bililive_login_contract.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/login/bililive_login_info_collection_view.h"
#include "bililive/bililive/ui/views/login/bililive_login_logining_status_view.h"
#include "bililive/bililive/ui/views/login/livehime_captcha_frame_view.h"
#include "bililive/secret/public/login_info.h"

class BililiveLoginMainView
    : public BililiveWidgetDelegate,
      public views::ButtonListener,
      public LoginInfoCollectionDelegate,
      public LiveHimeCaptchaFrameDelegate,
      public LoginingStatusDelegate,
      public contracts::BililiveLoginMainView {
public:
    static void DoModal(LoginResultCode* result_code);

    int NonClientHitTest(const gfx::Point &point);

    explicit BililiveLoginMainView(LoginResultCode* result_code);

    ~BililiveLoginMainView();

    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;

private:
    void InitViews();

    void InitData();

    void SetLoginUserFace(int64 mid);

    void GoLogining();

    void LoginErrorDefault();

    void LoginErrorDetails(int code, const std::string& url);

    void LoginErrorStatus(int status);

    void ResizeWidget();

    // login_presenter_impl
    void OnCheckAuthToken(bool valid_response, int code, const std::string& token) override;

    void OnGetAuthKeyError(bool valid_response, int code) override;

    void OnLogin(
        bool valid_response,
        int code,
        const std::string& account,
        const secret::UserAccountService::LoginResponseInfo& info) override;

    void OnAccessToken(
        bool valid_response,
        int code,
        const std::string& account,
        const secret::UserAccountService::LoginResponseInfo& info,
        const secret::SecondaryVerifyInfo& verify_info) override;

    void OnTokenExpired() override;

    // WidgetDelegate
    ui::ModalType GetModalType() const{ return ui::MODAL_TYPE_WINDOW; }
    views::NonClientFrameView* CreateNonClientFrameView(views::Widget* widget) override;
    views::View* GetContentsView() override { return this; }
    string16 GetWindowTitle() const override;
    bool ShouldShowWindowIcon() const override{ return true; }
    gfx::ImageSkia GetWindowIcon() override;

    // WidgetObserver
    void OnWidgetClosing(views::Widget* widget) override;

    // View
    void OnPaintBackground(gfx::Canvas* canvas) override;
    gfx::Size GetPreferredSize() override;
    void Layout() override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // LoginInfoCollectionDelegate
    void OnInfoCollectionButtonPressed() override;
    void OnInfoCollectionLinkClicked(int cid) override;

    // LiveHimeCaptchaFrameDelegate
    void OnCaptchaVerifySuccessed(const secret::CaptchaInfo& captcha_info) override;
    void OnSecondaryVerifyValidateLogin(const secret::SecondaryVerifyInfo& verify_info) override;
    void OnSecondaryVerifySuccessed() override;
    void OnCaptchaFrameClosed(bool captcha_verify_succeesed) override;
    void OnChangePasswordSuccessed() override;

    // LoginingStatusDelegate
    void OnLoginingStatusStop() override;
    void OnLoginingStatusMsgChanged(const string16& msg) override;

    void OnLoginSuccessed();
    void OnLoginFailed();

private:
    LoginResultCode* result_code_ = nullptr;

    gfx::ImageSkia main_page_bk_image_;

    gfx::ImageSkia main_page_logo_image_;

    gfx::ImageSkia waiting_page_logo_image_;

    scoped_ptr<gfx::ImageSkia> user_face_image_;

    SkColor waiting_page_logo_color_;

    views::View *title_bar_ = nullptr;

    views::Label *version_label_ = nullptr;

    views::ImageButton *close_button_ = nullptr;

    LoginInfoCollectionView* info_collection_view_ = nullptr;

    LoginingStatusView* logining_status_view_ = nullptr;

    std::unique_ptr<contracts::BililiveLoginPresenter> presenter_;

    DISALLOW_COPY_AND_ASSIGN(BililiveLoginMainView);
};

#endif  // BILILIVE_SECRET_UI_VIEWS_LOGIN_BILILIVE_LOGIN_VIEW_H_