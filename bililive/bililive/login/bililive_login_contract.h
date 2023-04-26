#ifndef BILILIVE_SECRET_LOGIN_BILILIVE_LOGIN_CONTRACT_H_
#define BILILIVE_SECRET_LOGIN_BILILIVE_LOGIN_CONTRACT_H_

#include "bililive/secret/public/login_info.h"
#include "bililive/secret/public/user_account_service.h"


enum class LoginResultCode : int {
    UN_LOGIN,
    LOGIN_SUCCESS,
};


namespace contracts {

class BililiveLoginPresenter {
public:
    virtual ~BililiveLoginPresenter() {}

    virtual void OpenRegisterLink() = 0;

    virtual void OpenPwLostLink() = 0;

    virtual void CheckAuthToken(const secret::LoginInfo& info) = 0;

    virtual void SaveLoginInfo(const secret::LoginInfo& info, bool save_to_profile) = 0;

    virtual void SaveLoginInfoAndRefreshToken(const secret::LoginInfo& info, bool save_to_profile) = 0;

    virtual void RequestLogin(const std::string& account, const std::string& password,
                              const secret::CaptchaInfo* captcha_info) = 0;

    virtual void RequestAccessToken(const secret::SecondaryVerifyInfo& verify_info) = 0;
};

class BililiveLoginMainView {
public:
    virtual ~BililiveLoginMainView() {}

    virtual void OnCheckAuthToken(bool valid_response, int code, const std::string& token) = 0;

    virtual void OnGetAuthKeyError(bool valid_response, int code) = 0;

    virtual void OnLogin(
        bool valid_response,
        int code,
        const std::string& account,
        const secret::UserAccountService::LoginResponseInfo& info) = 0;

    virtual void OnAccessToken(
        bool valid_response,
        int code,
        const std::string& account,
        const secret::UserAccountService::LoginResponseInfo& info,
        const secret::SecondaryVerifyInfo& verify_info) = 0;

    virtual void OnTokenExpired() = 0;
};

}   // namespace contracts

#endif  // BILILIVE_SECRET_LOGIN_BILILIVE_LOGIN_CONTRACT_H_