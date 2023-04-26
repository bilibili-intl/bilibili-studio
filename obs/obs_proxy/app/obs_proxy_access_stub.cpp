#include "obs/obs_proxy/app/obs_proxy_access_stub.h"

#include "base/logging.h"

#include "bililive/public/bililive/bililive_process.h"

#include "obs/obs_proxy/public/proxy/obs_proxy_delegate.h"

namespace obs_proxy {

extern OBSProxyDelegate* g_proxy_delegate;

void SetProxyDelegate(OBSProxyDelegate* proxy_delegate)
{
    g_proxy_delegate = proxy_delegate;
}

OBSProxyDelegate* GetProxyDelegate()
{
    DCHECK(g_proxy_delegate != nullptr);
    return g_proxy_delegate;
}

BililiveProcess* GetBililiveProcess()
{
    auto* process = GetProxyDelegate()->GetBililiveProcess();
    DCHECK(process != nullptr);
    return process;
}

Profile* GetProfile()
{
    return GetBililiveProcess()->profile();
}

PrefService* GetPrefs()
{
    return GetBililiveProcess()->profile()->GetPrefs();
}

Profile* GetGlobalProfile()
{
    return GetBililiveProcess()->global_profile();
}

PrefService* GetGlobalPrefs()
{
    return GetBililiveProcess()->global_profile()->GetPrefs();
}

}   // namespace obs_proxy