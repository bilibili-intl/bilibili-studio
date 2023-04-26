#include "bililive/bililive/bililive_main_runner.h"

#include <atlbase.h>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/i18n/icu_util.h"
#include "base/logging_win.h"
#include "base/memory/scoped_ptr.h"
#include "base/notification/notification_service_impl.h"
#include "base/process/memory.h"
#include "ui/base/win/scoped_ole_initializer.h"
#include "ui/base/ui_base_paths.h"

#include "bililive/bililive/bililive_main_loop.h"
#include "bililive/bililive/utils/gdiplus/bililive_gdiplus_utils.h"
#include "bililive/common/bililive_features.h"
#include "bililive/public/common/bililive_paths.h"


class BililiveMainRunnerImpl : public BililiveMainRunner {
public:
    BililiveMainRunnerImpl()
        : is_initialized_(false),
          is_shutdown_(false)
    {
        GUID guid;
        com_module_.Init(NULL, ::GetModuleHandle(NULL), &guid);
    }

    virtual ~BililiveMainRunnerImpl()
    {
        if (is_initialized_ && !is_shutdown_)
        {
            BililiveMainRunnerImpl::Shutdown();
        }
    }

    int Initialize(const CommandLine& cmd_line) override
    {
        is_initialized_ = true;
        exit_manager_.reset(new base::AtExitManager);

        // We don't need this in a normal build right now.
#if defined(EXTRA_TRACE)
        // {53F8D3EA-C1B4-4dd5-8E93-F9C0A39877EF}
        const GUID kTraceProviderName {
            0x53f8d3ea, 0xc1b4, 0x4dd5,
            { 0x8e, 0x93, 0xf9, 0xc0, 0xa3, 0x98, 0x77, 0xef }
        };

        logging::LogEventProvider::Initialize(kTraceProviderName);
#endif  // EXTRA_TRACE

        CHECK(icu_util::Initialize());

        bililive::RegisterPathProvider();
        ui::RegisterPathProvider();

        notification_service_.reset(new base::NotificationServiceImpl);

        ole_initializer_.reset(new ui::ScopedOleInitializer);

        BililiveFeatures::Init();

        main_loop_.reset(new BililiveMainLoop(cmd_line));
        main_loop_->Init();
        main_loop_->EarlyInitialization();
        main_loop_->InitializeToolkit();
        main_loop_->MainMessageLoopStart();
        main_loop_->CreateStartupTasks();

        return main_loop_->GetResultCode();
    }

    int Run() override
    {
        DCHECK(is_initialized_);
        DCHECK(!is_shutdown_);

        main_loop_->RunMainMessageLoopParts();

        return main_loop_->GetResultCode();
    }

    void Shutdown() override
    {
        DCHECK(is_initialized_);
        DCHECK(!is_shutdown_);

        main_loop_->ShutdownThreadsAndCleanUp();

        main_loop_.reset(nullptr);

        ole_initializer_.reset(nullptr);

        notification_service_.reset(nullptr);

        exit_manager_.reset(nullptr);

        is_shutdown_ = true;
    }

private:
    bool is_initialized_;
    bool is_shutdown_;

    scoped_ptr<base::NotificationServiceImpl> notification_service_;
    scoped_ptr<BililiveMainLoop> main_loop_;
    scoped_ptr<ui::ScopedOleInitializer> ole_initializer_;
    scoped_ptr<base::AtExitManager> exit_manager_;

    CGdiplusObject gdiplus_object;
    CComModule com_module_;

    DISALLOW_COPY_AND_ASSIGN(BililiveMainRunnerImpl);
};

// static
BililiveMainRunner* BililiveMainRunner::Create()
{
    return new BililiveMainRunnerImpl();
}
