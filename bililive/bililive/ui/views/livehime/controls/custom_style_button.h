#pragma once

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "ui/gfx/font.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/views/controls/button/custom_button.h"


//按钮,可以附带一张图片
class CustomStyleButton :
    public views::CustomButton
{
public:
    enum class ImageAlign { kLeft, kTop, kRight, kBottom };

public:
    CustomStyleButton(
        views::ButtonListener* listener,
        const base::string16& text = L"",
        const gfx::Font& font = ftTwelve);
    ~CustomStyleButton() override;

    void SetText(const base::string16& text);
    void SetFont(const gfx::Font& font);

    void SetPreferredSize(const gfx::Size& size);

    void SetTextColor(views::Button::ButtonState state, SkColor text_color);
    void SetImage(views::Button::ButtonState state, const gfx::ImageSkia& image);
    void SetImageSize(views::Button::ButtonState state, const gfx::Size& size);
    void SetImageAlign(views::Button::ButtonState state, ImageAlign align);
    void SetImagePadding(views::Button::ButtonState state, int padding);
    void SetBackgroundColor(views::Button::ButtonState state, SkColor color);
    void SetBackgroundRoundRatio(views::Button::ButtonState state, float ratio);
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
    void SetAllStateBackgroundRoundRatio(float ratio);
    void SetAllStateBorder(bool has_border);
    void SetAllStateBorderLineWidth(float border_width);
    void SetAllStateBorderRoundRatio(float round_ratio);
    void SetAllStateBorderColor(SkColor color);

    bool ImageHitTest(views::Button::ButtonState state, const gfx::Point& pt);
    void SetNoSubpixelRendering(bool no_subpixel_rendering);

    gfx::Size GetPreferredSize() override;

protected:
    //views
    void OnPaint(gfx::Canvas* canvas) override;
    gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override;

private:
    bool no_subpixel_rendering_ = false;
    gfx::Rect GetImageRect(views::Button::ButtonState state);
    gfx::Size GetMinSiz();

    gfx::Size                   preferred_size_;
    base::string16              text_;
    gfx::Font                   font_;

    SkColor                     text_color_[STATE_COUNT] = {};
    gfx::ImageSkia              images_[STATE_COUNT];
    gfx::Size                   image_size_[STATE_COUNT];
    ImageAlign                  image_align_[STATE_COUNT] = {};
    int                         image_padding_[STATE_COUNT] = {};
    SkColor                     background_color_[STATE_COUNT] = {};
    float                       background_round_ratio_[STATE_COUNT] = {};
    bool                        has_border_[STATE_COUNT] = {};
    float                       border_width_[STATE_COUNT] = {};
    float                       border_round_ratio_[STATE_COUNT] = {};
    SkColor                     border_color_[STATE_COUNT] = {};
};
