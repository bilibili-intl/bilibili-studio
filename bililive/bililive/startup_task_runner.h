#ifndef BILILIVE_BILILIVE_STARTUP_TASK_RUNNER_H
#define BILILIVE_BILILIVE_STARTUP_TASK_RUNNER_H

#include <list>

#include "base/callback.h"
#include "base/memory/ref_counted.h"
#include "base/single_thread_task_runner.h"

// A return value of > 0 indicates a failure, and that no further startup tasks should be run.
using StartupTask = base::Callback<int()> ;

class StartupTaskRunner
    : public base::RefCounted<StartupTaskRunner>
{

public:
    StartupTaskRunner(bool may_start_asynchronously,
                      base::Callback<void(int)> startup_complete_callback,
                      scoped_refptr<base::SingleThreadTaskRunner> proxy);

    void AddTask(StartupTask &callback);

    void StartRunningTasks();

private:
    friend class base::RefCounted<StartupTaskRunner>;
    virtual ~StartupTaskRunner();

    std::list<StartupTask> task_list_;
    void WrappedTask();

    const bool asynchronous_startup_;
    base::Callback<void(int)> startup_complete_callback_;
    scoped_refptr<base::SingleThreadTaskRunner> proxy_;

    DISALLOW_COPY_AND_ASSIGN(StartupTaskRunner);
};

#endif