#include "bililive/bililive/livehime/settings/settings_presenter_impl.h"

#include "base/ext/callable_callback.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "bilibase/basic_types.h"

#include "bililive/bililive/livehime/common_pref/common_pref_service.h"
#include "bililive/bililive/livehime/danmaku_hime/danmaku_hime_pref_service.h"
#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/livehime/main_view/livehime_main_close_pref_constants.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"
#include "bililive/bililive/ui/views/livehime/settings/denoise_slider.h"
#include "bililive/common/bililive_features.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/public/common/pref_names.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/livehime/user_info/user_info_service.h"
#include "bililive/bililive/ui/views/livehime/settings/global_settings_view.h"

#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"
#include "obs/obs_proxy/public/proxy/obs_audio_devices.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"

static bool voice_barrage_switch_ = true;
static bool update_anchor_switch_lock_ = false;

obs_proxy::SceneCollection* ApplyStreamingSettings() {
    // 重设obs底层audio参数
    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
    obs_proxy::SceneCollection* scene_collection
        = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    if (!scene_collection) {
        return nullptr;
    }

    std::vector<obs_proxy::AudioDevice*> vct = scene_collection->GetAudioDevices();
    // 先设置麦克风
    auto mic = std::find_if(vct.begin(), vct.end(), [](obs_proxy::AudioDevice* dev)-> bool {
        return dev->name() == obs_proxy::kDefaultInputAudio; });
    if (mic != vct.end()) {
        // 麦克风设备
        std::string micid = pref->GetString(prefs::kAudioMicDeviceInUse);
        obs_proxy::PropertyValue& propv = (*mic)->GetPropertyValues();
        propv.Set("device_id", micid);

        (*mic)->UpdatePropertyValues();
    }
    // 再设置扬声器
    auto speaker = std::find_if(vct.begin(), vct.end(), [](obs_proxy::AudioDevice* dev)-> bool {
        return dev->name() == obs_proxy::kDefaultOutputAudio; });
    if (speaker != vct.end()) {
        // 麦克风设备
        std::string spkid = pref->GetString(prefs::kAudioSpeakerDeviceInUse);
        obs_proxy::PropertyValue& propv = (*speaker)->GetPropertyValues();
        propv.Set("device_id", spkid);

        (*speaker)->UpdatePropertyValues();
    }

    return scene_collection;
}

SettingsPresenterImpl::SettingsPresenterImpl(
    contracts::SettingsFrameView* view)
    : settings_frame_view_(view),
      weak_ptr_factory_(this) {
}

SettingsPresenterImpl::SettingsPresenterImpl(
    contracts::AcceleratorSettingsView* view)
    : accelerator_settings_view_(view),
      weak_ptr_factory_(this) {
}

SettingsPresenterImpl::SettingsPresenterImpl(
    contracts::AudioSettingsView* view)
    : audio_settings_view_(view),
      weak_ptr_factory_(this) {
}

SettingsPresenterImpl::SettingsPresenterImpl(
    contracts::VideoSettingsView* view)
    : video_settings_view_(view),
      weak_ptr_factory_(this) {
}

SettingsPresenterImpl::SettingsPresenterImpl(
    contracts::RecordSettingsView* view)
    : record_settings_view_(view),
      weak_ptr_factory_(this) {}

SettingsPresenterImpl::SettingsPresenterImpl(
    contracts::StreamingSettingsView* view)
    : streaming_settings_view_(view),
      weak_ptr_factory_(this) {
}

SettingsPresenterImpl::SettingsPresenterImpl(
    contracts::GlobalSettingsView* view)
    : global_settings_view_(view),
      weak_ptr_factory_(this) {
}

void SettingsPresenterImpl::SaveOrCheckInteger(const char *path, int value,
    BaseSettingsView::ChangeType flag, bool check, BaseSettingsView::ChangeType &result) {
    using namespace bilibase::enum_ops;
    PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();

    if (check) {
        if (pref->GetInteger(path) != value) {
            result |= flag;
        }
    } else {
        pref->SetInteger(path, value);
    }
}

void SettingsPresenterImpl::SaveOrCheckString(const char *path, const std::string &value,
    BaseSettingsView::ChangeType flag, bool check, BaseSettingsView::ChangeType &result) {
    using namespace bilibase::enum_ops;
    PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();

    if (check) {
        if (pref->GetString(path) != value) {
            result |= flag;
        }
    } else {
        pref->SetString(path, value);
    }
}

void SettingsPresenterImpl::SaveOrCheckBool(const char *path, bool value,
    BaseSettingsView::ChangeType flag, bool check, BaseSettingsView::ChangeType &result) {
    using namespace bilibase::enum_ops;
    PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();

    if (check) {
        if (pref->GetBoolean(path) != value) {
            result |= flag;
        }
    } else {
        pref->SetBoolean(path, value);
    }
}

void SettingsPresenterImpl::SaveOrCheckDictionary(const char *path, const base::DictionaryValue &value,
    BaseSettingsView::ChangeType flag, bool check, BaseSettingsView::ChangeType &result) {
    using namespace bilibase::enum_ops;
    PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();

    if (check) {
        if (!pref->GetDictionary(path)->Equals(&value)) {
            result |= flag;
        }
    } else {
        pref->Set(path, value);
    }
}

//SettingFrameView
void SettingsPresenterImpl::DiscardSettings() {
    // 弹幕姬设置返回到上一次保存的状态
    // 不直接操作弹幕姬指针，主程序退出时级联关闭窗口，弹幕姬有可能先于设置框关闭
    base::NotificationService::current()->Notify(
        bililive::NOTIFICATION_LIVEHIME_DANMAKU_RELOADPREF,
        base::NotificationService::AllSources(),
        base::NotificationService::NoDetails());
}

void SettingsPresenterImpl::RequestLivehimeEvent() {
    auto pref = GetBililiveProcess()->profile()->GetPrefs();

    std::vector<std::string> vec_set;
    // Video settings
    vec_set.push_back(base::StringPrintf("bitrate:%d", pref->GetInteger(prefs::kVideoBitRate))); //码率

    std::string fps = "frame_rate:";
    fps.append(pref->GetString(prefs::kVideoFPSCommon));
    vec_set.push_back(fps); //帧率

    std::string output = "live_resolution:";
    output.append(base::IntToString(pref->GetInteger(prefs::kVideoOutputCX)));
    output.append("*");
    output.append(base::IntToString(pref->GetInteger(prefs::kVideoOutputCY)));
    vec_set.push_back(output); //推流分辨率

    vec_set.push_back(base::StringPrintf("performance_balance:%d",
        pref->GetInteger(prefs::kOutputStreamVideoQuality))); // 性能平衡设置

    // Audio settings
    vec_set.push_back(base::StringPrintf("sound_bitrate:%d",
        pref->GetInteger(prefs::kAudioBitRate))); // 音频码率设置

    vec_set.push_back(base::StringPrintf("encoder:%d",
        pref->GetString(prefs::kOutputStreamVideoEncoder) == prefs::kDefaultOutputStreamVideoEncoder ? 1 : 0)); // 编码器选择

    // Audio settings
    auto noise_id = static_cast<DenoiseLevel>(pref->GetInteger(prefs::kAudioDenoise));

    const std::map<DenoiseLevel, int> kNoiseTable
    {
        { DenoiseLevel::NOLEVEL, 0 },
        { DenoiseLevel::LOW, 1 },
        { DenoiseLevel::MIDDLE, 2 },
        { DenoiseLevel::HIGH, 3 }
    };

    vec_set.push_back(base::StringPrintf("denoise:%d", kNoiseTable.at(noise_id)));

    // Video settings
    auto bitrate_control = pref->GetString(prefs::kVideoBitRateControl);
    if (bitrate_control == "CBR") {
        vec_set.push_back(base::StringPrintf("bitrate_control:%d", 1));
    } else if (bitrate_control == "ABR") {
        vec_set.push_back(base::StringPrintf("bitrate_control:%d", 0));
    }

    // DanmakuHime settings
    /*bool split_gift = pref->GetBoolean(prefs::kDanmakuHimeShowGiftView);
    vec_set.push_back(base::StringPrintf("danmuku_screen_seperate:%d", split_gift ? 1 : 0));

    bool quality_checking = pref->GetBoolean(prefs::kDanmakuHimeEnableQualityChecking);
    vec_set.push_back(base::StringPrintf("stream_quality:%d", quality_checking ? 1 : 0));*/

    if (BililiveFeatures::current()->Enabled(BililiveFeatures::TTSDanmaku)) {
        bool danmaku_tts = pref->GetBoolean(prefs::kDanmakuHimeEnableTTS);
        vec_set.push_back(base::StringPrintf("danmuku_speech:%d", danmaku_tts ? 1 : 0));
    }

    bool danmaku_image = pref->GetBoolean(prefs::kDanmakuHimeShowGiftIcon);
    vec_set.push_back(base::StringPrintf("danmuku_image:%d", danmaku_image ? 1 : 0));

    bool danmaku_gift_effects = pref->GetBoolean(prefs::kDanmakuHimeEnableGiftEffects);
    vec_set.push_back(base::StringPrintf("danmaku_gift_effects:%d", danmaku_gift_effects ? 1 : 0));

    bool danmaku_hwa = pref->GetBoolean(prefs::kDanmakuHimeEnableHWA);
    vec_set.push_back(base::StringPrintf("danmaku_hwa:%d", danmaku_hwa ? 1 : 0));

    int dmkhime_theme = pref->GetInteger(prefs::kDanmakuHimeInteractionTheme);
    vec_set.push_back(base::StringPrintf("danmaku_theme:%d", dmkhime_theme));

    int dmkhime_fluency = pref->GetInteger(prefs::kDanmakuHimeFluency);
    vec_set.push_back(base::StringPrintf("danmaku_fluency:%d", dmkhime_fluency));

    std::string event_msg;

    std::string fields = JoinString(vec_set, ";");
    event_msg.append(fields);

    auto secret_core = GetBililiveProcess()->secret_core();
    secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
        secret::LivehimeBehaviorEvent::LivehimeSet, secret_core->account_info().mid(), event_msg).Call();
}

void SettingsPresenterImpl::SettingChanged() {
    // 检查与推流有关的设置是否改变
    auto ss_change_type = BaseSettingsView::TYPE_NONE;
    settings_frame_view_->SaveOrCheckStreamingSettingsChange(true, ss_change_type);

    // 与推流有关的设置有改变
    if (ss_change_type & BaseSettingsView::TYPE_LOCAL ||
        ss_change_type & BaseSettingsView::TYPE_SERVER ||
        ss_change_type & BaseSettingsView::TYPE_CHANGE_TO_AUTO
        )
    {
        if (!settings_frame_view_->CheckExclusive()) {
            settings_frame_view_->SaveNormalSettingsChange();
            return;
        }

        bool recording = bililive::OutputController::GetInstance()->IsRecording();
        bool streaming = bililive::OutputController::GetInstance()->IsStreaming();
        if (recording || streaming)
        {
            LOG(INFO) << "[SC] video setting change when streaming or recording, show restream confirm dialog.";

            if (settings_frame_view_->ShowRestreamingDialog()) {
				if (!settings_frame_view_)
				{
					return;
				}
                auto result = BaseSettingsView::TYPE_NONE;
                settings_frame_view_->SaveNormalSettingsChange();
                settings_frame_view_->SaveOrCheckStreamingSettingsChange(false, result);

                // 服务端埋点
                if (streaming)
                {
                    livehime::BehaviorEventReportViaServer(secret::LivehimeViaServerBehaviorEvent::ChangeDefinition, {});
                }

                auto scene_collection = ApplyStreamingSettings();
                if (scene_collection) {
                    LiveRestartMode mode;
                    if (recording) {
                        mode.SetRecording();
                    }
                    if (streaming) {

                        // 如果是自定义切到自动的，先断流，底层切实断流了再开启测速，测速完了再重推流
                        if (ss_change_type & BaseSettingsView::TYPE_CHANGE_TO_AUTO)
                        {
                            mode.SetStreamUntilSpeedTest();
                        }
                        else
                        {
                            if (ss_change_type & BaseSettingsView::TYPE_LOCAL) {
                                mode.SetJustRestartStream();
                            }
                            if (ss_change_type & BaseSettingsView::TYPE_SERVER) {
                                mode.SetRedetermineStreamAddr();
                            }
                        }

                        bililive::OutputController::GetInstance()->Restart(mode);
                    }

                    scene_collection->Save();
                }
            }
        }
        else
        {
            auto result = BaseSettingsView::TYPE_NONE;
            settings_frame_view_->SaveNormalSettingsChange();
            settings_frame_view_->SaveOrCheckStreamingSettingsChange(false, result);

            auto scene_collection = ApplyStreamingSettings();
            if (scene_collection) {
                OBSProxyService::GetInstance().GetOBSCoreProxy()->UpdateVideoSettings();
                OBSProxyService::GetInstance().GetOBSCoreProxy()->GetOBSOutputService()->ReloadEncoders();
                scene_collection->Save();
            }

            if (ss_change_type & BaseSettingsView::TYPE_CHANGE_TO_AUTO)
            {
                // 从自定义切到自动，确认之后就发起一下测速
            }
        }
    }
    else {
        settings_frame_view_->SaveNormalSettingsChange();

        if (ss_change_type & BaseSettingsView::TYPE_SAVE) {
            auto result = BaseSettingsView::TYPE_NONE;
            settings_frame_view_->SaveOrCheckStreamingSettingsChange(false, result);
            ApplyAudioDeviceSettings();
        }
    }
}

void SettingsPresenterImpl::ApplyAudioDeviceSettings() {
    auto scene_collection = ApplyStreamingSettings();
    if (scene_collection) {
        scene_collection->Save();
    }
}

//AcceleratorSettingView

void SettingsPresenterImpl::AcceleratorInitData() {
    PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();
    std::vector<int> hotkey;

    hotkey.push_back(pref->GetInteger(prefs::kHotkeyMicSwitch));
    hotkey.push_back(pref->GetInteger(prefs::kHotkeySysVolSwitch));
    hotkey.push_back(pref->GetInteger(prefs::kHotkeyScene1));
    hotkey.push_back(pref->GetInteger(prefs::kHotkeyScene2));
    hotkey.push_back(pref->GetInteger(prefs::kHotkeyScene3));
    hotkey.push_back(pref->GetInteger(prefs::kHotkeyLiveSwitch));
    hotkey.push_back(pref->GetInteger(prefs::kHotkeyRecordSwitch));

    if (BililiveFeatures::current()->Enabled(BililiveFeatures::TTSDanmaku)) {
        hotkey.push_back(pref->GetInteger(prefs::kHotkeyClearDanmakuTTSQueue));
    }

    accelerator_settings_view_->OnAcceleratorInitData(hotkey);
}

void SettingsPresenterImpl::AcceleratorSaveChange(const std::map<std::string, int>& hotkey) {
    PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();

    for (auto iter = hotkey.begin(); iter != hotkey.end(); ++iter) {
        pref->SetInteger((iter->first).c_str(), iter->second);
    }

    CommonPrefService::RegisterBililiveHotkey();
}

void SettingsPresenterImpl::UnregisterHotKey(int id) {
    CommonPrefService::UnregisterBililiveHotKey(id);
}

//AudioSettingView
void SettingsPresenterImpl::AudioSaveChange(
    const std::map<std::string, std::string>& device_string,
    const std::map<std::string, int>& device_integer,
    bool check, BaseSettingsView::ChangeType &result) {

    for (auto iter = device_string.begin(); iter != device_string.end(); ++iter) {
        SaveOrCheckString(iter->first.c_str(), iter->second.c_str(),
            BaseSettingsView::ChangeType::TYPE_SAVE, check, result);
    }

    for (auto iter = device_integer.begin(); iter != device_integer.end(); ++iter) {
        SaveOrCheckInteger(iter->first.c_str(), iter->second,
            BaseSettingsView::ChangeType::TYPE_LOCAL, check, result);
    }
}

void SettingsPresenterImpl::InitComboData(const std::string& device) {
    obs_proxy::SceneCollection *scene_collection
        = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    std::vector<obs_proxy::AudioDevice*> vct = scene_collection->GetAudioDevices();

    auto value = std::find_if(vct.begin(), vct.end(), [device](obs_proxy::AudioDevice* dev)->bool { return dev->name() == device; });
    if (value != vct.end()) {
        std::string name;
        const obs_proxy::Properties &props = (*value)->GetProperties();
        for (obs_proxy::Properties::iterator iter = props.begin(); iter != props.end(); ++iter) {
            auto pt = iter->type();
            name = iter->name();//"device_id"
            if ((pt == obs_proxy::PropertyType::List) && (stricmp(name.c_str(), "device_id") == 0)) {
                auto deviceList = GetPropertyStringList(props, name);

                std::wstring devname;
                std::string devid;
                for (unsigned int i = 0; i < deviceList.size(); i++) {
                    devname = std::get<0>(deviceList[i]);
                    devid = std::get<1>(deviceList[i]);

                    audio_settings_view_->OnInitComboData(device, devname, devid);
                }
            }
        }
    }
}

std::string SettingsPresenterImpl::GetCurMicId() const {
    // 音频页直接从obs配置文件获取
    obs_proxy::SceneCollection *scene_collection
        = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    std::vector<obs_proxy::AudioDevice*> vct = scene_collection->GetAudioDevices();
    // 先设置麦克风
    std::string def;
    auto mic = std::find_if(
        vct.begin(), vct.end(),
        [](obs_proxy::AudioDevice* dev)->bool { return dev->name() == obs_proxy::kDefaultInputAudio; });
    if (mic != vct.end())
    {
        // 设备列表
        obs_proxy::PropertyValue &propv = (*mic)->GetPropertyValues();
        propv.Get("device_id", def);
    }
    return def;
}

std::string SettingsPresenterImpl::GetCurSpeakerId() const {
    // 音频页直接从obs配置文件获取
    obs_proxy::SceneCollection *scene_collection
        = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    std::vector<obs_proxy::AudioDevice*> vct = scene_collection->GetAudioDevices();
    // 再设置扬声器
    std::string def;
    auto speaker = std::find_if(
        vct.begin(), vct.end(),
        [](obs_proxy::AudioDevice* dev)->bool { return dev->name() == obs_proxy::kDefaultOutputAudio; });
    if (speaker != vct.end())
    {
        // 设备列表
        obs_proxy::PropertyValue &propv = (*speaker)->GetPropertyValues();
        propv.Get("device_id", def);
    }
    return def;
}

//VideoSettingView
void SettingsPresenterImpl::VideoSaveChange(
    const std::map<std::string, std::string>& device_string,
    const std::map<std::string, int>& device_integer,
    const std::map<std::string, bool>& device_boolean,
    bool check, BaseSettingsView::ChangeType &result) {

    for (auto iter = device_string.begin(); iter != device_string.end(); ++iter) {
        SaveOrCheckString(iter->first.c_str(), iter->second.c_str(),
            BaseSettingsView::ChangeType::TYPE_LOCAL, check, result);
    }

    for (auto iter = device_integer.begin(); iter != device_integer.end(); ++iter) {
        SaveOrCheckInteger(iter->first.c_str(), iter->second,
            BaseSettingsView::ChangeType::TYPE_LOCAL, check, result);
    }

    for (auto iter = device_boolean.begin(); iter != device_boolean.end(); ++iter) {
        SaveOrCheckBool(iter->first.c_str(), iter->second,
            BaseSettingsView::ChangeType::TYPE_LOCAL, check, result);
    }
}

const contracts::MediaPrefsInfo& SettingsPresenterImpl::GetMediaPrefsInfo() const
{
    static contracts::MediaPrefsInfo media_prefs_info;

    // KV下发的有效就用KV下发的，无效就用本地写死的
    media_prefs_info = AppFunctionController::GetInstance()->media_settings();
    return media_prefs_info;
}

// RecordSettingsView
void SettingsPresenterImpl::RecordSaveChange(
    const std::map<std::string, std::string>& device_string,
    const std::map<std::string, int>& device_integer,
    const std::map<std::string, bool>& device_boolean,
    bool check, BaseSettingsView::ChangeType& result)
{
    for (auto iter = device_string.begin(); iter != device_string.end(); ++iter) {
        SaveOrCheckString(iter->first.c_str(), iter->second.c_str(),
            BaseSettingsView::ChangeType::TYPE_LOCAL, check, result);
    }

    for (auto iter = device_integer.begin(); iter != device_integer.end(); ++iter) {
        SaveOrCheckInteger(iter->first.c_str(), iter->second,
            BaseSettingsView::ChangeType::TYPE_LOCAL, check, result);
    }

    for (auto iter = device_boolean.begin(); iter != device_boolean.end(); ++iter) {
        SaveOrCheckBool(iter->first.c_str(), iter->second,
            BaseSettingsView::ChangeType::TYPE_LOCAL, check, result);
    }
}

//DanmakuSettingView
void SettingsPresenterImpl::DanmakuSaveChange(const std::vector<contracts::IntegerPrefsInfo>& integer_prefs,
    const std::vector<contracts::BooleanPrefsInfo>& boolean_prefs,
    const std::vector<contracts::StringPrefsInfo>& string_prefs) {
    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();

    for (unsigned i = 0; i < integer_prefs.size(); i++) {
        pref->SetInteger(integer_prefs[i].path.c_str(), integer_prefs[i].value);
    }

    for (unsigned i = 0; i < boolean_prefs.size(); i++) {
        pref->SetBoolean(boolean_prefs[i].path.c_str(), boolean_prefs[i].value);
    }

    for (unsigned i = 0; i < string_prefs.size(); i++) {
        pref->SetString(string_prefs[i].path.c_str(), string_prefs[i].value);
    }
}

void SettingsPresenterImpl::RequestGetAnchorSwitchStatus() {

}

void SettingsPresenterImpl::RequestUpdateAnchorSwitch(bool switch_val) {

    //没请求回来前return
    if (update_anchor_switch_lock_)
        return;

    std::vector<std::string> switch_keys_open;
    std::vector<std::string> switch_keys_close;
    if (!voice_barrage_switch_) {
        switch_keys_open.push_back("voiceBarrage");
    }
    else {
        switch_keys_close.push_back("voiceBarrage");
    }

    auto secret = GetBililiveProcess()->secret_core();

    auto callback = base::MakeCallable(
        base::Bind(
            &SettingsPresenterImpl::OnUpdateAnchorSwitch,
            weak_ptr_factory_.GetWeakPtr()));

    livehime::PolarisEventReport(
        secret::LivehimePolarisBehaviorEvent::VoiceDanmuSwitchClick,
        "switch:" + std::to_string(switch_val));
    update_anchor_switch_lock_ = true;
}

void SettingsPresenterImpl::OnUpdateAnchorSwitch(bool valid_response, int code) {
    if (valid_response && code == 0) {
        voice_barrage_switch_ = !voice_barrage_switch_;

    }
    update_anchor_switch_lock_ = false;
}

void SettingsPresenterImpl::RequestGetNewRoomSwitch() {

    int area_parent_id = GetBililiveProcess()->bililive_obs()->user_info_service()->GetCurrentAreaParentId();
    int area_id = GetBililiveProcess()->bililive_obs()->user_info_service()->GetCurrentAreaId();
    int64_t uid = GetBililiveProcess()->secret_core()->account_info().mid();

    auto secret = GetBililiveProcess()->secret_core();

    auto callback = base::MakeCallable(
        base::Bind(
            &SettingsPresenterImpl::OnRequestGetNewRoomSwitchRes,
            weak_ptr_factory_.GetWeakPtr()));

}

void SettingsPresenterImpl::OnRequestGetNewRoomSwitchRes(bool valid_response, int code, bool swtich_on) {
    if (valid_response && code == 0) {
        if (global_settings_view_) {
            static_cast<GlobalSettingsView*>(global_settings_view_)->SetNewRoomViewVisible(swtich_on);
        }
    }
}

void SettingsPresenterImpl::GlobalSaveChange(
    bool performance_check,
    bool exclude_from_capture,
    bool drag_full_window_check,
    bool min_radio_check,
    bool remind_choice_check)
{
    PrefService* global_prefs = GetBililiveProcess()->global_profile()->GetPrefs();

    global_prefs->SetBoolean(prefs::kWindowCaptureOptimizeForPerformance, performance_check);
    global_prefs->SetBoolean(prefs::kExcludeMainWindowFromCapture, exclude_from_capture);
    global_prefs->SetBoolean(prefs::kDragFullWindows, !drag_full_window_check);

    if (min_radio_check) {
        global_prefs->SetInteger(prefs::kApplicationExitMode, prefs::kMinimizeToTray);
    } else {
        global_prefs->SetInteger(prefs::kApplicationExitMode, prefs::kExitImmediately);
    }

    global_prefs->SetBoolean(prefs::kApplicationExitRememberChoice, !remind_choice_check);
}
