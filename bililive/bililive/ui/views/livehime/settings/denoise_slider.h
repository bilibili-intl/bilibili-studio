#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_DENOISE_SLIDER_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_DENOISE_SLIDER_H_

#include "bililive/bililive/ui/views/livehime/controls/livehime_slider.h"


enum DenoiseLevel {
    NOLEVEL = 0,
    LOW = -10,
    MIDDLE = -20,
    HIGH = -30
};

class DenoiseSlider : public LivehimeSlider
{
    class DenoiseSliderListener : public views::SliderListener
    {
    public:
        explicit DenoiseSliderListener(views::SliderListener *listener)
            : listener_(listener) {}

    protected:
        // SliderListener
        void SliderValueChanged(views::Slider* sender,
            float value,
            float old_value,
            views::SliderChangeReason reason) override
        {
            if (reason == views::VALUE_CHANGED_BY_USER) {
                float tmp = value;
                if (value < 1.0f / 6.0f) {
                    tmp = 0;
                }
                else if (value < 1.0f / 2.0f) {
                    tmp = 1.0f / 3.0f;
                }
                else if (value < 5.0f / 6.0f) {
                    tmp = 2.0f / 3.0f;
                }
                else {
                    tmp = 1.0f;
                }

                if (tmp != value) {
                    sender->SetValue(tmp);

                    if (listener_) {
                        listener_->SliderValueChanged(sender, tmp, value, views::VALUE_CHANGED_BY_API);
                    }
                }
            }
        }

    private:
        views::SliderListener *listener_;
    };

public:
    explicit DenoiseSlider(views::SliderListener* listener);
    ~DenoiseSlider();

    int stay_time() const;
    void set_stay_time(int stay_time);

protected:
    // views::View overrides:
    void OnPaint(gfx::Canvas* canvas) override;
    gfx::Size GetPreferredSize() override;

private:
    DISALLOW_COPY_AND_ASSIGN(DenoiseSlider);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_DENOISE_SLIDER_H_