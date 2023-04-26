#ifndef BILILIVE_BILILIVE_BILILIVE_MAIN_LOOP_H_
#define BILILIVE_BILILIVE_BILILIVE_MAIN_LOOP_H_

#include "base/basictypes.h"
#include "base/command_line.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/power_monitor/power_monitor.h"
#include "base/system_monitor/system_monitor.h"
#include "base/timer/hi_res_timer_manager.h"

#include "bililive/bililive/bililive_thread_impl.h"
#include "bililive/bililive/bililive_process_sub_thread.h"
#include "bililive/public/bililive/bililive_main_parts.h"

class BililiveMainLoop {
public:
    explicit BililiveMainLoop(const CommandLine& cmd_line);

    virtual ~BililiveMainLoop();

    // Must be called after main message loop has been started.
    static BililiveMainLoop *GetInstance();

    void Init();

    void EarlyInitialization();

    void InitializeToolkit();

    void MainMessageLoopStart();

    void CreateStartupTasks();

    void RunMainMessageLoopParts();

    void ShutdownThreadsAndCleanUp();

    int GetResultCode() const
    {
        return result_code_;
    }

    BililiveMainParts* main_parts() const
    {
        return parts_.get();
    }

private:
    DISALLOW_COPY_AND_ASSIGN(BililiveMainLoop);

    friend class BililiveShutdownImpl;

    void InitializeMainThread();

    int PreCreateThreads();

    int CreateThreads();

    int BililiveThreadsStarted();

    int PreMainMessageLoopRun();

    void MainMessageLoopRun();

private:
    const CommandLine& parsed_command_line_;
    int result_code_;
    bool created_threads_;
    scoped_ptr<base::MessageLoop> main_message_loop_;
    scoped_ptr<base::SystemMonitor> system_monitor_;
    scoped_ptr<base::PowerMonitor> power_monitor_;
    scoped_ptr<base::HighResolutionTimerManager> hi_res_timer_manager_;
    scoped_ptr<BililiveMainParts> parts_;
    scoped_ptr<BililiveThreadImpl> main_thread_;
    scoped_ptr<BililiveProcessSubThread> db_thread_;
    scoped_ptr<BililiveProcessSubThread> file_thread_;
    scoped_ptr<BililiveProcessSubThread> cache_thread_;
    scoped_ptr<BililiveProcessSubThread> io_thread_;
};

#endif  // BILILIVE_BILILIVE_BILILIVE_MAIN_LOOP_H_