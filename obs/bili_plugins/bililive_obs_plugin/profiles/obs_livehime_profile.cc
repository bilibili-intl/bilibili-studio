#include "obs_livehime_profile.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/prefs/json_pref_store.h"
#include "base/prefs/pref_service_builder.h"
#include "base/prefs/pref_value_store.h"
#include "base/strings/string_util.h"

#include "public/obs_livehime_pref_constants.h"


namespace
{
    void RegisterGlobalProfilePrefs(PrefRegistrySimple* registry)
    {
        registry->RegisterBooleanPref(obs_plugin_prefs::kPluginEntryFirstRun, obs_plugin_prefs::kDefaultEntryFirstRun);
    }
}


scoped_refptr<Profile> Profile::CreateGlobalProfile(scoped_refptr<base::MessageLoopProxy> main_task_runner,
    const base::FilePath& save_dir_path, scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner)
{
    scoped_refptr<Profile> profile(new Profile(save_dir_path, RegisterGlobalProfilePrefs));
    if (profile)
    {
        main_task_runner->PostTask(FROM_HERE, 
            base::Bind(&Profile::CreatePrefService, profile, sequenced_task_runner));
    }
    return profile;
}

Profile::Profile(const base::FilePath& save_dir_path,
    ProfilePrefsRegisterFunc prefs_registerar)
    : save_dir_path_(save_dir_path)
    , pref_registry_(new PrefRegistrySimple())
    , pref_read_error_(PersistentPrefStore::PREF_READ_ERROR_NONE)
{
    DCHECK(!save_dir_path.empty()) << "Using an empty path will attempt to write " <<
        "profile files to the root directory!";

    prefs_registerar(pref_registry_.get());
}

Profile::~Profile()
{
}

PrefService *Profile::GetPrefs()
{
    DCHECK(prefs_);
    return prefs_.get();
}

const base::FilePath& Profile::GetPrefDir() const
{
    return save_dir_path_;
}

base::FilePath Profile::GetPrefFilePath()
{
    return save_dir_path_.AppendASCII(obs_plugin_prefs::kProfileFilename);
}

void Profile::HandleReadError(PersistentPrefStore::PrefReadError error)
{
    pref_read_error_ = error;
}

void Profile::CreatePrefService(scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner)
{
    PrefServiceBuilder builder;
    builder.WithAsync(false);
    builder.WithUserFilePrefs(GetPrefFilePath(), sequenced_task_runner);
    builder.WithReadErrorCallback(base::Bind(&Profile::HandleReadError, this));
    prefs_.reset(builder.Create(pref_registry_));

    // test[
    /*base::MessageLoopProxy::current()->PostTask(FROM_HERE,
        base::Bind(&PrefService::SetBoolean, base::Unretained(prefs_.get()),
            obs_plugin_prefs::kPluginEntryFirstRun, false));*/
    //]
}

bool Profile::IsNewProfile()
{
    return pref_read_error_ == PersistentPrefStore::PREF_READ_ERROR_NO_FILE;
}

void Profile::CommitPendingWrite()
{
    if (prefs_)
    {
        prefs_->CommitPendingWrite();
    }
}
