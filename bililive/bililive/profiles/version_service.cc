#include "version_service.h"

#include "base/prefs/pref_service.h"
#include "base/version.h"
#include "assistant/common/pref_names.h"
#include "assistant/common/version_info.h"

// static
void VersionService::RegisterProfilePrefs(
    PrefRegistrySimple* registry) {
  registry->RegisterStringPref(
      prefs::kProfileCreatedByVersion,
      "1.0.0.0");
}

// static
void VersionService::SetVersion(PrefService* prefs,
                                      const std::string& version) {
  prefs->SetString(prefs::kProfileCreatedByVersion, version);
}

// static
std::string VersionService::GetVersion(PrefService* prefs) {
  return prefs->GetString(prefs::kProfileCreatedByVersion);
}

// static
void VersionService::OnProfileLoaded(PrefService* prefs,
                                           bool is_new_profile) {
  // Obtain the Chrome version info.
  assistant::VersionInfo version_info;

  // If this is a new profile set version to current version, otherwise
  // (pre-existing profile), leave pref at default value (1.0.0.0) to
  // avoid any first-run behavior.
  std::string version = version_info.Version();
  if (prefs->FindPreference(prefs::kProfileCreatedByVersion)->
      IsDefaultValue() && is_new_profile) {
    SetVersion(prefs, version);
  }
}
