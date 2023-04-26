// livehime_dev_tool.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "livehime_dev_tool.h"

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/i18n/icu_util.h"
#include "base/memory/scoped_ptr.h"
#include "base/process/memory.h"
#include "base/strings/string_number_conversions.h"
#include "base/time/time.h"

#include "ui/base/ui_base_paths.h"
#include "ui/base/resource/resource_bundle.h"

#include "streaming_qmcs/streaming_qmcs_window.h"

namespace
{
    void Initialize()
    {
        ::OleInitialize(NULL);

        base::FilePath exe;
        PathService::Get(base::FILE_EXE, &exe);
        base::FilePath log_filename = exe.ReplaceExtension(FILE_PATH_LITERAL("log"));
        logging::LoggingSettings settings;
        settings.logging_dest = logging::LOG_TO_ALL;
        settings.log_file = log_filename.value().c_str();
        settings.delete_old = logging::DELETE_OLD_LOG_FILE;
        logging::InitLogging(settings);
        logging::SetLogItems(true, true, true, true);

        base::Time::EnableHighResolutionTimer(true);

        icu_util::Initialize();

        ui::RegisterPathProvider();
        ui::ResourceBundle::InitSharedInstanceWithLocale("en-US", NULL);
    }

    void Shutdown()
    {
        ::OleUninitialize();
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    base::EnableTerminationOnHeapCorruption();
    base::EnableTerminationOnOutOfMemory();

    CommandLine::Init(0, nullptr);

    base::AtExitManager exit_manager;

    base::Time::EnableHighResolutionTimer(true);
    base::Time::ActivateHighResolutionTimer(true);

    Initialize();

    base::MessageLoopForUI message_loop;

    livehime::streaming_qmcs::ShowStreamingQMCSWindow();

    base::RunLoop run_loop;
    run_loop.Run();

    Shutdown();

    CommandLine::Reset();

    return 0;
}

