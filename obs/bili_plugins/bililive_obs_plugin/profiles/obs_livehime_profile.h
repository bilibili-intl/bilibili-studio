#ifndef OBS_PLUGINS_OBS_PROFILES_PROFILE_IMPL_H_
#define OBS_PLUGINS_OBS_PROFILES_PROFILE_IMPL_H_

#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/prefs/pref_service.h"
#include "base/prefs/pref_registry_simple.h"

class NetPrefObserver;
class PrefService;
class PrefServiceSyncable;
class SSLConfigServiceManager;

namespace base
{
    class SequencedTaskRunner;
}

class Profile : public base::RefCountedThreadSafe<Profile>
{
public:
    static scoped_refptr<Profile> CreateGlobalProfile(scoped_refptr<base::MessageLoopProxy> main_task_runner,
        const base::FilePath& save_dir_path, scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner);

    virtual ~Profile();

    const base::FilePath& GetPrefDir() const;
    base::FilePath GetPrefFilePath();
    PrefService *GetPrefs();
    bool IsNewProfile();
    void CommitPendingWrite();

private:
    using ProfilePrefsRegisterFunc = void(*)(PrefRegistrySimple* registry);

    Profile(const base::FilePath& path, ProfilePrefsRegisterFunc prefs_registerar);

    void HandleReadError(PersistentPrefStore::PrefReadError error);
    void CreatePrefService(scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner);

private:
    friend class base::RefCountedThreadSafe<Profile>;

    base::FilePath save_dir_path_;
    base::FilePath base_cache_path_;

    scoped_refptr<PrefRegistrySimple> pref_registry_;
    scoped_ptr<PrefService> prefs_;

    PersistentPrefStore::PrefReadError pref_read_error_;

    DISALLOW_COPY_AND_ASSIGN(Profile);
};

#endif