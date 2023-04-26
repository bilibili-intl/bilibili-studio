#ifndef BILILIVE_SECRET_PUBLIC_LOGIN_INFO_H_
#define BILILIVE_SECRET_PUBLIC_LOGIN_INFO_H_

#include <string>


namespace secret {

struct LoginInfo {

    std::string name;   // µÇÂ¼ÕËºÅ
    int64_t mid = 0;
    std::string token;
    std::string refresh_token;
    int64_t expires = 0;
    bool auto_login = false;
    std::string cookies;
    std::string domains;

    bool mini_login = false;
};

struct CaptchaInfo
{
    std::string challenge;
    std::string validate;
    std::string seccode;

    std::string captcha;
};

struct SecondaryVerifyInfo
{
    std::string ticket;
    std::string target_url;
    std::string callbackId;
};

}

#endif  // BILILIVE_SECRET_PUBLIC_LOGIN_INFO_H_