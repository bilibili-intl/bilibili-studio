#ifndef LIVEHIME_SLIDER_H
#define LIVEHIME_SLIDER_H

#include "bililive/bililive/ui/views/controls/bililive_slider.h"

class LivehimeSlider : public BililiveSlider
{
public:
    LivehimeSlider(views::SliderListener* listener, views::Slider::Orientation orientation, SliderBarAlignment align = SBA_CENTER);
    virtual ~LivehimeSlider();

protected:
    // View
    virtual void OnEnabledChanged() override;
    virtual gfx::Size GetPreferredSize() override;

private:
};

#endif
