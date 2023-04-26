#ifndef BILILIVE_BILILIVE_LIVEHIME_VOLUME_SOUND_EFFECT_PROPERTY_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_VOLUME_SOUND_EFFECT_PROPERTY_PRESENTER_IMPL_H_

#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/livehime/volume/sound_effect_property_contract.h"
#include "bililive/bililive/livehime/volume/sound_effect_property_snapshot.h"

#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"


class SoundEffectPropertyPresenterImpl
    : public contracts::SoundEffectPropertyPresenter
{
public:
    SoundEffectPropertyPresenterImpl();

    ~SoundEffectPropertyPresenterImpl(){}

    void UpdateFilters() override;

    void StartPreview() override;
    void EndPreview() override;

    bool GetIsReverbEnabled() override;
    void SetIsReverbEnabled(bool value) override;

    void SetAllValueToDefault() override;

    float GetRoomSize() override;
    void SetRoomSize(float value) override;

    float GetDamping() override;
    void SetDamping(float value) override;

    float GetWetLevel() override;
    void SetWetLevel(float value) override;

    float GetDryLevel() override;
    void SetDryLevel(float val) override;

    float GetWidth() override;
    void SetWidth(float value) override;

    float GetFreezeMode() override;
    void SetFreezeMode(float value) override;

    long long GetDenoiseLevel() override;
    void SetDenoiseLevel(int device_integer) override;
    
    int GetMonoStatus() override;
    void SetMonoStatus(const std::string& device_string) override;

    void AudioSettingSaveOrCancel(const std::pair<std::string, int>* device_integer,
        const std::pair<std::string, std::string>* device_string) override;

    void Snapshot() override;
    void Restore() override;

    static obs_proxy::SceneCollection* ApplyStreamingSettings();

    static std::vector<DeviceInfo> GetDeviceList(const std::string& device_name);

    static const MediaSettings& GetMediaPrefsInfo();

    static std::string GetCurMicId();

    static std::string GetCurSpeakerId();

private:
    std::unique_ptr<MicphoneDeviceHelper> micphone_helper_;
    std::unique_ptr<livehime::MicphonePropertySnapshot> snapshot_;

    DISALLOW_COPY_AND_ASSIGN(SoundEffectPropertyPresenterImpl);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_VOLUME_SOUND_EFFECT_PROPERTY_PRESENTER_IMPL_H_