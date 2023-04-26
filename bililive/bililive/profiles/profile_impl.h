#ifndef BILILIVE_BILILIVE_PROFILES_PROFILE_IMPL_H_
#define BILILIVE_BILILIVE_PROFILES_PROFILE_IMPL_H_

#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/prefs/pref_service.h"
#include "base/prefs/pref_registry_simple.h"

#include "bililive/bililive/profiles/profile.h"

class NetPrefObserver;
class PrefService;
class PrefServiceSyncable;
class SSLConfigServiceManager;

namespace base
{
    class SequencedTaskRunner;
}

class ProfileImpl
    : public Profile
{
public:
    static const char *const kPrefExitTypeNormal;

    virtual ~ProfileImpl();

    const base::FilePath& GetPath() const override;
    PrefService *GetPrefs() override;
    ThemeService *GetThemeService() override;

private:
    friend class Profile;

    using ProfilePrefsRegisterFunc = void(*)(PrefRegistrySimple* registry);

    ProfileImpl(const base::FilePath& path,
                Delegate* delegate,
                ProfilePrefsRegisterFunc prefs_registerar,
                base::SequencedTaskRunner* sequenced_task_runner);

    void DoFinalInit();

    void OnInitializationCompleted(PrefService *pref_service,
                                   bool succeeded);

    void OnPrefsLoaded(bool success);

    void HandleReadError(PersistentPrefStore::PrefReadError error);

    bool IsNewProfile();

    base::FilePath GetPrefFilePath();

    base::FilePath path_;
    base::FilePath base_cache_path_;

    scoped_refptr<PrefRegistrySimple> pref_registry_;
    scoped_ptr<PrefService> prefs_;
    scoped_ptr<ThemeService> theme_service_;

    Profile::Delegate *delegate_;

    PersistentPrefStore::PrefReadError pref_read_error_;

    DISALLOW_COPY_AND_ASSIGN(ProfileImpl);
};

#endif  // BILILIVE_BILILIVE_PROFILES_PROFILE_IMPL_H_