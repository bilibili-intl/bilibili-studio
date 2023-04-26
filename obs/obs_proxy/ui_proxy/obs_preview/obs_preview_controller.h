#ifndef BILILIVE_OBS_OBS_PROXY_UI_PROXY_OBS_PREVIEW_OBS_PREVIEW_DELEGATE_H_
#define BILILIVE_OBS_OBS_PROXY_UI_PROXY_OBS_PREVIEW_OBS_PREVIEW_DELEGATE_H_

#include "ui/gfx/rect.h"

#include "obs/obs-studio/libobs/obs.h"

namespace obs_proxy_ui {

class OBSPreview;

class OBSPreviewController
{
public:
    OBSPreviewController();
    virtual ~OBSPreviewController();

    virtual HWND GetNativeHWND() = 0;
    virtual gfx::Rect GetViewRect() = 0;
    virtual OBSPreview* obs_preview() const = 0;

    virtual obs_source_t* GetRenderSource() { return nullptr; }
    virtual obs_sceneitem_t* GetRenderScenceItem() { return nullptr; }
};

} //namespace obs_proxy_ui

#endif // BILILIVE_OBS_OBS_PROXY_UI_PROXY_OBS_PREVIEW_OBS_PREVIEW_DELEGATE_H_