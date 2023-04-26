#ifndef BILILIVE_PUBLIC_COMMON_BILILIVE_CONSTANTS_H_
#define BILILIVE_PUBLIC_COMMON_BILILIVE_CONSTANTS_H_

#include "base/files/file_path.h"

namespace bililive {

extern const char kBililiveIntl[];
extern const wchar_t kBililiveMainDll[];
extern const wchar_t kBililiveCefProxyDll[];
extern const wchar_t kBililiveOBSPluginDll[];
extern const char kBililiveVersionEnvVar[];
extern const char kBililiveMainDirectoryEnvVar[];
extern const base::FilePath::CharType kBililiveProcessExecutableName[];
extern const base::FilePath::CharType kBililiveUninsExecutableName[];
extern const base::FilePath::CharType kBililiveUninsDatName[];
extern const wchar_t kBililiveLogFileName[];
extern const wchar_t kBililiveLog1FileName[];
extern const wchar_t kBililiveLog2FileName[];
extern const wchar_t kBililiveLogRecord[];
extern const wchar_t kStatusTrayWindowClass[];
extern const wchar_t kBililiveResourcesDll[];
extern const wchar_t kUserDataDirname[];
extern const wchar_t kGlobalProfileDirName[];
extern const wchar_t kFirstRunSentinelName[];
extern const wchar_t kLiveReportDirname[];
extern const base::FilePath::CharType kPreferencesFilename[];
extern const char kHttpScheme[];
extern const char kHttpsScheme[];
extern const char kFileSystemScheme[];
extern const char kFileScheme[];
extern const char kFtpScheme[];
extern const char kLivehimeScheme[];
extern const char kStandardSchemeSeparator[];
extern const char kSwitchRelogin[];
extern const char kSwitchProcessType[];
extern const char kSwitchHandshakeHandle[];
extern const char kSwitchDebugConsole[];
extern const wchar_t kSwitchHotUpdate[];
extern const char kSwitchIncHotUpdate[];
extern const char kSwitchEnableFullMinidump[];
extern const char kSwitchHandleVectoredExceptions[];
extern const char kSwitchReportEvent[];
extern const char kSwitchReportTimeout[];
extern const char kSwitchLaunchUIExamples[];
extern const char kSwitchDanmakuRoomId[];
extern const char kSwitchIgnoreUpdate[];
extern const char kSwitchOBSPlugin[];
extern const char kSwitchOBSPluginStartLive[];
extern const char kSwitchCustomRtmp[];
extern const char kSwitchCustomEventLogId[];
extern const char kSwitchUseTestAgoraAppID[];
extern const char kProcessCrashHandler[];
extern const char kProcessReporter[];
extern const char kProcessScheme[];
extern const base::FilePath::CharType kCmdlineTempFileName[];
extern const base::FilePath::CharType kAppUpdateDirName[];
extern const base::FilePath::CharType kInstallerName[];
extern const base::FilePath::CharType kUpdatePublishListName[];
extern const base::FilePath::CharType kAppTempDirName[];
extern const wchar_t kUpdatedRunSentinelName[];
extern const wchar_t kOBSSourceDataUpgradeMarkName[];
extern const wchar_t kInjectOBSPluginsSentinelName[];
extern const int kIntervalOfSetChannel;
extern const wchar_t kUserFaceFileName[];
extern const char kFallbackCrashID[];
extern const int64 kMaxFileSizeAllowedToUpload;

extern const char kLogBoundary[];
extern const char kSwitchRestart[];

}   // namespace bililive

#endif  // BILILIVE_PUBLIC_COMMON_BILILIVE_CONSTANTS_H_
