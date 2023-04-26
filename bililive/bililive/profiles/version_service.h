#ifndef ASSISTANT_ASSISTANT_PROFILES_VERSION_SERVICE_H
#define ASSISTANT_ASSISTANT_PROFILES_VERSION_SERVICE_H

#include <string>

#include "base/basictypes.h"
#include "bililive/bililive/prefs/bililive_prefs.h"
#include "base/prefs/pref_service.h"
#include "base/prefs/pref_registry_simple.h"

class PrefService;

namespace user_prefs {
class PrefRegistrySyncable;
}

class VersionService {
 public:
  static void RegisterProfilePrefs(PrefRegistrySimple* registry);

  static void SetVersion(PrefService* prefs, const std::string& version);

  static std::string GetVersion(PrefService* prefs);

  static void OnProfileLoaded(PrefService* prefs, bool is_new_profile);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(VersionService);
};

#endif  
