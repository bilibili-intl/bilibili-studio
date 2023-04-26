#include "obs/obs_proxy/app/obs_proxy_access_stub.h"
#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"
#include "obs/obs_proxy/public/obs_proxy_import.h"
#include "obs/obs_proxy/public/proxy/obs_proxy_delegate.h"
#include "obs/obs_proxy/ui_proxy/ui_proxy_impl.h"

namespace obs_proxy {

// Lifetime is maintained by upper layer.
OBSProxyDelegate* g_proxy_delegate = nullptr;

}   // namespace obs_proxy

void __cdecl BindOBSProxy(obs_proxy::OBSProxyDelegate* delegate)
{
    obs_proxy::SetProxyDelegate(delegate);

    delegate->SetCoreProxy(new obs_proxy::CoreProxyImpl());
    delegate->SetUIProxy(new obs_proxy_ui::UIProxyImpl());
}