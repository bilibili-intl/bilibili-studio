#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_CONTROLS_LIVEHIME_SVGA_IMAGE_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_CONTROLS_LIVEHIME_SVGA_IMAGE_VIEW_H_

#include "base/memory/weak_ptr.h"

#include "ui/views/view.h"
#include "ui/base/animation/linear_animation.h"

#include "bililive/bililive/livehime/svga_player/utils/svga_scale_info.h"



namespace svga {
    class SVGADrawable;
    class SVGAVideoEntity;
    class SVGADynamicEntity;
}

class LivehimeSVGADelegate;

class LivehimeSVGAImageView :
    public views::View,
    public ui::AnimationDelegate {
public:
    struct Range {
        int location = 0;
        int length = INT_MAX;
    };

    LivehimeSVGAImageView();
    ~LivehimeSVGAImageView();

    void setPreferredSize(const gfx::Size& size);

    void setDelegate(LivehimeSVGADelegate* d);

    /**
     * 设置 SVGA 原始数据。将在 FILE 线程中加载数据。
     * 随时都可设置，且立即生效。
     * 加载完成前的多次调用将以最后一次为准。
     * 若加载失败，播放器中仍会保留最近一次加载成功的结果。
     * 若加载成功，播放器中当前的动画会被替换掉。
     * 加载结束后，无论加载成功或失败，都会通过 SVGADelegate::onParseComplete() 进行通知。
     * {cache_path} 指定缓存目录，中间文件会放在该目录中。
     */
    void setSVGAData(const std::string& data, const string16& cache_path);

    /**
     * 设置 SVGA 原始数据文件。将在 FILE 线程中加载数据。
     * 随时都可设置，且立即生效。
     * 加载完成前的多次调用将以最后一次为准。
     * 若加载失败，播放器中仍会保留最近一次加载成功的结果。
     * 若加载成功，播放器中当前的动画会被替换掉。
     * 加载结束后，无论加载成功或失败，都会通过 SVGADelegate::onParseComplete() 进行通知。
     * {cache_path} 指定缓存目录，中间文件会放在该目录中。
     */
    void setSVGAFilePath(const string16& file_path, const string16& cache_path);

    /**
     * 用于设置 SVGA 资源的便捷方法。
     * 缓存目录位于直播姬的 UserData 目录的 Cache\Motions\{id} 目录中。
     * 该方法在内部会调用 setSVGAData() 方法。
     */
    void setSVGAResourceId(int id);

    /**
     * 设置动画播放次数。 随时都可设置，且立即生效。
     * 若将 {loop_count} 设置为 -1，表示无限循环；
     * 若将 {loop_count} 设置为 0，表示只播放一次；
     * 若将 {loop_count} 设置为大于 0 的某个值，表示将动画播放 {loop_count} 次；
     * 若设置为其他值，则等同于 0。
     * 默认值为 0。
     */
    void setLoopCount(int loop_count);

    /**
     * 指定动画停止后是否清除画布。
     * 该设置仅在动画停止之前设置才有效。
     * 若动画设置为无限循环，则该设置无效果。
     * 默认值为 true。
     */
    void setClearAfterStop(bool clear);

    /**
     * 指定是否开启抗锯齿。
     * 随时都可设置，且立即生效。
     * 默认值为 true。
     */
    void setAntialias(bool antialias);

    /**
     * 指定是否在加载完成后自动开始播放。
     * 该设置仅在加载完成前设置才有效。
     * 默认值为 true。
     */
    void setAutoPlay(bool auto_play);

    /**
     * 设置缩放形式。
     * 随时都可设置，且立即生效。
     */
    void setScaleType(svga::ScaleType type);

    /**
     * 从头开始播放动画。
     * 该调用仅在加载成功后才有效。
     * 若动画之前已在播放，则调用后动画会从头开始播放。
     * 将通过 SVGADelegate::onStart() 进行通知。
     */
    void startAnimation();

    /**
     * 播放指定范围的动画。
     * 该调用仅在加载成功后才有效。
     * 若动画之前已在播放，则调用后动画会从指定范围的第一帧开始播放。
     * {reverse} 指定是否倒放动画。
     * 将通过 SVGADelegate::onStart() 进行通知。
     */
    void startAnimation(const Range& range, bool reverse = false);

    /**
     * 暂停动画播放。
     * 该调用仅在动画播放时才有效。
     * 可通过 startAnimation() 的任一重载恢复播放，当前播放进度将被继承。
     * 将通过 SVGADelegate::onPause() 进行通知。
     */
    void pauseAnimation();

    /**
     * 停止动画播放。
     * 该调用仅在加载成功后才有效。
     * 将根据 setClearAfterStop() 的设置决定是否清除画布。
     */
    void stopAnimation();

    /**
     * 停止动画播放。
     * 该调用仅在加载成功后才有效。
     * 将根据 {clear} 的设置决定是否清除画布。
     */
    void stopAnimation(bool clear);

    /**
     * 暂停当前动画，并跳至指定帧。
     * 该调用仅在加载成功后才有效。
     * {frame} 从 0 开始计数。和循环无关。
     * {and_play} 指定跳帧后是否启动动画。
     */
    void stepToFrame(int frame, bool and_play);

    /**
     * 暂停当前动画，并跳至指定的以百分比表示的播放位置。
     * 该调用仅在加载成功后才有效。
     * {percentage} 从 0 到 1。和循环无关。
     * {and_play} 指定跳帧后是否启动动画。
     */
    void stepToPercentage(double percentage, bool and_play);

    /**
     * 指定下一次加载成功时的跳转位置。
     * 该方法即使在未加载成功的状态下也可以调用。
     * {enabled} 为 true 时，下一次加载成功后将跳转到 {percentage} 指定的位置。
     */
    void prepareToPercentage(bool enabled, double percentage);

    /**
     * 不接收下一次的加载完成内部回调。
     * 随时都可设置，且立即生效。
     * 该方法将吊销加载完成内部回调的弱指针。
     */
    void stopParsing();

    /**
     * 获取动画播放次数。
     * 任何时候均可获取。
     * 详见 setLoopCount() 方法。
     */
    int getLoops() const;

    /**
     * 获取动画总帧数。和循环无关。
     * 该调用仅在加载成功后才有效，否则返回 0。
     */
    int getFrameCount() const;

    /**
     * 获取当前动画帧。和循环无关。
     * 该调用仅在加载成功后才有效，否则返回 -1。
     */
    int getCurFrame() const;

    /**
     * 获取动画包围盒大小。
     * 该调用仅在加载成功后才有效，否则返回 {0, 0}。
     */
    gfx::Size getVideoSize() const;

    /**
     * 获取当前的抗锯齿状态。
     * 任何时候均可获取。
     */
    bool isAntialias() const;

    /**
     * 获取当前动画状态。
     * 任何时候均可获取。
     */
    bool isAnimating() const;

    /**
     * 检查是否存在加载成功的动画。
     * 任何时候均可获取。
     * 当前，只要有一次加载成功，该方法就会一直返回 true。
     */
    bool isLoadingSucceeded() const;

    void setAlpha(uint8_t alpha);
    uint8_t getAlpha() const;

    // views::View
    gfx::Size GetPreferredSize() override;
    void OnPaint(gfx::Canvas* canvas) override;

protected:
    // ui::AnimationDelegate
    void AnimationEnded(const ui::Animation* animation) override;
    void AnimationProgressed(const ui::Animation* animation) override;
    void AnimationCanceled(const ui::Animation* animation) override;

    // views::View
    bool HitTestRect(const gfx::Rect& rect) const override { return false; }

private:
    using VideoItemPtr = std::shared_ptr<svga::SVGAVideoEntity>;

    void setVideoItem(const svga::SVGAVideoEntity& video_item);
    void setVideoItem(const svga::SVGAVideoEntity& video_item, const svga::SVGADynamicEntity& dynamic_item);

    void OnPostAnimationEnd();
    void ProcessStop();

    static void ParseSVGAFileOnWordThread(
        const std::wstring& file_path, const string16& cache_path,
        std::shared_ptr<VideoItemPtr> video_item);
    static void ParseSVGADataOnWordThread(
        const std::string& data, const string16& cache_path,
        std::shared_ptr<VideoItemPtr> video_item);
    void OnSVGADataParsed(std::shared_ptr<VideoItemPtr> video_item);

    int start_frame_ = 0;
    int end_frame_ = 0;
    int cur_repeat_count_ = 0;
    bool is_reverse_ = false;
    double cur_anim_val_ = 0;

    bool is_antialias_ = true;
    bool is_animating_ = false;
    bool is_paused_ = false;
    int loops_ = 0;
    bool clears_after_stop_ = true;
    bool is_auto_play_ = true;
    svga::ScaleType scale_type_ = svga::ScaleType::CENTER_INSIDE;

    bool is_pbp_enabled_ = false;
    double percent_before_parsed_ = 0;

    uint8_t alpha_ = 255;
    bool is_using_pref_size_ = false;
    gfx::Size preferred_size_;

    std::unique_ptr<ui::LinearAnimation> animation_;
    LivehimeSVGADelegate* delegate_ = nullptr;
    std::unique_ptr<svga::SVGADrawable> drawable_;

    base::WeakPtrFactory<LivehimeSVGAImageView> parser_wpf_;
    base::WeakPtrFactory<LivehimeSVGAImageView> loop_wpf_;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_CONTROLS_LIVEHIME_SVGA_IMAGE_VIEW_H_