#include "custom_style_button.h"

#include "ui/gfx/canvas.h"

#include "bililive/bililive/utils/bililive_canvas_drawer.h"

CustomStyleButton::CustomStyleButton(
    views::ButtonListener* listener,
    const base::string16& text,
    const gfx::Font& font) :
    views::CustomButton(listener),
    text_(text),
    font_(font)
{

}

CustomStyleButton::~CustomStyleButton()
{
}

void CustomStyleButton::SetText(const base::string16& text)
{
    text_ = text;
}

void CustomStyleButton::SetFont(const gfx::Font& font)
{
    font_ = font;
}

void CustomStyleButton::SetPreferredSize(const gfx::Size& size)
{
    preferred_size_ = size;
}

void CustomStyleButton::SetTextColor(views::Button::ButtonState state, SkColor text_color)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    text_color_[state] = text_color;
}

void CustomStyleButton::SetImage(views::Button::ButtonState state, const gfx::ImageSkia& image)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    images_[state] = image;
}

void CustomStyleButton::SetImageSize(views::Button::ButtonState state, const gfx::Size& size)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    image_size_[state] = size;
}

void CustomStyleButton::SetImageAlign(views::Button::ButtonState state, ImageAlign align)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    image_align_[state] = align;
}

void CustomStyleButton::SetImagePadding(views::Button::ButtonState state, int padding)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    image_padding_[state] = padding;
}

void CustomStyleButton::SetBackgroundColor(views::Button::ButtonState state, SkColor color)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    background_color_[state] = color;
}

void CustomStyleButton::SetBackgroundRoundRatio(views::Button::ButtonState state, float ratio)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    background_round_ratio_[state] = ratio;
}

void CustomStyleButton::SetBorder(views::Button::ButtonState state, bool has_border)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    has_border_[state] = has_border;
}

void CustomStyleButton::SetBorderLineWidth(views::Button::ButtonState state, float border_width)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    border_width_[state] = border_width;
}

void CustomStyleButton::SetBorderRoundRatio(views::Button::ButtonState state, float round_ratio)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    border_round_ratio_[state] = round_ratio;
}

void CustomStyleButton::SetBorderColor(views::Button::ButtonState state, SkColor color)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    border_color_[state] = color;
}

void CustomStyleButton::SetAllStateTextColor(SkColor color)
{
    for (auto& v : text_color_)
    {
        v = color;
    }
}

void CustomStyleButton::SetAllStateImage(const gfx::ImageSkia& image)
{
    for (auto& v : images_)
    {
        v = image;
    }
}

void CustomStyleButton::SetAllStateImageSize(const gfx::Size& size)
{
    for (auto& v : image_size_)
    {
        v = size;
    }
}

void CustomStyleButton::SetAllStateImageAlign(ImageAlign align)
{
    for (auto& v : image_align_)
    {
        v = align;
    }
}

void CustomStyleButton::SetAllStateImagePadding(int padding)
{
    for (auto& v : image_padding_)
    {
        v = padding;
    }
}

void CustomStyleButton::SetAllStateBackgroundColor(SkColor color)
{
    for (auto& v : background_color_)
    {
        v = color;
    }
}

void CustomStyleButton::SetAllStateBackgroundRoundRatio(float ratio)
{
    for (auto &v : background_round_ratio_)
    {
        v = ratio;
    }
}

void CustomStyleButton::SetAllStateBorder(bool has_border)
{
    for (auto& v : has_border_)
    {
        v = has_border;
    }
}

void CustomStyleButton::SetAllStateBorderLineWidth(float border_width)
{
    for (auto &v : border_width_)
    {
        v = border_width;
    }
}

void CustomStyleButton::SetAllStateBorderRoundRatio(float round_ratio)
{
    for (auto& v : border_round_ratio_)
    {
        v = round_ratio;
    }
}

void CustomStyleButton::SetAllStateBorderColor(SkColor color)
{
    for (auto& v : border_color_)
    {
        v = color;
    }
}

bool CustomStyleButton::ImageHitTest(views::Button::ButtonState state, const gfx::Point& pt)
{
    return GetImageRect(state).Contains(pt);
}

void CustomStyleButton::SetNoSubpixelRendering(bool no_subpixel_rendering)
{
    if (no_subpixel_rendering_ == no_subpixel_rendering)
    {
        return;
    }
    no_subpixel_rendering_ = no_subpixel_rendering;
    this->SchedulePaint();
}

void CustomStyleButton::OnPaint(gfx::Canvas* canvas)
{
    views::CustomButton::OnPaint(canvas);

    auto st = state();
    const auto& text_color = text_color_[st];
    const auto& image = images_[st];
    const auto& image_size = image_size_[st];
    const auto& image_align = image_align_[st];
    const auto& image_padding = image_padding_[st];
    const auto& background_color = background_color_[st];
    const auto& background_round_ratio = background_round_ratio_[st];
    const auto& has_border = has_border_[st];
    const float& border_width = border_width_[st];
    const float& border_round_ratio = border_round_ratio_[st];
    const SkColor& border_color  = border_color_[st];

    auto content_bounds = this->GetContentsBounds();
    int text_width = canvas->GetStringWidth(text_, font_);
    int text_height = font_.GetHeight();

    auto draw_image_size = image_size.IsEmpty() ? image.size() : image_size;
    bool need_draw_img = !image.isNull() && !draw_image_size.IsEmpty();

    if (background_color != 0)
    {
        if (background_round_ratio <= 0.0f)
        {
            canvas->FillRect(content_bounds, background_color);
        }
        else
        {
            SkPaint painter;
            painter.setStyle(SkPaint::kFill_Style);
            painter.setAntiAlias(true);
            painter.setColor(background_color);

            bililive::FillRoundRect(canvas, content_bounds.x(), content_bounds.y(), content_bounds.width(), content_bounds.height(), background_round_ratio, painter);
        }
    }

    int x{}, y{};
    if (need_draw_img)
    {
        auto img_rect = GetImageRect(state());
        switch (image_align)
        {
        case ImageAlign::kLeft:
            x = img_rect.right() + image_padding;
            y = (content_bounds.height() - text_height) / 2.0;
            break;

        case ImageAlign::kTop:
            x = (content_bounds.width() - text_width) / 2.0;
            y = img_rect.bottom() + image_padding;
            break;

        case ImageAlign::kRight:
            x = img_rect.x() - image_padding - text_width;
            y = (content_bounds.height() - text_height) / 2.0;
            break;

        case ImageAlign::kBottom:
            x = (content_bounds.width() - text_width) / 2.0;
            y = img_rect.y() - image_padding - text_height;
            break;

        default:
            DCHECK(0);
            break;
        }
        int flags = gfx::Canvas::DefaultCanvasTextAlignment() | (no_subpixel_rendering_ ? gfx::Canvas::NO_SUBPIXEL_RENDERING : 0);
        canvas->DrawStringInt(text_, font_, text_color, x, y, text_width, text_height, flags);
        canvas->DrawImageInt(image, 0, 0, image.width(), image.height(), img_rect.x(), img_rect.y(), img_rect.width(), img_rect.height(), true);
    }
    else
    {
        x = (content_bounds.width() - text_width) / 2.0;
        y = (content_bounds.height() - text_height) / 2.0;
		auto text_rect = content_bounds;
        text_rect.set_x(x);
        int flags = gfx::Canvas::DefaultCanvasTextAlignment() | (no_subpixel_rendering_ ? gfx::Canvas::NO_SUBPIXEL_RENDERING : 0);
        canvas->DrawStringInt(text_, font_, text_color, text_rect.x(), text_rect.y(), text_rect.width(), text_rect.height(), flags);
    }

    if (has_border)
    {
        SkPaint border_painter;
        border_painter.setStyle(SkPaint::kStroke_Style);
        border_painter.setAntiAlias(true);
        border_painter.setStrokeWidth(border_width);
        border_painter.setColor(border_color);

        bililive::DrawRoundRect(canvas, content_bounds.x(), content_bounds.y(), content_bounds.width(), content_bounds.height(), border_round_ratio, border_painter);
    }
}

gfx::NativeCursor CustomStyleButton::GetCursor(const ui::MouseEvent& event)
{
    return ::LoadCursor(nullptr, IDC_HAND);
}

gfx::Size CustomStyleButton::GetPreferredSize()
{
    if (preferred_size_.IsEmpty())
    {
        return GetMinSiz();
    }
    else
    {
        return preferred_size_;
    }
}

gfx::Rect CustomStyleButton::GetImageRect(views::Button::ButtonState state)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return{};
    }

    const auto& img = images_[state];
    const auto& img_size = image_size_[state];
    auto draw_image_size = img_size.IsEmpty() ? img.size() : img_size;

    if (img.isNull() || draw_image_size.IsEmpty())
    {
        return {};
    }

    int padding = image_padding_[state];
    auto content_bounds = GetContentsBounds();
    int text_width = font_.GetStringWidth(text_);
    int text_height = font_.GetHeight();

    int x{}, y{}, total_length{};

    switch (image_align_[state])
    {
    case ImageAlign::kLeft:
        total_length = draw_image_size.width() + padding + text_width;
        x = (content_bounds.width() - total_length) / 2.0;
        y = (content_bounds.height() - draw_image_size.height()) / 2.0;
        break;

    case ImageAlign::kTop:
        total_length = draw_image_size.height() + padding + text_height;
        x = (content_bounds.width() - draw_image_size.width()) / 2.0;
        y = (content_bounds.height() - total_length) / 2.0;
        break;

    case ImageAlign::kRight:
        total_length = draw_image_size.width() + padding + text_width;
        x = ((content_bounds.width() - total_length) / 2.0) + padding + text_width;
        y = (content_bounds.height() - draw_image_size.height()) / 2.0;
        break;

    case ImageAlign::kBottom:
        total_length = draw_image_size.height() + padding + text_height;
        x = (content_bounds.width() - draw_image_size.width()) / 2.0;
        y = (content_bounds.height() - total_length) / 2.0 + padding + text_height;
        break;
    }

    return gfx::Rect(x, y, draw_image_size.width(), draw_image_size.height());
}

gfx::Size CustomStyleButton::GetMinSiz()
{
    int text_width = font_.GetStringWidth(text_);
    int text_height = font_.GetHeight();

    const auto& img = images_[state()];
    const auto& img_size = image_size_[state()];
    auto draw_image_size = img_size.IsEmpty() ? img.size() : img_size;
    int padding = image_padding_[state()];


    auto align = image_align_[state()];
    int width{}, height{};
    switch (align)
    {
        case CustomStyleButton::ImageAlign::kLeft:
        case CustomStyleButton::ImageAlign::kRight:
            width = draw_image_size.width() + padding + text_width;
            height = std::max(draw_image_size.height(), text_height);
        break;

        case CustomStyleButton::ImageAlign::kTop:
        case CustomStyleButton::ImageAlign::kBottom:
            width = std::max(draw_image_size.width(), text_width);
            height = draw_image_size.height() + padding + text_height;
        break;

    }

    return { width, height };
}

