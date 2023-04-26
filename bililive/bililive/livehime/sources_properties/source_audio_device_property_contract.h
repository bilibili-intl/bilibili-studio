#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_AUDIO_DEVICE_PROPERTY_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_AUDIO_DEVICE_PROPERTY_CONTRACT_H_

#include "bililive/bililive/livehime/sources_properties/source_public_property_presenter.h"

namespace contracts {

    class SourceAudioDevicePropertyPresenter
        :public SourcePublicPropertyPresenter
    {
    public:
        explicit SourceAudioDevicePropertyPresenter(obs_proxy::SceneItem* scene_item)
            : SourcePublicPropertyPresenter(scene_item) {}

        ~SourceAudioDevicePropertyPresenter() {}

        virtual float GetVolumeValue() = 0;

        virtual void SetVolumeValue(float value) = 0;

        virtual bool IsMuted() = 0;

        virtual void SetMuted(bool muted) = 0;

        virtual void InitComboData(const std::string& device) = 0;

        virtual std::string GetSelectedAudioDevice() = 0;
        virtual void SetSelectedAudioDevice(std::string val) = 0;

        virtual bool IsUseDeviceTiming() = 0;
        virtual void SetUseDeviceTiming(bool use) = 0;

        virtual int GetAudioDeviceMonitor() = 0;
        virtual void SetAudioDeviceMonitor(int status) = 0;
    };

    class AudioDevicePropDetailView {
    public:
        virtual ~AudioDevicePropDetailView() = default;

        virtual void OnInitComboData(
            const std::string& device,
            const std::wstring& devname,
            const std::string& devid) = 0;
    };
}  // namespace contracts

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_AUDIO_DEVICE_PROPERTY_CONTRACT_H_