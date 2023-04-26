#include "bililive/bililive/ui/views/controls/bililive_slider.h"

#include "base/strings/string_number_conversions.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/gfx/canvas.h"

#include "third_party/skia/include/core/SkPaint.h"

#include "bililive/bililive/ui/views/controls/bililive_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "grit/ui_resources.h"
#include "ui/base/win/dpi.h"

namespace {

const int kScrollBarThickness = 2;

}   // namespace

BililiveSlider::BililiveSlider(views::SliderListener* listener, Orientation orientation, SliderBarAlignment align/* = SBA_CENTER*/)
    : views::Slider(listener, orientation)
    , mouse_enter_(false)
    , alignment_(align)
    , mousewheel_increment_(0.01f)
    , association_label_(NULL)
    , accept_key_press_(true)
    , accept_mouse_wheel_(true)
    , scroll_line_length_(0)
    , scrollbar_thickness_(kScrollBarThickness)
    , full_color_(clrSliderFullArea)
    , empty_color_(clrSliderEmptyArea)
{
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();
    SetThumbImage(rb.GetImageSkiaNamed(IDR_SLIDER_ACTIVE_THUMB),
        rb.GetImageSkiaNamed(IDR_SLIDER_ACTIVE_THUMB_HOVERED),
        rb.GetImageSkiaNamed(IDR_SLIDER_DISABLED_THUMB));
}

void BililiveSlider::SetBarDetails(int barsize, SkColor clrFull, SkColor clrEmpty, bool repaint/* = true*/)
{
    scrollbar_thickness_ = barsize;
    full_color_ = clrFull;
    empty_color_ = clrEmpty;
    if (repaint)
    {
        SchedulePaint();
    }
}

void BililiveSlider::SetThumbImage(const gfx::ImageSkia *thumb_nor, const gfx::ImageSkia *thumb_hv, const gfx::ImageSkia *thumb_dis)
{
    DCHECK(thumb_nor && thumb_hv && thumb_dis);
    thumb_nor_ = thumb_nor;
    thumb_hv_ = thumb_hv;
    thumb_dis_ = thumb_dis;
    if (enabled())
    {
        views::Slider::thumb_ = IsMouseHovered() ? thumb_hv : thumb_nor_;
    }
    else
    {
        views::Slider::thumb_ = thumb_dis_;
    }
}

bool BililiveSlider::OnMousePressed(const ui::MouseEvent& event)
{
    bool ret = __super::OnMousePressed(event);
    RequestFocus();
    return ret;
}

bool BililiveSlider::OnMouseWheel(const ui::MouseWheelEvent& event)
{
    if (views::View::enabled())
    {
        if (accept_mouse_wheel_)
        {
            static POINT point = { 0 };
            ::GetCursorPos(&point);
            gfx::Point pt(point);
            if (GetBoundsInScreen().Contains(pt))
            {
                float pre = value();
                pre += (event.y_offset() > 0) ? mousewheel_increment_ : -mousewheel_increment_;
                SetValueInternal(pre, views::SliderChangeReason::VALUE_CHANGED_BY_USER);
                return true;
            }
        }
    }
    return views::Slider::OnMouseWheel(event);
}

bool BililiveSlider::OnKeyPressed(const ui::KeyEvent& event)
{
    if (accept_key_press_)
    {
        return __super::OnKeyPressed(event);
    }
    return false;
}

void BililiveSlider::OnPaint(gfx::Canvas* canvas)
{
    OnPaintBackground(canvas);

    // 画滑条的时候在两端缩进一两个像素，防止滑块图边缘半透明时底部滑条色透上来
    static const int kThumbInsets = 1;

    gfx::Rect content = GetContentsBoundsImp();

    float value = __super::value();
    if (orientation_ == HORIZONTAL)
    {
        int thumb_y = content.y();
        switch (alignment_)
        {
        case SBA_LEADING:
            thumb_y = content.y();
            break;
        case SBA_CENTER:
            thumb_y = content.y() + std::round((content.height() - thumb_->height()) * 1.0f / 2);
            break;
        case SBA_TRAILING:
            thumb_y = content.bottom() - thumb_->height();
            break;
        default:
            break;
        }
        int bar_y = thumb_y + std::round((thumb_->height() - scrollbar_thickness_) * 1.0f / 2);

        int w = content.width() - thumb_->width();
        int full = value * w;
        int empty = w - full;

        canvas->FillRect(gfx::Rect(
            content.x() + kThumbInsets,
            bar_y,
            full + thumb_->width() / 2 - kThumbInsets,
            scrollbar_thickness_), full_color_);
        canvas->FillRect(gfx::Rect(
            content.x() + full + thumb_->width() / 2,
            bar_y,
            empty + thumb_->width() / 2 - kThumbInsets,
            scrollbar_thickness_), empty_color_);

        canvas->DrawImageInt(*thumb_, content.x() + full, thumb_y);
    }
    else
    {
        int thumb_x = content.x();
        switch (alignment_)
        {
        case SBA_LEADING:
            thumb_x = content.x();
            break;
        case SBA_CENTER:
            thumb_x = content.x() + std::round((content.width() - thumb_->width()) * 1.0f / 2);
            break;
        case SBA_TRAILING:
            thumb_x = content.right() - thumb_->width();
            break;
        default:
            break;
        }
        int bar_x = thumb_x + std::round((thumb_->width() - scrollbar_thickness_) * 1.0f / 2);

        int h = content.height() - thumb_->height();
        int full = value * h;
        int empty = h - full;

        canvas->FillRect(gfx::Rect(
            bar_x,
            content.y() + kThumbInsets,
            scrollbar_thickness_,
            empty + thumb_->height() / 2 - kThumbInsets), empty_color_);
        canvas->FillRect(gfx::Rect(
            bar_x,
            content.y() + empty + thumb_->height() / 2,
            scrollbar_thickness_,
            full + thumb_->height() / 2 - kThumbInsets), full_color_);

        canvas->DrawImageInt(*thumb_, thumb_x, content.y() + empty);
    }

    OnPaintBorder(canvas);
}

void BililiveSlider::OnEnabledChanged()
{
    views::Slider::thumb_ = enabled() ? thumb_nor_ : thumb_dis_;

    __super::OnEnabledChanged();
}

gfx::Size BililiveSlider::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    if (scroll_line_length_ > 0)
    {
        if (orientation_ == Slider::HORIZONTAL)
        {
            size.set_width(scroll_line_length_);
        }
        else
        {
            size.set_height(scroll_line_length_);
        }
    }
    return size;
}

bool BililiveSlider::HitTestRect(const gfx::Rect& rect) const
{
    if (!enabled())
    {
        return false;
    }
    return __super::HitTestRect(rect);
}

void BililiveSlider::OnMouseEntered(const ui::MouseEvent& event)
{
    __super::OnMouseEntered(event);

    views::Slider::thumb_ = thumb_hv_;
    SchedulePaint();

    ShowHelp(this);
}

void BililiveSlider::OnMouseExited(const ui::MouseEvent& event)
{
    __super::OnMouseExited(event);

    views::Slider::thumb_ = thumb_nor_;
    SchedulePaint();
}

void BililiveSlider::SetDrawInsets(const gfx::Insets &insets)
{
    // 不能为同方向边缘缩进，否则会和底层进行value计算的范围不一致
    if (orientation_ == HORIZONTAL)
    {
        DCHECK(insets.width() == 0);
    }
    else
    {
        DCHECK(insets.height() == 0);
    }
    draw_insets_ = insets;
}

gfx::Rect BililiveSlider::GetContentsBoundsImp() const
{
    gfx::Rect rect = GetContentsBounds();
    rect.Inset(draw_insets_);
    return rect;
}

views::Slider::Orientation BililiveSlider::orientation() const
{
    return orientation_;
}
