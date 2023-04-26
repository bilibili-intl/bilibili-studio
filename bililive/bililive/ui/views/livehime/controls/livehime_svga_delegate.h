#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_SVGA_DELEGATE_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_SVGA_DELEGATE_H_


class LivehimeSVGAImageView;

class LivehimeSVGADelegate {
public:
    virtual ~LivehimeSVGADelegate() = default;

    /**
     * 在 SVGA 加载结束后被调用。
     * {succeeded} 指明是加载成功还是失败。
     */
    virtual void onSVGAParseComplete(LivehimeSVGAImageView* v, bool succeeded) {}

    /**
     * 在 SVGA 动画开始播放时被调用。
     * {resumed} 指明是否是从暂停状态恢复播放。
     */
    virtual void onSVGAStart(LivehimeSVGAImageView* v, bool resumed) {}

    /**
     * 在 SVGA 动画被暂停时被调用。
     * 只有有效的暂停操作才会触发该回调，在已经处于暂停的状态下，
     * 再次调用 pauseAnimation() 并不会触发该回调。
     */
    virtual void onSVGAPause(LivehimeSVGAImageView* v) {}

    /**
     * 在 SVGA 动画结束时被调用。
     * 具体来说：
     * - 在动画中或暂停中时才可能触发该回调；
     * - 循环之间不会触发该回调；
     * - 调用 stopAnimation() 会触发该回调；
     * - 动画结束，没有下一次循环时会触发该回调；
     * - 在动画中或暂停中，设置了新的 SVGA 并解析成功后会触发该回调。
     */
    virtual void onSVGAFinished(LivehimeSVGAImageView* v) {}

    /**
     * 在 SVGA 动画循环之间被调用。
     * {repeat_count} 指明了当前重复的次数。
     * 例如，设置了循环播放，在动画播放了一次后，开始第二次时，该值为 1。
     */
    virtual void onSVGARepeat(LivehimeSVGAImageView* v, int repeat_count) {}

    /**
     * 在 SVGA 动画的每一帧显示时被调用。
     * {anim_triggered} 为 true 时，表明当前帧的显示是由动画驱动的；
     * 为 false 时表明是由 stepToFrame() 或 stepToPercentage() 的调用导致的。
     * {frame} 表示当前显示的帧索引（从零开始）。
     * {total_frame} 表示动画的总帧数，和调用 getFrameCount() 获得的结果相同。
     * {percentage} 表示当前动画的进度，范围为 [0, 1]。
     * 在循环时，{frame} 和 {percentage} 的值会循环。倒放时，两值会倒退。
     */
    virtual void onSVGAStep(
        LivehimeSVGAImageView* v,
        bool anim_triggered, int frame, int total_frame, double percentage) {}
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_SVGA_DELEGATE_H_