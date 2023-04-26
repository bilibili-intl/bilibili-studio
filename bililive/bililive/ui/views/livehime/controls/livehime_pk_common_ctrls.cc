#include "livehime_pk_common_ctrls.h"
#include "livehime_theme_common.h"

#include "ui/views/layout/fill_layout.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"


// ReadyGoMotionView
ReadyGoMotionView::ReadyGoMotionView()
    : BililiveSequenceImageButton(nullptr)
{
}

void ReadyGoMotionView::OnSequenceMotionStop()
{
    SetVisible(false);
}


// SVGAReadyGoMotionView
SVGAReadyGoMotionView::SVGAReadyGoMotionView() {
    setLoopCount(0);
}

void SVGAReadyGoMotionView::onSVGAFinished(LivehimeSVGAImageView* v) {
    SetVisible(false);
}


// SVGABaseImageView
SVGABaseImageView::SVGABaseImageView() {
    base_img_view_ = new LivehimeImageView();
    svga_img_view_ = new LivehimeSVGAImageView();

    AddChildView(svga_img_view_);
    AddChildView(base_img_view_);
}


// PkResultMotionView
PkResultMotionView::PkResultMotionView() {
    base_img()->SetImageHoriAlign(LivehimeImageView::CENTER);
    base_img()->SetImageVertAlign(LivehimeImageView::TRAILING);
    base_img()->SetImageBoundSize(true, GetSizeByDPIScale(gfx::Size(83, 42)));
    svga_img()->setClearAfterStop(true);
    svga_img()->setLoopCount(-1);
    svga_img()->setPreferredSize(GetSizeByDPIScale(gfx::Size(170, 170)));
}

gfx::Size PkResultMotionView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.Enlarge(0, GetLengthByImgDPIScale(21));
    return size;
}


// MatchingMotionView
MatchingMotionView::MatchingMotionView()
    : BililiveSequenceImageButton(nullptr),
    degree_(0),
    is_animating_(false)
{

}

void MatchingMotionView::ResetDegree()
{
    degree_ = 0;
}

void MatchingMotionView::OnSequenceMotionStart()
{
    __super::OnSequenceMotionStart();
    is_animating_ = true;
}

void MatchingMotionView::OnSequenceMotionStop()
{
    __super::OnSequenceMotionStop();
    is_animating_ = false;
}

void MatchingMotionView::OnPaint(gfx::Canvas* canvas)
{
    if (is_animating_)
    {
        SkScalar dx = SkFloatToScalar(width() * 0.5f);
        SkScalar dy = SkFloatToScalar(height() * 0.5f);
        SkCanvas *sk_canvas = canvas->sk_canvas();
        sk_canvas->translate(dx, dy);
        sk_canvas->rotate(degree_);
        sk_canvas->translate(-dx, -dy);
    }

    __super::OnPaint(canvas);

    if (is_animating_)
    {
        degree_ += 14.4f;
        if (degree_ > 360)
        {
            degree_ = 0;
        }
    }
}
