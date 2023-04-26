#include "bililive/bililive/ui/views/livehime/danmaku_hime/visions/danmaku_vision.h"

#include "bililive/bililive/ui/views/livehime/danmaku_hime/sculptor.h"


namespace dmkhime {

DanmakuVision::DanmakuVision()
    : position_(-1) {
}

void DanmakuVision::SetPosition(int pos) {
    position_ = pos;
}

void DanmakuVision::SetRenderer(const DanmakuRendererPtr& dp) {
    renderer_ = dp;
}

void DanmakuVision::SetRenderingParams(const DanmakuRenderingParams& params) {
    rendering_params_ = params;
}

void DanmakuVision::Invalid() {
    renderer_->Invalid();
}

int DanmakuVision::GetPosition() const {
    return position_;
}

const DanmakuRendererPtr& DanmakuVision::GetRenderer() const {
    return renderer_;
}

bool DanmakuVision::IsClickable(const gfx::Point& p) const {

    if (renderer_->GetType() == DRT_COMMON_NOTICE_DANMAKU || renderer_->GetType() == DRT_ANCHOR_BROADCAST ||
        renderer_->GetType() == DRT_VOICE_LINK_NOTICE)
    {
        return renderer_->HitTest(p);
    }

    return false;
}

bool DanmakuVision::OnMousePressed(
    const ui::MouseEvent& event, DmkHitType* type, const DmkDataObject** data)
{

    auto func = [&](bool pressed) {
        if (is_pressed_) {
            switch (renderer_->GetType()) {
            case DRT_COMMON:
                *type = DmkHitType::Danmaku;
                *data = renderer_->GetData();
                break;
            case DRT_DETAILED_SUPER_CHAT:
                *type = DmkHitType::SuperChat;
                *data = renderer_->GetData();
                break;
            case DRT_COMMON_NOTICE_DANMAKU:
                *type = DmkHitType::CommonNoticeDanmakuBtn;
                *data = renderer_->GetData();
                break;
            case DRT_ANCHOR_BROADCAST:
                *type = DmkHitType::LiveAsistantDanmakuBtn;
                *data = renderer_->GetData();
                break;
            default:
                break;
            }
        }
    };

    is_pressed_ = renderer_->HitTest(event.location());
    if (event.IsRightMouseButton()) {
        is_pressed_ = true;
    }
    func(is_pressed_);

    return is_pressed_;
}

void DanmakuVision::OnMouseReleased(
    const ui::MouseEvent& event, DmkHitType* type, const DmkDataObject** data)
{
    if (is_pressed_) {
        is_pressed_ = false;

        switch (renderer_->GetType()) {
        case DRT_COMMON:
            *type = DmkHitType::Danmaku;
            *data = renderer_->GetData();
            break;
        case DRT_DETAILED_SUPER_CHAT:
            *type = DmkHitType::SuperChat;
            *data = renderer_->GetData();
            break;
        case DRT_ANCHOR_BROADCAST:
            *type = DmkHitType::LiveAsistantDanmakuBtn;
            *data = renderer_->GetData();
            break;
        case DRT_COMMON_NOTICE_DANMAKU:
            *type = DmkHitType::CommonNoticeDanmakuBtn;
            *data = renderer_->GetData();
            break;
		case DRT_VOICE_LINK_NOTICE:
			*type = DmkHitType::VoiceLinkNoticeDanmakuBtn;
			*data = renderer_->GetData();
			break;
        default:
            break;
        }
    }
}

void DanmakuVision::OnMouseCaptureLost() {
    if (is_pressed_) {
        is_pressed_ = false;
    }
}

void DanmakuVision::OnMeasure(int width, int height) {
    renderer_->Refactor(rendering_params_.show_icon);
    renderer_->Measure(width, rendering_params_.base_font_size);
    SetMeasuredSize(renderer_->GetWidth(), renderer_->GetHeight());
}

void DanmakuVision::OnLayout(int left, int top, int right, int bottom) {
    // 什么也不做
}

void DanmakuVision::OnSizeChanged(int width, int height, int old_w, int old_h) {
    // 什么也不做
}

void DanmakuVision::OnDraw(Sculptor* s) {
    Vision::OnDraw(s);

    renderer_->Refactor(rendering_params_.show_icon);
    renderer_->Measure(renderer_->GetWidth(), rendering_params_.base_font_size);
    renderer_->Render(
        rendering_params_.opacity,
        rendering_params_.theme,
        rendering_params_.hardware_acceleration_);

    if (renderer_->GetBitmap()) {
        s->DrawBitmap(renderer_->GetBitmap(), GetLeft(), GetTop());
    }
}

}
