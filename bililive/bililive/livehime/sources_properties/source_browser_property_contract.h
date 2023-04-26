#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_BROWSER_PROPERTY_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_BROWSER_PROPERTY_CONTRACT_H_

#include "bililive/bililive/livehime/sources_properties/source_public_property_presenter.h"

namespace contracts {

    class SourceBrowserPropertyPresenter
        : public SourcePublicPropertyPresenter
    {
    public:
        explicit SourceBrowserPropertyPresenter(obs_proxy::SceneItem* scene_item)
            : SourcePublicPropertyPresenter(scene_item){}

        ~SourceBrowserPropertyPresenter() {}

        virtual void SetUrl(const std::string& url) = 0;
        virtual std::string GetUrl() = 0;

        virtual void SetCSS(const std::string& css) = 0;
        virtual std::string GetCSS() = 0;

        virtual void SetWidth(int width) = 0;
        virtual int GetWidth() = 0;

        virtual void SetHeight(int height) = 0;
        virtual int GetHeight() = 0;

        virtual void Refresh() = 0;

        virtual void Shutdown() = 0;

        virtual void SetRestartWhenActive(bool use) = 0;
        virtual bool GetRestartWhenActive() = 0;

        virtual float GetVolumeValue() = 0;
        virtual void SetVolumeValue(float value) = 0;

        virtual bool IsMuted() = 0;
        virtual void SetMuted(bool muted) = 0;
    };

}   // namespace contracts

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_BROWSER_PROPERTY_CONTRACT_H_