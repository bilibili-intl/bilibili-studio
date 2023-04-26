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
     * ���� SVGA ԭʼ���ݡ����� FILE �߳��м������ݡ�
     * ��ʱ�������ã���������Ч��
     * �������ǰ�Ķ�ε��ý������һ��Ϊ׼��
     * ������ʧ�ܣ����������Իᱣ�����һ�μ��سɹ��Ľ����
     * �����سɹ����������е�ǰ�Ķ����ᱻ�滻����
     * ���ؽ��������ۼ��سɹ���ʧ�ܣ�����ͨ�� SVGADelegate::onParseComplete() ����֪ͨ��
     * {cache_path} ָ������Ŀ¼���м��ļ�����ڸ�Ŀ¼�С�
     */
    void setSVGAData(const std::string& data, const string16& cache_path);

    /**
     * ���� SVGA ԭʼ�����ļ������� FILE �߳��м������ݡ�
     * ��ʱ�������ã���������Ч��
     * �������ǰ�Ķ�ε��ý������һ��Ϊ׼��
     * ������ʧ�ܣ����������Իᱣ�����һ�μ��سɹ��Ľ����
     * �����سɹ����������е�ǰ�Ķ����ᱻ�滻����
     * ���ؽ��������ۼ��سɹ���ʧ�ܣ�����ͨ�� SVGADelegate::onParseComplete() ����֪ͨ��
     * {cache_path} ָ������Ŀ¼���м��ļ�����ڸ�Ŀ¼�С�
     */
    void setSVGAFilePath(const string16& file_path, const string16& cache_path);

    /**
     * �������� SVGA ��Դ�ı�ݷ�����
     * ����Ŀ¼λ��ֱ������ UserData Ŀ¼�� Cache\Motions\{id} Ŀ¼�С�
     * �÷������ڲ������ setSVGAData() ������
     */
    void setSVGAResourceId(int id);

    /**
     * ���ö������Ŵ����� ��ʱ�������ã���������Ч��
     * ���� {loop_count} ����Ϊ -1����ʾ����ѭ����
     * ���� {loop_count} ����Ϊ 0����ʾֻ����һ�Σ�
     * ���� {loop_count} ����Ϊ���� 0 ��ĳ��ֵ����ʾ���������� {loop_count} �Σ�
     * ������Ϊ����ֵ�����ͬ�� 0��
     * Ĭ��ֵΪ 0��
     */
    void setLoopCount(int loop_count);

    /**
     * ָ������ֹͣ���Ƿ����������
     * �����ý��ڶ���ֹ֮ͣǰ���ò���Ч��
     * ����������Ϊ����ѭ�������������Ч����
     * Ĭ��ֵΪ true��
     */
    void setClearAfterStop(bool clear);

    /**
     * ָ���Ƿ�������ݡ�
     * ��ʱ�������ã���������Ч��
     * Ĭ��ֵΪ true��
     */
    void setAntialias(bool antialias);

    /**
     * ָ���Ƿ��ڼ�����ɺ��Զ���ʼ���š�
     * �����ý��ڼ������ǰ���ò���Ч��
     * Ĭ��ֵΪ true��
     */
    void setAutoPlay(bool auto_play);

    /**
     * ����������ʽ��
     * ��ʱ�������ã���������Ч��
     */
    void setScaleType(svga::ScaleType type);

    /**
     * ��ͷ��ʼ���Ŷ�����
     * �õ��ý��ڼ��سɹ������Ч��
     * ������֮ǰ���ڲ��ţ�����ú󶯻����ͷ��ʼ���š�
     * ��ͨ�� SVGADelegate::onStart() ����֪ͨ��
     */
    void startAnimation();

    /**
     * ����ָ����Χ�Ķ�����
     * �õ��ý��ڼ��سɹ������Ч��
     * ������֮ǰ���ڲ��ţ�����ú󶯻����ָ����Χ�ĵ�һ֡��ʼ���š�
     * {reverse} ָ���Ƿ񵹷Ŷ�����
     * ��ͨ�� SVGADelegate::onStart() ����֪ͨ��
     */
    void startAnimation(const Range& range, bool reverse = false);

    /**
     * ��ͣ�������š�
     * �õ��ý��ڶ�������ʱ����Ч��
     * ��ͨ�� startAnimation() ����һ���ػָ����ţ���ǰ���Ž��Ƚ����̳С�
     * ��ͨ�� SVGADelegate::onPause() ����֪ͨ��
     */
    void pauseAnimation();

    /**
     * ֹͣ�������š�
     * �õ��ý��ڼ��سɹ������Ч��
     * ������ setClearAfterStop() �����þ����Ƿ����������
     */
    void stopAnimation();

    /**
     * ֹͣ�������š�
     * �õ��ý��ڼ��سɹ������Ч��
     * ������ {clear} �����þ����Ƿ����������
     */
    void stopAnimation(bool clear);

    /**
     * ��ͣ��ǰ������������ָ��֡��
     * �õ��ý��ڼ��سɹ������Ч��
     * {frame} �� 0 ��ʼ��������ѭ���޹ء�
     * {and_play} ָ����֡���Ƿ�����������
     */
    void stepToFrame(int frame, bool and_play);

    /**
     * ��ͣ��ǰ������������ָ�����԰ٷֱȱ�ʾ�Ĳ���λ�á�
     * �õ��ý��ڼ��سɹ������Ч��
     * {percentage} �� 0 �� 1����ѭ���޹ء�
     * {and_play} ָ����֡���Ƿ�����������
     */
    void stepToPercentage(double percentage, bool and_play);

    /**
     * ָ����һ�μ��سɹ�ʱ����תλ�á�
     * �÷�����ʹ��δ���سɹ���״̬��Ҳ���Ե��á�
     * {enabled} Ϊ true ʱ����һ�μ��سɹ�����ת�� {percentage} ָ����λ�á�
     */
    void prepareToPercentage(bool enabled, double percentage);

    /**
     * ��������һ�εļ�������ڲ��ص���
     * ��ʱ�������ã���������Ч��
     * �÷�����������������ڲ��ص�����ָ�롣
     */
    void stopParsing();

    /**
     * ��ȡ�������Ŵ�����
     * �κ�ʱ����ɻ�ȡ��
     * ��� setLoopCount() ������
     */
    int getLoops() const;

    /**
     * ��ȡ������֡������ѭ���޹ء�
     * �õ��ý��ڼ��سɹ������Ч�����򷵻� 0��
     */
    int getFrameCount() const;

    /**
     * ��ȡ��ǰ����֡����ѭ���޹ء�
     * �õ��ý��ڼ��سɹ������Ч�����򷵻� -1��
     */
    int getCurFrame() const;

    /**
     * ��ȡ������Χ�д�С��
     * �õ��ý��ڼ��سɹ������Ч�����򷵻� {0, 0}��
     */
    gfx::Size getVideoSize() const;

    /**
     * ��ȡ��ǰ�Ŀ����״̬��
     * �κ�ʱ����ɻ�ȡ��
     */
    bool isAntialias() const;

    /**
     * ��ȡ��ǰ����״̬��
     * �κ�ʱ����ɻ�ȡ��
     */
    bool isAnimating() const;

    /**
     * ����Ƿ���ڼ��سɹ��Ķ�����
     * �κ�ʱ����ɻ�ȡ��
     * ��ǰ��ֻҪ��һ�μ��سɹ����÷����ͻ�һֱ���� true��
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