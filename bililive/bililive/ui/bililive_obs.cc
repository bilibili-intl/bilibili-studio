#include "bililive/bililive/ui/bililive_obs.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/common_pref/common_pref_service.h"
#include "bililive/bililive/livehime/danmaku_hime/danmakuhime_data_handler.h"
#include "bililive/bililive/livehime/danmaku_hime/danmaku_hime_pref_service.h"
#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/livehime/kv/kv_colive_settings.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/livehime/notify_ui_control/notify_ui_controller.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"
#include "bililive/bililive/livehime/streaming_report/streaming_report_service.h"
#include "bililive/bililive/livehime/user_info/user_info_service.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/viddup/main_view/livehime_main_viddup_view.h"
#include "bililive/bililive/viddup/live_model/viddup_live_model_controller.h"
#include "bililive/bililive/viddup/live_room/viddup_live_controller.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"

#include "bililive/bililive/viddup/server_broadcast/broadcast_viddup_service.h"
#include <bililive/bililive/livehime/preset_material/preset_material_ui_presenter.h>

// static
void BililiveOBS::RegisterProfilePrefs(PrefRegistrySimple* registry)
{
    CommonPrefService::RegisterProfilePrefs(registry);
    DanmakuHimePrefService::RegisterProfilePrefs(registry);
}

BililiveOBS::BililiveOBS()
    : obs_view_(nullptr)
{
    GetBililiveProcess()->AddRefModule();
}

BililiveOBS::~BililiveOBS()
{
}

void BililiveOBS::Init()
{
    app_func_controller_.reset(new AppFunctionController());

    broadcast_viddup_service_.reset(new BililiveBroadcastViddupService());

    user_info_service_.reset(new UserInfoService());

    live_room_controller_.reset(new ViddupLiveRoomController());

    live_model_controller_.reset(new ViddupLiveModelController());

    InitMainWindow();

    //obs_proxy_ui access bililive_widget_delegate through g_proxy_degate in proxy.dll
    //bililive_ui access obs_ui through g_shared_instance->obs_proxy_ui_ in main.dll
    OBSProxyService::GetInstance().set_bililive_view(obs_view_);

    AppFunctionController::GetInstance()->Initialize();

    PresetMaterailUIPresenter::GetInstance()->Initialize();   
}

void BililiveOBS::Shutdown()
{
    obs_view_ = nullptr;

    DanmakuhimeDataHandler::GetInstance()->Uninit();
    OBSProxyService::GetInstance().set_bililive_view(nullptr);
    broadcast_viddup_service_->StopListening();
    live_room_controller_->Uninitialize();
    live_model_controller_->Uninitialize();

    base::MessageLoop::current()->PostTask(FROM_HERE,
        base::Bind(&BililiveOBS::ActuallyShutdown, this));
}

void BililiveOBS::ActuallyShutdown()
{
    GetBililiveProcess()->ReleaseModule();
}

void BililiveOBS::InitMainWindow()
{
    obs_view_ = BililiveOBSViddupView::CreateBililiveObsView();
    
    DanmakuhimeDataHandler::GetInstance()->Init(obs_view_);
    DanmakuhimeDataHandler::GetInstance()->SetMainView(obs_view_);

    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
    bool is_maximized = pref->GetBoolean(prefs::kLastMainWinMaximized);

    if (is_maximized) {
        obs_view_->GetWidget()->Maximize();
    } else {
        obs_view_->GetWidget()->Show();
    }
}

BililiveOBSView* BililiveOBS::obs_view() const
{
    DCHECK(obs_view_);
    return obs_view_;
}

bool BililiveOBS::main_window_is_minimized() {
    DCHECK(obs_view_);
    if (obs_view_) {
        return obs_view_->IsWinddowMinimized();
    }
    return true;
}

//DmkhimeWindowBridge* BililiveOBS::dmkhime_bridge() const
//{
//    DCHECK(dmkhime_bridge_);
//    return dmkhime_bridge_.get();
//}

BililiveBroadcastService* BililiveOBS::broadcast_service()
{
    DCHECK(broadcast_service_);
    return broadcast_service_.get();
}

BililiveBroadcastViddupService* BililiveOBS::broadcast_viddup_service()
{
    DCHECK(broadcast_viddup_service_);
    return broadcast_viddup_service_.get();
}

UserInfoService* BililiveOBS::user_info_service()
{
    DCHECK(user_info_service_);
    return user_info_service_.get();
}

LivehimeLiveRoomController* BililiveOBS::live_room_controller()
{
    DCHECK(live_room_controller_);
    return live_room_controller_.get();
}
