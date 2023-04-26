#include "bililive/bililive/first_run/first_run.h"

#include <fstream>

#include "base/bind.h"
#include "base/files/file_path.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/path_service.h"

#include "bililive/bililive/first_run/first_run_tasks.h"
#include "bililive/bililive/startup_task_runner.h"
#include "bililive/common/bililive_context.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/bililive_paths.h"

namespace {

void CreateFirstRunSentinel(const base::FilePath& sentinel)
{
    if (sentinel.empty()) {
        return;
    }

    std::ofstream(sentinel.value());
}

bool FirstRunSentinelExists(const base::FilePath& sentinel)
{
    return sentinel.empty() ? false : base::PathExists(sentinel);
}

void SetTaskComplete(bool* completion_flag, int result_code)
{
    *completion_flag = result_code == 0 ? true : false;
}

}   // namespace

namespace bililive {

base::FilePath GetFirstRunSentinelPath()
{
    base::FilePath user_data_dir;
    bool rv = PathService::Get(bililive::DIR_USER_DATA, &user_data_dir);
    if (!rv) {
        NOTREACHED() << "Failed to acquire user data directory";
        return base::FilePath();
    }

    return user_data_dir.Append(bililive::kGlobalProfileDirName).Append(bililive::kFirstRunSentinelName);
}

bool IsBililiveFirstRun()
{
    // Looks like fine now, maybe we need a --force-first-run switch in the future.
    return !FirstRunSentinelExists(GetFirstRunSentinelPath());
}

bool StartFirstRunTasks()
{
    bool completed = false;

    // Force executing in sequence.
    scoped_refptr<StartupTaskRunner> task_runner(new StartupTaskRunner(
                                                        false,
                                                        base::Bind(&SetTaskComplete, &completed),
                                                        base::MessageLoopProxy::current()));

    if (BililiveContext::Current()->InApplicationMode(ApplicationMode::BililiveLivehime))
    {
        auto validate_d3d_component = base::Bind(&ValidateD3DComponent);
        task_runner->AddTask(validate_d3d_component);
    }

    task_runner->StartRunningTasks();

    return completed;
}

void MarkFirstRunPassed()
{
    CreateFirstRunSentinel(GetFirstRunSentinelPath());
}

}   // namespace bililive