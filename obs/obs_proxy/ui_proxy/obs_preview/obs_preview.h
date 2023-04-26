#ifndef BILILIVE_OBS_OBS_PROXY_UI_PROXY_OBS_PREVIEW_OBS_PREVIEW_H_
#define BILILIVE_OBS_OBS_PROXY_UI_PROXY_OBS_PREVIEW_OBS_PREVIEW_H_

#include "bilibase/basic_macros.h"

#include "obs/obs_proxy/public/proxy/obs_proxy.h"
#include "obs/obs_proxy/ui_proxy/obs_preview/obs_preview_mouse_handle.h"

#include "ui/base/events/event.h"
#include "ui/gfx/rect.h"

#include "obs_obj_ptr.hpp"


//#if defined(_MSC_VER)
///* alignment warning - despite the fact that alignment is already fixed */
//#pragma warning (disable : 4316)
//#endif
namespace obs_proxy_ui {

class OBSPreviewController;

class OBSPreview
    : public OBSPreviewMouseHandle
{
public:
    OBSPreview(OBSPreviewController *controller, PreviewType type);
    virtual ~OBSPreview();

    void CreateDisplay();
    void ResetSourcesPos(obs_sceneitem_t* si, obs_video_info& old_ovi);
    void SetClearOnly(bool clear_only);

    void GetBackgroundColor(float *r, float *g, float *b);
    void SetBackgroundColor(float r, float g, float b);

    // override from OBSPreviewMouseHandle
    int GetHTComponent(const gfx::Point &location) override;

    // 重新计算实际渲染区在display中的范围，便于设置绘制viewport
    // 针对场景，需要重算发生于：
    // 1、控件尺寸发生变化时，即BoundsChanged
    // 2、推流分辨率发生变化时
    // 针对单一源，重算发生于：
    // 1、控件尺寸发生变化时，即BoundsChanged
    // 2、源的尺寸发生变化时（当前暂未有在源预览时还能改变其尺寸的交互，所以这一case不会发生，2019.02.22-v3.3）
    gfx::Size RecalculateDisplayViewport();

    OBSPreviewController *obs_preview_controller() { return controller_; }
    PreviewType type() const { return preview_type_; }

    // call by LivehimeOBSPreviewControl
    void OnOBSPreviewControlSizeChanged(const gfx::Rect& bounds);
    void OnOBSPreviewControlDestorying();
    void OnOBSPreviewControlMouseEvent(ui::MouseEvent *event);
    void OnOBSPreviewControlMouseWheelEvent(ui::MouseWheelEvent *event);
    void OnOBSPreviewControlKeyEvent(ui::KeyEvent *event);
    void OnOBSPreviewControlFocusEvent(bool focus);
    void OnOBSPreviewControlVisibility(bool visible);
    OBSObjPtr<OBSDisplay> GetMainDisplay() const
    {
        return display_;
    }

private:
    void InitGraphicPrimitives();
    void AddDisplayRender();
    void DrawBackdrop(float cx, float cy);
    void DrawSceneEditing();
    gfx::Size DetermineLogicRenderSize();

    // obs callbacks
    static void Render(void *data, uint32_t cx, uint32_t cy);

    static bool DrawSelectedItem(obs_scene_t *scene, obs_sceneitem_t *item, void *param);

    // PREVIEW_MAIN
    void OnMainPreviewMouseEvent(ui::MouseEvent *event);
    void OnMainPreviewKeyEvent(ui::KeyEvent *event);

    // PREVIEW_SOURCE
    bool GetSourceRelativeXY(int mouseX, int mouseY, int &relX, int &relY);
    void OnSourcePreviewMouseEvent(ui::MouseEvent *event);
    void OnSourcePreviewMouseWheelEvent(ui::MouseWheelEvent *event);
    void OnSourcePreviewKeyEvent(ui::KeyEvent *event);
    void OnSourcePreviewFocusEvent(bool focus);

private:
    OBSPreviewController *controller_;

    OBSObjPtr<OBSDisplay> display_;

    float background_color_[3] = { 0.8666f, 0.8823f, 0.8941f, };

    gs_vertbuffer_t* box_ = nullptr;
    gs_vertbuffer_t* box_left_ = nullptr;
    gs_vertbuffer_t* box_top_ = nullptr;
    gs_vertbuffer_t* box_right_ = nullptr;
    gs_vertbuffer_t* box_bottom_ = nullptr;
    gs_vertbuffer_t* circle_ = nullptr;

    bool is_bind_render_ = false;
    /**
     * 是否允许连麦场景可交互。
     * 目前是禁用的状态，若要启用，需要先解决跨线程访问 SceneCollection 的
     * 问题。
     */
    std::atomic_bool should_render_;
    std::atomic_bool should_clear_only_;

    DISABLE_COPY(OBSPreview);
};

}//namespace obs_proxy_ui

#endif // BILILIVE_OBS_OBS_PROXY_UI_PROXY_OBS_PREVIEW_OBS_PREVIEW_H_