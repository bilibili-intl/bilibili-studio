#pragma once

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/ui/views/preview/livehime_obs_preview_control.h"

#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"
#include "ui/views/view.h"

class PreStartLivePreviewControl :
    public LivehimeOBSPreviewControl
{
public:
    PreStartLivePreviewControl(obs_proxy::SceneItem* scene_item);
    virtual ~PreStartLivePreviewControl();
    void StopPreview();

protected:
    gfx::Rect GetViewRect() override;
    obs_source_t* GetRenderSource() override;
    obs_sceneitem_t* GetRenderScenceItem() override;

private:
    std::atomic_bool stop_preview_ = false;
    obs_proxy::SceneItem* scene_item_ = nullptr;
};

class PreStartLiveSourcePreviewView
    : public views::View
{
public:
    PreStartLiveSourcePreviewView(obs_proxy::SceneItem* scene_item);
    virtual ~PreStartLiveSourcePreviewView();
    void StopPreview();

protected:
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    gfx::Size GetPreferredSize() override;

private:
    void InitView();

    obs_proxy::SceneItem*           scene_item_ = nullptr;
    PreStartLivePreviewControl*     obs_preview_control_ = nullptr;
};
