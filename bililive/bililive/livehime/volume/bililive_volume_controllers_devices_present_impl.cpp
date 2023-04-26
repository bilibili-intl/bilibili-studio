#include "bililive/bililive/livehime/volume/bililive_volume_controllers_devices_present_impl.h"

#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/livehime/volume/bililive_audio_device_presenter_impl.h"
#include "bililive/bililive/livehime/volume/bililive_volume_controller_presenter_impl.h"

#include <map>


namespace
{
    std::list<BililiveVolumeControllersPresenterImpl*> g_controllers_list;
}


BililiveVolumeControllersPresenterImpl::BililiveVolumeControllersPresenterImpl()
    : controller_(std::make_unique<BililiveVolumeControllerPresenterImpl>())
{
    g_controllers_list.push_back(this);
}

BililiveVolumeControllersPresenterImpl::~BililiveVolumeControllersPresenterImpl()
{
    g_controllers_list.remove(this);
}

void BililiveVolumeControllersPresenterImpl::SelectController(const string16 &controller_name)
{
    if (controller_name == base::UTF8ToUTF16(obs_proxy::kDefaultOutputAudio))
    {
        volume_controller_ = controller_->GetSystemControllerDevice();
    }
    else if (controller_name == base::UTF8ToUTF16(obs_proxy::kDefaultInputAudio))
    {
        volume_controller_ = controller_->GetMicControllerDevice();
    }
    else
    {
        volume_controller_ = nullptr;
    }

    static std::map<string16, bool> only_once_reg_record {
        { base::UTF8ToUTF16(obs_proxy::kDefaultOutputAudio), false },
        { base::UTF8ToUTF16(obs_proxy::kDefaultInputAudio), false }
    };

    if (!volume_controller_)
    {
        return;
    }

    if (only_once_reg_record[controller_name])
    {
        return;
    }

    only_once_reg_record[controller_name] = true;
    volume_controller_->RegisterVolumeLevelUpdatedHandler(OnControllerVolumeLevelUpdated);
    volume_controller_->RegisterVolumeMuteChangedHandler(OnControllerVolumeMuteChanged);
    volume_controller_->RegisterVolumeChangedHandler(OnControllerVolumeChanged);
}

bool BililiveVolumeControllersPresenterImpl::ControllerIsValid()
{
    return volume_controller_ ? true : false;
}

bool BililiveVolumeControllersPresenterImpl::GetMuted()
{
    return volume_controller_->IsMuted();
}

void BililiveVolumeControllersPresenterImpl::SetMuted(bool muted)
{
    volume_controller_->SetMuted(muted);
}

float BililiveVolumeControllersPresenterImpl::GetDB() const
{
    return volume_controller_->GetDB();
}

void BililiveVolumeControllersPresenterImpl::SetDB(float gain_db)
{
    volume_controller_->SetDB(gain_db);
}

float BililiveVolumeControllersPresenterImpl::GetDeflection() const
{
    return volume_controller_->GetDeflection();
}

void BililiveVolumeControllersPresenterImpl::SetDeflection(float value)
{
    volume_controller_->SetDeflection(value);
}

string16 BililiveVolumeControllersPresenterImpl::GetBoundSourceName() const
{
    return base::UTF8ToUTF16(volume_controller_->GetBoundSourceName());
}

void BililiveVolumeControllersPresenterImpl::RegisterVolumeChangedHandler(VolumeChangedHandler handler)
{
    volume_changed_handle_ = handler;
}

void BililiveVolumeControllersPresenterImpl::RegisterVolumeLevelUpdatedHandler(VolumeLevelUpdatedHandler handler)
{
    volume_level_updated_handle_ = handler;
}

void BililiveVolumeControllersPresenterImpl::RegisterVolumeMuteChangedHandler(VolumeMuteChangedHandler handler)
{
    volume_mute_changed_handle_ = handler;
}

int BililiveVolumeControllersPresenterImpl::GetRenderError() const
{
    if (volume_controller_)
    {
        return volume_controller_->GetRenderError();
    }
    return 0;
}

// static
void BililiveVolumeControllersPresenterImpl::OnControllerVolumeLevelUpdated(const std::string& source_name,
    const std::vector<float>& magnitude,
    const std::vector<float>& peak,
    const std::vector<float>& inputPeak)
{
    for (auto &instance : g_controllers_list)
    {
        if (!instance)
        {
            continue;
        }

        if (!instance->volume_level_updated_handle_)
        {
            continue;
        }

        instance->volume_level_updated_handle_(source_name, magnitude, peak, inputPeak);
    }
}

void BililiveVolumeControllersPresenterImpl::OnControllerVolumeMuteChanged(const std::string& source_name, bool muted)
{
    for (auto &instance : g_controllers_list)
    {
        if (instance && instance->volume_mute_changed_handle_)
        {
            instance->volume_mute_changed_handle_(source_name, muted);
        }
    }
}

void BililiveVolumeControllersPresenterImpl::OnControllerVolumeChanged(const std::string& source_name, float volume)
{
    for (auto &instance : g_controllers_list)
    {
        if (instance && instance->volume_changed_handle_)
        {
            instance->volume_changed_handle_(source_name, volume);
        }
    }
}



// BililiveAudioDevicesPresenterImpl
BililiveAudioDevicesPresenterImpl::BililiveAudioDevicesPresenterImpl()
    : device_(std::make_unique<BililiveAudioDeviceImpl>()),
      audio_device_(nullptr)
{}

BililiveAudioDevicesPresenterImpl::~BililiveAudioDevicesPresenterImpl()
{
}

void BililiveAudioDevicesPresenterImpl::SelectAudioDevice(const string16 &device_name)
{
    if (device_name == base::UTF8ToUTF16(obs_proxy::kDefaultOutputAudio))
    {
        audio_device_ = device_->GetSystemAudioDevice();
    }
    else if (device_name == base::UTF8ToUTF16(obs_proxy::kDefaultInputAudio))
    {
        audio_device_ = device_->GetMicAudioDevice();
    }
    else
    {
        audio_device_ = nullptr;
    }
}

obs_proxy::AudioDevice* BililiveAudioDevicesPresenterImpl::GetAudioDevice() const
{
    return audio_device_;
}

bool BililiveAudioDevicesPresenterImpl::DeviceIsValid()
{
    return audio_device_ ? true : false;
}

string16 BililiveAudioDevicesPresenterImpl::GetDeviceName() const
{
    return base::UTF8ToUTF16(audio_device_->name());
}

uint32_t BililiveAudioDevicesPresenterImpl::BoundChannelNumber() const
{
    return audio_device_->bound_channel_number();
}

obs_proxy::Filter* BililiveAudioDevicesPresenterImpl::AddNewFilter(obs_proxy::FilterType type, const string16& name)
{
    return audio_device_->AddNewFilter(type, base::UTF16ToUTF8(name));
}

void BililiveAudioDevicesPresenterImpl::RemoveFilter(const string16& name)
{
    audio_device_->RemoveFilter(base::UTF16ToUTF8(name));
}

obs_proxy::Filter* BililiveAudioDevicesPresenterImpl::GetFilter(const string16& name) const
{
    return audio_device_->GetFilter(base::UTF16ToUTF8(name));
}

std::vector<obs_proxy::Filter*> BililiveAudioDevicesPresenterImpl::GetFilters() const
{
    return audio_device_->GetFilters();
}

bool BililiveAudioDevicesPresenterImpl::ReorderFilters(const std::vector<obs_proxy::Filter*>& filters)
{
    return audio_device_->ReorderFilters(filters);
}

obs_proxy::PropertyValue& BililiveAudioDevicesPresenterImpl::GetPropertyValues()
{
    return audio_device_->GetPropertyValues();
}

void BililiveAudioDevicesPresenterImpl::UpdatePropertyValues()
{
    audio_device_->UpdatePropertyValues();
}

const obs_proxy::Properties& BililiveAudioDevicesPresenterImpl::GetProperties() const
{
    return audio_device_->GetProperties();
}
