#ifndef LIVEHIME_PK_COMMON_CTRLS_H_
#define LIVEHIME_PK_COMMON_CTRLS_H_

#include "bililive/bililive/ui/views/controls/bililive_sequence_image_button.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_svga_delegate.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_svga_image_view.h"


// ƥ���ж�Ч�����ã�
class MatchingMotionView : public BililiveSequenceImageButton
{
public:
    MatchingMotionView();

    void ResetDegree();

protected:
    void OnSequenceMotionStart() override;

    void OnSequenceMotionStop() override;

    // View
    void OnPaint(gfx::Canvas* canvas) override;

private:
    float degree_;
    bool is_animating_;
};


// ����ʱ�������Զ����أ�����PK�ã�
class ReadyGoMotionView : public BililiveSequenceImageButton
{
public:
    ReadyGoMotionView();

protected:
    // BililiveSequenceImageDelegate
    void OnSequenceMotionStop() override;

    // views::View
    bool HitTestRect(const gfx::Rect& rect) const override { return false; }
};


class LivehimeImageView;

class SVGABaseImageView : public BililiveSingleChildShowContainerView {
public:
    SVGABaseImageView();

    LivehimeImageView* base_img() const { return base_img_view_; }
    LivehimeSVGAImageView* svga_img() const { return svga_img_view_; }

protected:
    // views::View
    bool HitTestRect(const gfx::Rect& rect) const override { return false; }

private:
    LivehimeImageView* base_img_view_ = nullptr;
    LivehimeSVGAImageView* svga_img_view_ = nullptr;
};


// PK�������22/33ʤ��ƽ��Чͼ�����ã�
class PkResultMotionView : public SVGABaseImageView
{
public:
    PkResultMotionView();

    // View
    gfx::Size GetPreferredSize() override;
};


// ����ʱ�������Զ����أ����Ҷ��ã�
class SVGAReadyGoMotionView :
    public LivehimeSVGAImageView, public LivehimeSVGADelegate
{
public:
    SVGAReadyGoMotionView();

protected:
    // LivehimeSVGADelegate
    void onSVGAFinished(LivehimeSVGAImageView* v) override;
};


#endif  // LIVEHIME_PK_COMMON_CTRLS_H_
