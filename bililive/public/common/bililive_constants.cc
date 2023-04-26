#include "bililive/public/common/bililive_constants.h"

namespace bililive {

const char kBililiveIntl[] = "Bilibili Studio";
const wchar_t kBililiveMainDll[] = L"bililive.dll";
const wchar_t kBililiveCefProxyDll[] = L"cef_proxy.dll";
const wchar_t kBililiveOBSPluginDll[] = L"bililive_obs_plugin.dll";
const char kBililiveVersionEnvVar[] = "BILILIVE_VERSION";
const char kBililiveMainDirectoryEnvVar[] = "BILILIVE_MAIN_DIR";
const base::FilePath::CharType kBililiveProcessExecutableName[] = FILE_PATH_LITERAL("Bilibili Studio.exe");
const base::FilePath::CharType kBililiveUninsExecutableName[] = FILE_PATH_LITERAL("unins000.exe");
const base::FilePath::CharType kBililiveUninsDatName[] = FILE_PATH_LITERAL("unins000.dat");
const wchar_t kBililiveLogFileName[] = L"bililive_debug.log";
const wchar_t kBililiveLog1FileName[] = L"bililive_debug_1.log";
const wchar_t kBililiveLog2FileName[] = L"bililive_debug_2.log";
const wchar_t kBililiveLogRecord[] = L"log_record.log";
const wchar_t kStatusTrayWindowClass[] = L"Bililive_StatusTrayWindow";
const wchar_t kBililiveResourcesDll[] = L"bililive.dll";
const wchar_t kUserDataDirname[] = L"User Data";
const wchar_t kGlobalProfileDirName[] = L"Global";
const wchar_t kFirstRunSentinelName[] = L"First Run";
const wchar_t kLiveReportDirname[] = L"Live Report";
const base::FilePath::CharType kPreferencesFilename[] = FILE_PATH_LITERAL("Preferences");
const char kHttpScheme[] = "http";
const char kHttpsScheme[] = "https";
const char kFileSystemScheme[] = "filesystem";
const char kFileScheme[] = "file";
const char kFtpScheme[] = "ftp";
const char kLivehimeScheme[] = "livehime";
const char kStandardSchemeSeparator[] = "://";
const char kSwitchRelogin[] = "relogin";
const char kSwitchProcessType[] = "livehime-type";
const char kSwitchHandshakeHandle[] = "handshake-handle";
const char kSwitchDebugConsole[] = "debug-console";
const char kSwitchEnableFullMinidump[] = "enable-full-minidump";
const char kSwitchHandleVectoredExceptions[] = "handle-vectored-exceptions";
const char kSwitchReportEvent[] = "event";
const char kSwitchReportTimeout[] = "timeout";
const char kSwitchLaunchUIExamples[] = "ui-examples";
const char kSwitchDanmakuRoomId[] = "danmaku-roomid";
const char kSwitchIgnoreUpdate[] = "ignore-update";
const char kSwitchOBSPlugin[] = "obs-plugin-run";
const char kSwitchOBSPluginStartLive[] = "obs-plugin-start-live";
const char kSwitchCustomRtmp[] = "custom-rtmp";
const char kSwitchCustomEventLogId[] = "custom-logid";
const char kSwitchUseTestAgoraAppID[] = "use-test-agora-appid";
const char kProcessCrashHandler[] = "crash-handler";
const char kProcessReporter[] = "reporter";
const char kProcessScheme[] = "scheme";
const base::FilePath::CharType kCmdlineTempFileName[] = FILE_PATH_LITERAL("BililiveArgs");
const base::FilePath::CharType kAppUpdateDirName[] = FILE_PATH_LITERAL("BililiveUD");
const base::FilePath::CharType kInstallerName[] = FILE_PATH_LITERAL("bililive_installer.exe");
const base::FilePath::CharType kUpdatePublishListName[] = FILE_PATH_LITERAL("publish_list");

const wchar_t kSwitchHotUpdate[] = L"--hot-update"; // The command line arguments taken by the boot installation package
const char kSwitchIncHotUpdate[] = "inc-hot-update"; // The command line argument that starts the new updater is the process ID of the old program, 
                                                     // so that the new process listens and waits for the old process to exit

const wchar_t kUpdatedRunSentinelName[] = L"Updated Run";

// If the status identifier file of "Inject OBS plug-in" is selected by the installation package or the Hime itself, 
// the installation package determines whether to inject OBS during installation/update based on the existence of this identifier£¬
// This identifier is deleted during reverse injection and created during injection
const wchar_t kInjectOBSPluginsSentinelName[] = L"Inject OBS";

const wchar_t kOBSSourceDataUpgradeMarkName[] = L"bililive_upgrade_obs_source_data";
const int kIntervalOfSetChannel = 5;
const wchar_t kUserFaceFileName[] = L"avatar.jpg";
const char kFallbackCrashID[] = "unknown";
const base::FilePath::CharType kAppTempDirName[] = FILE_PATH_LITERAL("Bililive");
const int64 kMaxFileSizeAllowedToUpload = 5 * 1024 * 1024;

const char kLogBoundary[] = "----------livehimelogboundary";
const char kSwitchRestart[] = "restart";
}   // namespace bililive
