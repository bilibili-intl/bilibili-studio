#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_MEDIA_PROPERTY_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_MEDIA_PROPERTY_PRESENTER_IMPL_H_

#include "bililive/bililive/livehime/sources_properties/source_media_property_contract.h"
#include "bililive/bililive/livehime/sources_properties/source_property_snapshot.h"

#include "obs/obs-studio/libobs/obs.hpp"


class SourceMediaPropertyPresenterImpl
    : public contracts::SourceMediaPropertyPresenter
{
public:
    explicit SourceMediaPropertyPresenterImpl(
        obs_proxy::SceneItem* scene_item,
        contracts::SourceMediaPropertyView* view);
    ~SourceMediaPropertyPresenterImpl();

    bool Initialize() override;

    string16 GetFilePath() override;
    void SetFilePath(const string16& path) override;
    void SetMP4MotionFilePath(const string16& path) override;

    float GetVolumeValue() override;
    void SetVolumeValue(float value) override;

    bool GetIsLoop() override;
    void SetIsLoop(bool value) override;

    void Snapshot() override;
    void Restore() override;

    bool IsMuted() override;
    void SetMuted(bool muted) override;

private:
    void RegisterSignals(const obs_source_t* source);
    static void OnMediaSignal(void* data, calldata_t* params);

    OBSSignal media_signal_;
    MediaSceneItemHelper media_scene_item_;
    std::unique_ptr<livehime::MediaPropertySnapshot> snapshot_;
    contracts::SourceMediaPropertyView* view_;
    std::shared_ptr<int> canary_;

    DISALLOW_COPY_AND_ASSIGN(SourceMediaPropertyPresenterImpl);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_MEDIA_PROPERTY_PRESENTER_IMPL_H_