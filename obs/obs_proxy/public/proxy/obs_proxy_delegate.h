#ifndef OBS_OBS_PROXY_PUBLIC_PROXY_OBS_PROXY_DELEGATE_H_
#define OBS_OBS_PROXY_PUBLIC_PROXY_OBS_PROXY_DELEGATE_H_

#include "base/notification/notification_service.h"

#include "obs/obs_proxy/public/proxy/obs_proxy.h"

class BililiveThread;
class BililiveMainParts;
class BililiveProcess;
class BililiveOBSView;

namespace obs_proxy {

class OBSProxyDelegate {
public:
    virtual ~OBSProxyDelegate() {}

    virtual BililiveProcess* GetBililiveProcess() = 0;

    virtual BililiveMainParts* GetBililiveMainParts() = 0;

    virtual base::NotificationService* GetNotificationService() = 0;

    virtual BililiveOBSView* GetBililiveOBSView() = 0;

    virtual void SetCoreProxy(OBSCoreProxy* proxy) = 0;

    virtual void SetUIProxy(obs_proxy_ui::OBSUIProxy* proxy_ui) = 0;
};

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_PUBLIC_PROXY_OBS_PROXY_DELEGATE_H_