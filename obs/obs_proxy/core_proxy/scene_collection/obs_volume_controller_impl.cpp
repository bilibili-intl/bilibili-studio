#include "obs/obs_proxy/core_proxy/scene_collection/obs_volume_controller_impl.h"
#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"
#include "obs/obs_proxy/common/obs_proxy_constants.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"
#include "obs/obs_proxy/public/proxy/obs_source_property_values.h"

#include "bililive/public/bililive/bililive_thread.h"


namespace {

int64 kVolumeLevelUpdatedMilliseconds = 100;
const char* kGainFilterPropertyDB = "db";
using VolumeLevelUpdatedHandler = obs_proxy::VolumeController::VolumeLevelUpdatedHandler;

}   // namespace

namespace obs_proxy {

VolumeControllerImpl::VolumeControllerImpl(obs_source_t* source_to_bind, const std::string& source_name)
    : bound_audio_source_(source_to_bind), bound_source_name_(source_name),
      fader_(obs_fader_create(OBS_FADER_CUBIC)), volmeter_(obs_volmeter_create(OBS_FADER_LOG)),
      weak_ptr_factory_(this), weak_bound_(weak_ptr_factory_.GetWeakPtr())
{
    // Volume-changed and volume-level-updated events are often raised extremely frequently, thus causing
    // pending a slew of notification calls in UI message loop. A window may exist between the time the
    // volume controller instance, associated with a media source, gets destroyed, and the time pending
    // calls are processed.
    // Therefore, we need a weak bind identification to check if the instance was destroyed.
    //obs_fader_add_callback(fader_, &OnVolumeChanged, &weak_bound_);

    obs_volmeter_add_callback(volmeter_, &OnVolumeLevelUpdated, this);

    volume_mute_changed_signal_handler_.second.Connect(obs_source_get_signal_handler(bound_audio_source_),
                                                       kSignalMute,
                                                       &OnVolumeMuteChanged,
                                                       &weak_bound_);

    volume_changed_signal_handler_.second.Connect(obs_source_get_signal_handler(bound_audio_source_),
                                                  kSignalVolume,
                                                  &OnVolumeChanged,
                                                  &weak_bound_);

    obs_fader_attach_source(fader_, bound_audio_source_);
    obs_volmeter_attach_source(volmeter_, bound_audio_source_);
}

VolumeControllerImpl::~VolumeControllerImpl()
{
    obs_volmeter_remove_callback(volmeter_, &OnVolumeLevelUpdated, this);

    //obs_fader_remove_callback(fader_, &OnVolumeChanged, &weak_bound_);

    volume_mute_changed_signal_handler_.second.Disconnect();
    volume_changed_signal_handler_.second.Disconnect();
    obs_volmeter_detach_source(volmeter_);
    obs_fader_detach_source(fader_);

    obs_volmeter_destroy(volmeter_);
    obs_fader_destroy(fader_);
}

void VolumeControllerImpl::RegisterVolumeChangedHandler(VolumeChangedHandler handler)
{
    //volume_changed_handler_ = handler;
    volume_changed_signal_handler_.first = handler;
}

void VolumeControllerImpl::RegisterVolumeLevelUpdatedHandler(VolumeLevelUpdatedHandler handler)
{
    volume_level_updated_handler_ = handler;
}

void VolumeControllerImpl::RegisterVolumeMuteChangedHandler(VolumeMuteChangedHandler handler)
{
    volume_mute_changed_signal_handler_.first = handler;
}

bool VolumeControllerImpl::IsMuted() const
{
    return obs_source_muted(bound_audio_source_);
}

void VolumeControllerImpl::SetMuted(bool muted)
{
    if (IsMuted() != muted)
    {
        obs_source_set_muted(bound_audio_source_, muted);
    }
}

float VolumeControllerImpl::GetDB() const
{
    //return obs_fader_get_db(fader_);

    double db = 0;
    auto scene_collection = obs_proxy::GetCoreProxy()->GetCurrentSceneCollection();
    obs_proxy::AudioDevice *audio_dev = static_cast<obs_proxy::AudioDevice *>(
        scene_collection->GetAudioSourceForVolumeController(const_cast<VolumeControllerImpl*>(this)));
    if (audio_dev)
    {
        std::vector<obs_proxy::Filter*> filters = audio_dev->GetFilters();
        for (auto filter : filters)
        {
            if (filter->type() == obs_proxy::FilterType::Gain)
            {
                obs_proxy::PropertyValue &prop_val = filter->GetPropertyValues();
                prop_val.Get(kGainFilterPropertyDB, db);
                break;
            }
        }
    }
    else
    {
        db = obs_fader_get_db(fader_);
    }
    return db;
}

void VolumeControllerImpl::SetDB(float gain_db)
{
    auto scene_collection = obs_proxy::GetCoreProxy()->GetCurrentSceneCollection();
    obs_proxy::AudioDevice *audio_dev = static_cast<obs_proxy::AudioDevice *>(
        scene_collection->GetAudioSourceForVolumeController(const_cast<VolumeControllerImpl*>(this)));
    if (audio_dev)
    {
        std::vector<obs_proxy::Filter*> filters = audio_dev->GetFilters();
        for (auto filter : filters)
        {
            if (filter->type() == obs_proxy::FilterType::Gain)
            {
                obs_proxy::PropertyValue &prop_val = filter->GetPropertyValues();
                prop_val.Set(kGainFilterPropertyDB, gain_db);
                filter->UpdatePropertyValues();
            }
        }
    }
}

float VolumeControllerImpl::GetDeflection() const
{
    return obs_fader_get_deflection(fader_);
}

void VolumeControllerImpl::SetDeflection(float value)
{
    obs_fader_set_deflection(fader_, value);
}

int VolumeControllerImpl::GetRenderError() const
{
    obs_data_t* property_values = obs_source_get_settings(bound_audio_source_);
    if (property_values)
    {
        int val = obs_data_get_int(property_values, "audio_render_error");
        obs_data_release(property_values);
        return val;
    }
    return 0;
}

void VolumeControllerImpl::SetMonitoringType(int status)
{
    obs_monitoring_type type = OBS_MONITORING_TYPE_NONE;
    switch (status)
    {
    case 0:
        type = OBS_MONITORING_TYPE_NONE;
        break;
    case 1:
        type = OBS_MONITORING_TYPE_MONITOR_ONLY;
        break;
    case 2:
        type = OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT;
        break;
    default:
        break;
    }
    if (bound_audio_source_)
    {
        obs_source_set_monitoring_type(bound_audio_source_, type);
    }
}

int VolumeControllerImpl::GetMonitoringType() const
{
    int ret = 0;
    if (bound_audio_source_)
    {
        obs_monitoring_type type = obs_source_get_monitoring_type(bound_audio_source_);
        switch (type)
        {
        case OBS_MONITORING_TYPE_NONE:
            ret = 0;
            break;
        case OBS_MONITORING_TYPE_MONITOR_ONLY:
            ret = 1;
            break;
        case OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT:
            ret = 2;
            break;
        default:
            break;
        }
    }
    return ret;
}

const std::string& VolumeControllerImpl::GetBoundSourceName() const
{
    return bound_source_name_;
}

// static
void VolumeControllerImpl::OnVolumeMuteChanged(void* data, calldata_t* calldata)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    auto& instance = *static_cast<base::WeakPtr<VolumeControllerImpl>*>(data);
    bool muted = calldata_bool(calldata, "muted");
    if (instance && instance->volume_mute_changed_signal_handler_.first)
    {
        instance->volume_mute_changed_signal_handler_.first(instance->GetBoundSourceName(), muted);
    }
}

void VolumeControllerImpl::OnVolumeChanged(void* data, calldata_t* calldata)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    auto& instance = *static_cast<base::WeakPtr<VolumeControllerImpl>*>(data);
    float volume = calldata_float(calldata, "volume");
    if (instance && instance->volume_changed_signal_handler_.first)
    {
        instance->volume_changed_signal_handler_.first(instance->GetBoundSourceName(), volume);
    }
}

void VolumeControllerImpl::OnVolumeLevelUpdated(void* data,
                                                const float magnitude[MAX_AUDIO_CHANNELS],
                                                const float peak[MAX_AUDIO_CHANNELS],
                                                const float inputPeak[MAX_AUDIO_CHANNELS])
{
    VolumeControllerImpl* instance = reinterpret_cast<VolumeControllerImpl*>(data);
    DCHECK(instance);
    if (instance)
    {
        base::Time curTime = base::Time::Now();
        if ((curTime - instance->last_volume_updated_time_).InMilliseconds() >= kVolumeLevelUpdatedMilliseconds)
        {
            std::vector<float> vct_magnitude(magnitude, magnitude + MAX_AUDIO_CHANNELS);
            std::vector<float> vct_peak(peak, peak + MAX_AUDIO_CHANNELS);
            std::vector<float> vct_inputPeak(inputPeak, inputPeak + MAX_AUDIO_CHANNELS);

            BililiveThread::PostTask(BililiveThread::UI,
                                     FROM_HERE,
                                     base::Bind(&VolumeControllerImpl::NotifyVolumeLevelUpdated,
                                                instance->weak_bound_,
                                                vct_magnitude,
                                                vct_peak,
                                                vct_inputPeak));

            instance->last_volume_updated_time_ = curTime;
        }
    }
}

void VolumeControllerImpl::NotifyVolumeLevelUpdated(const base::WeakPtr<VolumeControllerImpl>& instance,
    const std::vector<float>& magnitude,
    const std::vector<float>& peak,
    const std::vector<float>& inputPeak)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    if (instance)
    {
        auto& handler = instance->volume_level_updated_handler_;
        if (handler)
        {
            handler(instance->GetBoundSourceName(), magnitude, peak, inputPeak);
        }
    }
}

}   // namespace obs_proxy