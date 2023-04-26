#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_ALBUM_PROPERTY_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_ALBUM_PROPERTY_CONTRACT_H_

#include "bililive/bililive/livehime/sources_properties/source_public_property_presenter.h"

namespace contracts {

class SourceAlbumPropertyPresenter
    : public SourcePublicPropertyPresenter
{
public:
    virtual ~SourceAlbumPropertyPresenter() {}

    virtual float GetSlideTime() = 0;
    virtual void SetSlideTime(float value) = 0;

    virtual float GetTransitionTime() = 0;
    virtual void SetTransitionTime(float value) = 0;

    virtual SlideShowSceneItemHelper::TransitionT GetTransition() = 0;
    virtual void SetTransition(SlideShowSceneItemHelper::TransitionT value) = 0;

    virtual bool GetRandom() = 0;
    virtual void SetRandom(bool value) = 0;

    virtual bool GetLoop() = 0;
    virtual void SetLoop(bool value) = 0;

    virtual std::vector<std::wstring> GetFileList() = 0;
    virtual void SetFileList() = 0;

    virtual void AddFile(const string16& file_path) = 0;
    virtual void RemoveFile(int index) = 0;
    virtual void EmptyFileList() = 0;

protected:
    explicit SourceAlbumPropertyPresenter(obs_proxy::SceneItem* scene_item)
        : SourcePublicPropertyPresenter(scene_item){}
};

}   // namespace contracts

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_ALBUM_PROPERTY_CONTRACT_H_