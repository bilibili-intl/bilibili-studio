#ifndef BILILIVE_BILILIVE_PREFS_BILILIVE_PREFS_H_
#define BILILIVE_BILILIVE_PREFS_BILILIVE_PREFS_H_

class PrefRegistrySimple;
class PrefService;
class Profile;

namespace bililive {

void RegisterGlobalProfilePrefs(PrefRegistrySimple* registry);

void RegisterUserProfilePrefs(PrefRegistrySimple* registry);

} // namespace bililive

#endif  // BILILIVE_BILILIVE_PREFS_BILILIVE_PREFS_H_