#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_AUDIO_DEVICE_PROPERTY_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_AUDIO_DEVICE_PROPERTY_PRESENTER_IMPL_H_

#include "bililive/bililive/livehime/sources_properties/source_audio_device_property_contract.h"
#include "bililive/bililive/livehime/sources_properties/source_property_snapshot.h"
#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"

#include "obs/obs-studio/libobs/obs.hpp"


class SourceAudioDevicePropertyPresenterImpl
    : public contracts::SourceAudioDevicePropertyPresenter
{
public:
    SourceAudioDevicePropertyPresenterImpl(obs_proxy::SceneItem* scene_item,contracts::AudioDevicePropDetailView* audio_device_prop_view);

    ~SourceAudioDevicePropertyPresenterImpl();

    float GetVolumeValue() override;

    void SetVolumeValue(float value) override;

    bool IsMuted() override;

    void SetMuted(bool muted) override;

    void Snapshot() override;

    void Restore() override;

    void InitComboData(const std::string& device) override;

    std::string GetSelectedAudioDevice() override;
    void SetSelectedAudioDevice(std::string val) override;

    bool IsUseDeviceTiming() override;
    void SetUseDeviceTiming(bool use) override;

    int GetAudioDeviceMonitor() override;
    void SetAudioDeviceMonitor(int status) override;

private:
    AudioDeviceItemHelper audio_device_scene_item_;

    std::unique_ptr<livehime::AudioDevicePropertySnapshot> snapshot_;

    contracts::AudioDevicePropDetailView* audio_device_prop_view_;

    DISALLOW_COPY_AND_ASSIGN(SourceAudioDevicePropertyPresenterImpl);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_AUDIO_DEVICE_PROPERTY_PRESENTER_IMPL_H_