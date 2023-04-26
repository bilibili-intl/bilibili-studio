#include "startup_task_runner.h"
#include "base/bind.h"
#include "base/location.h"
#include "base/message_loop/message_loop.h"

StartupTaskRunner::StartupTaskRunner(
    bool may_start_asynchronously,
    base::Callback<void(int)> const startup_complete_callback,
    scoped_refptr<base::SingleThreadTaskRunner> proxy)
    : asynchronous_startup_(may_start_asynchronously),
      startup_complete_callback_(startup_complete_callback),
      proxy_(proxy)
{

}

void StartupTaskRunner::AddTask(StartupTask &callback)
{
    task_list_.push_back(callback);
}

void StartupTaskRunner::StartRunningTasks()
{
    DCHECK(proxy_);
    int result = 0;
    if (asynchronous_startup_ && !task_list_.empty())
    {
        const base::Closure next_task =
            base::Bind(&StartupTaskRunner::WrappedTask, this);
        proxy_->PostNonNestableTask(FROM_HERE, next_task);
    }
    else
    {
        for (auto it = task_list_.begin(); it != task_list_.end(); ++it)
        {
            result = it->Run();
            if (result > 0)
            {
                break;
            }
        }
        if (!startup_complete_callback_.is_null())
        {
            startup_complete_callback_.Run(result);
        }
    }
}

void StartupTaskRunner::WrappedTask()
{
    int result = task_list_.front().Run();
    task_list_.pop_front();
    if (result > 0 || task_list_.empty())
    {
        if (!startup_complete_callback_.is_null())
        {
            startup_complete_callback_.Run(result);
        }
    }
    else
    {
        const base::Closure next_task =
            base::Bind(&StartupTaskRunner::WrappedTask, this);
        proxy_->PostNonNestableTask(FROM_HERE, next_task);
    }
}

StartupTaskRunner::~StartupTaskRunner()
{

}