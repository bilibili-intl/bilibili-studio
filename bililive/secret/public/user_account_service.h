/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_PUBLIC_USER_ACCOUNT_SERVICE_H_
#define BILILIVE_SECRET_PUBLIC_USER_ACCOUNT_SERVICE_H_

#include <functional>
#include <string>
#include <vector>

#include "bililive/secret/public/request_connection_proxy.h"
#include "cef/cef_proxy_dll/public/livehime_cef_proxy_calldata.h"

namespace secret {

class UserAccountService {
public:
    enum ErrorCode : int {
        OK = 0,
        CaptchaError = -105,
        PasswordLeaked = -628,
        PasswordError = -629,
        AccessTokenNotFound = -901,
        AccessTokenExpired = -902,
        AbnormalAccount = -2100,
        AccessKeyNotFound = -2,
        AccessKeyExpired = -101
    };

    enum ErrorLoginStatus : int
    {
        VerifySuccessed = 0,
        NeedSecondaryValidation = 1,// 需要二次验证
        NeedBindPhoneNumber = 2,    // 需要绑定手机号
        NeedVertifyDevice = 3,       // 当前设备需要验证
    };

    virtual ~UserAccountService() {}

    // set cookie
    virtual void SetAuthCookie(const cef_proxy::cookies& cookie) = 0;

    struct LoginResponseInfo
    {
        int status = VerifySuccessed;
        int64_t mid = 0;
        std::string token;
        std::string refresh_token;
        int64_t expires_in = 0;
        std::string url;

        std::string cookies;
        std::string domains;
    };
    //virtual RequestProxy GetCaptcha(GetCaptchaHandler handler) = 0;

    struct UserInfoData {
        std::string nickname;
        std::string avatar_url;
        int is_nft = false;
        int64_t mid = 0;
    };

    struct ViddupUserInfoData {
        int64_t mid;
        std::string face;
        std::string uname;
        std::string sign;
        std::string birthday;
        std::string sex;
    };

    // 海外开播 - 用户信息相关
    using GetViddupUserInfoHandler =
        std::function<void(bool valid_response, int code, const ViddupUserInfoData& info_data)>;

    virtual RequestProxy GetViddupUserInfo(const std::string& token, GetViddupUserInfoHandler handler) = 0;

    using RequsetLogoutViddupHandler = std::function<void(bool success)>;

    virtual RequestProxy RequsetLogoutViddup(RequsetLogoutViddupHandler handler, const std::string& token) = 0;
    // - 海外开播 
    
    using AvatarData = std::vector<char>;

    struct UserAvatarInfo {
        AvatarData avatar;
        bool incomplete = false;
        std::string etag;
        int content_length = 0;
    };

    using GetUserAvatarHandler = std::function<void(bool success, const UserAvatarInfo& avatar_info)>;

    // If `etag` is not empty, the request first checks to see if cached avatar is still valid.
    virtual RequestProxy GetUserAvatar(const std::string& avatar_url,
                                       const std::string& etag,
                                       GetUserAvatarHandler handler) = 0;

};

}   // namespace secret

#endif  // BILILIVE_SECRET_PUBLIC_USER_ACCOUNT_SERVICE_H_
