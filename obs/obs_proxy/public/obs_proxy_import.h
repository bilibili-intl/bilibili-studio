#ifndef OBS_OBS_PROXY_APP_OBS_PROXY_MAIN_H_
#define OBS_OBS_PROXY_APP_OBS_PROXY_MAIN_H_

#include "obs/obs_proxy/app/obs_proxy_export.h"

class BililiveMainExtraParts;

extern "C" OBS_PROXY_EXPORT void __cdecl BindOBSProxy(obs_proxy::OBSProxyDelegate* delegate);

#endif  // OBS_OBS_PROXY_APP_OBS_PROXY_MAIN_H_