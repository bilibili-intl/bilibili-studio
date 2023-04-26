
#include "bililive_label.h"
#include "bililive/bililive/ui/views/controls/bililive_theme_common.h"

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/base/text/text_elider.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/canvas.h"
#include "ui/views/widget/widget.h"
#include "ui/views/layout/fill_layout.h"


namespace
{
    const int kImageThickness = 5 * ui::GetDPIScale();
}


BililiveLabel::BililiveLabel()
{
    InitParam();
}

BililiveLabel::BililiveLabel(const string16& text)
    : views::Label(text)
{
    InitParam();
}

BililiveLabel::BililiveLabel(const string16& text, const gfx::Font& font)
    : views::Label(text, font)
{
    InitParam();
}

void BililiveLabel::InitParam()
{
    interactive_ = false;
    text_color_ = clrLabelText;

    image_ = new views::ImageView();
    image_->set_interactive(false);
    AddChildView(image_);

    vertical_alignment_ = gfx::ALIGN_VCENTER;
    SetAutoColorReadabilityEnabled(false);
    SetEnabledColor(text_color_);
    SetDisabledColor(clrLabelDisableText);
}

void BililiveLabel::SetPreferredSize(const gfx::Size& size)
{
    user_preferred_size_ = size;
}

void BililiveLabel::SetImage(const gfx::ImageSkia& img)
{
    image_->SetImage(img);
}

void BililiveLabel::SetImage(const gfx::ImageSkia* image_skia)
{
    image_->SetImage(image_skia);
}

void BililiveLabel::SetImageSize(const gfx::Size &size)
{
    image_->SetImageSize(size);
}

gfx::Size BililiveLabel::GetPreferredSize()
{
    gfx::Size pref_size = __super::GetPreferredSize();
    gfx::Size img_size = image_->GetPreferredSize();
    if (!img_size.IsEmpty())
    {
        pref_size.Enlarge(img_size.width() + kImageThickness, 0);
        pref_size.set_height(std::max(pref_size.height(), img_size.height()));
    }
    if (max_words_ != 0)
    {
        base::string16 etext = ui::ElideText(text(), max_words_, true);
        pref_size.set_width(std::min(pref_size.width(), font().GetStringWidth(etext)));
    }
    else if (!user_preferred_size_.IsEmpty())
    {
        pref_size.SetToMax(user_preferred_size_);
    }
    if (!limit_size_.IsEmpty())
    {
        pref_size.set_width(std::min(pref_size.width(), limit_size_.width()));
    }
    return pref_size;
}

void BililiveLabel::Layout()
{
    gfx::Size img_size = image_->GetPreferredSize();
    if (!img_size.IsEmpty())
    {
        gfx::Insets insets = GetInsets();
        image_->SetBounds(insets.left(), insets.top() + (height() - insets.height() - img_size.height()) / 2, img_size.width(), img_size.height());
    }
}

void BililiveLabel::SetTextColor(SkColor clr)
{
    text_color_ = clr;

    SetEnabledColor(text_color_);
    SetDisabledColor(SkColorSetA(text_color_, 0.3f * 255));
    SchedulePaint();
}

void BililiveLabel::SetAlpha(U8CPU a)
{
    SetEnabledColor(SkColorSetA(text_color_, a));
    SetDisabledColor(SkColorSetA(enabled_color(), 0.3f * 255));
    SchedulePaint();
}

void BililiveLabel::SetLimitText(unsigned int max_words)
{
    max_words_ = max_words;
    PreferredSizeChanged();
}

void BililiveLabel::SetLimitSize(const gfx::Size& size)
{
    limit_size_ = size;
}

void BililiveLabel::PaintText(gfx::Canvas* canvas,
    const string16& text,
    const gfx::Rect& text_bounds,
    int flags)
{
    gfx::Rect local_bound = GetLocalBounds();
    local_bound.Inset(GetInsets());
    gfx::Size img_size = image_->GetPreferredSize();
    if (!img_size.IsEmpty())
    {
        img_size.Enlarge(kImageThickness, 0);
    }
    local_bound.Inset(img_size.width(), 0, 0, 0);
    gfx::Size pref_size = GetPreferredSize();
    int x = local_bound.x();
    int y = text_bounds.y();
    if (flags & gfx::Canvas::TEXT_ALIGN_LEFT)
    {
        x = local_bound.x();
    }
    else if (flags & gfx::Canvas::TEXT_ALIGN_CENTER)
    {
        x = local_bound.x() + (local_bound.width() - text_bounds.width()) / 2;
    }
    else if (flags & gfx::Canvas::TEXT_ALIGN_RIGHT)
    {
        x = local_bound.right() - text_bounds.width();
    }

    if (vertical_alignment_ == gfx::ALIGN_TOP)
    {
        y = local_bound.y();
    }
    else if (vertical_alignment_ == gfx::ALIGN_BOTTOM)
    {
        y = local_bound.bottom() - text_bounds.height();
    }
    else
    {
        y = text_bounds.y();
    }
    gfx::Rect text_rect(x, y, text_bounds.width(), text_bounds.height());

    base::string16 draw_text = text;
    gfx::Size text_size = GetTextSize();
    if (text_size.width() > text_bounds.width())
    {
        if (!img_size.IsEmpty())
        {
            // 表明当前绘制的字符是带尾部省略号的
            draw_text = draw_text.substr(0, draw_text.length() - 4);
            draw_text.append(L"...");
        }
    }

    __super::PaintText(canvas, draw_text, text_rect, flags);
}


BililiveBackgroundLabel::BililiveBackgroundLabel()
{

}

BililiveBackgroundLabel::BililiveBackgroundLabel(const SkColor& bg_color, int height)
{
    bg_color_ = bg_color;
    height_ = height;
}

void BililiveBackgroundLabel::OnPaintBackground(gfx::Canvas* canvas)
{
	auto bound = GetContentsBounds();
	SkPaint paint;
	paint.setAntiAlias(true);
	paint.setColor(bg_color_);
	paint.setStyle(SkPaint::kFill_Style);
	canvas->DrawRoundRect(bound, 0, paint);
}

gfx::Size BililiveBackgroundLabel::GetPreferredSize()
{
	gfx::Size size = __super::GetPreferredSize();
	size.set_height(height_);
	return size;
}
