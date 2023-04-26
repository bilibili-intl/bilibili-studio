#include "startup_bililive_creator.h"

#include <algorithm>   // For max().

#include "base/command_line.h"

#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/common/bililive_context.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"

StartupBililiveCreator::StartupBililiveCreator()
{
}

StartupBililiveCreator::~StartupBililiveCreator()
{

}

// static
bool StartupBililiveCreator::ProcessCmdLineImpl(
    const CommandLine &command_line,
    const base::FilePath &cur_dir,
    bool process_startup,
    int *return_code,
    StartupBililiveCreator *bililive_creator)
{
    if (!bililive_creator->LaunchBililive(command_line,
                                          cur_dir,
                                          return_code))
    {
        return false;
    }

    return true;
}

bool StartupBililiveCreator::LaunchBililive(const CommandLine &command_line, const base::FilePath &cur_dir, int *return_code)
{
    scoped_refptr<BililiveOBS> bililive(new BililiveOBS());
    GetBililiveProcess()->set_bililive_obs(bililive);

    bililive->Init();

    return true;
}

bool StartupBililiveCreator::LaunchUploader(const CommandLine &command_line,
    const base::FilePath &cur_dir,
    int *return_code)
{
    /*BililiveUploader *uploader = new BililiveUploader();
    GetBililiveProcess()->set_bililive_uploader(uploader);

    uploader->Init();*/

    return true;
}