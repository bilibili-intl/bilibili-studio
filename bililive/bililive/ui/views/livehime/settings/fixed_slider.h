#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_FIXED_SLIDER_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_FIXED_SLIDER_H_

#include "bililive/bililive/ui/views/livehime/controls/livehime_slider.h"


class FixedSlider : public LivehimeSlider
{
public:
    explicit FixedSlider(int min, int max, views::SliderListener* listener);
    ~FixedSlider();

    void SetNumber(int number);
    int GetNumber() const;

    virtual void OnPaint(gfx::Canvas* canvas) override;

private:
    class FixedSliderListener : public views::SliderListener
    {
    public:
        explicit FixedSliderListener(int interval_count, views::SliderListener *listener)
            : listener_(listener),
            interval_count_(interval_count),
            prev_interval_index_(0),
            freeze_notify_(false) {
            DCHECK(interval_count >= 1);
        }

    protected:
        // SliderListener
        void SliderValueChanged(views::Slider* sender,
            float value,
            float old_value,
            views::SliderChangeReason reason) override;

    private:
        int interval_count_;
        int prev_interval_index_;
        bool freeze_notify_;
        views::SliderListener* listener_;
    };

    int min_, max_;

    DISALLOW_COPY_AND_ASSIGN(FixedSlider);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_FIXED_SLIDER_H_