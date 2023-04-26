#ifndef CONTROL_UTILS_SLIDER_HELPER_H_
#define CONTROL_UTILS_SLIDER_HELPER_H_

#include <functional>
#include "bililive/bililive/ui/views/controls/bililive_slider.h"

class BililiveSliderHelper;
using SliderValueChangedHandler = std::function<void(BililiveSliderHelper*, float, float, views::SliderChangeReason)>;
using SliderDragHandler = std::function<void(BililiveSliderHelper*)>;

class BililiveSliderHelper : public BililiveSlider, public views::SliderListener
{
    SliderValueChangedHandler slider_value_changed_;
    SliderDragHandler slider_drag_started_;
    SliderDragHandler slider_drag_ended_;

protected:
    void SliderValueChanged(Slider* sender,
        float value,
        float old_value,
        views::SliderChangeReason reason) override;
    void SliderDragStarted(Slider* sender) override;
    void SliderDragEnded(Slider* sender) override;

public:
    BililiveSliderHelper(views::Slider::Orientation orientation, SliderBarAlignment align = SBA_CENTER);

    void SetOnValueChanged(const SliderValueChangedHandler& handler);
    void SetOnDragStarted(const SliderDragHandler& handler);
    void SetOnDragEnded(const SliderDragHandler& handler);
};

#endif
