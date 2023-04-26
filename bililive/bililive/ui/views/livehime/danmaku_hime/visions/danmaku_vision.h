#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISIONS_DANMAKU_VISION_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISIONS_DANMAKU_VISION_H_

#include "ui/base/events/event.h"

#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/vision.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_renderer.h"


namespace dmkhime {

class DanmakuRenderer;

// 表示一条弹幕
class DanmakuVision : public Vision {
public:
    DanmakuVision();

    void SetPosition(int pos);
    void SetRenderer(const DanmakuRendererPtr& dp);
    void SetRenderingParams(const DanmakuRenderingParams& params);
    void Invalid();

    int GetPosition() const;
    const DanmakuRendererPtr& GetRenderer() const;

    bool IsClickable(const gfx::Point& p) const;
    bool OnMousePressed(
        const ui::MouseEvent& event, DmkHitType* type, const DmkDataObject** data);
    void OnMouseReleased(
        const ui::MouseEvent& event, DmkHitType* type, const DmkDataObject** data);
    void OnMouseCaptureLost();

protected:
    void OnMeasure(int width, int height) override;
    void OnLayout(int left, int top, int right, int bottom) override;
    void OnSizeChanged(int width, int height, int old_w, int old_h) override;
    void OnDraw(Sculptor* s) override;

private:
    int position_;
    bool is_pressed_ = false;
    //for test
    bool update_bitmaps = true;
    DanmakuRendererPtr renderer_;
    DanmakuRenderingParams rendering_params_;
};

}

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_VISIONS_DANMAKU_VISION_H_