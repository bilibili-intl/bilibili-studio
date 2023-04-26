#include "bililive/bililive/bililive_process_impl.h"

#include <psapi.h>
#include <shlwapi.h>

#include "base/command_line.h"
#include "base/debug/alias.h"
#include "base/message_loop/message_loop.h"
#include "base/notification/notification_service.h"
#include "base/path_service.h"
#include "base/prefs/json_pref_store.h"
#include "base/prefs/pref_registry_simple.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/tracked_objects.h"
#include "base/strings/string_number_conversions.h"

#include "ui/gfx/canvas.h"
#include "ui/views/widget/widget.h"

#include "bililive/bililive/first_run/first_run.h"
#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/main_view/livehime_main_close_pref_constants.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/log_report/app_integrity_checking.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/livehime/theme/livehime_theme_service.h"
#include "bililive/bililive/ui/views/login/bililive_login_main_view.h"
#include "bililive/bililive/ui/views/login/bililive_mini_login_viddup_widget.h"
#include "bililive/common/bililive_context.h"
#include "bililive/common/bililive_features.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/pref_constants.h"
#include "bililive/public/common/pref_names.h"


BililiveProcessImpl* g_bililive_process = nullptr;

BililiveProcess* GetBililiveProcess()
{
    DCHECK(g_bililive_process);
    return g_bililive_process;
}

namespace {

using BililiveSecretCreator = BililiveSecret* (*)();

const wchar_t kBililiveSecretDLL[] = L"bililive_secret.dll";

void NotifyAppTerminating()
{
    static bool notified = false;
    if (notified) {
        return;
    }

    notified = true;
    base::NotificationService::current()->Notify(
        bililive::NOTIFICATION_APP_TERMINATING,
        base::NotificationService::AllSources(),
        base::NotificationService::NoDetails());
}

void OnAppExiting()
{
    static bool notified = false;
    if (notified) {
        return;
    }

    notified = true;

    views::Widget::CloseAllSecondaryWidgets();
}

HMODULE LoadSecretDLL(base::FilePath& secret_dll_path)
{
    auto main_dir = BililiveContext::Current()->GetMainDirectory();
    auto dll_path = main_dir.Append(kBililiveSecretDLL);
    secret_dll_path = dll_path;
    return LoadLibraryExW(dll_path.value().c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
}

BililiveSecretCreator GetBililiveSecretCreator()
{
    base::FilePath dll_path;
    HMODULE secret_dll = LoadSecretDLL(dll_path);
    if (!secret_dll) {
        PLOG(ERROR) << "Failed to load secret dll from " << dll_path.AsUTF8Unsafe();
        return nullptr;
    }

    BililiveSecretCreator creator_func =
        reinterpret_cast<BililiveSecretCreator>(GetProcAddress(secret_dll, "CreateBililiveSecret"));

    if (!creator_func) {
        PLOG(ERROR) << "No entry point in secret dll found; possibly defected!";
        return nullptr;
    }

    return creator_func;
}

class ProfileCreationDelegate : public Profile::Delegate {
public:
    void OnProfileCreated(Profile* profile, bool success, bool is_new_profile) override
    {
        DCHECK(!EndsWith(profile->GetPath().value(), bililive::kGlobalProfileDirName, true));
        DCHECK(success);

        OBSProxyService::TweakProfilePrefs(profile->GetPrefs());
    }
};

void CheckDWriteEnabled()
{
    HANDLE process = GetCurrentProcess();

    HMODULE module[1024];
    DWORD res;
    if (EnumProcessModules(process, module, sizeof(module), &res))
    {
        std::unique_ptr<wchar_t[]> long_mod_name;

        for (unsigned long i = 0; i < (res / sizeof(*module)); i++)
        {
            wchar_t mod_name[MAX_PATH];
            DWORD ret = GetModuleFileName(module[i], mod_name, MAX_PATH);
            if (ret > 0)
            {
                wchar_t* p_name = mod_name;

                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                {
                    if (!long_mod_name)
                    {
                        long_mod_name = std::unique_ptr<wchar_t[]>(new wchar_t[32767]);
                    }

                    DWORD ret = GetModuleFileName(module[i], long_mod_name.get(), 32767);
                    if (ret > 0)
                    {
                        p_name = long_mod_name.get();
                    }
                }

                if (!wcsicmp(PathFindFileName(p_name), L"SS2OSD.dll") ||
                    !wcsicmp(PathFindFileName(p_name), L"Nahimic2OSD.dll") ||
                    !wcsicmp(PathFindFileName(p_name), L"SSAudioOSD.dll") ||
                    !wcsicmp(PathFindFileName(p_name), L"nhAsusROG71OSD.dll") ||
                    !wcsicmp(PathFindFileName(p_name), L"nhAsusU7MKIIOSD.dll"))
                {
                    gfx::Canvas::SetDWriteEnabled(false);
                    LOG(INFO) << "SetDWriteEnabled false";
                    break;
                }
            }
        }
    }
}

void ReportRequiredFilesMissing()
{
    auto missing_files = livehime::FilesIntegrityChecking();
    if (!missing_files.empty())
    {
        std::string str = JoinString(missing_files, "&");

        std::string event_msg = base::StringPrintf("files:%s", str.c_str());
        GetBililiveProcess()->secret_core()->event_tracking_service()->ReportLivehimeBehaviorEvent(
            secret::LivehimeBehaviorEvent::LivehimeIntegrityChecking, 0, event_msg
        ).Call();
    }
}

}   // namespace

BililiveProcessImpl::BililiveProcessImpl(const base::FilePath& user_data_dir,
                                         const CommandLine& command_line)
    : module_ref_count_(0),
      did_start_(false),
      user_data_dir_(user_data_dir),
      bililive_obs_(nullptr),
      first_run_(false),
      updated_run_(false),
      logout_(false),
      restart_(false)
{
    g_bililive_process = this;
}

BililiveProcessImpl::~BililiveProcessImpl()
{
    tracked_objects::ThreadData::EnsureCleanupWasCalled(4);
    g_bililive_process = nullptr;
}

// static
void BililiveProcessImpl::RegisterGlobalProfilePrefs(PrefRegistrySimple* registry)
{
    registry->RegisterBooleanPref(prefs::kWindowCaptureOptimizeForPerformance, true);
    registry->RegisterBooleanPref(prefs::kExcludeMainWindowFromCapture, false);

    BOOL drag_full_windows = TRUE;
    ::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, NULL, &drag_full_windows, NULL);
    registry->RegisterBooleanPref(prefs::kDragFullWindows, drag_full_windows ? true : false);
    registry->RegisterIntegerPref(prefs::kApplicationExitMode, prefs::kMinimizeToTray);
    registry->RegisterBooleanPref(prefs::kApplicationExitRememberChoice, false);
    registry->RegisterBooleanPref(prefs::kMainViewNoviceGuide, true);
    registry->RegisterBooleanPref(prefs::kPresetMaterialGuide, false);
    registry->RegisterStringPref(prefs::kPlayCenterLoaclList, "");
    registry->RegisterBooleanPref(prefs::kSingIdentifyNoticeShow, true);
    registry->RegisterInt64Pref(prefs::kSingIdentifyNoticeTime, 0);
    registry->RegisterStringPref(prefs::kUnityVersionList, "");
    registry->RegisterStringPref(prefs::kSecretFigmaList, "");
    registry->RegisterStringPref(prefs::kFaceEffectLoaclList, "");
    registry->RegisterBooleanPref(prefs::kAnchorTaskGuide, false);
    registry->RegisterBooleanPref(prefs::kVoiceLinkGuide, false);
    registry->RegisterBooleanPref(prefs::kStreamingQualityGuide, false);
    registry->RegisterBooleanPref(prefs::kDanmakuFunctionViewRedPointGuide, false);
    registry->RegisterBooleanPref(prefs::kFastForwardSwitch, false);
    registry->RegisterStringPref(prefs::kFastForwardHeadersProxyId, "");
    registry->RegisterStringPref(prefs::kFastForwardHeadersProxyEnv, "");
    registry->RegisterStringPref(prefs::kFastForwardWebviewHostPrefix, "");
    registry->RegisterStringPref(prefs::kFastForwardWebviewQueryArgsProxyId,"");
    registry->RegisterBooleanPref(prefs::kAddBtnBubbleShowed, false);
    registry->RegisterBooleanPref(prefs::kCoreUserSettingBtRedPointGuide, false);
    registry->RegisterBooleanPref(prefs::kCoreUserSettingViewNewFlagGuide, false);
    registry->RegisterBooleanPref(prefs::kCoreUserSettingBtDisturbGuide, false);
    registry->RegisterInt64Pref(prefs::kAnchorTaskGuideLastShowTime, 0);
    registry->RegisterBooleanPref(prefs::kRTCDropRateIsException, false);
    registry->RegisterStringPref(prefs::kRTCDropRateChannelId, "");
    registry->RegisterStringPref(prefs::kRTCDropRateRoomId, "");
    registry->RegisterStringPref(prefs::kRTCDropRateLinkType, "");
    registry->RegisterStringPref(prefs::kRTCDropRateMid, "");
    registry->RegisterStringPref(prefs::kRTCDropRateTimeStamp, "");

    // Native multilanguage
    registry->RegisterStringPref(prefs::kLivehimeAppLocale, prefs::kLocaleEnUS);
}

// static
void BililiveProcessImpl::RegisterProfilePrefs(PrefRegistrySimple* registry)
{
    registry->RegisterStringPref(prefs::kAvatarCacheTag, "");
    registry->RegisterStringPref(prefs::kLastLoginDate, "");
    registry->RegisterBooleanPref(prefs::kColiveMainWindowFirstShow, true);
    registry->RegisterDoublePref(prefs::kColoveMainWindowHorizCrop, 0.5f);
    registry->RegisterDoublePref(prefs::kColoveMainWindowVertCrop, 0.18359375f);
}

// static
BililiveProcessImpl* BililiveProcessImpl::instance()
{
    DCHECK(g_bililive_process);
    return g_bililive_process;
}

void BililiveProcessImpl::StartTearDown()
{
    set_bililive_obs(nullptr);

    single_instance_gurantor_.UnInstall();

    db_ = nullptr;

    if (secret_core_) {
        secret_core_->UnInit();
    }

    if (global_profile_) {
        global_profile_->GetPrefs()->CommitPendingWrite();
    }

    if (profile_) {
        profile_->GetPrefs()->CommitPendingWrite();
    }
}

void BililiveProcessImpl::PostDestroyThreads()
{}

unsigned int BililiveProcessImpl::AddRefModule()
{
    DCHECK(CalledOnValidThread());

    if (IsShuttingDown()) {
        base::debug::StackTrace callstack = release_last_reference_callstack_;
        base::debug::Alias(&callstack);
        CHECK(false);
    }

    did_start_ = true;
    module_ref_count_++;
    return module_ref_count_;
}

unsigned int BililiveProcessImpl::ReleaseModule()
{
    DCHECK(CalledOnValidThread());
    DCHECK_NE(0u, module_ref_count_);
    module_ref_count_--;
    if (0 == module_ref_count_) {
        release_last_reference_callstack_ = base::debug::StackTrace();

        CHECK(base::MessageLoop::current()->is_running());

        NotifyAppTerminating();
        OnAppExiting();

        base::MessageLoop::current()->Quit();

        LOG(INFO) << "Main MessageLoop Quit";
    }

    return module_ref_count_;
}

bool BililiveProcessImpl::SetupSingleInstance()
{
    return single_instance_gurantor_.Install();
}

void BililiveProcessImpl::TransmitCommandLine()
{
    single_instance_gurantor_.TransmitCommandLine();
}

bool BililiveProcessImpl::IsShuttingDown()
{
    DCHECK(CalledOnValidThread());
    return did_start_ && 0 == module_ref_count_;
}

const std::string& BililiveProcessImpl::GetApplicationLocale()
{
    DCHECK(!locale_.empty());
    return locale_;
}

void BililiveProcessImpl::SetApplicationLocale(const std::string& locale)
{
    DCHECK(!locale.empty());
    locale_ = locale;
}

void BililiveProcessImpl::PreCreateThreads()
{}

bool BililiveProcessImpl::PreMainMessageLoopRun()
{
    if (bililive::IsBililiveFirstRun()) {
        MarkProcessFirstRun();
    }

    if (first_run()) {
        if (!bililive::StartFirstRunTasks()) {
            return false;
        }

        bililive::MarkFirstRunPassed();
    }

    return true;
}

void BililiveProcessImpl::PostProfileInit()
{
    db_ = std::make_unique<BililiveDatabase>();

    DCHECK(theme_service_);
    if (theme_service_)
    {
        theme_service_->Init(profile_.get());
    }
}

void BililiveProcessImpl::PreBililiveStart()
{}

void BililiveProcessImpl::PostBililiveStart()
{
    // Updated run tasks are not necessarily required to complete successfully.
    if (updated_run()) {
       
    }

    single_instance_gurantor_.StartMonitor();

    EventTrackingDpi();
}

Profile* BililiveProcessImpl::global_profile()
{
    DCHECK(global_profile_.get());
    return global_profile_.get();
}

Profile *BililiveProcessImpl::profile()
{
    DCHECK(profile_.get());
    return profile_.get();
}

BililiveSecret* BililiveProcessImpl::secret_core()
{
    return secret_core_.get();
}

bool BililiveProcessImpl::InitSecretCore()
{
    if (secret_core_) {

        // Report missing necessary documents
        ReportRequiredFilesMissing();

        // Check D2D availability
        CheckDWriteEnabled();

        int ret = ShowUserLogin();

        if (ret != 0) {
            return false;
        }

        InitProfile();
    } else {
        MessageBoxW(nullptr, L"Secret-free mode is currently not supported!", L"Critical Error",
                    MB_OK | MB_ICONERROR);
        return false;
    }

    return true;
}

bool BililiveProcessImpl::InitCheckVersion()
{
    DCHECK(secret_core_ == nullptr) << "Can't initialize secret core more than once!";

    BililiveSecretCreator creator = GetBililiveSecretCreator();

    if (creator) {
        secret_core_ = creator();
        CHECK(secret_core_) << "Create secret core failure!";
        secret_core_->Init(base::MessageLoopForUI::current()->message_loop_proxy(),
            global_profile()->GetPath(),
            GetBililiveProcess());

        // Load cef_proxy_dll dynamically just before the login box runs
        cef_core_.reset(new CefProxyWrapper());
        cef_core_->LoadCefProxyCore();

        return true;
    }
    else {
        MessageBoxW(nullptr, L"Secret-free mode is currently not supported!", L"Critical Error",
            MB_OK | MB_ICONERROR);
        return false;
    }
}

void BililiveProcessImpl::InitGlobalProfile()
{
    auto global_profile_dir = user_data_dir_.Append(bililive::kGlobalProfileDirName);
    if (!base::DirectoryExists(global_profile_dir)) {
        file_util::CreateDirectory(global_profile_dir);
    }

    if (!sequenced_task_runner_.get()) {
        sequenced_task_runner_ = JsonPrefStore::GetTaskRunnerForFile(
                    user_data_dir_.Append(bililive::kPreferencesFilename),
                    BililiveThread::GetBlockingPool());
    }

    global_profile_.reset(Profile::CreateGlobalProfile(global_profile_dir,
                                                       nullptr,
                                                       sequenced_task_runner_.get()));

    theme_service_.reset(LivehimeThemeService::Create());
}

void BililiveProcessImpl::InitProfile()
{
    if (!sequenced_task_runner_.get()) {
        sequenced_task_runner_ =
                JsonPrefStore::GetTaskRunnerForFile(
                    user_data_dir_.Append(bililive::kPreferencesFilename),
                    BililiveThread::GetBlockingPool());
    }

    if (!profile_.get()) {
        base::FilePath user_data_dir = user_data_dir_.AppendASCII(std::to_string(
            secret_core_->account_info().mid()));
        if (!base::DirectoryExists(user_data_dir)) {
            CHECK(file_util::CreateDirectory(user_data_dir));
        }

        ProfileCreationDelegate delegate;

        profile_.reset(Profile::CreateProfile(
            user_data_dir,
            &delegate,
            sequenced_task_runner_.get()));
    }
}

bool BililiveProcessImpl::ExitIsLogout()
{
    return logout_;
}

void BililiveProcessImpl::SetLogout(void)
{
    logout_ = true;
    secret_core_->SetLogout();
}

void BililiveProcessImpl::SetTokenInvalid()
{
    secret_core_->SetTokenInvalid();
}

void BililiveProcessImpl::set_bililive_obs(BililiveOBS* bililive_obs)
{
    bililive_obs_ = bililive_obs;
}

scoped_refptr<BililiveOBS> BililiveProcessImpl::bililive_obs()
{
    DCHECK(bililive_obs_ && !bililive_obs_->HasOneRef()) <<
        "invoke in the wrong time";

    return bililive_obs_;
}

bool BililiveProcessImpl::ExitIsRestart()
{
    return restart_;
}

void BililiveProcessImpl::SetRestart()
{
    restart_ = true;
}

int BililiveProcessImpl::ShowUserLogin()
{
    LoginResultCode result = LoginResultCode::UN_LOGIN;

    if (BililiveFeatures::current()->Enabled(BililiveFeatures::NativeLogin))
    {
        BililiveLoginMainView::DoModal(&result);
    }
    else
    {
        EventLaunchDuration();   // Reporting cold start takes time
        BililiveMiniLoginViddupView::DoModal(&result);
        login_time_ = base::Time().Now();
    }

    if (LoginResultCode::LOGIN_SUCCESS == result)
    {
        secret_core()->SaveSecretProfile();

        return 0;
    }

    return -1;
}

void BililiveProcessImpl::EventLaunchDuration() {
    base::StringPairs event_ext;

    int64 luanch_time = (base::Time::Now() - BililiveContext::Current()->GetApplaunchStartTime()).InMilliseconds();
    event_ext.push_back(std::pair<std::string, std::string>("duration", base::Int64ToString(luanch_time)));
    event_ext.push_back(std::pair<std::string, std::string>("type", "launch"));

    livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::LaunchDurationTracker, event_ext);
}

void BililiveProcessImpl::EventTrackingDpi()
{
    auto last_date = profile()->GetPrefs()->GetString(prefs::kLastLoginDate);

    base::Time::Exploded time_exploded;
    base::Time::Now().LocalExplode(&time_exploded);

    auto login_date = base::StringPrintf("%d%02d%02d",
        time_exploded.year,
        time_exploded.month,
        time_exploded.day_of_month);

    if (last_date != login_date)
    {
        int x = GetSystemMetrics(SM_CXSCREEN);
        int y = GetSystemMetrics(SM_CYSCREEN);

        auto screen = GetDC(nullptr);
        auto scale = GetDeviceCaps(screen, LOGPIXELSX) * 100.0 / 96;
        ReleaseDC(nullptr, screen);

        auto msg = JoinString({ base::StringPrintf("resolution:%d*%d", x, y),
            base::StringPrintf("dpi:%d", static_cast<int>(scale)) },
            ";");

        secret_core()->event_tracking_service()->ReportLivehimeBehaviorEvent(
            secret::LivehimeBehaviorEvent::LivehimeDPI,
            secret_core()->account_info().mid(),
            msg).Call();

        profile()->GetPrefs()->SetString(prefs::kLastLoginDate, login_date);
    }
}
