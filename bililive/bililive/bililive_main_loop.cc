#include "bililive/bililive/bililive_main_loop.h"

#include <CommCtrl.h>

#include "base/message_loop/message_loop.h"
#include "base/power_monitor/power_monitor_source.h"
#include "base/power_monitor/power_monitor_device_source.h"
#include "base/run_loop.h"
#include "base/threading/thread_restrictions.h"
#include "base/win/metro.h"
#include "ui/base/clipboard/clipboard.h"
#include "ui/base/l10n/l10n_util_win.h"
#include "ui/views/controls/messagebox/message_box.h"

#include "bililive/bililive/bililive_main_extra_parts_livehime_intl.h"
#include "bililive/bililive/bililive_main_parts_impl.h"
#include "bililive/bililive/livehime/cef/cef_proxy_wrapper.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/startup_task_runner.h"
#include "bililive/bililive/ui/views/bililive_main_extra_parts_views.h"
#include "bililive/common/bililive_context.h"
#include "bililive/common/bililive_result_codes.h"

BililiveMainLoop* g_current_bililive_main_loop = nullptr;

BililiveMainParts* CreateBililiveMainParts()
{
    BililiveMainParts* main_parts = new BililiveMainPartsImpl();
    main_parts->AddParts(new BililiveMainExtraPartsViews());
    return main_parts;
}

// static
BililiveMainLoop* BililiveMainLoop::GetInstance()
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));
    DCHECK(g_current_bililive_main_loop != nullptr);
    return g_current_bililive_main_loop;
}

BililiveMainLoop::BililiveMainLoop(const CommandLine& cmd_line)
    : parsed_command_line_(cmd_line),
      result_code_(bililive::ResultCodeNormalExit),
      created_threads_(false)
{
    DCHECK(!g_current_bililive_main_loop);
    g_current_bililive_main_loop = this;
}

BililiveMainLoop::~BililiveMainLoop()
{
    DCHECK_EQ(this, g_current_bililive_main_loop);

    ui::Clipboard::DestroyClipboardForCurrentThread();

    g_current_bililive_main_loop = nullptr;
}

void BililiveMainLoop::Init()
{
    parts_.reset(CreateBililiveMainParts());

    if (BililiveContext::Current()->InApplicationMode(ApplicationMode::BililiveLivehime)) {
        parts_->AddParts(new MainExtraPartsLivehimeIntl());
        CHECK(OBSProxyService::InitInstance(parts_.get())) << "Failed to bind obs proxy";
    }
}

void BililiveMainLoop::EarlyInitialization()
{
    if (parts_)
    {
        parts_->PreEarlyInitialization();
        parts_->PostEarlyInitialization();
    }
}

void BililiveMainLoop::MainMessageLoopStart()
{
    if (parts_)
    {
        parts_->PreMainMessageLoopStart();
    }

    l10n_util::OverrideLocaleWithUILanguageList();

    if (!base::MessageLoop::current())
    {
        main_message_loop_.reset(new base::MessageLoop(base::MessageLoop::TYPE_UI));
    }

    InitializeMainThread();

    // We don't need this in a normal build right now.
#if defined(EXTRA_TRACE)
    system_monitor_.reset(new base::SystemMonitor);

    {
        scoped_ptr<base::PowerMonitorSource> power_monitor_source(
            new base::PowerMonitorDeviceSource());
        power_monitor_.reset(new base::PowerMonitor(power_monitor_source.Pass()));
    }

    hi_res_timer_manager_.reset(new base::HighResolutionTimerManager);
#endif  // EXTRA_TRACE

    if (parts_)
    {
        parts_->PostMainMessageLoopStart();
    }
}

int BililiveMainLoop::PreCreateThreads()
{
    if (parts_)
    {
        result_code_ = parts_->PreCreateThreads();
    }

    return result_code_;
}

void BililiveMainLoop::CreateStartupTasks()
{
    scoped_refptr<StartupTaskRunner> task_runner (
        new StartupTaskRunner(false,
                              base::Callback<void(int)>(),
                              base::MessageLoop::current()->message_loop_proxy()));

    // If one task runs to failure, no further tasks will be run.
    StartupTask pre_create_threads =
        base::Bind(&BililiveMainLoop::PreCreateThreads, base::Unretained(this));
    task_runner->AddTask(pre_create_threads);

    StartupTask create_threads =
        base::Bind(&BililiveMainLoop::CreateThreads, base::Unretained(this));
    task_runner->AddTask(create_threads);

    StartupTask bililive_thread_started =
        base::Bind(&BililiveMainLoop::BililiveThreadsStarted, base::Unretained(this));
    task_runner->AddTask(bililive_thread_started);

    StartupTask pre_main_message_loop_run =
        base::Bind(&BililiveMainLoop::PreMainMessageLoopRun, base::Unretained(this));
    task_runner->AddTask(pre_main_message_loop_run);

    task_runner->StartRunningTasks();
}

int BililiveMainLoop::CreateThreads()
{
    base::Thread::Options default_options;
    base::Thread::Options io_message_loop_options;
    io_message_loop_options.message_loop_type = base::MessageLoop::TYPE_IO;
    base::Thread::Options ui_message_loop_options;
    ui_message_loop_options.message_loop_type = base::MessageLoop::TYPE_UI;

    for (size_t thread_id = BililiveThread::UI + 1;
         thread_id < BililiveThread::ID_COUNT;
         ++thread_id)
    {
        scoped_ptr<BililiveProcessSubThread> *thread_to_start = nullptr;
        base::Thread::Options *options = &default_options;

        switch (thread_id)
        {
        case BililiveThread::DB:
            thread_to_start = &db_thread_;
            break;
        case BililiveThread::FILE:
            thread_to_start = &file_thread_;
            options = &ui_message_loop_options;
            break;
        case BililiveThread::IO:
            thread_to_start = &io_thread_;
            options = &io_message_loop_options;
            break;
        case BililiveThread::UI:
        case BililiveThread::ID_COUNT:
        default:
            NOTREACHED();
            break;
        }

        BililiveThread::ID id = static_cast<BililiveThread::ID>(thread_id);

        if (thread_to_start)
        {
            (*thread_to_start).reset(new BililiveProcessSubThread(id));
            (*thread_to_start)->StartWithOptions(*options);
        }
        else
        {
            NOTREACHED();
        }
    }

    created_threads_ = true;
    return result_code_;
}

int BililiveMainLoop::PreMainMessageLoopRun()
{
    if (parts_)
    {
        parts_->PreMainMessageLoopRun();
    }

    // If the UI thread blocks, the whole UI is unresponsive.
    // Do not allow disk IO from the UI thread.
    base::ThreadRestrictions::SetIOAllowed(false);
    base::ThreadRestrictions::DisallowWaiting();
    return result_code_;
}

void BililiveMainLoop::RunMainMessageLoopParts()
{
    bool ran_main_loop = false;
    if (parts_)
    {
        ran_main_loop = parts_->MainMessageLoopRun(&result_code_);
    }

    if (!ran_main_loop)
    {
        MainMessageLoopRun();
    }
}

void BililiveMainLoop::ShutdownThreadsAndCleanUp()
{
    if (!created_threads_)
    {
        return;
    }

    // Teardown may start in PostMainMessageLoopRun, and during teardown we
    // need to be able to perform IO.
    base::ThreadRestrictions::SetIOAllowed(true);
    BililiveThread::PostTask(
        BililiveThread::IO, FROM_HERE,
        base::Bind(base::IgnoreResult(&base::ThreadRestrictions::SetIOAllowed),
                   true));

    if (parts_)
    {
        parts_->PostMainMessageLoopRun();
    }

    OBSProxyService::GetInstance().CleanupInstance();

    if (CefProxyWrapper::GetInstance())
    {
        CefProxyWrapper::GetInstance()->Shutdown();
    }

    for (size_t thread_id = BililiveThread::ID_COUNT - 1;
         thread_id >= (BililiveThread::UI + 1);
         --thread_id)
    {
        switch (thread_id)
        {
        case BililiveThread::DB:
            db_thread_.reset();
            break;
        case BililiveThread::FILE:
            file_thread_.reset();
            break;
        case BililiveThread::IO:
            io_thread_.reset();
            break;
        case BililiveThread::UI:
        case BililiveThread::ID_COUNT:
        default:
            NOTREACHED();
            break;
        }
    }

    BililiveThreadImpl::ShutdownThreadPool();

    if (parts_)
    {
        parts_->PostDestroyThreads();
    }
}

void BililiveMainLoop::InitializeMainThread()
{
    const char *kThreadName = "BililiveMain";
    base::PlatformThread::SetName(kThreadName);
    if (main_message_loop_)
    {
        main_message_loop_->set_thread_name(kThreadName);
    }

    main_thread_.reset(
        new BililiveThreadImpl(BililiveThread::UI, base::MessageLoop::current()));
}

int BililiveMainLoop::BililiveThreadsStarted()
{
    std::vector<base::PlatformThreadId> allowed_clipboard_threads;
    allowed_clipboard_threads.push_back(base::PlatformThread::CurrentId());
    allowed_clipboard_threads.push_back(file_thread_->thread_id());
    allowed_clipboard_threads.push_back(io_thread_->thread_id());
    ui::Clipboard::SetAllowedThreads(allowed_clipboard_threads);

    return result_code_;
}

void BililiveMainLoop::InitializeToolkit()
{
    // Init common control sex.
    INITCOMMONCONTROLSEX config;
    config.dwSize = sizeof(config);
    config.dwICC = ICC_WIN95_CLASSES;
    if (!InitCommonControlsEx(&config))
    {
        LOG_GETLASTERROR(FATAL);
    }

    if (parts_)
    {
        parts_->ToolkitInitialized();
    }
}

void BililiveMainLoop::MainMessageLoopRun()
{
    DCHECK_EQ(base::MessageLoop::TYPE_UI, base::MessageLoop::current()->type());
    base::RunLoop run_loop;
    run_loop.Run();
}