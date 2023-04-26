#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_BROWSER_PROPERTY_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_BROWSER_PROPERTY_PRESENTER_IMPL_H_

#include "bililive/bililive/livehime/sources_properties/source_property_snapshot.h"
#include "bililive/bililive/livehime/sources_properties/source_browser_property_contract.h"

class SourceBrowserPropertyPresenterImpl
    : public contracts::SourceBrowserPropertyPresenter
{
public:
    explicit SourceBrowserPropertyPresenterImpl(obs_proxy::SceneItem* scene_item);

    ~SourceBrowserPropertyPresenterImpl() {}

    void SetUrl(const std::string& url) override;
    std::string GetUrl() override;

    void SetCSS(const std::string& css) override;
    std::string GetCSS() override;

    void SetWidth(int width) override;
    int GetWidth() override;

    void SetHeight(int height) override;
    int GetHeight() override;

    void SetRestartWhenActive(bool use) override;
    bool GetRestartWhenActive() override;

    void Refresh() override;
    void Shutdown() override;

    void Snapshot() override;
    void Restore() override;

    float GetVolumeValue() override;
    void SetVolumeValue(float value) override;

    bool IsMuted() override;
    void SetMuted(bool muted) override;


private:
    BrowserItemHelper browser_scene_item_;
    std::unique_ptr<livehime::BrowserPropertySnapshot> snapshot_;

    DISALLOW_COPY_AND_ASSIGN(SourceBrowserPropertyPresenterImpl);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_BROWSER_PROPERTY_PRESENTER_IMPL_H_