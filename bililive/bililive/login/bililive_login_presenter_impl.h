#ifndef BILILIVE_SECRET_LOGIN_BILILIVE_LOGIN_PRESENTER_IMPL_H_
#define BILILIVE_SECRET_LOGIN_BILILIVE_LOGIN_PRESENTER_IMPL_H_

#include <memory>

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"

#include "bililive/bililive/login/bililive_login_contract.h"


class BililiveLoginPresenterImpl
    : public contracts::BililiveLoginPresenter
{
public:
    explicit BililiveLoginPresenterImpl(contracts::BililiveLoginMainView* view);

    ~BililiveLoginPresenterImpl() = default;

    void OpenRegisterLink() override;

    void OpenPwLostLink() override;

    void CheckAuthToken(const secret::LoginInfo& info) override;

    void SaveLoginInfoAndRefreshToken(const secret::LoginInfo& info, bool save_to_profile) override;

    void SaveLoginInfo(const secret::LoginInfo& info, bool save_to_profile) override;

    void RequestLogin(const std::string& account, const std::string& password,
                      const secret::CaptchaInfo* captcha_info) override;

    void RequestAccessToken(const secret::SecondaryVerifyInfo& verify_info) override;

private:
    void OnCheckAuthToken(bool valid_response, int code, 
        int64_t mid, int expires_in, const std::string& token);

    void RefreshAuthToken(const std::string& token, const std::string& refresh_token);

    void OnGetAuthKey(bool valid_response, int code, const std::string& hash, const std::string& key);

    void OnLogin(
        bool valid_response, int code,
        const secret::UserAccountService::LoginResponseInfo& info);

    void OnAccessToken(
        bool valid_response, int code,
        const secret::UserAccountService::LoginResponseInfo& info);

private:
    contracts::BililiveLoginMainView* view_;
    base::WeakPtrFactory<BililiveLoginPresenterImpl> weak_ptr_factory_;
    std::string account_;
    std::string password_;
    std::unique_ptr<secret::CaptchaInfo> captcha_info_;
    secret::SecondaryVerifyInfo secondary_verify_info_;

    DISALLOW_COPY_AND_ASSIGN(BililiveLoginPresenterImpl);
};

#endif  // BILILIVE_SECRET_LOGIN_BILILIVE_LOGIN_PRESENTER_IMPL_H_