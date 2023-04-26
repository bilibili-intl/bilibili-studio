#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_PUBLIC_PROPERTY_PRESENTER_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_PUBLIC_PROPERTY_PRESENTER_H_

#include "base/strings/string16.h"

#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"

class SourcePublicPropertyPresenter {
public:
    virtual ~SourcePublicPropertyPresenter();

    virtual string16 GetName();

    virtual bool SetName(const string16& name);

    virtual void Update();

    virtual void UpdateFilters();

    virtual obs_proxy::SceneItem* GetSceneItem() const;

    virtual void Snapshot() {}
    virtual void Restore() {}

protected:
    explicit SourcePublicPropertyPresenter(obs_proxy::SceneItem* scene_item);

private:
    class Impl;
    std::unique_ptr<Impl> presenter_;

    DISALLOW_COPY_AND_ASSIGN(SourcePublicPropertyPresenter);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_SOURCES_PROPERTIES_SOURCE_PUBLIC_PROPERTY_PRESENTER_H_