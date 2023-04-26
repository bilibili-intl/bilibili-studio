#include <windows.h>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/string_number_conversions.h"

#include "breakpad/client/windows/handler/exception_handler.h"

#include "bililive/app/main_dll_loader.h"
#include "bililive/common/bililive_context.h"
#include "bililive/common/bililive_logging.h"
#include "bililive/install/run_as_reporter.h"
#include "bililive/public/common/bililive_constants.h"

namespace {

bool IsValidProcessType(const std::string& process_type)
{
    return process_type.empty() ||
        process_type == bililive::kProcessCrashHandler ||
        process_type == bililive::kProcessReporter ||
        process_type == bililive::kProcessScheme;
}

bool WaitForOldVerMainProcessExitIfNecessary()
{
    auto process_id_str = CommandLine::ForCurrentProcess()->GetSwitchValueASCII(bililive::kSwitchIncHotUpdate);
    if (!process_id_str.empty())
    {
        DWORD process_id = 0;
        if (base::StringToUint(process_id_str, (uint32*)&process_id))
        {
            HANDLE process_handle = ::OpenProcess(SYNCHRONIZE, false, process_id);
            if (process_handle)
            {
                // It doesn't have to wait forever. 10 seconds is the end of the line. The old version is still 
                // running and the new program is not running
                DWORD dwRet = ::WaitForSingleObject(process_handle, 10 * 1000);
                CloseHandle(process_handle);

                if (dwRet != WAIT_OBJECT_0)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

}   // namespace

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, wchar_t*, int)
{
    CommandLine::Init(0, nullptr);

    base::AtExitManager exit_manager;

    bililive::InitBililiveLogging();
    BililiveContext::Current()->Init();

    auto process_type = CommandLine::ForCurrentProcess()->GetSwitchValueASCII(bililive::kSwitchProcessType);
    DCHECK(IsValidProcessType(process_type));

    // Determine if it was started by an incrementally updated older version of the program. 
    // Only in this case will false be returned if you wait for the old process to timeout
    if (!WaitForOldVerMainProcessExitIfNecessary())
    {
        return 0;
    }

    if (process_type == bililive::kProcessReporter) {
        return bililive::RunAsReporter(*CommandLine::ForCurrentProcess());
    }

    scoped_ptr<MainDllLoader> loader(MakeMainDllLoader());
    int rc = loader->Launch(instance);

    CommandLine::Reset();
    return rc;
}