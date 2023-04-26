#include "bililive/bililive/ui/views/livehime/controls/livehime_svga_image_view.h"

#include "ui/base/resource/resource_bundle.h"

#include "bililive/bililive/livehime/svga_player/svga_parser.h"
#include "bililive/bililive/livehime/svga_player/entities/svga_video_entity.h"
#include "bililive/bililive/livehime/svga_player/svga_drawable.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_svga_delegate.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/common/bililive_logging.h"
#include "bililive/public/bililive/bililive_thread.h"


LivehimeSVGAImageView::LivehimeSVGAImageView()
    : parser_wpf_(this),
      loop_wpf_(this)
{
    //set_background(views::Background::CreateSolidBackground(0, 0, 0, 255));
}

LivehimeSVGAImageView::~LivehimeSVGAImageView() {}

void LivehimeSVGAImageView::setPreferredSize(const gfx::Size& size) {
    is_using_pref_size_ = true;
    preferred_size_ = size;
}

void LivehimeSVGAImageView::setDelegate(LivehimeSVGADelegate* d) {
    delegate_ = d;
}

void LivehimeSVGAImageView::setSVGAData(const std::string& data, const string16& cache_path) {
    parser_wpf_.InvalidateWeakPtrs();

    auto item_ptr = std::make_shared<VideoItemPtr>();
    BililiveThread::PostTaskAndReply(
        BililiveThread::FILE, FROM_HERE,
        base::Bind(&LivehimeSVGAImageView::ParseSVGADataOnWordThread, data, cache_path, item_ptr),
        base::Bind(&LivehimeSVGAImageView::OnSVGADataParsed, parser_wpf_.GetWeakPtr(), item_ptr));
}

void LivehimeSVGAImageView::setSVGAFilePath(const string16& file_path, const string16& cache_path) {
    parser_wpf_.InvalidateWeakPtrs();

    auto item_ptr = std::make_shared<VideoItemPtr>();
    BililiveThread::PostTaskAndReply(
        BililiveThread::FILE, FROM_HERE,
        base::Bind(&LivehimeSVGAImageView::ParseSVGAFileOnWordThread, file_path, cache_path, item_ptr),
        base::Bind(&LivehimeSVGAImageView::OnSVGADataParsed, parser_wpf_.GetWeakPtr(), item_ptr));
}

void LivehimeSVGAImageView::setSVGAResourceId(int id) {
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();
    auto svga_data = rb.GetRawDataResource(id).as_string();

    auto dir_path = bililive::GetBililiveUserDataDirectory();
    if (!dir_path.empty()) {
        dir_path = dir_path.Append(L"Cache").Append(L"Motions").Append(std::to_wstring(id));
    }

    setSVGAData(svga_data, dir_path.value());
}

void LivehimeSVGAImageView::setLoopCount(int loop_count) {
    loops_ = loop_count;
}

void LivehimeSVGAImageView::setClearAfterStop(bool clear) {
    clears_after_stop_ = clear;
}

void LivehimeSVGAImageView::setAntialias(bool antialias) {
    is_antialias_ = antialias;
    if (drawable_) {
        drawable_->video_item_->antialias_ = antialias;
        SchedulePaint();
    }
}

void LivehimeSVGAImageView::setAutoPlay(bool auto_play) {
    is_auto_play_ = auto_play;
}

void LivehimeSVGAImageView::setScaleType(svga::ScaleType type) {
    scale_type_ = type;
    if (drawable_) {
        drawable_->setScaleType(type);
        SchedulePaint();
    }
}

void LivehimeSVGAImageView::startAnimation() {
    startAnimation({}, false);
}

void LivehimeSVGAImageView::startAnimation(const Range& range, bool reverse) {
    if (!is_paused_) {
        loop_wpf_.InvalidateWeakPtrs();
        if (animation_ && animation_->is_animating()) {
            cur_anim_val_ = 0;
            animation_->Stop();
            animation_->set_delegate(nullptr);
        }
    }
    if (!drawable_) {
        return;
    }

    drawable_->setCleared(false);
    drawable_->setScaleType(scale_type_);
    {
        auto it = drawable_->video_item_;

        int start_frame = std::max(0, range.location);
        int end_frame = std::min(it->frames_ - 1, range.location + range.length - 1);

        start_frame_ = start_frame;
        end_frame_ = end_frame;

        if (!is_paused_) {
            cur_repeat_count_ = 0;
            is_reverse_ = reverse;
        }

        bool is_resumed = is_paused_;

        is_animating_ = true;
        is_paused_ = false;

        animation_.reset(new ui::LinearAnimation(it->fps_, this));
        animation_->SetDuration((end_frame - start_frame + 1) * (1000.f / it->fps_));
        animation_->Start();
        animation_->SetCurrentValue(cur_anim_val_);

        if (delegate_) {
            delegate_->onSVGAStart(this, is_resumed);
        }
    }
}

void LivehimeSVGAImageView::pauseAnimation() {
    loop_wpf_.InvalidateWeakPtrs();

    if (animation_) {
        is_paused_ = true;
        if (animation_->is_animating()) {
            cur_anim_val_ = animation_->GetCurrentValue();
            animation_->Stop();
        } else if (is_animating_) {
            cur_anim_val_ = 0;
            is_animating_ = false;
            if (delegate_) {
                delegate_->onSVGAPause(this);
            }
        }
    }
}

void LivehimeSVGAImageView::stopAnimation() {
    stopAnimation(clears_after_stop_);
}

void LivehimeSVGAImageView::stopAnimation(bool clear) {
    cur_anim_val_ = 0;
    loop_wpf_.InvalidateWeakPtrs();

    if (animation_) {
        if (is_paused_) {
            if (delegate_) {
                delegate_->onSVGAFinished(this);
            }
            is_paused_ = false;
        } else {
            if (animation_->is_animating()) {
                animation_->Stop();
            } else if (is_animating_) {
                is_animating_ = false;
                if (delegate_) {
                    delegate_->onSVGAFinished(this);
                }
            }
        }
        animation_->set_delegate(nullptr);
    }

    if (drawable_) {
        if (drawable_->setCleared(clear)) {
            SchedulePaint();
        }
    }
}

void LivehimeSVGAImageView::stepToFrame(int frame, bool and_play) {
    pauseAnimation();
    if (!drawable_) {
        return;
    }

    frame = std::min(drawable_->video_item_->frames_ - 1, frame);
    frame = std::max(0, frame);

    drawable_->setCleared(false);
    if (drawable_->setCurrentFrame(frame)) {
        SchedulePaint();
    }

    int total_frame = drawable_->video_item_->frames_;
    cur_anim_val_ = std::max(0.0, std::min(1.0, double(frame + 1) / total_frame));

    if (delegate_) {
        delegate_->onSVGAStep(this, false, frame, total_frame, cur_anim_val_);
    }

    if (and_play) {
        startAnimation();
    }
}

void LivehimeSVGAImageView::stepToPercentage(double percentage, bool and_play) {
    if (!drawable_) {
        return;
    }

    int frame = drawable_->video_item_->frames_ * percentage;
    if (frame >= drawable_->video_item_->frames_ && frame > 0) {
        frame = drawable_->video_item_->frames_ - 1;
    }
    stepToFrame(frame, and_play);
}

void LivehimeSVGAImageView::prepareToPercentage(bool enabled, double percentage) {
    is_pbp_enabled_ = enabled;
    if (enabled) {
        percent_before_parsed_ = std::min(1.0, std::max(0.0, percentage));
    }
}

void LivehimeSVGAImageView::stopParsing() {
    parser_wpf_.InvalidateWeakPtrs();
}

int LivehimeSVGAImageView::getLoops() const {
    return loops_;
}

int LivehimeSVGAImageView::getFrameCount() const {
    if (drawable_) {
        return drawable_->video_item_->frames_;
    }
    return 0;
}

int LivehimeSVGAImageView::getCurFrame() const {
    if (drawable_) {
        return drawable_->getCurrentFrame();
    }
    return -1;
}

gfx::Size LivehimeSVGAImageView::getVideoSize() const {
    if (drawable_) {
        gfx::SizeF size = drawable_->video_item_->video_size_.size();
        return gfx::Size(size.width(), size.height());
    }
    return {};
}

bool LivehimeSVGAImageView::isAntialias() const {
    return is_antialias_;
}

bool LivehimeSVGAImageView::isAnimating() const {
    return is_animating_;
}

bool LivehimeSVGAImageView::isLoadingSucceeded() const {
    return drawable_ != nullptr;
}

void LivehimeSVGAImageView::setAlpha(uint8_t alpha) {
    alpha_ = alpha;
}

uint8_t LivehimeSVGAImageView::getAlpha() const {
    return alpha_;
}

void LivehimeSVGAImageView::setVideoItem(const svga::SVGAVideoEntity& video_item) {
    setVideoItem(video_item, {});
}

void LivehimeSVGAImageView::setVideoItem(
    const svga::SVGAVideoEntity& video_item, const svga::SVGADynamicEntity& dynamic_item)
{
    drawable_.reset(new svga::SVGADrawable(
        std::make_shared<svga::SVGAVideoEntity>(video_item),
        std::make_shared<svga::SVGADynamicEntity>(dynamic_item)));
    drawable_->setCleared(clears_after_stop_);
    drawable_->setScaleType(scale_type_);

    InvalidateLayout();
    Layout();
    SchedulePaint();
}

gfx::Size LivehimeSVGAImageView::GetPreferredSize() {
    if (is_using_pref_size_) {
        return preferred_size_;
    }
    gfx::Size size;
    if (drawable_) {
        auto rect = drawable_->video_item_->video_size_;
        size.SetSize(rect.width(), rect.height());
    }
    return size;
}

void LivehimeSVGAImageView::OnPaint(gfx::Canvas* canvas) {
    __super::OnPaint(canvas);

    if (drawable_) {
        if (alpha_ != 255) {
            canvas->SaveLayerAlpha(alpha_);
            drawable_->draw(canvas, GetContentsBounds().size());
            canvas->Restore();
        } else {
            drawable_->draw(canvas, GetContentsBounds().size());
        }
    }
}

void LivehimeSVGAImageView::AnimationEnded(const ui::Animation* animation) {
    if (!is_paused_ && (loops_ == -1 || (loops_ > 0 && cur_repeat_count_ < loops_))) {
        BililiveThread::PostTask(
            BililiveThread::UI, FROM_HERE,
            Bind(&LivehimeSVGAImageView::OnPostAnimationEnd, loop_wpf_.GetWeakPtr()));
    } else {
        ProcessStop();
    }
}

void LivehimeSVGAImageView::AnimationProgressed(const ui::Animation* animation) {
    int cur_frame;
    if (is_reverse_) {
        cur_frame = (1 - animation->GetCurrentValue()) * (end_frame_ - start_frame_) + start_frame_;
    } else {
        cur_frame = animation->GetCurrentValue() * (end_frame_ - start_frame_) + start_frame_;
    }
    bool need_redraw = drawable_->setCurrentFrame(cur_frame);
    if (need_redraw) {
        SchedulePaint();
    }

    if (delegate_) {
        int total_frame = drawable_->video_item_->frames_;
        delegate_->onSVGAStep(
            this, true, cur_frame, total_frame, double(cur_frame + 1) / total_frame);
    }
}

void LivehimeSVGAImageView::AnimationCanceled(const ui::Animation* animation) {
    ProcessStop();
}

void LivehimeSVGAImageView::OnPostAnimationEnd() {
    if (animation_) {
        animation_->Start();
        ++cur_repeat_count_;

        if (delegate_) {
            delegate_->onSVGARepeat(this, cur_repeat_count_);
        }
    }
}

void LivehimeSVGAImageView::ProcessStop() {
    is_animating_ = false;

    if (is_paused_) {
        if (delegate_) {
            delegate_->onSVGAPause(this);
        }
    } else {
        if (clears_after_stop_) {
            drawable_->setCleared(clears_after_stop_);
            SchedulePaint();
        }

        if (delegate_) {
            delegate_->onSVGAFinished(this);
        }
    }
}

void LivehimeSVGAImageView::ParseSVGAFileOnWordThread(
    const std::wstring& file_path, const string16& cache_path,
    std::shared_ptr<VideoItemPtr> video_item)
{
    svga::SVGAParser parser;
    svga::SVGAVideoEntity out;
    parser.setCachePath(cache_path);
    if (parser.parseFromFile(file_path, &out)) {
        *video_item = std::make_shared<svga::SVGAVideoEntity>(out);
    }
}
void LivehimeSVGAImageView::ParseSVGADataOnWordThread(
    const std::string& data, const string16& cache_path,
    std::shared_ptr<VideoItemPtr> video_item)
{
    svga::SVGAParser parser;
    svga::SVGAVideoEntity out;
    parser.setCachePath(cache_path);
    if (parser.parseFromData(data, &out)) {
        *video_item = std::make_shared<svga::SVGAVideoEntity>(out);
    }
}

void LivehimeSVGAImageView::OnSVGADataParsed(std::shared_ptr<VideoItemPtr> video_item) {
    auto vi_ptr = video_item->get();
    if (!vi_ptr || vi_ptr->frames_ <= 0) {
        if (delegate_) {
            delegate_->onSVGAParseComplete(this, false);
        }
        return;
    }

    stopAnimation(true);

    vi_ptr->antialias_ = is_antialias_;
    setVideoItem(*vi_ptr);

    if (delegate_) {
        delegate_->onSVGAParseComplete(this, true);
    }

    if (is_pbp_enabled_) {
        stepToPercentage(percent_before_parsed_, false);
    }

    if (is_auto_play_ && !is_animating_) {
        startAnimation();
    }

    PreferredSizeChanged();
}