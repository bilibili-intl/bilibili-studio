#include "livehime_slider.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "ui/base/resource/resource_bundle.h"
#include "grit/theme_resources.h"
#include "grit/generated_resources.h"
#include "grit/ui_resources.h"

namespace
{
    const int slider_bar_size = 2;
}

LivehimeSlider::LivehimeSlider(views::SliderListener* listener, Orientation orientation, SliderBarAlignment align/* = SBA_CENTER*/)
    : BililiveSlider(listener, orientation, align)
{
    SetKeyboardIncrement(0.01f);
    SetBarDetails(slider_bar_size, clrProgressFull, clrProgressEmpty, false);
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();
    SetThumbImage(rb.GetImageSkiaNamed(IDR_LIVEHIME_VOLUME), 
        rb.GetImageSkiaNamed(IDR_LIVEHIME_VOLUME_HV),
        rb.GetImageSkiaNamed(IDR_LIVEHIME_VOLUME));
}

LivehimeSlider::~LivehimeSlider()
{
}

void LivehimeSlider::OnEnabledChanged()
{
    if (enabled())
    {
        SetBarDetails(slider_bar_size, clrProgressFull, clrProgressEmpty);
    }
    else
    {
        SetBarDetails(slider_bar_size, SkColorSetA(clrProgressFull, disable_alpha), SkColorSetA(clrProgressEmpty, disable_alpha));
    }

    __super::OnEnabledChanged();
}

gfx::Size LivehimeSlider::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    if (orientation_ == views::Slider::HORIZONTAL)
    {
        size.set_height(GetLengthByDPIScale(18));
    }
    return size;
}