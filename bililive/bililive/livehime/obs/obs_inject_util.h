#ifndef BILILIVE_BILILIVE_LIVEHIME_OBS_INJECT_H_
#define BILILIVE_BILILIVE_LIVEHIME_OBS_INJECT_H_

#include "base/files/file_path.h"


namespace livehime
{
    base::FilePath GetObsInjectFlagPath();

    void CreateOrRemoveObsInjectFlag(bool create);
}

#endif  // BILILIVE_BILILIVE_LIVEHIME_OBS_COLIVE_STATUS_H_