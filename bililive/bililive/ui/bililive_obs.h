#ifndef BILILIVE_BILILIVE_UI_BILILIVE_OBS_H_
#define BILILIVE_BILILIVE_UI_BILILIVE_OBS_H_

#include "base/basictypes.h"
#include "base/prefs/pref_member.h"

#include "bililive/bililive/ui/bililive_command_receiver.h"

class BililiveOBSView;
class BililiveBroadcastService;
class BililiveBroadcastViddupService;
class PrefRegistrySimple;
class UserInfoService;
class LivehimeLiveRoomController;
class LiveModelController;
class AppFunctionController;
class StreamingReportService;

namespace livehime {
class LiveEvaluateController;

}

namespace contracts
{
    class CheckForUpdatesPresenter;
}

namespace biliyun
{
    class BiliyunUploadService;
}

class BililiveOBS
    : public BililiveCommandReceiver
    , public base::RefCounted<BililiveOBS>
{
public:
    BililiveOBS();

    virtual ~BililiveOBS();

    static void RegisterProfilePrefs(PrefRegistrySimple* registry);

    virtual void Init();
    virtual void Shutdown();

    virtual BililiveOBSView* obs_view() const;
    virtual bool main_window_is_minimized();

    //DmkhimeWindowBridge* dmkhime_bridge() const;

    virtual BililiveBroadcastService* broadcast_service();

    virtual BililiveBroadcastViddupService* broadcast_viddup_service();

    virtual UserInfoService* user_info_service();

    virtual LivehimeLiveRoomController* live_room_controller();

private:
    void ActuallyShutdown();

private:
    friend class base::RefCounted<BililiveOBS>;

    BililiveOBSView* obs_view_;
    //std::unique_ptr<DmkhimeWindowBridge> dmkhime_bridge_;
    scoped_ptr<BililiveBroadcastService> broadcast_service_;
    scoped_ptr<BililiveBroadcastViddupService> broadcast_viddup_service_;
    scoped_ptr<UserInfoService> user_info_service_;
    scoped_ptr<LivehimeLiveRoomController> live_room_controller_;
    std::unique_ptr<LiveModelController> live_model_controller_;
    scoped_ptr<AppFunctionController> app_func_controller_;
    scoped_refptr<StreamingReportService> streaming_report_service_;

    void InitMainWindow();

    DISALLOW_COPY_AND_ASSIGN(BililiveOBS);
};

#endif  // BILILIVE_BILILIVE_UI_BILILIVE_OBS_H_