#include "bililive/bililive/prefs/bililive_prefs.h"

#include "bililive/bililive/bililive_process_impl.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/themes/theme_service.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/livehime/theme/livehime_theme_service.h"


namespace bililive {

void RegisterGlobalProfilePrefs(PrefRegistrySimple* registry)
{
    BililiveProcessImpl::RegisterGlobalProfilePrefs(registry);
    ThemeService::RegisterProfilePrefs(registry);
}

void RegisterUserProfilePrefs(PrefRegistrySimple* registry)
{
    BililiveProcessImpl::RegisterProfilePrefs(registry);
    BililiveOBS::RegisterProfilePrefs(registry);
    ThemeService::RegisterProfilePrefs(registry);
    OBSProxyService::RegisterProfilePrefs(registry);
    LivehimeThemeService::RegisterProfilePrefs(registry);
}

} // namespace bililive
