#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_CONTROLS_DMKHIME_CONTROLS_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_CONTROLS_DMKHIME_CONTROLS_H_

#include "SkPaint.h"

#include "ui/views/view.h"

#include "bililive/bililive/ui/views/controls/bililive_floating_scroll_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"


class LivehimeContentLabel;

namespace dmkhime {

    /**
     * ���ܰ��б�����������ؼ�
     */
    class HighEnergyRankView : public views::View {
    public:
        HighEnergyRankView();

        /**
         * ��������������
         * @param rank �� rank >= 0 ʱ��ʾʵ���������֣�����һ��������ʾͼƬ��
         *             �� rank == -1 ʱ��ʾ��ܣ�
         *             �� rank == -2 ʱ��ʾ��ʱ��
         *             ����ֵ�������ԡ�
         */
        void SetRank(int64_t rank);

        // views::View
        gfx::Size GetPreferredSize() override;
        int GetHeightForWidth(int w) override;
        void Layout() override;
        void SetNoSubpixelRender(bool force);

    private:
        LivehimeImageView* img_view_;
        LivehimeContentLabel* label_;
        DISALLOW_COPY_AND_ASSIGN(HighEnergyRankView);
    };

    /**
     * ���ܰ��б������
     */
    class HighEnergyVerticalScrollBar : public FloatingScrollBar {
    public:
        explicit HighEnergyVerticalScrollBar(ScrollViewWithFloatingScrollbar* bar);

        int GetScrollBarWidth() const override;
        int GetScrollBarHeight() const override;
        int GetScrollBarVisibleWidth() const override;
        int GetScrollBarVisibleHeight() const override;
        int GetScrollBarBorderThickness() const override;

    protected:
        // View
        void OnPaint(gfx::Canvas* canvas) override;

    private:
        DISALLOW_COPY_AND_ASSIGN(HighEnergyVerticalScrollBar);
    };

}

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_CONTROLS_DMKHIME_CONTROLS_H_