#include "bililive/bililive/viddup/live_model/viddup_live_model_controller.h"

#include "base/bind.h"
#include "base/ext/callable_callback.h"
#include "base/notification/notification_service.h"
#include "base/prefs/pref_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
//#include "bililive/bililive/livehime/obs/output_controller.h"
#include "bililive/bililive/livehime/obs/source_creator.h"
#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/utils/net_util.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/public/user_info.h"

#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"
#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"

using namespace std::placeholders;

namespace
{
    LiveModelController* g_single_instance = nullptr;

    std::vector<std::string> kLandscapeScenes{
        prefs::kFirstDefaultSceneName,
        prefs::kSecondDefaultSceneName,
        prefs::kThirdDefaultSceneName,
    };
    std::vector<std::string> kPortraitScenes{
        prefs::kVerticalFirstDefaultSceneName,
        prefs::kVerticalSecondDefaultSceneName,
        prefs::kVerticalThirdDefaultSceneName,
    };
}


LiveModelController* ViddupLiveModelController::GetInstance()
{
    return g_single_instance;
}

ViddupLiveModelController::ViddupLiveModelController():
    weak_ptr_factory_(this)
{
    g_single_instance = this;
}

ViddupLiveModelController::~ViddupLiveModelController()
{
    g_single_instance = nullptr;
    Uninitialize();
}

void ViddupLiveModelController::Initialize()
{
    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
    int type = pref->GetInteger(prefs::kLivehimeLiveModelType);
    if (type < (int)ModelType::Landscape || type >(int)ModelType::Portrait)
    {
        type = (int)ModelType::Landscape;
    }

    ModelType model_type = (ModelType)type;

    last_landscape_model_scene_name_ = pref->GetString(prefs::kLivehimeLastFocusLandscapeScene);
    last_portrait_model_scene_name_ = pref->GetString(prefs::kLivehimeLastFocusPortraitScene);
    if (kLandscapeScenes.end() == std::find(kLandscapeScenes.begin(), kLandscapeScenes.end(), last_landscape_model_scene_name_))
    {
        last_landscape_model_scene_name_ = prefs::kFirstDefaultSceneName;
    }
    if (kPortraitScenes.end() == std::find(kPortraitScenes.begin(), kPortraitScenes.end(), last_portrait_model_scene_name_))
    {
        last_portrait_model_scene_name_ = prefs::kVerticalFirstDefaultSceneName;
    }

    ChangeLayoutModel(model_type, false);
}

bool ViddupLiveModelController::ChangeLayoutModel(ModelType type, bool user_invoke/* = true*/)
{
    if (model_type_ == type)
    {
        return true;
    }

    if (!LivehimeLiveRoomController::GetInstance()->IsLiveReady() ||
        !bililive::OutputController::GetInstance()->IsRecordingReady())
    {
        // Do not cut horizontal and vertical screens during the broadcast
        livehime::ShowMessageBox(GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeWindow(),
            GetLocalizedString(IDS_TIP_DLG_TIP), L"You cannot switch between horizontal and portrait screens during broadcasting/recording", GetLocalizedString(IDS_IKNOW));
        return false;
    }

    // Switch the current scene according to the mode, remember the scene selected in the mode before switching, 
    // and continue to select it when you switch back
    model_type_ = type;

    // First set the configuration. obs-proxy needs to determine whether the resolution width and height 
    // are interchangeable according to the configuration
    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
    pref->SetInteger(prefs::kLivehimeLiveModelType, (int)model_type_);

    // Change the OBS push flow resolution, OBS layer by reading prefs: : kLivehimeLiveModelType configuration 
    // will be wide high resolution swaps themselves
    OBSProxyService::GetInstance().GetOBSCoreProxy()->UpdateVideoSettings();

    // The obs layer switches back to the previous scenario
    ChangeCurrentScene(IsLandscapeModel() ? last_landscape_model_scene_name_ : last_portrait_model_scene_name_);

    FOR_EACH_OBSERVER(LiveModelControllerObserver, observer_list_,
        OnLiveLayoutModelChanged(user_invoke));

    bool Land_scape = model_type_ == ModelType::Landscape ? true : false;
    GetBililiveProcess()->secret_core()->anchor_info().set_land_scape_model(Land_scape);

    return true;
}

LiveModelController::ModelType ViddupLiveModelController::layout_model() const
{
    return model_type_;
}

bool ViddupLiveModelController::IsLandscapeModel() const
{
    return layout_model() == ModelType::Landscape;
}

void ViddupLiveModelController::ShowFauxAudientEffect(bool show, bool landscape)
{
    FOR_EACH_OBSERVER(LiveModelControllerObserver, observer_list_,
        OnLiveRoomPreviewStatusChanged(show, landscape));
}

bool ViddupLiveModelController::ChangeCurrentScene(const std::string& scene_name)
{
    if (current_model_scene_name_ == scene_name)
    {
        return true;
    }

    // Based on the current mode, you can determine whether the current scene is in this mode or not
    if (model_type_ == ModelType::Landscape)
    {
        if (kLandscapeScenes.end() == std::find(kLandscapeScenes.begin(), kLandscapeScenes.end(), scene_name))
        {
            return false;
        }
        last_landscape_model_scene_name_ = scene_name;
    }
    else
    {
        if (kPortraitScenes.end() == std::find(kPortraitScenes.begin(), kPortraitScenes.end(), scene_name))
        {
            return false;
        }
        last_portrait_model_scene_name_ = scene_name;
    }
    current_model_scene_name_ = scene_name;

    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
    pref->SetString(IsLandscapeModel() ? prefs::kLivehimeLastFocusLandscapeScene : prefs::kLivehimeLastFocusPortraitScene,
        current_model_scene_name_);

    obs_proxy::GetCoreProxy()->GetCurrentSceneCollection()->TransitToScene(current_model_scene_name_);
    livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::SceneChange, "button_type:" + scene_name);

    GetBililiveProcess()->secret_core()->event_tracking_service()->ReportLivehimeBehaviorEvent(
        secret::LivehimeBehaviorEvent::LivehimeSceneChange,
        GetBililiveProcess()->secret_core()->account_info().mid(),
        std::string()).Call();

    return true;
}

std::string ViddupLiveModelController::current_model_scene_name() const
{
    return current_model_scene_name_;
}

void ViddupLiveModelController::OnOBSUIProxyInitialized()
{
    Initialize();
}

void ViddupLiveModelController::Uninitialize()
{
    if (OBSProxyService::GetInstance().obs_ui_proxy())
    {
        OBSProxyService::GetInstance().obs_ui_proxy()->RemoveObserver(this);
    }
    observer_list_.Clear();
}
