#include "bililive_bubble.h"

#include "base/bind.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/painter.h"
#include "ui/views/layout/fill_layout.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "grit/theme_resources.h"


namespace
{
    int kShadowBorderThickness = 9; // 阴影边框宽度
    // 从实际内容页向外扩展的宽度，用以绘制三角和阴影边框，应大于等于kShadowBorderThickness
    const int kBubbleBorderInsets = 10;
    // 箭头边缘距离矩形边缘的距离，即箭头不允许刚好在四个角落，需在四条边上
    const int kBubbleBorderArrowSideThickness = 5;
    const int kBubbleArrowWidth = 16;// 三角底边宽度
    const int kBubbleArrowHeight = 8;// 三角高度
    int kBubbleRadius = 0;// 当前的bubbble采用直角风格

    scoped_ptr<views::Painter> g_shadow_border_painter;

    class BililiveBubbleBorder : public views::BubbleBorder
    {
    public:
        BililiveBubbleBorder(Arrow arrow, Shadow shadow, SkColor color)
            : views::BubbleBorder(arrow, shadow, color)
        {
        }

        // BubbleBorder
        gfx::Rect GetBounds(const gfx::Rect& anchor_rect,
                            const gfx::Size& contents_size) const override
        {
            const int kStroke =0;
            // Enlarge the contents size by the thickness of the border images.
            gfx::Size size(contents_size);
            const gfx::Insets insets = GetInsets();
            size.Enlarge(insets.width(), insets.height());

            // Ensure the bubble is large enough to not overlap border and arrow images.
            const int min = 2 * GetLengthByImgDPIScale(kShadowBorderThickness);
            const int min_with_arrow_width = 2 * GetLengthByImgDPIScale(kBubbleBorderInsets);
            const int min_with_arrow_thickness = 2 * GetLengthByImgDPIScale(kBubbleBorderInsets);
            // Only take arrow image sizes into account when the bubble tip is shown.
            if (paint_arrow() == PAINT_TRANSPARENT || !has_arrow(arrow()))
                size.SetToMax(gfx::Size(min, min));
            else if (is_arrow_on_horizontal(arrow()))
                size.SetToMax(gfx::Size(min_with_arrow_width, min_with_arrow_thickness));
            else
                size.SetToMax(gfx::Size(min_with_arrow_thickness, min_with_arrow_width));

            int x = anchor_rect.x();
            int y = anchor_rect.y();
            int w = anchor_rect.width();
            int h = anchor_rect.height();
            const int arrow_offset = GetArrowOffset(size);
            //const int arrow_size = kBubbleArrowWidth;
                //images_->arrow_interior_thickness + kStroke - images_->arrow_thickness;
            const bool mid_anchor = (alignment() == ALIGN_ARROW_TO_MID_ANCHOR);

            // Calculate the bubble coordinates based on the border and arrow settings.
            if (is_arrow_on_horizontal(arrow()))
            {
                if (is_arrow_on_left(arrow()))
                {
                    x += mid_anchor ? w / 2 - arrow_offset : kStroke - GetBorderThickness();
                }
                else if (is_arrow_at_center(arrow()))
                {
                    x += w / 2 - arrow_offset;
                }
                else
                {
                    x += mid_anchor ? w / 2 + arrow_offset - size.width() :
                        w - size.width() + GetBorderThickness() - kStroke;
                }
                y += is_arrow_on_top(arrow()) ? h : -size.height();
            }
            else if (has_arrow(arrow()))
            {
                x += is_arrow_on_left(arrow()) ? w : -size.width();
                if (is_arrow_on_top(arrow()))
                {
                    y += mid_anchor ? h / 2 - arrow_offset : kStroke - GetBorderThickness();
                }
                else if (is_arrow_at_center(arrow()))
                {
                    y += h / 2 - arrow_offset;
                }
                else
                {
                    y += mid_anchor ? h / 2 + arrow_offset - size.height() :
                        h - size.height() + GetBorderThickness() - kStroke;
                }
            }
            else
            {
                x += (w - size.width()) / 2;
                y += (arrow() == NONE) ? h : (h - size.height()) / 2;
            }

            return gfx::Rect(x, y, size.width(), size.height());
        }

    protected:
        // Overridden from Border:
        gfx::Insets GetInsets() const override
        {
            static int insets = GetLengthByImgDPIScale(kBubbleBorderInsets);
            return gfx::Insets(insets, insets, insets, insets);
        }

        void Paint(const views::View& view, gfx::Canvas* canvas) override
        {
            static int insets = GetLengthByImgDPIScale(kShadowBorderThickness);

            // 绘制矩形背景边缘的阴影渐变
            if (!g_shadow_border_painter)
            {
                ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
                gfx::ImageSkia* img = rb.GetImageSkiaNamed(IDR_LIVEHIME_SHADOW_BORDER);
                g_shadow_border_painter.reset(views::Painter::CreateImagePainter(*img,
                    gfx::Insets(insets, insets, insets, insets)));
            }
            gfx::Rect shadow_bounds(view.GetContentsBounds());
            shadow_bounds.Inset(-insets, -insets);
            views::Painter::PaintPainterAt(canvas, g_shadow_border_painter.get(), shadow_bounds);

            // 绘制矩形背景
            gfx::Rect bounds(view.GetContentsBounds());
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setColor(background_color());
            canvas->DrawRoundRect(bounds, GetLengthByImgDPIScale(kBubbleRadius), paint);

            // 画三角
            const gfx::Rect arrow_bounds = GetArrowRect(view.GetLocalBounds());
            DrawArrow(canvas, arrow_bounds);
        }

        // BubbleBorder
        int GetArrowOffset(const gfx::Size& border_size) const override
        {
            const int edge_length = is_arrow_on_horizontal(arrow()) ?
                border_size.width() : border_size.height();
            if (is_arrow_at_center(arrow()) && arrow_offset() == 0)
                return edge_length / 2;

            // Calculate the minimum offset to not overlap arrow and corner images.
            //const int min = images_->border_thickness + (images_->top_arrow.width() / 2);
            const int min = GetLengthByImgDPIScale(
                kBubbleBorderInsets + kBubbleBorderArrowSideThickness + (kBubbleArrowWidth / 2));
            // Ensure the returned value will not cause image overlap, if possible.
            return std::max(min, std::min(arrow_offset(), edge_length - min));
        }

    private:
        int GetBorderThickness() const
        {
            return GetLengthByImgDPIScale(kBubbleBorderInsets);
        }

        gfx::Size GetArrowSize() const
        {
            if (is_arrow_on_horizontal(arrow()))
            {
                return GetSizeByImgDPIScale(gfx::Size(kBubbleArrowWidth, kBubbleArrowHeight));
                /*return is_arrow_on_top(arrow()) ?
                    &images_->top_arrow : &images_->bottom_arrow;*/
            }
            else if (has_arrow(arrow()))
            {
                return GetSizeByImgDPIScale(gfx::Size(kBubbleArrowHeight, kBubbleArrowWidth));
                /*return is_arrow_on_left(arrow()) ?
                    &images_->left_arrow : &images_->right_arrow;*/
            }
            return gfx::Size();
        }

        gfx::Rect GetArrowRect(const gfx::Rect& bounds) const
        {
            if (!has_arrow(arrow()) || paint_arrow() != PAINT_NORMAL)
                return gfx::Rect();

            gfx::Point origin;
            int offset = GetArrowOffset(bounds.size());
            const int half_length = GetLengthByImgDPIScale(kBubbleArrowWidth) / 2;
            const gfx::Insets insets = GetInsets();

            if (is_arrow_on_horizontal(arrow()))
            {
                origin.set_x(is_arrow_on_left(arrow()) || is_arrow_at_center(arrow()) ?
                    offset : bounds.width() - offset);
                origin.Offset(-half_length, 0);
                if (is_arrow_on_top(arrow()))
                    origin.set_y(bounds.y() + insets.top() - GetLengthByImgDPIScale(kBubbleArrowHeight));
                else
                    origin.set_y(bounds.height() - insets.bottom());
            }
            else
            {
                origin.set_y(is_arrow_on_top(arrow()) || is_arrow_at_center(arrow()) ?
                    offset : bounds.height() - offset);
                origin.Offset(0, -half_length);
                if (is_arrow_on_left(arrow()))
                    origin.set_x(bounds.x() + insets.left() - GetLengthByImgDPIScale(kBubbleArrowHeight));
                else
                    origin.set_x(bounds.right() - insets.right());
            }
            return gfx::Rect(origin, GetArrowSize());
        }

        void DrawArrow(gfx::Canvas* canvas, const gfx::Rect& arrow_bounds) const
        {
            SkPoint head, tail, mid;
            if (!arrow_bounds.IsEmpty())
            {
                //static gfx::ImageSkia* arrow_img = ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_V3_BUBBLE_TOP_ARROW);

                const bool horizontal = is_arrow_on_horizontal(arrow());
                if (horizontal)
                {
                    if (is_arrow_on_top(arrow()))
                    {
                        head = SkPoint::Make(arrow_bounds.x(), arrow_bounds.bottom());
                        tail = SkPoint::Make(arrow_bounds.right(), arrow_bounds.bottom());
                        mid = SkPoint::Make(arrow_bounds.x() + arrow_bounds.width() * 1.0f / 2, arrow_bounds.y());
                    }
                    else
                    {
                        head = SkPoint::Make(arrow_bounds.x(), arrow_bounds.y());
                        tail = SkPoint::Make(arrow_bounds.right(), arrow_bounds.y());
                        mid = SkPoint::Make(arrow_bounds.x() + arrow_bounds.width() * 1.0f / 2, arrow_bounds.bottom());
                    }
                }
                else
                {
                    if (is_arrow_on_left(arrow()))
                    {
                        head = SkPoint::Make(arrow_bounds.right(), arrow_bounds.y());
                        tail = SkPoint::Make(arrow_bounds.right(), arrow_bounds.bottom());
                        mid = SkPoint::Make(arrow_bounds.x(), arrow_bounds.y() + arrow_bounds.height() * 1.0f / 2);
                    }
                    else
                    {
                        head = SkPoint::Make(arrow_bounds.x(), arrow_bounds.y());
                        tail = SkPoint::Make(arrow_bounds.x(), arrow_bounds.bottom());
                        mid = SkPoint::Make(arrow_bounds.right(), arrow_bounds.y() + arrow_bounds.height() * 1.0f / 2);
                    }
                }

                SkPath path;
                path.moveTo(head);
                path.lineTo(tail);
                path.lineTo(mid);
                path.close();

                SkPaint paint;
                paint.setStyle(SkPaint::kFill_Style);
                paint.setColor(background_color());
                paint.setAntiAlias(true);
                canvas->DrawPath(path, paint);
            }
        }

    private:

        DISALLOW_COPY_AND_ASSIGN(BililiveBubbleBorder);
    };
}


BililiveBubbleView::BililiveBubbleView(views::View* anchor_view, views::BubbleBorder::Arrow arrow,
    const base::string16 &tooltip)
    : BubbleDelegateView(anchor_view, arrow)
    , tooltip_(tooltip)
    , content_view_(nullptr)
    , weakptr_factory_(this)
{
    InitStyle();
}

BililiveBubbleView::BililiveBubbleView(views::View* anchor_view, views::BubbleBorder::Arrow arrow,
    views::View* content_view)
    : BubbleDelegateView(anchor_view, arrow)
    , content_view_(content_view)
    , weakptr_factory_(this)
{
    InitStyle();
}

BililiveBubbleView::BililiveBubbleView(views::View* anchor_view, views::BubbleBorder::Arrow arrow)
    : BililiveBubbleView(anchor_view, arrow, nullptr)
{
    InitStyle();
}

BililiveBubbleView::~BililiveBubbleView()
{
}

void BililiveBubbleView::InitStyle()
{
    text_color_ = SkColorSetRGB(48, 48, 48);

    set_background_color(SK_ColorWHITE);
    set_close_on_esc(true);
    set_close_on_deactivate(true);
    set_adjust_if_offscreen(true);
    set_shadow(views::BubbleBorder::SMALL_SHADOW);
    //set_use_focusless(true);
    //set_move_with_anchor(true);
    if (!anchor_view() || !anchor_view()->GetWidget())
    {
        NOTREACHED();
        LOG(WARNING) << "show bubble while anchor_view invalid.";
    }
    if (anchor_view() && anchor_view()->GetWidget())
    {
        set_parent_window(anchor_view()->GetWidget()->GetNativeWindow());
    }
}

views::NonClientFrameView* BililiveBubbleView::CreateNonClientFrameView(views::Widget* widget)
{
    views::BubbleFrameView* frame = new views::BubbleFrameView(margins());
    const views::BubbleBorder::Arrow adjusted_arrow = base::i18n::IsRTL() ?
        views::BubbleBorder::horizontal_mirror(arrow()) : arrow();
    BililiveBubbleBorder *bubble_border = new BililiveBubbleBorder(adjusted_arrow, shadow(), color());
    frame->SetBubbleBorder(bubble_border);
    return frame;
}

void BililiveBubbleView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            if (!content_view_)
            {
                SetLayoutManager(new views::BoxLayout(views::BoxLayout::kVertical, 0, 0, 0));
                def_label_ = new BililiveLabel(tooltip_);
                def_label_->SetTextColor(text_color_);
                def_label_->SetFont(text_font_);
                def_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
                AddChildView(def_label_);
            }
            else
            {
                SetLayoutManager(new views::FillLayout());
                AddChildView(content_view_);
            }
        }
    }
}

void BililiveBubbleView::OnNativeThemeChanged(const ui::NativeTheme* theme)
{
}

void BililiveBubbleView::StartFade(bool fade_in, int after_timespan_ms, int fade_duration_ms)
{
    fade_duration_ms_ = std::max(fade_duration_ms, 0);
    base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
        base::Bind(&views::BubbleDelegateView::StartFade, weakptr_factory_.GetWeakPtr(), fade_in),
        base::TimeDelta::FromMilliseconds(after_timespan_ms));
}

void BililiveBubbleView::ResetFade()
{
    // 先把原先可能未执行的StartFade任务废止
    weakptr_factory_.InvalidateWeakPtrs();

    BubbleDelegateView::ResetFade();
}

int BililiveBubbleView::GetFadeDuration()
{
    return fade_duration_ms_;
}

void BililiveBubbleView::set_background_color(SkColor clr)
{
    // content_view以外的背景
    set_color(clr);
    // 具体的content_view的背景，如label区域
    set_background(views::Background::CreateSolidBackground(clr));

    views::BubbleFrameView* frame_view = GetBubbleFrameView();
    if (frame_view && frame_view->bubble_border())
    {
        frame_view->bubble_border()->set_background_color(clr);
    }
}

void BililiveBubbleView::set_text_color(SkColor clr)
{
    text_color_ = clr;
    if (def_label_)
    {
        def_label_->SetTextColor(text_color_);
    }
}

void BililiveBubbleView::SetFont(const gfx::Font& font)
{
    text_font_ = font;
    if (def_label_)
    {
        def_label_->SetFont(text_font_);
    }
}

void BililiveBubbleView::SetRadius(const int radius) 
{
    kBubbleRadius = radius;// 当前的bubbble采用直角风格
}

void BililiveBubbleView::SetShadowBorderThickness(const int borderThickness)
{
    kShadowBorderThickness = borderThickness;// 阴影边框宽度
}

void BililiveBubbleView::UpdatePosition()
{
    SizeToContents();
}
