#include "slider_helper.h"

BililiveSliderHelper::BililiveSliderHelper(views::Slider::Orientation orientation, SliderBarAlignment align)
    : BililiveSlider(this, orientation, align)
{
}

void BililiveSliderHelper::SetOnValueChanged(const SliderValueChangedHandler& handler)
{
    slider_value_changed_ = handler;
}

void BililiveSliderHelper::SetOnDragStarted(const SliderDragHandler& handler)
{
    slider_drag_started_ = handler;
}

void BililiveSliderHelper::SetOnDragEnded(const SliderDragHandler& handler)
{
    slider_drag_ended_ = handler;
}

void BililiveSliderHelper::SliderValueChanged(Slider* sender, float value, float old_value, views::SliderChangeReason reason)
{
    if (slider_value_changed_)
        slider_value_changed_(this, value, old_value, reason);
}

void BililiveSliderHelper::SliderDragStarted(Slider* sender)
{
    if (slider_drag_started_)
        slider_drag_started_(this);
}

void BililiveSliderHelper::SliderDragEnded(Slider* sender)
{
    if (slider_drag_ended_)
        slider_drag_ended_(this);
}
