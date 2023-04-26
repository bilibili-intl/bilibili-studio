#include "bililive/bililive/livehime/volume/sound_effect_property_presenter_impl.h"

#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/volume/bililive_volume_controllers_devices_contract.h"
#include "bililive/bililive/livehime/volume/bililive_volume_controllers_devices_present_impl.h"
#include "bililive/public/bililive/bililive_process.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"
#include "sound_effect_property_presenter_impl.h"


namespace {
    const int kDenoiseSliderId = 1013;

    const int kAudioNoMixRadioButtonId = 10031;
    const int kAudioLToDualRadioButtonId = 10032;
    const int kAudioRToDualRadioButtonId = 10033;
    const int kAudioMixRadioButtonId = 10034;
}

using DeviceInfo = SoundEffectPropertyPresenterImpl::DeviceInfo;

SoundEffectPropertyPresenterImpl::SoundEffectPropertyPresenterImpl()
{
    std::unique_ptr<contracts::BililiveAudioDevicesContract> audio_device(std::make_unique<BililiveAudioDevicesPresenterImpl>());
    audio_device->SelectAudioDevice(base::UTF8ToUTF16(obs_proxy::kDefaultInputAudio));

    if (audio_device->DeviceIsValid())
    {
        micphone_helper_ = std::make_unique<MicphoneDeviceHelper>(audio_device->GetAudioDevice());
    }

    DCHECK(micphone_helper_);
}

void SoundEffectPropertyPresenterImpl::UpdateFilters()
{
    micphone_helper_->UpdateFilters();
}

void SoundEffectPropertyPresenterImpl::StartPreview()
{
    micphone_helper_->StartPreview();
}

void SoundEffectPropertyPresenterImpl::EndPreview()
{
    micphone_helper_->EndPreview();
}

bool SoundEffectPropertyPresenterImpl::GetIsReverbEnabled()
{
    return micphone_helper_->IsReverbEnabled();
}

void SoundEffectPropertyPresenterImpl::SetIsReverbEnabled(bool value)
{
    micphone_helper_->IsReverbEnabled(value);
    UpdateFilters();
}

void SoundEffectPropertyPresenterImpl::SetAllValueToDefault()
{
    micphone_helper_->Rb_SetDefault();
}

float SoundEffectPropertyPresenterImpl::GetRoomSize()
{
    return static_cast<float>(micphone_helper_->Rb_RoomSize());
}

void SoundEffectPropertyPresenterImpl::SetRoomSize(float value)
{
    micphone_helper_->Rb_RoomSize(static_cast<double>(value));
}

float SoundEffectPropertyPresenterImpl::GetDamping()
{
    return static_cast<float>(micphone_helper_->Rb_Damping());
}

void SoundEffectPropertyPresenterImpl::SetDamping(float value)
{
    micphone_helper_->Rb_Damping(static_cast<double>(value));
}

float SoundEffectPropertyPresenterImpl::GetWetLevel()
{
    return static_cast<float>(micphone_helper_->Rb_WetLevel());
}

void SoundEffectPropertyPresenterImpl::SetWetLevel(float value)
{
    micphone_helper_->Rb_WetLevel(static_cast<double>(value));
}

float SoundEffectPropertyPresenterImpl::GetDryLevel()
{
    return static_cast<float>(micphone_helper_->Rb_DryLevel());
}

void SoundEffectPropertyPresenterImpl::SetDryLevel(float value)
{
    micphone_helper_->Rb_DryLevel(static_cast<double>(value));
}

float SoundEffectPropertyPresenterImpl::GetWidth()
{
    return static_cast<float>(micphone_helper_->Rb_Width());
}

void SoundEffectPropertyPresenterImpl::SetWidth(float value)
{
    micphone_helper_->Rb_Width(static_cast<double>(value));
}

float SoundEffectPropertyPresenterImpl::GetFreezeMode()
{
    return static_cast<float>(micphone_helper_->Rb_FreezeMode());
}

void SoundEffectPropertyPresenterImpl::SetFreezeMode(float value)
{
    micphone_helper_->Rb_FreezeMode(static_cast<double>(value));
}

long long SoundEffectPropertyPresenterImpl::GetDenoiseLevel()
{
    long long nDenoise = 0;
    // 音频页直接从obs配置文件获取
    obs_proxy::SceneCollection *scene_collection
        = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    std::vector<obs_proxy::AudioDevice*> vct = scene_collection->GetAudioDevices();
    
    auto mic = std::find_if(vct.begin(), vct.end(), [](obs_proxy::AudioDevice* dev)->bool 
    { return dev->name() == obs_proxy::kDefaultInputAudio; });
    if (mic != vct.end())
    {
        // 降噪
        obs_proxy::Filter *noise = (*mic)->GetFilter(prefs::kAudioFilterDenoiser);
        if (noise)
        {
            // 有filter就认为选中了，属性为空
            obs_proxy::PropertyValue &propv = noise->GetPropertyValues();
            propv.Get(prefs::kFilterNoiseSuppressLevel, nDenoise);
        }
    }

    return nDenoise;
}

void SoundEffectPropertyPresenterImpl::SetDenoiseLevel(int device_integer)
{
    obs_proxy::SceneCollection* scene_collection
        = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    if (!scene_collection) {
        return;
    }
    std::vector<obs_proxy::AudioDevice*> vct = scene_collection->GetAudioDevices();

    // 先获取麦克风
    auto mic = std::find_if(vct.begin(), vct.end(), [](obs_proxy::AudioDevice* dev)-> bool {
        return dev->name() == obs_proxy::kDefaultInputAudio; });
    if (mic != vct.end()) {
        // 降噪filter
        if (device_integer == 0) {
            (*mic)->RemoveFilter(prefs::kAudioFilterDenoiser);
        }
        else {
            obs_proxy::Filter* filter = (*mic)->GetFilter(prefs::kAudioFilterDenoiser);
            if (!filter) {
                filter = (*mic)->AddNewFilter(
                    obs_proxy::FilterType::NoiseSuppress, prefs::kAudioFilterDenoiser);
            }
            if (filter) {
                filter->GetPropertyValues().Set(
                    prefs::kFilterNoiseSuppressLevel, static_cast<long long>(device_integer));
                filter->UpdatePropertyValues();
            }
        }
    }
}


int SoundEffectPropertyPresenterImpl::GetMonoStatus()
{
    int nChannel = kAudioNoMixRadioButtonId;
    // 音频页直接从obs配置文件获取
    obs_proxy::SceneCollection *scene_collection
        = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    std::vector<obs_proxy::AudioDevice*> vct = scene_collection->GetAudioDevices();

    auto mic = std::find_if(vct.begin(), vct.end(), [](obs_proxy::AudioDevice* dev)->bool
    { return dev->name() == obs_proxy::kDefaultInputAudio; });
    if (mic != vct.end())
    {
        // 声道
        obs_proxy::Filter *mono = (*mic)->GetFilter(prefs::kAudioFilterMono);
        if (mono)
        {
            obs_proxy::PropertyValue& f_propv = mono->GetPropertyValues();
            std::string mode;
            f_propv.Get("mode", mode);
            if (mode.compare(prefs::kAudioMonoNoMix) == 0)
            {
                nChannel = kAudioNoMixRadioButtonId;
            }
            if (mode.compare(prefs::kAudioMonoLToDouble) == 0)
            {
                nChannel = kAudioLToDualRadioButtonId;
            }
            if (mode.compare(prefs::kAudioMonoRToDouble) == 0)
            {
                nChannel = kAudioRToDualRadioButtonId;
            }
            if (mode.compare(prefs::kAudioMonoMix) == 0)
            {
                nChannel = kAudioMixRadioButtonId;
            }
        }
    }
    
    return nChannel;
}

void SoundEffectPropertyPresenterImpl::SetMonoStatus(const std::string& device_string)
{
    obs_proxy::SceneCollection* scene_collection
        = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    if (!scene_collection) {
        return;
    }
    std::vector<obs_proxy::AudioDevice*> vct = scene_collection->GetAudioDevices();

    // 先获取麦克风
    auto mic = std::find_if(vct.begin(), vct.end(), [](obs_proxy::AudioDevice* dev)-> bool {
        return dev->name() == obs_proxy::kDefaultInputAudio; });
    if (mic != vct.end()) {

        // 单声道应用filter
        if (device_string.empty() || device_string == prefs::kAudioMonoNoMix) {
            (*mic)->RemoveFilter(prefs::kAudioFilterMono);
        }
        else {
            obs_proxy::Filter* filter = (*mic)->GetFilter(prefs::kAudioFilterMono);
            if (!filter) {
                filter = (*mic)->AddNewFilter(obs_proxy::FilterType::ForceMono, prefs::kAudioFilterMono);
            }
            if (filter) {
                obs_proxy::PropertyValue& f_propv = filter->GetPropertyValues();
                f_propv.Set("mode", device_string);
                filter->UpdatePropertyValues();
            }
        }
    }
}


void SoundEffectPropertyPresenterImpl::AudioSettingSaveOrCancel(const std::pair<std::string, int>* device_integer,
    const std::pair<std::string, std::string>* device_string)
{
    PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();
    if (!pref)
    {
        return;
    }
    if (device_integer && device_string)
    {
        pref->SetInteger(device_integer->first.c_str(), device_integer->second);
        pref->SetString(device_string->first.c_str(), device_string->second);
    }

    obs_proxy::SceneCollection* scene_collection
        = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    if (!scene_collection) {
        return;
    }
    std::vector<obs_proxy::AudioDevice*> vct = scene_collection->GetAudioDevices();

    // 先获取麦克风
    auto mic = std::find_if(vct.begin(), vct.end(), [](obs_proxy::AudioDevice* dev)-> bool {
        return dev->name() == obs_proxy::kDefaultInputAudio; });
    if (mic != vct.end()) {
        // 降噪filter
        int denoise = pref->GetInteger(prefs::kAudioDenoise);

        if (denoise == 0) {
            (*mic)->RemoveFilter(prefs::kAudioFilterDenoiser);
        }
        else {
            obs_proxy::Filter* filter = (*mic)->GetFilter(prefs::kAudioFilterDenoiser);
            if (!filter) {
                filter = (*mic)->AddNewFilter(
                    obs_proxy::FilterType::NoiseSuppress, prefs::kAudioFilterDenoiser);
            }
            if (filter) {
                filter->GetPropertyValues().Set(
                    prefs::kFilterNoiseSuppressLevel, static_cast<long long>(denoise));
                filter->UpdatePropertyValues();
            }
        }

        // 单声道应用filter
        std::string mode = pref->GetString(prefs::kAudioMonoChannelSetup);
        if (mode.empty() || mode == prefs::kAudioMonoNoMix) {
            (*mic)->RemoveFilter(prefs::kAudioFilterMono);
        }
        else {
            obs_proxy::Filter* filter = (*mic)->GetFilter(prefs::kAudioFilterMono);
            if (!filter) {
                filter = (*mic)->AddNewFilter(obs_proxy::FilterType::ForceMono, prefs::kAudioFilterMono);
            }
            if (filter) {
                obs_proxy::PropertyValue& f_propv = filter->GetPropertyValues();
                f_propv.Set("mode", mode);
                filter->UpdatePropertyValues();
            }
        }
    }
}


void SoundEffectPropertyPresenterImpl::Snapshot()
{
    snapshot_ = livehime::MicphonePropertySnapshot::NewTake(micphone_helper_.get());
}

void SoundEffectPropertyPresenterImpl::Restore()
{
    DCHECK(snapshot_);

    snapshot_->Restore();
}

std::vector<DeviceInfo> SoundEffectPropertyPresenterImpl::GetDeviceList(const std::string& device_name)
{
    std::vector<DeviceInfo> ret;

    obs_proxy::SceneCollection* scene_collection
        = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    std::vector<obs_proxy::AudioDevice*> vct = scene_collection->GetAudioDevices();

    auto value = std::find_if(vct.begin(), vct.end(), [device_name](obs_proxy::AudioDevice* dev)->bool { return dev->name() == device_name; });
    if (value != vct.end()) {
        std::string name;
        const obs_proxy::Properties& props = (*value)->GetProperties();
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

                    ret.push_back({ devname, devid });
                }
            }
        }
    }

    return ret;
}

const MediaSettings& SoundEffectPropertyPresenterImpl::GetMediaPrefsInfo()
{
    static MediaSettings media_prefs_info;

    // KV下发的有效就用KV下发的，无效就用本地写死的
    media_prefs_info = AppFunctionController::GetInstance()->media_settings();
    return media_prefs_info;
}

std::string SoundEffectPropertyPresenterImpl::GetCurMicId() 
{
    // 音频页直接从obs配置文件获取
    obs_proxy::SceneCollection* scene_collection
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
        obs_proxy::PropertyValue& propv = (*mic)->GetPropertyValues();
        propv.Get("device_id", def);
    }
    return def;
}

std::string SoundEffectPropertyPresenterImpl::GetCurSpeakerId()
{
    // 音频页直接从obs配置文件获取
    obs_proxy::SceneCollection* scene_collection
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
        obs_proxy::PropertyValue& propv = (*speaker)->GetPropertyValues();
        propv.Get("device_id", def);
    }
    return def;
}

obs_proxy::SceneCollection* SoundEffectPropertyPresenterImpl::ApplyStreamingSettings() 
{
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
