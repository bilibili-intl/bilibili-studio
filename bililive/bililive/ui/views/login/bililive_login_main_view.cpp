#include "bililive/bililive/ui/views/login/bililive_login_main_view.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/icon_util.h"
#include "ui/views/focus/accelerator_handler.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/app/bililive_dll_resource.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/login/bililive_login_frame_view.h"
#include "bililive/bililive/utils/bililive_image_util.h"
#include "bililive/common/bililive_context.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/bililive_paths.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/utils/bililive_crypto.h"


namespace
{
    const int kImageWidth = GetLengthByDPIScale(430);

    enum CtrlID
    {
        Button_Close = 1,
    };

    string16 FormatErrorMsg(int code, int message_id)
    {
        ResourceBundle &resource_handle = ResourceBundle::GetSharedInstance();
        string16 error_msg;
        return error_msg.append(std::to_wstring(code)).
            append(L" ").
            append(resource_handle.GetLocalizedString(message_id));
    }

}

void BililiveLoginMainView::DoModal(LoginResultCode* result_code)
{
    BililiveLoginMainView *login_view = new BililiveLoginMainView(result_code);

    views::Widget *widget = new views::Widget();
    views::Widget::InitParams params;
    params.native_widget = new BililiveNativeWidgetWin(widget);
    params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
    BililiveWidgetDelegate::ShowWidget(login_view, widget, params);
    widget->UpdateWindowIcon();

    views::AcceleratorHandler accelerator_handler;
    base::RunLoop run_loop(&accelerator_handler);
    run_loop.Run();
}

BililiveLoginMainView::BililiveLoginMainView(LoginResultCode* result_code)
    : result_code_(result_code),
      title_bar_(nullptr),
      version_label_(nullptr),
      close_button_(nullptr),
      info_collection_view_(nullptr),
      logining_status_view_(nullptr) {
    user_face_image_.reset(new gfx::ImageSkia());
}

BililiveLoginMainView::~BililiveLoginMainView() {
}

// WidgetDelegate
string16 BililiveLoginMainView::GetWindowTitle() const {
    return ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_LOGIN_TITLE);
}

gfx::ImageSkia BililiveLoginMainView::GetWindowIcon() {
    scoped_ptr<SkBitmap> bitmap = IconUtil::CreateSkBitmapFromIconResource(
        GetModuleHandle(bililive::kBililiveResourcesDll), IDR_MAINFRAME, 0);
    return gfx::ImageSkia(gfx::ImageSkiaRep(*bitmap, ui::GetScaleFactorFromScale(ui::win::GetDeviceScaleFactor())));
}

views::NonClientFrameView *BililiveLoginMainView::CreateNonClientFrameView(views::Widget* widget) {
    return new BililiveLoginFrameView(this);
}

int BililiveLoginMainView::NonClientHitTest(const gfx::Point &point)
{
    if (title_bar_->bounds().Contains(point))
    {
        return close_button_->bounds().Contains(point) ? HTNOWHERE : HTCAPTION;
    }

    if (info_collection_view_->visible())
    {
        return info_collection_view_->bounds().Contains(point) ? HTNOWHERE : HTCAPTION;
    }

    if (logining_status_view_->visible())
    {
        return logining_status_view_->bounds().Contains(point) ? HTNOWHERE : HTCAPTION;
    }

    return HTNOWHERE;
}

// WidgetObserver
void BililiveLoginMainView::OnWidgetClosing(views::Widget* widget) {

    base::MessageLoop::current()->Quit();
}

// View
void BililiveLoginMainView::OnPaintBackground(gfx::Canvas* canvas) {
    canvas->FillRect(GetLocalBounds(), SK_ColorWHITE);
    if (!main_page_bk_image_.isNull()) {
        static float main_img_scale = main_page_bk_image_.width() * 1.0f / main_page_bk_image_.height();
        int img_cy = width() / main_img_scale;
        canvas->DrawImageInt(main_page_bk_image_, 0, 0, main_page_bk_image_.width(), main_page_bk_image_.height(),
            0, 0, width(), img_cy, true);
    }
}

gfx::Size BililiveLoginMainView::GetPreferredSize() {
    if (!title_bar_ || !info_collection_view_)
    {
        NOTREACHED();
        LOG(WARNING) << "login frame prefersize invalid ptr";
        return gfx::Size();
    }

    gfx::Size title_size = title_bar_->GetPreferredSize();
    gfx::Size colt_size = info_collection_view_->GetPreferredSize();
    int cx = std::max(title_size.width(), colt_size.width());
    int cy = colt_size.height();
    if (!main_page_bk_image_.isNull()) {
        cx = std::max(kImageWidth, cx);

        static float main_img_scale = main_page_bk_image_.width() * 1.0f / main_page_bk_image_.height();
        int img_cy = cx / main_img_scale;
        cy += img_cy;
    } else {
        cy += title_size.height();
    }

    return gfx::Size(cx, cy);
}

void BililiveLoginMainView::Layout()
{
    title_bar_->SetBounds(0, 0, width(), title_bar_->GetPreferredSize().height());
    gfx::Size colt_size = info_collection_view_->GetPreferredSize();
    gfx::Size status_size = logining_status_view_->GetPreferredSize();

    info_collection_view_->SetBounds(0, height() - colt_size.height(), width(), colt_size.height());
    logining_status_view_->SetBounds(0, height() - colt_size.height(), width(), colt_size.height());
}

void BililiveLoginMainView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) {
    if (details.is_add && details.child == this) {
        InitViews();

        InitData();
    }
}

void BililiveLoginMainView::InitViews() {

    ui::ResourceBundle &rb = ui::ResourceBundle::GetSharedInstance();
    main_page_bk_image_ = *rb.GetImageSkiaNamed(IDR_LIVEHIME_LOGIN_BACKGROUND);

    title_bar_ = new views::View();
    {
        views::GridLayout *grid_layout = new views::GridLayout(title_bar_);
        title_bar_->SetLayoutManager(grid_layout);

        views::ColumnSet *columnset = grid_layout->AddColumnSet(0);
        columnset->AddPaddingColumn(0, GetLengthByDPIScale(10));
        columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
        columnset->AddPaddingColumn(1.0f, 0);
        columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

        version_label_ = new views::Label(BililiveContext::Current()->GetExecutableVersion());
        version_label_->SetAutoColorReadabilityEnabled(false);
        version_label_->SetEnabledColor(SK_ColorWHITE);
        version_label_->SetFont(ftFourteen);

        close_button_ = new views::ImageButton(this);
        close_button_->SetImageAlignment(views::ImageButton::HorizontalAlignment::ALIGN_CENTER, views::ImageButton::VerticalAlignment::ALIGN_MIDDLE);
        close_button_->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(IDR_LIVEHIME_LOGIN_CLOSE_BTN));
        close_button_->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(IDR_LIVEHIME_LOGIN_CLOSE_BTN_HV));
        close_button_->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(IDR_LIVEHIME_LOGIN_CLOSE_BTN_PRESSED));
        close_button_->SetTooltipText(rb.GetLocalizedString(IDS_UGC_CLOSE));
        close_button_->set_id(Button_Close);

        grid_layout->StartRow(0, 0);
        grid_layout->AddView(version_label_);
        grid_layout->AddView(close_button_);
    }

    AddChildView(title_bar_);

    info_collection_view_ = new LoginInfoCollectionView(this);
    AddChildView(info_collection_view_);

    logining_status_view_ = new LoginingStatusView(this);
    AddChildView(logining_status_view_);
    logining_status_view_->SetVisible(false);
}

void BililiveLoginMainView::InitData()
{
    auto info = GetBililiveProcess()->secret_core()->GetLastValidLoginInfo();
    if (info)
    {
        info_collection_view_->SetLoginInfo(*info);

        SetLoginUserFace(info->mid);

        // 如果是由OBS启动的那么如果记住了密码的就直接登录了
        // （这意味着如果是多账号的用户他就需要在进入到主界面之后以注销登录的形式来切换账户，似乎也问题不大）
        bool auto_login = info->auto_login;
        auto_login |= CommandLine::ForCurrentProcess()->HasSwitch(bililive::kSwitchOBSPluginStartLive);

        if (auto_login && !info->token.empty())
        {
            GoLogining();
        }
    }
}

void BililiveLoginMainView::SetLoginUserFace(int64 mid)
{
    logining_status_view_->SetUserFace(mid);
}

// ButtonListener
void BililiveLoginMainView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    switch (sender->id())
    {
    case Button_Close:
        GetWidget()->Close();
        break;
    default:
        break;
    }
}

// LoginInfoCollectionDelegate
void BililiveLoginMainView::OnInfoCollectionButtonPressed()
{
    GoLogining();

    if (info_collection_view_->is_token_verify())
    {
        auto& account_info = info_collection_view_->account_info();
        if (account_info)
        {
            presenter_->CheckAuthToken(*account_info);

            SetLoginUserFace(account_info->mid);
        }
        else
        {
            NOTREACHED() << "unexpected";
            return;
        }
    }
    else
    {
        string16 id = info_collection_view_->id_text();
        string16 pw = info_collection_view_->pw_text();
        int64 mid = info_collection_view_->mid();
        //TrimWhitespace(id, TRIM_ALL, &id);// 账号全都不能有空格，@树懒 说Trim这个事还是服务端做比较好
        //TrimWhitespace(pw, TRIM_ALL, &pw);// 密码双端和中间都可以有空格
        presenter_->RequestLogin(base::WideToUTF8(id), base::WideToUTF8(pw), nullptr);

        SetLoginUserFace(mid);
    }
}

void BililiveLoginMainView::OnInfoCollectionLinkClicked(int cid)
{
    if (LoginInfoCollectionView::LinkId_Register == cid)
    {
        presenter_->OpenRegisterLink();
    }
    else if (LoginInfoCollectionView::LinkId_Lost == cid)
    {
        presenter_->OpenPwLostLink();
    }
}

// LoginCaptchaDelegate
void BililiveLoginMainView::OnCaptchaVerifySuccessed(const secret::CaptchaInfo& captcha_info)
{
    string16 id = info_collection_view_->id_text();
    string16 pw = info_collection_view_->pw_text();

    presenter_->RequestLogin(base::WideToUTF8(id), base::WideToUTF8(pw), &captcha_info);
}

void BililiveLoginMainView::OnSecondaryVerifyValidateLogin(const secret::SecondaryVerifyInfo& verify_info)
{
    presenter_->RequestAccessToken(verify_info);
}

void BililiveLoginMainView::OnSecondaryVerifySuccessed()
{
    GetWidget()->Close();
}

void BililiveLoginMainView::OnCaptchaFrameClosed(bool captcha_verify_succeesed)
{
    if (!captcha_verify_succeesed)
    {
        // 验证码校验没过用户就关闭验证窗口了那么回退到初始UI状态
        OnLoginingStatusStop();

        OnLoginFailed();
    }
    else
    {
        // 验证码校验已经通过了，那么即使当前验证窗显示的是其他内容（比如修改密码、客服咨询等等）
        // 只要它关闭了就校验一下主界面的状态是不是已经把必要的登录流程接口都成功执行并通过了
        if (*result_code_ == LoginResultCode::LOGIN_SUCCESS)
        {
            GetWidget()->Close();
        }
    }
}

void BililiveLoginMainView::OnChangePasswordSuccessed()
{
    // 重置登录成功状态
    *result_code_ = LoginResultCode::UN_LOGIN;
    // 清空密码框文本
    info_collection_view_->ClearPassword();
    // 清除修改密码前已经拿到的登录态信息
    GetBililiveProcess()->SetTokenInvalid();
}

// LoginingStatusDelegate
void BililiveLoginMainView::OnLoginingStatusStop()
{
    info_collection_view_->SetVisible(true);
    logining_status_view_->SetVisible(false);

    SchedulePaint();
}

void BililiveLoginMainView::OnLoginingStatusMsgChanged(const string16& msg)
{
    ResizeWidget();
}

void BililiveLoginMainView::GoLogining() {
    info_collection_view_->SetVisible(false);
    logining_status_view_->SetVisible(true);
    logining_status_view_->ShowBeLogining();
}

void BililiveLoginMainView::OnGetAuthKeyError(bool valid_response, int code) {

    if (!valid_response)
    {
        LOG(WARNING) << "login get auth key failed.";
        LoginErrorDefault();
    }
    else
    {
        LOG(WARNING) << "login get auth key error.";
        LoginErrorDetails(code, "");
    }
}

void BililiveLoginMainView::OnLogin(
    bool valid_response,
    int code,
    const std::string& account,
    const secret::UserAccountService::LoginResponseInfo& info)
{
    bool success = valid_response && (code == secret::UserAccountService::ErrorCode::OK);
    if (success)
    {
        switch (info.status)
        {
        case secret::UserAccountService::ErrorLoginStatus::VerifySuccessed:
        {
            secret::LoginInfo login_info;
            login_info.name = account;
            login_info.mid = info.mid;
            login_info.token = info.token;
            login_info.refresh_token = info.refresh_token;
            login_info.expires = base::Time::Now().ToTimeT() + info.expires_in;
            login_info.auto_login = info_collection_view_->auto_login();
            login_info.cookies = info.cookies;
            login_info.domains = info.domains;

            presenter_->SaveLoginInfo(login_info, info_collection_view_->pw_remain());

            OnLoginSuccessed();

            *result_code_ = LoginResultCode::LOGIN_SUCCESS;
            GetWidget()->Close();
            return;
        }
        break;
        case secret::UserAccountService::ErrorLoginStatus::NeedSecondaryValidation:
        {
            LivehimeGeetestCaptchaView::ShowGeetestCaptcha(
                GetWidget(), info.url, LivehimeGeetestCaptchaView::VerifyStatus::SecondaryVerify, this);
            return;
        }
            break;
        case secret::UserAccountService::ErrorLoginStatus::NeedBindPhoneNumber:
        case secret::UserAccountService::ErrorLoginStatus::NeedVertifyDevice:
        default:
            NOTREACHED() << "底层应该拦下这种情况了的！";
            LoginErrorStatus(info.status);
            return;
            break;
        }
    }

    if (!valid_response) {
        LOG(WARNING) << "login a/p failed.";
        LoginErrorDefault();
    }
    else {
        LOG(WARNING) << "login a/p error, "
            "mid = " << info.mid << ", "
            "account = '" << account.c_str() << "'.";
        LoginErrorDetails(code, info.url);
    }
}

void BililiveLoginMainView::OnCheckAuthToken(bool valid_response, int code, const std::string& token)
{
    bool success = valid_response && (code == secret::UserAccountService::ErrorCode::OK);

    if (success)
    {
        auto& account_info = info_collection_view_->account_info();
        if (account_info)
        {
            account_info->auto_login = info_collection_view_->auto_login();
            presenter_->SaveLoginInfoAndRefreshToken(*account_info, info_collection_view_->pw_remain());
        }
        else
        {
            NOTREACHED() << "不应该会出现这种情况";
        }

        OnLoginSuccessed();

        *result_code_ = LoginResultCode::LOGIN_SUCCESS;
        GetWidget()->Close();
    }
    else if (!valid_response)
    {
        LOG(WARNING) << "login check token failed, code = " << code << ".";
        LoginErrorDefault();
    }
    else
    {
        LOG(WARNING) << "login check token error, "
            "code = " << code << ", "
            "account = '" << info_collection_view_->id_text().c_str() << "'. ";
            //"token = '" << token.c_str() << "'.";
        LoginErrorDetails(code, std::string());
    }
}

void BililiveLoginMainView::OnAccessToken(
    bool valid_response,
    int code,
    const std::string& account,
    const secret::UserAccountService::LoginResponseInfo& info,
    const secret::SecondaryVerifyInfo& verify_info)
{
    bool success = valid_response && (code == secret::UserAccountService::ErrorCode::OK);
    if (success)
    {
        secret::LoginInfo login_info;
        login_info.name = account;
        login_info.mid = info.mid;
        login_info.token = info.token;
        login_info.refresh_token = info.refresh_token;
        login_info.expires = base::Time::Now().ToTimeT() + info.expires_in;
        login_info.auto_login = info_collection_view_->auto_login();
        login_info.cookies = info.cookies;
        login_info.domains = info.domains;

        presenter_->SaveLoginInfo(login_info, info_collection_view_->pw_remain());

        OnLoginSuccessed();

        // 登录态已经拿完，登录完成了，判断一下二次验证窗是否还在，还在就允许用户继续耍一耍WEB，
        // 不在了就直接把登录框关了
        *result_code_ = LoginResultCode::LOGIN_SUCCESS;

        // 二次验证通过不要急着关闭登录窗，发消息到CEF，让其跳转到完成/修改密码提示页面
        if (!LivehimeGeetestCaptchaView::ShowSecondaryVerify(verify_info))
        {
            // 二次验证窗在接口返回的过程中被关掉了没关系，因为验证本身已经通过了，
            // 关掉就不需要提示用户账号异常了
            OnSecondaryVerifySuccessed();
        }

        return;
    }

    // 错误直接关闭二次验证框
    LivehimeGeetestCaptchaView::CloseVerifyWindow();

    if (!valid_response)
    {
        LOG(WARNING) << "login ticket token failed.";
        LoginErrorDefault();
    }
    else
    {
        LOG(WARNING) << "login ticket token error, "
            "mid = " << info.mid << ", "
            "account = '" << account.c_str() << "', "
            "ticket = '" << verify_info.ticket.c_str() << "'.";
        LoginErrorDetails(code, info.url);
    }
}

void BililiveLoginMainView::OnTokenExpired() {
    info_collection_view_->SetVisible(false);
    logining_status_view_->SetVisible(true);

    ResourceBundle &resource_handle = ResourceBundle::GetSharedInstance();
    logining_status_view_->ShowErrorMessage(resource_handle.GetLocalizedString(IDS_LOGIN_TOKEN_ERROR));

    info_collection_view_->TokenExpired();

    OnLoginFailed();
}

void BililiveLoginMainView::LoginErrorDefault() {
    info_collection_view_->SetVisible(false);
    logining_status_view_->SetVisible(true);

    ResourceBundle &resource_handle = ResourceBundle::GetSharedInstance();
    logining_status_view_->ShowErrorMessage(resource_handle.GetLocalizedString(IDS_LOGIN_ERROR_ELSE));

    LOG(WARNING) << "login failed. request is unavailable";

    OnLoginFailed();
}

void BililiveLoginMainView::LoginErrorDetails(int code, const std::string& url) {
    ResourceBundle &resource_handle = ResourceBundle::GetSharedInstance();

    LOG(WARNING) << "login error. error code = " << code;

    if (code == secret::UserAccountService::ErrorCode::CaptchaError) {
        LivehimeGeetestCaptchaView::ShowGeetestCaptcha(GetWidget(), url, LivehimeGeetestCaptchaView::VerifyStatus::Captcha, this);
    } else {
        info_collection_view_->SetVisible(false);
        logining_status_view_->SetVisible(true);

        if (code == secret::UserAccountService::ErrorCode::PasswordLeaked) {
            logining_status_view_->ShowErrorMessage(FormatErrorMsg(code, IDS_LOGIN_PASSWORD_LEAK));
            info_collection_view_->ClearPassword();
        } else if (code == secret::UserAccountService::ErrorCode::PasswordError) {
            logining_status_view_->ShowErrorMessage(FormatErrorMsg(code, IDS_LOGIN_PASSWORD_ERROR));
            info_collection_view_->ClearPassword();
        } else if (code == secret::UserAccountService::ErrorCode::AccessTokenNotFound ||
                   code == secret::UserAccountService::ErrorCode::AccessTokenExpired ||
                   code == secret::UserAccountService::ErrorCode::AccessKeyNotFound ||
                   code == secret::UserAccountService::ErrorCode::AccessKeyExpired) {
            logining_status_view_->ShowErrorMessage(FormatErrorMsg(code, IDS_LOGIN_TOKEN_ERROR));
            info_collection_view_->ClearPassword();
        } else if (code == secret::UserAccountService::ErrorCode::AbnormalAccount) {
            logining_status_view_->ShowErrorMessage(FormatErrorMsg(code, IDS_LOGIN_ABNORMAL_ACCOUNT));
        } else {
            logining_status_view_->ShowErrorMessage(FormatErrorMsg(code, IDS_LOGIN_ERROR));
        }

        OnLoginFailed();
    }
}

void BililiveLoginMainView::LoginErrorStatus(int status) {
    info_collection_view_->SetVisible(false);
    logining_status_view_->SetVisible(true);

    if (status == secret::UserAccountService::ErrorLoginStatus::NeedBindPhoneNumber) {
        logining_status_view_->ShowErrorMessage(GetLocalizedString(IDS_LOGIN_ERROR_STATUS_PHONE));
    } else if (status == secret::UserAccountService::ErrorLoginStatus::NeedVertifyDevice) {
        logining_status_view_->ShowErrorMessage(GetLocalizedString(IDS_LOGIN_ERROR_STATUS_DEVICE));
    } else {
        logining_status_view_->ShowErrorMessage(GetLocalizedString(IDS_LOGIN_ERROR_STATUS_DEFAULT));
    }

    OnLoginFailed();
}

void BililiveLoginMainView::ResizeWidget() {
    gfx::Size pref_size = GetWidget()->GetRootView()->GetPreferredSize();
    if (pref_size != size()) {
        GetWidget()->SetSize(pref_size);

        InvalidateLayout();
        Layout();
    }
    SchedulePaint();
}

void BililiveLoginMainView::OnLoginSuccessed()
{
    auto& info = GetBililiveProcess()->secret_core()->account_info();
    LOG(INFO) << "login successed, "
        "account = '" << info.account().c_str() << "', "
        "mid = " << info.mid() << ".";

    // 登录成功埋点
    livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::LoginResult,
        base::StringPrintf("tag_type:1;land_type:%d", info.auto_login() ? 1 : 2));
}

void BililiveLoginMainView::OnLoginFailed()
{
    auto& info = GetBililiveProcess()->secret_core()->account_info();
    // 登录失败埋点
    livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::LoginResult,
        base::StringPrintf("tag_type:2;land_type:%d", info.auto_login() ? 1 : 2));
}
