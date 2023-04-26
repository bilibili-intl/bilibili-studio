#include "denoise_slider.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"


DenoiseSlider::DenoiseSlider(views::SliderListener* listener)
    : LivehimeSlider(new DenoiseSliderListener(listener),
        views::Slider::HORIZONTAL, BililiveSlider::SBA_LEADING) {
    SetKeyboardIncrement(1.0f / 3);
    SetMouseWheelIncrement(1.0f / 3);
};

DenoiseSlider::~DenoiseSlider() {
    if (listener_) {
        delete (DenoiseSliderListener*)listener_;
    }
}

int DenoiseSlider::stay_time() const {
    int pos = 0;
    float vl = value();

    if (vl < 1.0f / 6.0f) {
        pos = NOLEVEL;
    }
    else if (vl < 1.0f / 2.0f) {
        pos = LOW;
    }
    else if (vl < 5.0f / 6.0f) {
        pos = MIDDLE;
    }
    else {
        pos = HIGH;
    }

    return pos;
}

void DenoiseSlider::set_stay_time(int stay_time) {
    float vl = 0;
    if (stay_time > LOW) {
        vl = 0;
    }
    else if (stay_time > MIDDLE) {
        vl = 1.0f / 3.0f;
    }
    else if (stay_time > HIGH) {
        vl = 2.0f / 3.0f;
    }
    else {
        vl = 1.0f;
    }

    SetValue(vl);
}


// views::View overrides:
void DenoiseSlider::OnPaint(gfx::Canvas* canvas)
{
    LivehimeSlider::OnPaint(canvas);

    gfx::Rect content = GetContentsBoundsImp();
    if (orientation_ == HORIZONTAL)
    {
        const gfx::Font &ft = ftPrimary;

        int x = content.x();
        int y = content.bottom() - ft.GetHeight();

        base::string16 text = GetLocalizedString(IDS_CONFIG_AUDIO_SOUND_DENOISE_CLOSE);
        int cxChar = ft.GetStringWidth(text);
        canvas->DrawStringInt(text, ft, clrLabelContent,
            gfx::Rect(0, y, cxChar, ft.GetHeight()));

        text = GetLocalizedString(IDS_CONFIG_AUDIO_SOUND_DENOISE_LEVEL1);
        cxChar = ft.GetStringWidth(text);
        canvas->DrawStringInt(text, ft, clrLabelContent,
            gfx::Rect(x + content.width() * 1.0f / 3 - cxChar / 2, y, cxChar, ft.GetHeight()));

        text = GetLocalizedString(IDS_CONFIG_AUDIO_SOUND_DENOISE_LEVEL2);
        cxChar = ft.GetStringWidth(text);
        canvas->DrawStringInt(text, ft, clrLabelContent,
            gfx::Rect(x + content.width() * 2.0f / 3 - cxChar / 2, y, cxChar, ft.GetHeight()));

        text = GetLocalizedString(IDS_CONFIG_AUDIO_SOUND_DENOISE_LEVEL3);
        cxChar = ft.GetStringWidth(text);
        canvas->DrawStringInt(text, ft, clrLabelContent,
            gfx::Rect(content.right() - cxChar, y, cxChar, ft.GetHeight()));
    }
}

gfx::Size DenoiseSlider::GetPreferredSize()
{
    DCHECK(thumb_);
    gfx::Size size = __super::GetPreferredSize();
    size.set_height(std::max(size.height(), ftPrimary.GetHeight() + 5 + thumb_->height()));
    return size;
};
