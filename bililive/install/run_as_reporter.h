/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_INSTALL_RUN_AS_REPORTER_H_
#define BILILIVE_INSTALL_RUN_AS_REPORTER_H_

#include "base/command_line.h"

namespace bililive {

// Reporter mode commandline: --type=reporter --event={install, uninstall} [--timeout=seconds]
// Reporter handles install and upgrade as different events internally itself, the invoker
// doesn't bother doing it.

int RunAsReporter(const CommandLine& cmdline_for_process);

}   // namespace bililive

#endif  // BILILIVE_INSTALL_RUN_AS_REPORTER_H_
