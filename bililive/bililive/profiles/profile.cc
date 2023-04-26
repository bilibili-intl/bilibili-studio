#include "profile.h"

#include <string>

#include "base/prefs/pref_service.h"
#include "build/build_config.h"


Profile::Profile()
{
}

Profile::~Profile()
{
}

Profile::Delegate::~Delegate()
{
}

// static
const char Profile::kProfileKey[] = "__PROFILE__";

// static
//void Profile::RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry) {
//registry->RegisterBooleanPref(
//    prefs::kSearchSuggestEnabled,
//    true,
//    user_prefs::PrefRegistrySyncable::SYNCABLE_PREF);
//registry->RegisterBooleanPref(
//    prefs::kSessionExitedCleanly,
//    true,
//    user_prefs::PrefRegistrySyncable::UNSYNCABLE_PREF);
//}