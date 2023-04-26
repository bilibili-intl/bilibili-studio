#include "custom_style_checkbox.h"

#include "bililive/bililive/utils/bililive_canvas_drawer.h"


CustomStyleCheckbox::CustomStyleCheckbox(
    views::ButtonListener* listener,
    const base::string16& text,
    const gfx::Font& font) :
    views::Checkbox(text),
    text_(text),
    font_(font)
{
    set_listener(listener);
}

CustomStyleCheckbox::~CustomStyleCheckbox()
{

}

void CustomStyleCheckbox::SetText(const base::string16& text)
{
    text_ = text;
}

base::string16 CustomStyleCheckbox::GetText() const
{
    return  text_;
}

void CustomStyleCheckbox::SetFont(const gfx::Font& font)
{
    font_ = font;
}

const gfx::Font CustomStyleCheckbox::GetFont() const
{
    return font_;
}

void CustomStyleCheckbox::SetPreferredSize(const gfx::Size& size)
{
    preferred_size_ = size;
}

void CustomStyleCheckbox::SetTextColor(views::Button::ButtonState state, SkColor text_color)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    text_color_[state] = text_color;
}

void CustomStyleCheckbox::SetImage(views::Button::ButtonState state, const gfx::ImageSkia& image)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    images_[state] = image;
}

void CustomStyleCheckbox::SetImageSize(views::Button::ButtonState state, const gfx::Size& size)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    image_size_[state] = size;
}

void CustomStyleCheckbox::SetImageAlign(views::Button::ButtonState state, ImageAlign align)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    image_align_[state] = align;
}

void CustomStyleCheckbox::SetImagePadding(views::Button::ButtonState state, int padding)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    image_padding_[state] = padding;
}

void CustomStyleCheckbox::SetBackgroundColor(views::Button::ButtonState state, SkColor color)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    background_color_[state] = color;
}

void CustomStyleCheckbox::SetBorder(views::Button::ButtonState state, bool has_border)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    has_border_[state] = has_border;
}

void CustomStyleCheckbox::SetBorderLineWidth(views::Button::ButtonState state, float border_width)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    border_width_[state] = border_width;
}

void CustomStyleCheckbox::SetBorderRoundRatio(views::Button::ButtonState state, float round_ratio)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    border_round_ratio_[state] = round_ratio;
}

void CustomStyleCheckbox::SetBorderColor(views::Button::ButtonState state, SkColor color)
{
    if (state < STATE_NORMAL || state >= STATE_COUNT)
    {
        DCHECK(0);
        return;
    }

    border_color_[state] = color;

}

void CustomStyleCheckbox::SetAllStateTextColor(SkColor color)
{
    for (auto& v : text_color_)
    {
        v = color;
    }
}

void CustomStyleCheckbox::SetAllStateImage(const gfx::ImageSkia& image)
{
    for (auto& v : images_)
    {
        v = image;
    }
}

void CustomStyleCheckbox::SetAllStateImageSize(const gfx::Size& size)
{
    for (auto& v : image_size_)
    {
        v = size;
    }
}

void CustomStyleCheckbox::SetAllStateImageAlign(ImageAlign align)
{
    for (auto& v : image_align_)
    {
        v = align;
    }
}

void CustomStyleCheckbox::SetAllStateImagePadding(int padding)
{
    for (auto& v : image_padding_)
    {
        v = padding;
    }
}

void CustomStyleCheckbox::SetAllStateBackgroundColor(SkColor color)
{
    for (auto& v : background_color_)
    {
        v = color;
    }
}

void CustomStyleCheckbox::SetAllStateBorder(bool has_border)
{
    for (auto& v : has_border_)
    {
        v = has_border;
    }
}

void CustomStyleCheckbox::SetAllStateBorderLineWidth(float border_width)
{
    for (auto& v : border_width_)
    {
        v = border_width;
    }
}

void CustomStyleCheckbox::SetAllStateBorderRoundRatio(float round_ratio)
{
    for (auto& v : border_round_ratio_)
    {
        v = round_ratio;
    }
}

void CustomStyleCheckbox::SetAllStateBorderColor(SkColor color)
{
    for (auto& v : border_color_)
    {
        v = color;
    }
}

void CustomStyleCheckbox::SetCheckedBorder(bool has_border)
{
    checked_border_ = has_border;
}

void CustomStyleCheckbox::SetCheckBorderRoundRatio(float round_ratio)
{
    checked_border_ratio_ = round_ratio;
}

void CustomStyleCheckbox::SetCheckedBorderWidth(float width)
{
    checked_border_width_ = width;
}

void CustomStyleCheckbox::SetCheckedBorderColor(SkColor color)
{
    checked_border_color_ = color;
}

void CustomStyleCheckbox::SetCheckedTextColor(SkColor color)
{
    checked_text_color_ = color;
}

void CustomStyleCheckbox::SetImageBorder(bool image_border)
{
    image_border_ = image_border;
}

bool CustomStyleCheckbox::ImageHitTest(views::Button::ButtonState state, const gfx::Point& pt)
{
    return GetImageRect(state).Contains(pt);
}

//void CustomStyleCheckbox::Paint(gfx::Canvas* canvas)
//{
//    views::View::Paint(canvas);
//}

void CustomStyleCheckbox::OnPaint(gfx::Canvas* canvas)
{
    auto st = state();
    auto text_color = text_color_[st];
    auto image = images_[st];
    auto image_size = image_size_[st];
    auto image_align = image_align_[st];
    auto image_padding = image_padding_[st];
    auto background_color = background_color_[st];
    auto has_border = has_border_[st];
    float border_width = border_width_[st];
    float border_round_ratio = border_round_ratio_[st];
    SkColor border_color = border_color_[st];

    if (checked()) 
    {
        has_border = checked_border_;
        border_width = checked_border_width_;
        border_round_ratio = checked_border_ratio_;
        border_color = checked_border_color_;
        text_color = checked_text_color_;
    }

    auto content_bounds = this->GetContentsBounds();
    int text_width = canvas->GetStringWidth(GetText(), GetFont());
    int text_height = GetFont().GetHeight();

    auto draw_image_size = image_size.IsEmpty() ? image.size() : image_size;
    bool need_draw_img = !image.isNull() && !draw_image_size.IsEmpty();
    gfx::Rect img_rect;

    if (background_color != 0)
    {
        canvas->FillRect(content_bounds, background_color);
    }

    int x{}, y{};
    if (need_draw_img)
    {
        img_rect = GetImageRect(state());
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

        canvas->DrawStringInt(GetText(), GetFont(), text_color, x, y, text_width, text_height);
        canvas->DrawImageInt(image, 0, 0, image.width(), image.height(), img_rect.x(), img_rect.y(), img_rect.width(), img_rect.height(), true);
    }
    else
    {
        x = (content_bounds.width() - text_width) / 2.0;
        y = (content_bounds.height() - text_height) / 2.0;
        auto text_rect = content_bounds;
        text_rect.set_x(x);
        canvas->DrawStringInt(GetText(), GetFont(), text_color, text_rect);
    }

    if (has_border)
    {
        SkPaint border_painter;
        border_painter.setStyle(SkPaint::kStroke_Style);
        border_painter.setAntiAlias(true);
        border_painter.setStrokeWidth(border_width);
        border_painter.setColor(border_color);

        gfx::Rect border_rect;
        if (image_border_) 
        {
            border_rect = img_rect;
            border_rect.Inset(GetLengthByDPIScale(1), 0);
        }
        else 
        {
            border_rect = content_bounds;
        }

        bililive::DrawRoundRect(canvas, border_rect.x(), border_rect.y(), border_rect.width(), border_rect.height(), border_round_ratio, border_painter);
    }
}

void CustomStyleCheckbox::PaintChildren(gfx::Canvas* canvas)
{
    //views::View::PaintChildren(canvas);
}

//gfx::NativeCursor CustomStyleCheckbox::GetCursor(const ui::MouseEvent& event)
//{
//    return ::LoadCursor(nullptr, IDC_HAND);
//}

//void CustomStyleCheckbox::NotifyClick(const ui::Event& event)
//{
//    SetChecked(!checked());
//    CustomButton::NotifyClick(event);
//}

//void CustomStyleCheckbox::OnFocus() 
//{
//    views::View::OnFocus();
//}
//
//void CustomStyleCheckbox::OnBlur() 
//{
//    views::View::OnBlur();
//}

gfx::Size CustomStyleCheckbox::GetPreferredSize()
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

gfx::Rect CustomStyleCheckbox::GetImageRect(views::Button::ButtonState state)
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
    int text_width = GetFont().GetStringWidth(GetText());
    int text_height = GetFont().GetHeight();

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

gfx::Size CustomStyleCheckbox::GetMinSiz()
{
    int text_width = GetFont().GetStringWidth(GetText());
    int text_height = GetFont().GetHeight();

    const auto& img = images_[state()];
    const auto& img_size = image_size_[state()];
    auto draw_image_size = img_size.IsEmpty() ? img.size() : img_size;
    int padding = image_padding_[state()];


    auto align = image_align_[state()];
    int width{}, height{};
    switch (align)
    {
    case CustomStyleCheckbox::ImageAlign::kLeft:
    case CustomStyleCheckbox::ImageAlign::kRight:
        width = draw_image_size.width() + padding + text_width;
        height = std::max(draw_image_size.height(), text_height);
        break;

    case CustomStyleCheckbox::ImageAlign::kTop:
    case CustomStyleCheckbox::ImageAlign::kBottom:
        width = std::max(draw_image_size.width(), text_width);
        height = draw_image_size.height() + padding + text_height;
        break;

    }

    return { width, height };
}
