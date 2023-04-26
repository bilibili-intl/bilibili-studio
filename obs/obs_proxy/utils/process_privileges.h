#ifndef OBS_OBS_PROXY_UTILS_PROCESS_PRIVILEGES_H_
#define OBS_OBS_PROXY_UTILS_PROCESS_PRIVILEGES_H_

#include <windows.h>

namespace obs_proxy {

bool EnableProcessSEPrivilege(HANDLE process);

bool DisableProcessSEPrivilege(HANDLE process);

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_UTILS_PROCESS_PRIVILEGES_H_