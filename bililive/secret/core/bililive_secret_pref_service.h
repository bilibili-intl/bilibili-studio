/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_CORE_BILILIVE_SECRET_PREF_SERVICE_H_
#define BILILIVE_SECRET_CORE_BILILIVE_SECRET_PREF_SERVICE_H_

#include <memory>

#include "base/basictypes.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/prefs/pref_registry_simple.h"
#include "base/prefs/pref_service.h"

namespace secret {

class SecretPrefService {
public:
    using PrefsRegistrar = void(*)(PrefRegistrySimple* registry);

    SecretPrefService(const base::FilePath& pref_path, base::SequencedTaskRunner* sequenced_task_runner,
                      PrefsRegistrar registrar);

    ~SecretPrefService();

    PrefService* GetPrefs() const;

    const base::FilePath& pref_file_path() const
    {
        return pref_file_path_;
    }

private:
    DISALLOW_COPY_AND_ASSIGN(SecretPrefService);

private:
    base::FilePath pref_file_path_;
    scoped_refptr<PrefRegistrySimple> pref_registry_;
    std::unique_ptr<PrefService> prefs_;
    PersistentPrefStore::PrefReadError pref_read_error_;
};

}   // namespace secret

#endif  //  BILILIVE_SECRET_CORE_BILILIVE_SECRET_PREF_SERVICE_H_