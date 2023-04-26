#ifndef BILILIVE_BILILIVE_BILILIVE_PROCESS_IMPL_H_
#define BILILIVE_BILILIVE_BILILIVE_PROCESS_IMPL_H_

#include <memory>

#include "base/basictypes.h"
#include "base/command_line.h"
#include "base/debug/stack_trace.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/prefs/pref_change_registrar.h"
#include "base/prefs/pref_registry_simple.h"
#include "base/threading/non_thread_safe.h"

#include "bililive/bililive/bililive_database.h"
#include "bililive/bililive/bililive_single_instance_guarantor.h"
#include "bililive/bililive/livehime/cef/cef_proxy_wrapper.h"
#include "bililive/bililive/log_report/log_reporter.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"

namespace base {

class SequencedTaskRunner;

}

class BililiveProcessImpl
    : public BililiveProcess,
      public base::NonThreadSafe {
public:
    BililiveProcessImpl(const base::FilePath& user_data_dir,
                        const CommandLine& command_line);

    ~BililiveProcessImpl();

    // Registers global preference items.
    // Global preferences, contrary to user-associated preferences, are application-associated.
    static void RegisterGlobalProfilePrefs(PrefRegistrySimple* registry);

    static void RegisterProfilePrefs(PrefRegistrySimple* registry);

    // Prefer GetBililiveProcess() whenever possible, unless leaking real runtime type is
    // what you want.
    static BililiveProcessImpl* instance();

    void PreCreateThreads();

    bool PreMainMessageLoopRun();

    void PostProfileInit();

    void PreBililiveStart();

    void PostBililiveStart();

    void StartTearDown();

    void PostDestroyThreads();

    unsigned int AddRefModule() override;

    unsigned int ReleaseModule() override;

    bool SetupSingleInstance();

    void TransmitCommandLine();

    bool IsShuttingDown() override;

    const std::string& GetApplicationLocale() override;

    void SetApplicationLocale(const std::string& locale) override;

    void InitGlobalProfile();

    void InitProfile();

    Profile* global_profile() override;

    Profile* profile() override;

    BililiveSecret* secret_core() override;

    bool InitSecretCore();

    bool InitCheckVersion();

    BililiveDatabase* database() const override
    {
        return db_.get();
    }

    LivehimeThemeService* ThemeService() override
    {
        return theme_service_.get();
    }

    bool ExitIsLogout() override;

    void SetLogout() override;

    base::Time GetLoginTime() {
        return login_time_;
    }

    void SetTokenInvalid() override;

    void set_bililive_obs(BililiveOBS* bililive_obs) override;

    scoped_refptr<BililiveOBS> bililive_obs() override;

    void MarkProcessFirstRun()
    {
        first_run_ = true;
    }

    void MarkProcessUpdatedRun()
    {
        updated_run_ = true;
    }

    bool first_run() const override
    {
        return first_run_;
    }

    bool updated_run() const override
    {
        return updated_run_;
    }

    void set_last_crash_exit(bool is_crash_exit) override
    {
        is_last_crash_exit_ = is_crash_exit;
    }

    bool is_last_crash_exit() override
    {
        return is_last_crash_exit_;
    }

    bool ExitIsRestart() override;

    void SetRestart() override;

private:
    int ShowUserLogin();

    void EventTrackingDpi();
    void EventLaunchDuration();

private:
    scoped_ptr<Profile> global_profile_;
    scoped_ptr<Profile> profile_;

    std::unique_ptr<CefProxyWrapper> cef_core_;

    scoped_refptr<BililiveSecret> secret_core_;

    std::unique_ptr<livehime::LogReporter> log_reporter_;

    scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner_;

    unsigned int module_ref_count_;
    bool did_start_;
    std::string locale_;

    PrefChangeRegistrar pref_change_registrar_;
    base::debug::StackTrace release_last_reference_callstack_;
    base::FilePath user_data_dir_;

    std::unique_ptr<BililiveDatabase> db_;
    std::unique_ptr<LivehimeThemeService> theme_service_;

    BililiveSingleInstanceGuarantor single_instance_gurantor_;

    scoped_refptr<BililiveOBS> bililive_obs_;

    base::Time login_time_;

    bool first_run_;
    bool updated_run_;

    bool logout_;
    bool is_last_crash_exit_ = false;
    bool restart_;
    DISALLOW_COPY_AND_ASSIGN(BililiveProcessImpl);
};

#endif  // BILILIVE_BILILIVE_BILILIVE_PROCESS_IMPL_H_