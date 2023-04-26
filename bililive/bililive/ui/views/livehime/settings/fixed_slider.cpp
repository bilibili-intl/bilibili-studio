#include "fixed_slider.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"


FixedSlider::FixedSlider(int min, int max, views::SliderListener* listener)
    : LivehimeSlider(new FixedSliderListener(max - min, listener), views::Slider::HORIZONTAL) {

    DCHECK(min >= 0 && max > min);

    min_ = min;
    max_ = max;
    SetKeyboardIncrement(1.0f / (max_ - min_));
    SetMouseWheelIncrement(1.0f / (max_ - min_));
};

FixedSlider::~FixedSlider() {
    if (listener_) {
        delete static_cast<FixedSliderListener*>(listener_);
    }
}

void FixedSlider::SetNumber(int number) {
    number = std::max(number, min_);
    number = std::min(number, max_);

    float val = 0;
    int interval_count = max_ - min_;
    val = static_cast<float>(number - min_) / interval_count;

    SetValue(val);
}

int FixedSlider::GetNumber() const {
    float pos = 0.f;
    float val = value();
    int interval_count = max_ - min_;

    int valid_point_count = interval_count * 2;
    int interval_index = 0;

    for (int i = 1; i < valid_point_count + 2; i += 2) {
        if (val < i / static_cast<float>(valid_point_count)) {
            pos = static_cast<float>(interval_index) / interval_count;
            break;
        }
        ++interval_index;
    }

    return static_cast<int>(pos * interval_count) + min_;
}

void FixedSlider::OnPaint(gfx::Canvas* canvas)
{
    __super::OnPaint(canvas);

    /*gfx::Rect rect = GetContentsBounds();
    float ff = 1.0f / 14;
    int cy = 0, x, cx = rect.width() - thumb_->width();
    for (int i = 0; i <= 14; ++i)
    {
        x = rect.x() + i * ff * cx;
        canvas->DrawLine(gfx::Point(x, 0), gfx::Point(x, (i % 2) ? height() : height() / 2), SK_ColorRED);
    }*/
}


void FixedSlider::FixedSliderListener::SliderValueChanged(
    views::Slider* sender, float value, float old_value, views::SliderChangeReason reason)
{
    if (freeze_notify_) {
        return;
    }

    int interval_index = 0;
    int detect_section_count = interval_count_ * 2;

    float tmp = value;
    for (int i = 1; i < detect_section_count + 2; i += 2) {
        if (value < i / static_cast<float>(detect_section_count)) {
            tmp = static_cast<float>(interval_index) / interval_count_;
            break;
        }
        ++interval_index;
    }

    if (reason == views::VALUE_CHANGED_BY_USER) {
        if (tmp != value) {
            freeze_notify_ = true;
            sender->SetValue(tmp);
            freeze_notify_ = false;
        }

        old_value = value;
        value = tmp;
    }

    if (prev_interval_index_ != interval_index) {
        prev_interval_index_ = interval_index;

        if (listener_) {
            listener_->SliderValueChanged(sender, value, old_value, reason);
        }
    }
}
