/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_SERVICES_NO_SECURE_REQUESTS_H_
#define BILILIVE_SECRET_SERVICES_NO_SECURE_REQUESTS_H_

#include "base/basictypes.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/path_service.h"
#include "base/strings/string_util.h"
#include "base/threading/thread_restrictions.h"

#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/bililive_paths.h"

namespace secret {

// We have no intention to maintain this class in a long run, just make it header-only.
class NoSecureRequests {
public:
    static NoSecureRequests* GetInstance()
    {
        return Singleton<NoSecureRequests>::get();
    }

    bool ForceNoSecure() const
    {
        return force_no_secure_;
    }

private:
    NoSecureRequests()
        : force_no_secure_(false)
    {
        base::FilePath user_data_dir;
        if (!PathService::Get(bililive::DIR_USER_DATA, &user_data_dir)) {
            PLOG(WARNING) << "Failed to quuery user-data directory";
            return;
        }

        const wchar_t* kFileName = L"no secure";

        auto sentinel_path = user_data_dir.Append(bililive::kGlobalProfileDirName).Append(kFileName);

        base::ThreadRestrictions::ScopedAllowIO allow_io;
        if (base::PathExists(sentinel_path)) {
            force_no_secure_ = true;
        }
    }

    ~NoSecureRequests() = default;

    friend struct DefaultSingletonTraits<NoSecureRequests>;

    DISALLOW_COPY_AND_ASSIGN(NoSecureRequests);

private:
    bool force_no_secure_;
};

inline std::string AllowInsecure(const std::string& url)
{
    const char* kInsecureScheme = "http:";
    const char* kSecureScheme = "https:";

    if (!NoSecureRequests::GetInstance()->ForceNoSecure() ||
        StartsWithASCII(url, kInsecureScheme, true)) {
        return url;
    }

    auto insecure_url(url);
    ReplaceFirstSubstringAfterOffset(&insecure_url, 0, kSecureScheme, kInsecureScheme);

    return insecure_url;
}

}   // namespace secret

#endif  // BILILIVE_SECRET_SERVICES_NO_SECURE_REQUESTS_H_
