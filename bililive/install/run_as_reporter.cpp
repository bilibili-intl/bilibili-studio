/*
 @ 0xCCCCCCCC
*/

#include "bililive/install/run_as_reporter.h"

#include "base/logging.h"

#include "bililive/install/event_tracking/simple_event_reporter.h"
#include "bililive/public/common/bililive_constants.h"

namespace {

const char kEventInstall[] = "install";
const char kEventUninstall[] = "uninstall";

}   // namespace

namespace bililive {

int RunAsReporter(const CommandLine& cmdline_for_process)
{
    auto report_event = cmdline_for_process.GetSwitchValueASCII(kSwitchReportEvent);
    if (report_event.empty()) {
        LOG(ERROR) << "Empty report event switch!";
        return 0;
    }

    // Install event may aslo implies an upgrade event, in which case, we want to retain our
    // credential file whenever possible.
    if (report_event == kEventInstall) {
        if (!EventReporter::GetInstance().RestoreCredential()) {
            EventReporter::GetInstance().InitializeCredential();
        }

        EventReporter::GetInstance().Report(EventReporter::Install);
    } else if (report_event == kEventUninstall) {
        EventReporter::GetInstance().RestoreCredential();
        EventReporter::GetInstance().Report(EventReporter::Uninstall);
    }

    return 0;
}

}   // namespace bililive
