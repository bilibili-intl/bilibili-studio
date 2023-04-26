#ifndef BILILIVE_COMMON_BILILIVE_PATHS_INTERNAL_H
#define BILILIVE_COMMON_BILILIVE_PATHS_INTERNAL_H


#include <string>

#include "base/files/file_path.h"
#include "build/build_config.h"


namespace base
{
    class FilePath;
}

namespace bililive
{
    bool GetDefaultUserDataDirectory(base::FilePath *result);
    void GetUserCacheDirectory(const base::FilePath &profile_dir, base::FilePath *result);
    bool GetUserDocumentsDirectory(base::FilePath *result);
    bool GetUserDownloadsDirectorySafe(base::FilePath *result);
    bool GetUserDownloadsDirectory(base::FilePath *result);
    bool GetUserMusicDirectory(base::FilePath *result);
    bool GetUserPicturesDirectory(base::FilePath *result);
    bool GetUserVideosDirectory(base::FilePath *result);
    bool ProcessNeedsProfileDir(const std::string &process_type);

    bool GetBililiveAccountConfigDirectory(const std::string &account_name,
                                           base::FilePath *result);

    base::FilePath QueryInstalledDirectoryFromRegistry();

    std::string  GetSystemBits();
}

#endif