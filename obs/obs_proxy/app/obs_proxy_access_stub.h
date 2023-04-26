#ifndef OBS_OBS_PROXY_APP_OBS_PROXY_ACCESS_STUB_H_
#define OBS_OBS_PROXY_APP_OBS_PROXY_ACCESS_STUB_H_

// These stubs intends to either hide implementation for access to crucial components,
// or provide shortcuts to components that will be frequently accessed.

class BililiveProcess;
class Profile;
class PrefService;

namespace obs_proxy {

class OBSProxyDelegate;

void SetProxyDelegate(OBSProxyDelegate* proxy_delegate);

OBSProxyDelegate* GetProxyDelegate();

BililiveProcess* GetBililiveProcess();

Profile* GetProfile();

PrefService* GetPrefs();

Profile* GetGlobalProfile();

PrefService* GetGlobalPrefs();

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_APP_OBS_PROXY_ACCESS_STUB_H_