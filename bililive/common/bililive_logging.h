#ifndef BILILIVE_COMMON_BILILIVE_LOGGING_H_
#define BILILIVE_COMMON_BILILIVE_LOGGING_H_

#include "base/files/file_path.h"

namespace bililive {

// Get path like "C:\Users\<UserName>\AppData\Local\bililive"
base::FilePath GetBililiveDirectory();

// Get path like "C:\Users\<UserName>\AppData\Local\bililive\User Data"
base::FilePath GetBililiveUserDataDirectory();

// Keep consistent with `bililive::DIR_LOGS`.
base::FilePath GetBililiveLogDirectory();

// Get C:\Program Files\Common Files
base::FilePath GetFilesCommonDirectory();

void BindStandardDevices();

void RequestConsole();

bool ConsoleExists();

void InitBililiveLogging(bool debug_mode);

// Initializes logging by default mode.
// This function internally calls its cousin overload.
void InitBililiveLogging();
void InitCurrentBililiveLogging(const base::FilePath& Dirpath);
void InitRetainBililiveLog(const base::FilePath& dir_log);

int64 GetStartupLogFileOffset();
int64 GetStartupCEFLogFileOffset();
int64 GetCurdayLogFileOffset();
int64 GetCurdayCEFLogFileOffset();

std::string GetCurrentLogText();
std::string GetCurrentCEFLogText();
std::vector<std::string> GetCurrentDayLogText();
std::vector<std::string> GetCurrentUserLogText();
std::vector<std::string> GetCurrentDayCEFLogText();
}   // namespace bililive

#endif  // BILILIVE_COMMON_BILILIVE_LOGGING_H_