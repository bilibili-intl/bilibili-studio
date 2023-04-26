#include "bililive/bililive/livehime/live_model/live_model_controller.h"

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


LiveModelController* LiveModelController::GetInstance()
{
    return g_single_instance;
}

LiveModelController::LiveModelController():
    weak_ptr_factory_(this)
{
    g_single_instance = this;
    OBSProxyService::GetInstance().obs_ui_proxy()->AddObserver(this);
}

LiveModelController::~LiveModelController()
{
    g_single_instance = nullptr;
    Uninitialize();
}

void LiveModelController::Initialize()
{
    // 从配置文件里读取上次的开播模式
    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
    int type = pref->GetInteger(prefs::kLivehimeLiveModelType);
    if (type < (int)ModelType::Landscape || type >(int)ModelType::Portrait)
    {
        type = (int)ModelType::Landscape;
    }
    ModelType model_type = (ModelType)type;

    // 上次记录的场景名也要进行一下校验，防止用户/测试同学手动改了配置或是开发童鞋变更了默认场景名
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

    // 设置当前场景，横竖屏模式的设置、展现和场景过滤完全是UI层自己的事情，obs层只管切换场景即可
    ChangeLayoutModel(model_type, false);
}

bool LiveModelController::ChangeLayoutModel(ModelType type, bool user_invoke/* = true*/)
{
    if (model_type_ == type)
    {
        return true;
    }

    if (!LivehimeLiveRoomController::GetInstance()->IsLiveReady() ||
        !bililive::OutputController::GetInstance()->IsRecordingReady())
    {
        // 开播中不能切横竖屏
        livehime::ShowMessageBox(GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeWindow(),
            GetLocalizedString(IDS_TIP_DLG_TIP), L"开播/录制中不能切换横竖屏", GetLocalizedString(IDS_IKNOW));
        return false;
    }

    // 根据模式切换当前场景，同时记住切换前的模式下选中的场景，当切回来之后要继续选中它
    model_type_ = type;

    // 先设置配置，obs-proxy需要根据配置去决定分辨率宽高是否互换
    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
    pref->SetInteger(prefs::kLivehimeLiveModelType, (int)model_type_);

    // 变更OBS推流分辩率，obs层通过读取prefs::kLivehimeLiveModelType配置自己会将分辨率进行宽高互换
    OBSProxyService::GetInstance().GetOBSCoreProxy()->UpdateVideoSettings();

    // obs层换回先前的场景
    ChangeCurrentScene(IsLandscapeModel() ? last_landscape_model_scene_name_ : last_portrait_model_scene_name_);

    FOR_EACH_OBSERVER(LiveModelControllerObserver, observer_list_,
        OnLiveLayoutModelChanged(user_invoke));

    bool Land_scape = model_type_ == ModelType::Landscape ? true : false;
    GetBililiveProcess()->secret_core()->anchor_info().set_land_scape_model(Land_scape);

    return true;
}

LiveModelController::ModelType LiveModelController::layout_model() const
{
    return model_type_;
}

bool LiveModelController::IsLandscapeModel() const
{
    return layout_model() == ModelType::Landscape;
}

void LiveModelController::ShowFauxAudientEffect(bool show, bool landscape)
{
    FOR_EACH_OBSERVER(LiveModelControllerObserver, observer_list_,
        OnLiveRoomPreviewStatusChanged(show, landscape));
}

bool LiveModelController::ChangeCurrentScene(const std::string& scene_name)
{
    if (current_model_scene_name_ == scene_name)
    {
        return true;
    }

    // 要根据当前的模式来决定当前要换的这个场景在不在这个模式下
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

    // 场景切换埋点
    livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::SceneChange, "button_type:" + scene_name);

    GetBililiveProcess()->secret_core()->event_tracking_service()->ReportLivehimeBehaviorEvent(
        secret::LivehimeBehaviorEvent::LivehimeSceneChange,
        GetBililiveProcess()->secret_core()->account_info().mid(),
        std::string()).Call();

    return true;
}

std::string LiveModelController::current_model_scene_name() const
{
    return current_model_scene_name_;
}

void LiveModelController::OnOBSUIProxyInitialized()
{
    Initialize();
}

void LiveModelController::Uninitialize()
{
    if (OBSProxyService::GetInstance().obs_ui_proxy())
    {
        OBSProxyService::GetInstance().obs_ui_proxy()->RemoveObserver(this);
    }
    observer_list_.Clear();
}
