#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_MEDIA_PROPERTY_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_MEDIA_PROPERTY_CONTRACT_H_

#include "bililive/bililive/livehime/sources_properties/source_public_property_presenter.h"

namespace contracts
{

// 注意跟 obs-source.h 中的 obs_media_state 保持一致
enum class MediaState {
    None,
    Playing,
    Opening,
    Buffered,
    Paused,
    Stopped,
    Ended,
    Error,
};

class SourceMediaPropertyView {
public:
    virtual ~SourceMediaPropertyView() {}

    virtual void OnMediaStateChanged(MediaState state) = 0;
};

class SourceMediaPropertyPresenter
    : public SourcePublicPropertyPresenter
{
public:
    explicit SourceMediaPropertyPresenter(obs_proxy::SceneItem* scene_item)
        : SourcePublicPropertyPresenter(scene_item){}

    ~SourceMediaPropertyPresenter(){}

    virtual bool Initialize() = 0;

    virtual string16 GetFilePath() = 0;
    virtual void SetFilePath(const string16& path) = 0;
    virtual void SetMP4MotionFilePath(const string16& path) = 0;

    virtual float GetVolumeValue() = 0;
    virtual void SetVolumeValue(float value) = 0;

    virtual bool GetIsLoop() = 0;
    virtual void SetIsLoop(bool value) = 0;

	virtual bool IsMuted() = 0;
	virtual void SetMuted(bool muted) = 0;
};

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_MEDIA_PROPERTY_CONTRACT_H_