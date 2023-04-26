#pragma once

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "ui/gfx/font.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/views/controls/button/custom_button.h"
#include "ui/views/controls/button/radio_button.h"

class CustomStyleRadioButton :
    public views::RadioButton
{
public:
    enum class ImageAlign { kLeft, kTop, kRight, kBottom };

public:
    CustomStyleRadioButton(views::ButtonListener* listener,
        int group_id,
        const base::string16& text = L"",
        const gfx::Font& font = ftTwelve);
    ~CustomStyleRadioButton() override;

    void SetText(const base::string16& text);
    base::string16 GetText() const;
    void SetFont(const gfx::Font& font);
    const gfx::Font GetFont() const;

    void SetPreferredSize(const gfx::Size& size);
    gfx::Size GetPreferredSize() override;

    void SetTextColor(views::Button::ButtonState state, SkColor text_color);
    void SetImage(views::Button::ButtonState state, const gfx::ImageSkia& image);
    void SetImageSize(views::Button::ButtonState state, const gfx::Size& size);
    void SetImageAlign(views::Button::ButtonState state, ImageAlign align);
    void SetImagePadding(views::Button::ButtonState state, int padding);
    void SetBackgroundColor(views::Button::ButtonState state, SkColor color);
    void SetBorder(views::Button::ButtonState state, bool has_border);
    void SetBorderLineWidth(views::Button::ButtonState state, float border_width);
    void SetBorderRoundRatio(views::Button::ButtonState state, float round_ratio);
    void SetBorderColor(views::Button::ButtonState state, SkColor color);

    void SetAllStateTextColor(SkColor color);
    void SetAllStateImage(const gfx::ImageSkia& image);
    void SetAllStateImageSize(const gfx::Size& size);
    void SetAllStateImageAlign(ImageAlign align);
    void SetAllStateImagePadding(int padding);
    void SetAllStateBackgroundColor(SkColor color);
    void SetAllStateBorder(bool has_border);
    void SetAllStateBorderLineWidth(float border_width);
    void SetAllStateBorderRoundRatio(float round_ratio);
    void SetAllStateBorderColor(SkColor color);

    void SetCheckedBorder(bool has_border);
    void SetCheckBorderRoundRatio(float round_ratio);
    void SetCheckedBorderWidth(float width);
    void SetCheckedBorderColor(SkColor color);
    void SetCheckedTextColor(SkColor color);
    void SetImageBorder(bool image_border);

    bool ImageHitTest(views::Button::ButtonState state, const gfx::Point& pt);

protected:
    //void Paint(gfx::Canvas* canvas) override;
    //views
    void OnPaint(gfx::Canvas* canvas) override;
    void PaintChildren(gfx::Canvas* canvas) override;
    //void OnPaintBackground()
    //gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override;
    //void OnFocus() override;
    //void OnBlur() override;

private:
    gfx::Rect GetImageRect(views::Button::ButtonState state);
    gfx::Size GetMinSiz();

    bool                        checked_ = false;
    base::string16              text_;
    gfx::Font                   font_;


    bool                        checked_border_ = false;
    float                       checked_border_width_ = 0.0f;
    float                       checked_border_ratio_ = 0.0f;
    SkColor                     checked_border_color_ = 0;
    SkColor                     checked_text_color_ = 0;
    bool                        image_border_ = false;

    gfx::Size                   preferred_size_;
    SkColor                     text_color_[STATE_COUNT] = {};
    gfx::ImageSkia              images_[STATE_COUNT];
    gfx::Size                   image_size_[STATE_COUNT];
    ImageAlign                  image_align_[STATE_COUNT] = {};
    int                         image_padding_[STATE_COUNT] = {};
    SkColor                     background_color_[STATE_COUNT] = {};
    bool                        has_border_[STATE_COUNT] = {};
    float                       border_width_[STATE_COUNT] = {};
    float                       border_round_ratio_[STATE_COUNT] = {};
    SkColor                     border_color_[STATE_COUNT] = {};
};
