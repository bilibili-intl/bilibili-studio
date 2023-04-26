#include <Windows.h>

#include "base/command_line.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/process/launch.h"
#include "base/process/memory.h"
#include "base/strings/string_util.h"
#include "base/time/time.h"

#include "bililive/bililive/bililive_main_runner.h"
#include "bililive/bililive/log_report/app_integrity_checking.h"
#include "bililive/common/bililive_context.h"
#include "bililive/common/bililive_logging.h"
#include "bililive/common/bililive_result_codes.h"
#include "bililive/public/common/bililive_constants.h"

#define DLLEXPORT __declspec(dllexport)

extern "C" DLLEXPORT int __cdecl BililiveMain(HINSTANCE instance);

namespace {

void RestartApp(const std::string& switch_string)
{
    base::LaunchOptions launch_options;

    CommandLine command_line(CommandLine::ForCurrentProcess()->GetProgram());
    command_line.AppendSwitch(switch_string);

    base::LaunchProcess(command_line.GetCommandLineString(), launch_options, nullptr);
}

void LogApplicationStartup()
{
    const char kStartupTag[] = "--- Application Startup ---";
    LOG(INFO) << kStartupTag;

    if (CommandLine::ForCurrentProcess()->HasSwitch(bililive::kSwitchOBSPluginStartLive))
    {
        LOG(INFO) << "startup by obs.";
    }
}

void LogApplicationExit(int result_code)
{
    const char kNormalExitTag[] = "--- Application Exit ---";
    LOG(INFO) << kNormalExitTag << "\nExit result code: " << result_code;
}

void AppIntegrityChecking()
{
    auto missing_files = livehime::FilesIntegrityChecking();
    if (!missing_files.empty())
    {
        std::string str = JoinString(missing_files, "\n");
        LOG(ERROR) << "Missing some required files:\n"
            << str;
    }
}

}   // namespace

DLLEXPORT int __cdecl BililiveMain(HINSTANCE instance)
{
    BililiveContext::Current()->ApplaunchStartTime();

    base::EnableTerminationOnHeapCorruption();
    base::EnableTerminationOnOutOfMemory();
    base::Time::EnableHighResolutionTimer(true);
    base::Time::ActivateHighResolutionTimer(true);

    int argc = 0;
    char **argv = nullptr;
    CommandLine::Init(argc, argv);

#if defined(NDEBUG)
    bool enable_debug_logging = false;
    if (CommandLine::ForCurrentProcess()->HasSwitch(bililive::kSwitchDebugConsole)) {
        enable_debug_logging = true;
    }
#else   // NDEBUG
    bool enable_debug_logging = true;
#endif  // NDEBUG

    bililive::InitBililiveLogging(enable_debug_logging);
    BililiveContext::Current()->Init();

    LogApplicationStartup();
    AppIntegrityChecking();

    if (CommandLine::ForCurrentProcess()->HasSwitch(bililive::kSwitchDebugConsole)) {
        if (!bililive::ConsoleExists()) {
            bililive::RequestConsole();
        }

        bililive::BindStandardDevices();
    }

    if (CommandLine::ForCurrentProcess()->HasSwitch(bililive::kSwitchCustomRtmp)) {
        CommandLine::StringVector args = CommandLine::ForCurrentProcess()->GetArgs();
        std::wstring rtmp;
        if (!args.empty())
        {
            auto iter = args.begin();
            rtmp = *iter;
        }
        std::wstring info = L"Push address£º" + rtmp;
        info += L"\nRemember to set your account to studio mode";
        MessageBox(NULL, info.c_str(), L"Customize the push mode", MB_OK | MB_ICONINFORMATION);
    }

    scoped_ptr<BililiveMainRunner> main_runner(BililiveMainRunner::Create());

    // If critical failures happened, like we couldn't even create worker threads, exit before
    // running into message loop.
    int result_code = main_runner->Initialize(*CommandLine::ForCurrentProcess());
    if (result_code >= bililive::ResultCodeErrorOccurred) {
        return result_code;
    }

    result_code = main_runner->Run();

    main_runner->Shutdown();

    LogApplicationExit(result_code);

    if (result_code == bililive::ResultCodeSignOut) {
        RestartApp(bililive::kSwitchRelogin);
    }
    else if (result_code == bililive::ResultCodeRestart) {
        RestartApp(bililive::kSwitchRestart);
    }

    return result_code;
}