#ifndef OBS_OBS_PROXY_CORE_PROXY_OBS_LOGGING_HOOK_H_
#define OBS_OBS_PROXY_CORE_PROXY_OBS_LOGGING_HOOK_H_

namespace obs_proxy {

// This function must be called prior to any obs-core functions.
void HookOBSLogging();

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_CORE_PROXY_OBS_LOGGING_HOOK_H_