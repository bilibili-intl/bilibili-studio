#ifndef BILILIVE_BILILIVE_UI_EXAMPLES_MISC_TAB_VIEW_H_
#define BILILIVE_BILILIVE_UI_EXAMPLES_MISC_TAB_VIEW_H_

#include "ui/views/view.h"
#include "ui/views/controls/button/button.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_combobox.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_slider.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_svga_delegate.h"


class BililiveImageButton;
class LivehimeContentLabel;
class LivehimeNativeEditView;
class LivehimeSVGAImageView;
class LivehimeActionLabelButton;

namespace examples {

    class MiscTabView :
        public views::View,
        public views::ButtonListener,
        public views::SliderListener,
        public BililiveComboboxExListener,
        public LivehimeSVGADelegate {
    public:
        MiscTabView();

        // views::View
        void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;

        // views::ButtonListener
        void ButtonPressed(views::Button* sender, const ui::Event& event) override;

        // views::SliderListener
        void SliderValueChanged(
            views::Slider* sender,
            float value, float old_value, views::SliderChangeReason reason) override;

        // BililiveComboboxExListener
        void OnSelectedIndexChanged(BililiveComboboxEx* combobox) override;

        // svga::SVGADelegate
        void onSVGAParseComplete(LivehimeSVGAImageView* v, bool succeeded) override;
        void onSVGAStart(LivehimeSVGAImageView* v, bool resumed) override;
        void onSVGAPause(LivehimeSVGAImageView* v) override;
        void onSVGAFinished(LivehimeSVGAImageView* v) override;
        void onSVGARepeat(LivehimeSVGAImageView* v, int repeat_count) override;
        void onSVGAStep(
            LivehimeSVGAImageView* v,
            bool anim_triggered, int frame, int total_frame, double percentage) override;

    private:
        void InitViews();
        void InitViewsData();

        LivehimeSlider* slider_;
        LivehimeCombobox* combobox_;
        LivehimeNativeEditView* edit_view_;

        std::vector<int> svga_ids_;
        LivehimeContentLabel* size_label_ = nullptr;
        LivehimeContentLabel* frame_label_ = nullptr;
        BililiveImageButton* pause_btn_ = nullptr;
        LivehimeSVGAImageView* svga_player_ = nullptr;
        LivehimeSlider* svga_slider_ = nullptr;

        DISALLOW_COPY_AND_ASSIGN(MiscTabView);
    };

}

#endif  // BILILIVE_BILILIVE_UI_EXAMPLES_MISC_TAB_VIEW_H_