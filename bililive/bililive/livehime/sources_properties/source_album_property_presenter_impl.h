#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_ALBUM_PROPERTY_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_ALBUM_PROPERTY_PRESENTER_IMPL_H_

#include "bililive/bililive/livehime/sources_properties/source_album_property_contract.h"
#include "bililive/bililive/livehime/sources_properties/source_property_snapshot.h"

class SourceAlbumPropertyPresenterImpl
    : public contracts::SourceAlbumPropertyPresenter
{
public:
    explicit SourceAlbumPropertyPresenterImpl(obs_proxy::SceneItem* scene_item);

    virtual ~SourceAlbumPropertyPresenterImpl() {}

    float GetSlideTime() override;
    void SetSlideTime(float value) override;

    float GetTransitionTime() override;
    void SetTransitionTime(float value) override;

    SlideShowSceneItemHelper::TransitionT GetTransition() override;
    void SetTransition(SlideShowSceneItemHelper::TransitionT value) override;

    bool GetRandom() override;
    void SetRandom(bool value) override;

    bool GetLoop() override;
    void SetLoop(bool value) override;

    std::vector<std::wstring> GetFileList() override;
    void SetFileList() override;

    void Snapshot() override;
    void Restore() override;

    void AddFile(const string16& file_path) override;
    void RemoveFile(int index) override;
    void EmptyFileList() override;

private:
    SlideShowSceneItemHelper slide_show_item_;
    std::unique_ptr<livehime::AlbumPropertySnapshot> snapshot_;

    std::vector<string16> file_list_;

    DISALLOW_COPY_AND_ASSIGN(SourceAlbumPropertyPresenterImpl);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_ALBUM_PROPERTY_PRESENTER_IMPL_H_