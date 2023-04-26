/*
 @ 0xCCCCCCCC
*/

#include "bililive/secret/core/bililive_secret_pref_service.h"

#include "base/prefs/pref_service_builder.h"

namespace secret {

SecretPrefService::SecretPrefService(const base::FilePath& pref_path,
                                     base::SequencedTaskRunner* sequenced_task_runner,
                                     PrefsRegistrar registrar)
    : pref_file_path_(pref_path),
      pref_registry_(new PrefRegistrySimple()),
      pref_read_error_(PersistentPrefStore::PREF_READ_ERROR_NONE)
{
    DCHECK(!pref_path.empty()) << "Using an empty path will write profile files to the root directory!";

    registrar(pref_registry_.get());

    PrefServiceBuilder builder;
    builder.WithAsync(false).WithUserFilePrefs(pref_file_path(), sequenced_task_runner);
    prefs_.reset(builder.Create(pref_registry_.get()));
}

SecretPrefService::~SecretPrefService()
{
    prefs_->CommitPendingWrite();
}

PrefService* SecretPrefService::GetPrefs() const
{
    DCHECK(prefs_);
    return prefs_.get();
}

}   // namespace secret
