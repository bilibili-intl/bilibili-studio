#ifndef BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_SVGA_DELEGATE_H_
#define BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_SVGA_DELEGATE_H_


class LivehimeSVGAImageView;

class LivehimeSVGADelegate {
public:
    virtual ~LivehimeSVGADelegate() = default;

    /**
     * �� SVGA ���ؽ����󱻵��á�
     * {succeeded} ָ���Ǽ��سɹ�����ʧ�ܡ�
     */
    virtual void onSVGAParseComplete(LivehimeSVGAImageView* v, bool succeeded) {}

    /**
     * �� SVGA ������ʼ����ʱ�����á�
     * {resumed} ָ���Ƿ��Ǵ���ͣ״̬�ָ����š�
     */
    virtual void onSVGAStart(LivehimeSVGAImageView* v, bool resumed) {}

    /**
     * �� SVGA ��������ͣʱ�����á�
     * ֻ����Ч����ͣ�����Żᴥ���ûص������Ѿ�������ͣ��״̬�£�
     * �ٴε��� pauseAnimation() �����ᴥ���ûص���
     */
    virtual void onSVGAPause(LivehimeSVGAImageView* v) {}

    /**
     * �� SVGA ��������ʱ�����á�
     * ������˵��
     * - �ڶ����л���ͣ��ʱ�ſ��ܴ����ûص���
     * - ѭ��֮�䲻�ᴥ���ûص���
     * - ���� stopAnimation() �ᴥ���ûص���
     * - ����������û����һ��ѭ��ʱ�ᴥ���ûص���
     * - �ڶ����л���ͣ�У��������µ� SVGA �������ɹ���ᴥ���ûص���
     */
    virtual void onSVGAFinished(LivehimeSVGAImageView* v) {}

    /**
     * �� SVGA ����ѭ��֮�䱻���á�
     * {repeat_count} ָ���˵�ǰ�ظ��Ĵ�����
     * ���磬������ѭ�����ţ��ڶ���������һ�κ󣬿�ʼ�ڶ���ʱ����ֵΪ 1��
     */
    virtual void onSVGARepeat(LivehimeSVGAImageView* v, int repeat_count) {}

    /**
     * �� SVGA ������ÿһ֡��ʾʱ�����á�
     * {anim_triggered} Ϊ true ʱ��������ǰ֡����ʾ���ɶ��������ģ�
     * Ϊ false ʱ�������� stepToFrame() �� stepToPercentage() �ĵ��õ��µġ�
     * {frame} ��ʾ��ǰ��ʾ��֡���������㿪ʼ����
     * {total_frame} ��ʾ��������֡�����͵��� getFrameCount() ��õĽ����ͬ��
     * {percentage} ��ʾ��ǰ�����Ľ��ȣ���ΧΪ [0, 1]��
     * ��ѭ��ʱ��{frame} �� {percentage} ��ֵ��ѭ��������ʱ����ֵ�ᵹ�ˡ�
     */
    virtual void onSVGAStep(
        LivehimeSVGAImageView* v,
        bool anim_triggered, int frame, int total_frame, double percentage) {}
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_SVGA_PLAYER_SVGA_DELEGATE_H_