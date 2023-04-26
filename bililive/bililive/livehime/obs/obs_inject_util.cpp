#include "obs_inject_util.h"

#include "base/file_util.h"
#include "base/path_service.h"
#include "base/threading/thread_restrictions.h"

#include "bililive/public/common/bililive_paths.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/secret/public/event_tracking_service.h"


namespace livehime
{
    base::FilePath GetObsInjectFlagPath()
    {
        base::FilePath path;
        base::FilePath user_data_dir;
        bool rv = PathService::Get(bililive::DIR_USER_DATA, &user_data_dir);
        if (!rv)
        {
            NOTREACHED() << "Failed to acquire user data directory";
            return base::FilePath();
        }

        return user_data_dir.Append(bililive::kGlobalProfileDirName).Append(bililive::kInjectOBSPluginsSentinelName);
    }

    void CreateOrRemoveObsInjectFlag(bool create)
    {
        base::ThreadRestrictions::ScopedAllowIO allow;
        base::FilePath flag = GetObsInjectFlagPath();
        if (!flag.empty())
        {
            if (create)
            {
                file_util::WriteFile(flag, "", 0);
            }
            else
            {
                base::DeleteFile(flag, false);
            }
        }
    }
}
