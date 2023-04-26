#include "pre_start_live_source_preview_view.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "ui/views/layout/fill_layout.h"

#include "obs/obs_proxy/core_proxy/scene_collection/obs_sources_wrapper_impl.h"
#include "obs/obs_proxy/public/proxy/obs_proxy.h"
#include "obs/obs_proxy/ui_proxy/obs_preview/obs_preview.h"


PreStartLivePreviewControl::PreStartLivePreviewControl(obs_proxy::SceneItem* scene_item) :
    LivehimeOBSPreviewControl(obs_proxy_ui::PreviewType::PREVIEW_SOURCE),
    scene_item_(scene_item)
{
}

PreStartLivePreviewControl::~PreStartLivePreviewControl()
{
}

gfx::Rect PreStartLivePreviewControl::GetViewRect()
{
    gfx::Size view_port_size;

    if (parent())
    {
        view_port_size = parent()->bounds().size();
    }
    else
    {
        view_port_size = GetSizeByDPIScale({ 640, 360 });
    }

    return { 0, 0, view_port_size.width(), view_port_size.height() };
}

obs_source_t* PreStartLivePreviewControl::GetRenderSource()
{
    if (stop_preview_)
    {
        return nullptr;
    }
    return static_cast<obs_proxy::SceneItemImpl*>(scene_item_)->AsSource();
}

obs_sceneitem_t* PreStartLivePreviewControl::GetRenderScenceItem()
{
    if (stop_preview_)
    {
        return nullptr;
    }

    return static_cast<obs_proxy::SceneItemImpl*>(scene_item_)->LeakUnderlyingSource();
}

void PreStartLivePreviewControl::StopPreview()
{
    stop_preview_ = true;
    scene_item_ = nullptr;
}


PreStartLiveSourcePreviewView::PreStartLiveSourcePreviewView(obs_proxy::SceneItem* scene_item) :
    scene_item_(scene_item)
{
}

PreStartLiveSourcePreviewView::~PreStartLiveSourcePreviewView()
{
}

void PreStartLiveSourcePreviewView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this && details.is_add)
    {
        InitView();
    }
}

gfx::Size PreStartLiveSourcePreviewView::GetPreferredSize()
{
    return GetSizeByDPIScale({ 533, 300 });
}

void PreStartLiveSourcePreviewView::InitView()
{
    SetLayoutManager(new views::FillLayout());

    obs_preview_control_ = new PreStartLivePreviewControl(scene_item_);
    AddChildView(obs_preview_control_);
    obs_preview_control_->SetVisible(true);

    // rgb(20, 21, 29)
    float r = (float)20 / (float)255;
    float g = (float)21 / (float)255;
    float b = (float)29 / (float)255;
    obs_preview_control_->SetBackgroundColor(r, g, b);
}

void PreStartLiveSourcePreviewView::StopPreview()
{
    if (obs_preview_control_)
    {
        obs_preview_control_->StopPreview();
    }
    scene_item_ = nullptr;
}

