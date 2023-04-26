#ifndef BILILIVE_BILILIVE_LIVEHIME_OBS_OBS_PROXY_SERVICE_H_
#define BILILIVE_BILILIVE_LIVEHIME_OBS_OBS_PROXY_SERVICE_H_

#include "base/basictypes.h"
#include "base/prefs/pref_service.h"

#include "obs/obs_proxy/public/proxy/obs_proxy_delegate.h"

class BililiveOBSView;

class OBSProxyService
    : obs_proxy::OBSProxyDelegate
{
public:
    OBSProxyService();

    ~OBSProxyService();

    static bool InitInstance(BililiveMainParts* main_parts);

    static void CleanupInstance();

    static bool HasInstance();

    static OBSProxyService& GetInstance();

    obs_proxy::OBSCoreProxy* GetOBSCoreProxy() const;

    BililiveProcess* GetBililiveProcess() override;

    BililiveMainParts* GetBililiveMainParts() override;

    base::NotificationService* GetNotificationService() override;

    BililiveOBSView* GetBililiveOBSView() override;

    void SetCoreProxy(obs_proxy::OBSCoreProxy* proxy) override;

    void SetUIProxy(obs_proxy_ui::OBSUIProxy* proxy_ui) override;

    // Registers preferences needed by obs-core.
    static void RegisterProfilePrefs(PrefRegistrySimple* registry);

    // Sometimes we want to initialize a preference item to an appropriate value in each
    // runime, which implies the preference item is used only as an in-memory value.
    static void TweakProfilePrefs(PrefService* prefs);

    void set_bililive_view(BililiveOBSView* bililive_obs_view)
    {
        bililive_obs_view_ = bililive_obs_view;
    }

    obs_proxy_ui::OBSUIProxy* obs_ui_proxy() const
    {
        return obs_proxy_ui_.get();
    }

private:
    DISALLOW_COPY_AND_ASSIGN(OBSProxyService);

private:
    std::unique_ptr<obs_proxy::OBSCoreProxy> obs_proxy_;
    std::unique_ptr<obs_proxy_ui::OBSUIProxy> obs_proxy_ui_;
    BililiveOBSView* bililive_obs_view_;
};


#endif  // BILILIVE_BILILIVE_LIVEHIME_OBS_OBS_PROXY_SERVICE_H_