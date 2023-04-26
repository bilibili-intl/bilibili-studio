#ifndef BILILIVE_BILILIVE_FIRST_RUN_FIRST_RUN_H_
#define BILILIVE_BILILIVE_FIRST_RUN_FIRST_RUN_H_

#include "base/files/file_path.h"

namespace bililive {

base::FilePath GetFirstRunSentinelPath();

bool IsBililiveFirstRun();

bool StartFirstRunTasks();

void MarkFirstRunPassed();

}   // namespace bililive

#endif  // BILILIVE_BILILIVE_FIRST_RUN_FIRST_RUN_H_