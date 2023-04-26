#include "bililive/bililive/login/bililive_login_presenter_impl.h"

#include <shellapi.h>

#include "base/ext/callable_callback.h"
#include "base/ext/count_down_latch.h"

#include "bilibase/scope_guard.h"

#include "bililive/bililive/livehime/cef/cef_proxy_wrapper.h"
#include "bililive/common/bililive_features.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/common/secret_pref_names.h"
#include "bililive/bililive/utils/fast_forward_url_convert.h"

namespace {

const int invalid_time = 15 * 86400; //15days

void SaveLoginInfoToSecretCore(const secret::LoginInfo& info, bool save_to_profile)
{
    GetBililiveProcess()->secret_core()->SaveAccountInfo(
        info.auto_login,
        info.mid,
        info.name,
        info.token,
        info.refresh_token,
        info.expires,
        info.cookies,
        info.domains,
        save_to_profile,
        info.mini_login);

    if (!info.mini_login)
    {
        // 登录成功，将登录态设置到CEF cookie中
        CefProxyWrapper::GetInstance()->SetTokenCookies();
    }
}

};

BililiveLoginPresenterImpl::BililiveLoginPresenterImpl(
    contracts::BililiveLoginMainView* view) : weak_ptr_factory_(this) {
    view_ = view;
}

void BililiveLoginPresenterImpl::OpenRegisterLink() {
   
}

void BililiveLoginPresenterImpl::OpenPwLostLink() {
    
}

void BililiveLoginPresenterImpl::CheckAuthToken(const secret::LoginInfo& info)
{
    int64_t current = base::Time::Now().ToTimeT();

    if (info.expires <= current)
    {
        view_->OnTokenExpired();
    }
    else
    {

    }
}

void BililiveLoginPresenterImpl::OnCheckAuthToken(
    bool valid_response, int code, int64_t mid, int expires_in, const std::string& token)
{
    view_->OnCheckAuthToken(valid_response, code, token);
}

void BililiveLoginPresenterImpl::SaveLoginInfo(const secret::LoginInfo& info, bool save_to_profile)
{
    SaveLoginInfoToSecretCore(info, save_to_profile);
}

void BililiveLoginPresenterImpl::SaveLoginInfoAndRefreshToken(const secret::LoginInfo& info, bool save_to_profile)
{
    SaveLoginInfo(info, save_to_profile);

    int64_t current = base::Time::Now().ToTimeT();

    // v3.5开始本地要缓存cookie信息，先前版本只记录token没记录cookie的就刷新一下token
    if ((info.expires - current < invalid_time) ||
        info.cookies.empty() ||
        info.domains.empty())
    {
        RefreshAuthToken(info.token, info.refresh_token);
    }
}

void BililiveLoginPresenterImpl::RefreshAuthToken(
    const std::string& token, const std::string& refresh_token)
{
    base::CountdownLatch list_ready(1);
   
    list_ready.Wait();
}

void BililiveLoginPresenterImpl::RequestLogin(const std::string& account, const std::string& password,
                                              const secret::CaptchaInfo* captcha_info)
{
    account_ = account;
    password_ = password;
    if (captcha_info)
    {
        captcha_info_ = std::make_unique<secret::CaptchaInfo>(*captcha_info);
    }
    else
    {
        captcha_info_.reset();
    }

}

void BililiveLoginPresenterImpl::RequestAccessToken(const secret::SecondaryVerifyInfo& verify_info)
{
    secondary_verify_info_ = verify_info;

}

void BililiveLoginPresenterImpl::OnGetAuthKey(
    bool valid_response,
    int code,
    const std::string& hash,
    const std::string& key) {
    if (valid_response && code == 0) {
       
    } else {
        view_->OnGetAuthKeyError(valid_response, code);
    }
}

void BililiveLoginPresenterImpl::OnLogin(bool valid_response, int code,
                                         const secret::UserAccountService::LoginResponseInfo& info) {
    view_->OnLogin(valid_response, code, account_, info);
}

void BililiveLoginPresenterImpl::OnAccessToken(bool valid_response, int code,
                                               const secret::UserAccountService::LoginResponseInfo& info)
{
    view_->OnAccessToken(valid_response, code, account_, info, secondary_verify_info_);
}
