/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_SERVICES_USER_ACCOUNT_SERVICE_IMPL_H_
#define BILILIVE_SECRET_SERVICES_USER_ACCOUNT_SERVICE_IMPL_H_

#include "base/basictypes.h"

#include "bililive/secret/net/request_connection_manager.h"
#include "bililive/secret/public/user_account_service.h"

namespace secret {

class UserAccountServiceImpl : public UserAccountService {
public:
    explicit UserAccountServiceImpl(RequestConnectionManager* manager);

    ~UserAccountServiceImpl();

    // set cookie
    void SetAuthCookie(const cef_proxy::cookies& cookie) override;

    RequestProxy RequsetLogoutViddup(RequsetLogoutViddupHandler handler, const std::string& token) override;

    RequestProxy GetViddupUserInfo(const std::string& token, GetViddupUserInfoHandler handler) override;

    RequestProxy GetUserAvatar(const std::string& avatar_url,
                               const std::string& etag,
                               GetUserAvatarHandler handler) override;

private:
    RequestProxy GetViddupUserInfoByCookie(GetViddupUserInfoHandler handler);


private:
    DISALLOW_COPY_AND_ASSIGN(UserAccountServiceImpl);

private:
    RequestConnectionManager* conn_manager_;
};

}   // namespace secret

#endif  // BILILIVE_SECRET_SERVICES_USER_ACCOUNT_SERVICE_IMPL_H_
