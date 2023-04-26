#ifndef BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_VOLUME_CONTROLLERS_DEVICES_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_VOLUME_CONTROLLERS_DEVICES_PRESENTER_IMPL_H_

#include "bililive/bililive/livehime/volume/bililive_audio_device_contract.h"
#include "bililive/bililive/livehime/volume/bililive_volume_controller_contract.h"
#include "bililive/bililive/livehime/volume/bililive_volume_controllers_devices_contract.h"

class BililiveVolumeControllersPresenterImpl
    : public contracts::BililiveVolumeControllersContract
{
public:
    BililiveVolumeControllersPresenterImpl();

    ~BililiveVolumeControllersPresenterImpl();

    void SelectController(const string16 &controller_name) override;

    bool ControllerIsValid() override;

    bool GetMuted() override;

    void SetMuted(bool muted) override;

    float GetDB() const override;

    void SetDB(float gain_db) override;

    float GetDeflection() const override;

    void SetDeflection(float value) override;

    string16 GetBoundSourceName() const override;

    void RegisterVolumeChangedHandler(VolumeChangedHandler handler) override;

    void RegisterVolumeLevelUpdatedHandler(VolumeLevelUpdatedHandler handler) override;

    void RegisterVolumeMuteChangedHandler(VolumeMuteChangedHandler handler) override;

    int GetRenderError() const override;
private:
    // static
    static void OnControllerVolumeLevelUpdated(const std::string& source_name, 
        const std::vector<float>& magnitude,
        const std::vector<float>& peak,
        const std::vector<float>& inputPeak);
    static void OnControllerVolumeMuteChanged(const std::string& source_name, bool muted);
    static void OnControllerVolumeChanged(const std::string& source_name, float volume);

private:
    std::unique_ptr<contracts::BililiveVolumeControllerContract> controller_;

    obs_proxy::VolumeController* volume_controller_ = nullptr;
    VolumeLevelUpdatedHandler volume_level_updated_handle_ = nullptr;
    VolumeMuteChangedHandler volume_mute_changed_handle_ = nullptr;
    VolumeChangedHandler volume_changed_handle_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(BililiveVolumeControllersPresenterImpl);
};

class BililiveAudioDevicesPresenterImpl
    : public contracts::BililiveAudioDevicesContract
{
public:
    BililiveAudioDevicesPresenterImpl();

    ~BililiveAudioDevicesPresenterImpl();

    void SelectAudioDevice(const string16 &device_name) override;

    obs_proxy::AudioDevice* GetAudioDevice() const override;

    bool DeviceIsValid() override;

    string16 GetDeviceName() const override;

    uint32_t BoundChannelNumber() const override;

    obs_proxy::Filter* AddNewFilter(obs_proxy::FilterType type, const string16& name) override;

    void RemoveFilter(const string16& name) override;

    obs_proxy::Filter* GetFilter(const string16& name) const override;

    std::vector<obs_proxy::Filter*> GetFilters() const override;

    bool ReorderFilters(const std::vector<obs_proxy::Filter*>& filters) override;

    obs_proxy::PropertyValue& GetPropertyValues() override;

    void UpdatePropertyValues() override;

    const obs_proxy::Properties& GetProperties() const override;

private:
    std::unique_ptr<contracts::BililiveAudioDeviceContract> device_;

    obs_proxy::AudioDevice* audio_device_;

    DISALLOW_COPY_AND_ASSIGN(BililiveAudioDevicesPresenterImpl);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_VOLUME_BILILIVE_VOLUME_CONTROLLERS_DEVICES_PRESENTER_IMPL_H_