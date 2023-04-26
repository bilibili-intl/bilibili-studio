#include "bililive/bililive/profiles/profile_impl.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/prefs/json_pref_store.h"
#include "base/prefs/pref_service_builder.h"
#include "base/prefs/pref_value_store.h"
#include "base/strings/string_util.h"

#include "bililive/bililive/prefs/bililive_prefs.h"
#include "bililive/bililive/profiles/version_service.h"
#include "bililive/public/common/bililive_constants.h"

Profile* Profile::CreateProfile(const base::FilePath& path,
                                Delegate* delegate,
                                base::SequencedTaskRunner* sequenced_task_runner)
{
    return new ProfileImpl(path,
                           delegate,
                           bililive::RegisterUserProfilePrefs,
                           sequenced_task_runner);
}

Profile* Profile::CreateGlobalProfile(const base::FilePath& path,
                                      Delegate* delegate,
                                      base::SequencedTaskRunner* sequenced_task_runner)
{
    return new ProfileImpl(path,
                           delegate,
                           bililive::RegisterGlobalProfilePrefs,
                           sequenced_task_runner);
}

const char *const ProfileImpl::kPrefExitTypeNormal = "Normal";

ProfileImpl::ProfileImpl(const base::FilePath& path,
                         Delegate* delegate,
                         ProfilePrefsRegisterFunc prefs_registerar,
                         base::SequencedTaskRunner* sequenced_task_runner)
    : path_(path),
      pref_registry_(new PrefRegistrySimple()),
      theme_service_(new ThemeService()),
      delegate_(delegate),
      pref_read_error_(PersistentPrefStore::PREF_READ_ERROR_NONE)
{
    DCHECK(!path.empty()) << "Using an empty path will attempt to write " <<
                          "profile files to the root directory!";

    prefs_registerar(pref_registry_.get());

    {
        PrefServiceBuilder builder;
        builder.WithAsync(false);
        builder.WithUserFilePrefs(GetPrefFilePath(), sequenced_task_runner);
        builder.WithReadErrorCallback(base::Bind(&ProfileImpl::HandleReadError, base::Unretained(this)));
        prefs_.reset(builder.Create(pref_registry_));
    }

    OnPrefsLoaded(true);
}

void ProfileImpl::DoFinalInit()
{
    if (delegate_)
    {
        delegate_->OnProfileCreated(this, true, IsNewProfile());
    }

    theme_service_->Init(this);
}


ProfileImpl::~ProfileImpl()
{

}

const base::FilePath& ProfileImpl::GetPath() const
{
    return path_;
}

void ProfileImpl::OnPrefsLoaded(bool success)
{
    if (!success)
    {
        if (delegate_)
        {
            delegate_->OnProfileCreated(this, false, false);
        }
        return;
    }

    DoFinalInit();
}

void ProfileImpl::HandleReadError(PersistentPrefStore::PrefReadError error)
{
    pref_read_error_ = error;
}

PrefService *ProfileImpl::GetPrefs()
{
    DCHECK(prefs_);  // Should explicitly be initialized.
    return prefs_.get();
}

base::FilePath ProfileImpl::GetPrefFilePath()
{
    return path_.Append(bililive::kPreferencesFilename);
}

ThemeService *ProfileImpl::GetThemeService()
{
    return theme_service_.get();
}

bool ProfileImpl::IsNewProfile()
{
    return pref_read_error_ == PersistentPrefStore::PREF_READ_ERROR_NO_FILE;
}

void ProfileImpl::OnInitializationCompleted(PrefService* pref_service, bool succeeded)
{}