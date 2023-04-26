/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_PUBLIC_ACCOUNT_INFO_H_
#define BILILIVE_SECRET_PUBLIC_ACCOUNT_INFO_H_

#include <string>

#include "bilibase/basic_macros.h"

namespace secret {

class AccountInfo {
public:
    AccountInfo(int64_t mid, const std::string& account, const std::string& token,
                const std::string& refresh_token, int64_t expires, bool auto_login,
                const std::string& cookies, const std::string& domains,
                bool save_to_profile, bool webcookie)
        : mid_(mid),
          account_(account),
          token_(token),
          refresh_token_(refresh_token),
          token_expires_(expires),
          auto_login_(auto_login),
          cookies_(cookies),
          domains_(domains),
          persisted_(save_to_profile),
          is_webcookie_(webcookie)
    {
    }

    AccountInfo() = default;

    ~AccountInfo() = default;

    DISABLE_COPY(AccountInfo);

    int64_t mid() const
    {
        return mid_;
    }

    const std::string& account() const
    {
        return account_;
    }

    const std::string& access_token() const
    {
        return token_;
    }

    const std::string& refresh_token() const
    {
        return refresh_token_;
    }

    const std::string& cookies() const
    {
        return cookies_;
    }

    const std::string& domains() const
    {
        return domains_;
    }

    // Returns unix-timestamp of the token expire date.
    int64_t token_expires() const
    {
        return token_expires_;
    }

    bool auto_login() const
    {
        return auto_login_;
    }

    bool mini_login() const
    {
        return is_webcookie_;
    }

    // Returns true, if some authentication information should be persisted on local storage.
    bool persisted() const
    {
        return persisted_;
    }

private:
    int64_t mid_ = -1;
    std::string account_;
    std::string token_;
    bool is_webcookie_ = false;
    std::string refresh_token_;
    int64_t token_expires_ = 0;
    bool auto_login_ = false;
    std::string cookies_;
    std::string domains_;

    bool persisted_ = false;
};

}   // namespace secret

#endif  // BILILIVE_SECRET_PUBLIC_ACCOUNT_INFO_H_
