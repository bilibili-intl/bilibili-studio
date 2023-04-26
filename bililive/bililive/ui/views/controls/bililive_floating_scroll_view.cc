#include "bililive_floating_scroll_view.h"

#include "ui/gfx/canvas.h"

#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

namespace
{
    const int kFloatingScrollbarWidth = GetLengthByDPIScale(17);// GetSystemMetrics(SM_CXVSCROLL);
    const int kFloatingScrollbarHeight = GetLengthByDPIScale(17);// GetSystemMetrics(SM_CYHSCROLL);
    const int kScrollButtonSize = GetLengthByDPIScale(6);
    const int kFloatingScrollbarVisibleWidth = GetLengthByDPIScale(10);
    const int kFloatingScrollbarVisibleHeight = GetLengthByDPIScale(10);
    const int kFloatingMinScrollbarVisibleWidth = GetLengthByDPIScale(7);
    const int kFloatingMinScrollbarVisibleHeight = GetLengthByDPIScale(7);
    const int kMinimumSize = GetLengthByDPIScale(10);
    const int nFloatingScrollbarBorderThickness = GetLengthByDPIScale(2);
    const float kFloatingScrollbarPressedIncrement = 0.2f;

    const SkColor scrollbar_hoverd_bk_color_ = SK_ColorWHITE;//SkColorSetRGB(203, 203, 203);

    // Returns the position for the view so that it isn't scrolled off the visible
    // region.
    int CheckScrollBounds(int viewport_size, int content_size, int current_pos)
    {
        int max = std::max(content_size - viewport_size, 0);
        if (current_pos < 0)
            return 0;
        if (current_pos > max)
            return max;
        return current_pos;
    }

    // Make sure the content is not scrolled out of bounds
    void CheckScrollBounds(views::View* viewport, views::View* view)
    {
        if (!view)
            return;

        int x = CheckScrollBounds(viewport->width(), view->width(), -view->x());
        int y = CheckScrollBounds(viewport->height(), view->height(), -view->y());

        // This is no op if bounds are the same
        view->SetBounds(-x, -y, view->width(), view->height());
    }

    // Used by ScrollToPosition() to make sure the new position fits within the
    // allowed scroll range.
    int AdjustPosition(int current_position,
        int new_position,
        int content_size,
        int viewport_size)
    {
        if (-current_position == new_position)
            return new_position;
        if (new_position < 0)
            return 0;
        const int max_position = std::max(0, content_size - viewport_size);
        return (new_position > max_position) ? max_position : new_position;
    }

}


// FloatingScrollBarBaseCtrl
FloatingScrollBarBaseCtrl::FloatingScrollBarBaseCtrl(FloatingScrollBar *scroll_bar, CtrlType ct)
    : scroll_bar_(scroll_bar)
    , ctrl_type_(ct)
    , scroll_view_(nullptr)
    , state_(ThumbState_OUTOFFCLT)
{
}

void FloatingScrollBarBaseCtrl::SetBounds(int x, int y, int width, int height)
{
    SetBoundsRect(gfx::Rect(x, y, width, height));
}

void FloatingScrollBarBaseCtrl::SetBoundsRect(const gfx::Rect& bounds)
{
    bounds_ = bounds;

    if (scroll_bar_->is_horiz())
    {
        int ori_y = bounds_.bottom()
            - scroll_bar_->GetScrollBarBorderThickness()
            - scroll_bar_->GetScrollBarVisibleHeight();
        switch (ctrl_type_)
        {
        case CT_BUTTON_PREV:
            visible_bounds_.SetRect(
                bounds_.right()
                    - scroll_bar_->GetScrollBarBorderThickness()
                    - scroll_bar_->GetScrollBarVisibleWidth(),
                ori_y,
                scroll_bar_->GetScrollBarVisibleWidth(),
                scroll_bar_->GetScrollBarVisibleHeight());
            break;
        case CT_THUMB:
            visible_bounds_.SetRect(
                bounds_.x(),
                ori_y,
                bounds_.width(),
                scroll_bar_->GetScrollBarVisibleHeight());
            break;
        case CT_BUTTON_NEXT:
            visible_bounds_.SetRect(
                bounds_.x() + scroll_bar_->GetScrollBarBorderThickness(),
                ori_y,
                scroll_bar_->GetScrollBarVisibleWidth(),
                scroll_bar_->GetScrollBarVisibleHeight());
            break;
        default:
            break;
        }
    }
    else
    {
        int ori_x = bounds_.right()
            - scroll_bar_->GetScrollBarBorderThickness()
            - scroll_bar_->GetScrollBarVisibleWidth();
        switch (ctrl_type_)
        {
        case CT_BUTTON_PREV:
            visible_bounds_.SetRect(
                ori_x,
                bounds_.bottom()
                    - scroll_bar_->GetScrollBarBorderThickness()
                    - scroll_bar_->GetScrollBarVisibleHeight(),
                scroll_bar_->GetScrollBarVisibleWidth(),
                scroll_bar_->GetScrollBarVisibleHeight());
            break;
        case CT_THUMB:
            visible_bounds_.SetRect(
                ori_x,
                bounds_.y(),
                scroll_bar_->GetScrollBarVisibleWidth(),
                bounds_.height());
            break;
        case CT_BUTTON_NEXT:
            visible_bounds_.SetRect(
                ori_x,
                bounds_.y() + scroll_bar_->GetScrollBarBorderThickness(),
                scroll_bar_->GetScrollBarVisibleWidth(),
                scroll_bar_->GetScrollBarVisibleHeight());
            break;
        default:
            break;
        }
    }
}

bool FloatingScrollBarBaseCtrl::OnMouseMoved(const ui::MouseEvent& event)
{
    bool request_paint = false;
    if (visible_bounds_.Contains(event.location()))
    {
        if (state_ != ThumbState_HOVERED)
        {
            state_ = ThumbState_HOVERED;
            request_paint = true;
        }
    }
    else
    {
        if (state_ != ThumbState_NORMAL)
        {
            state_ = ThumbState_NORMAL;
            request_paint = true;
        }
    }
    return request_paint;
}


// FloatingScrollBarButton
FloatingScrollBarButton::FloatingScrollBarButton(FloatingScrollBar *scroll_bar, bool prev, bool is_bar_narrow)
    : FloatingScrollBarBaseCtrl(scroll_bar, prev ? CT_BUTTON_PREV : CT_BUTTON_NEXT)
    , is_prev_(prev), is_bar_narrow_(is_bar_narrow)
{
    thumb_colors_[ThumbState_NORMAL] = SkColorSetA(SK_ColorBLACK, 0.3f * 255);
    thumb_colors_[ThumbState_HOVERED] = SkColorSetA(SK_ColorBLACK, 0.4f * 255);
    thumb_colors_[ThumbState_PRESSED] = SkColorSetA(SK_ColorBLACK, 0.5f * 255);
}

void FloatingScrollBarButton::SetColor(ThumbState state, SkColor color) {
    DCHECK(state < ThumbState_COUNT);
    thumb_colors_[state] = color;
}

void FloatingScrollBarButton::OnPaint(gfx::Canvas* canvas)
{
    SkColor clrThumb = SK_ColorRED;
    switch (state_)
    {
    case ThumbState_OUTOFFCLT:
        // 鼠标不在窗口内不显示滚动条
        break;
    case ThumbState_NORMAL:
        clrThumb = thumb_colors_[ThumbState_NORMAL];
        break;
    case ThumbState_HOVERED:
        clrThumb = thumb_colors_[ThumbState_HOVERED];
        break;
    case ThumbState_PRESSED:
        clrThumb = thumb_colors_[ThumbState_PRESSED];
        break;
    default:
        break;
    }
    if (state_ != ThumbState_OUTOFFCLT)
    {
        enum ArrowDir
        {
            UP, DOWN, LEFT, RIGHT
        }arrow;
        if (scroll_bar_->is_horiz())
        {
            arrow = is_prev_ ? LEFT : RIGHT;
        }
        else
        {
            arrow = is_prev_ ? UP : DOWN;
        }
        gfx::Rect rect = scroll_view_->ConvertRectToParent(visible_bounds());
        SkPoint pts[3];
        int scroll_button_size = is_bar_narrow_ ? kScrollButtonSize / 2 : kScrollButtonSize;
        switch (arrow)
        {
        case UP:
        {
            int center_pt_x, center_pt_y;
            center_pt_x = rect.x() + std::ceil(rect.width() / 2);
            center_pt_y = rect.y() + std::ceil(rect.height() - scroll_button_size) / 2;

            pts[0] = SkPoint::Make(center_pt_x, center_pt_y);

            pts[1] = SkPoint::Make(rect.x(), center_pt_y + scroll_button_size);

            pts[2] = SkPoint::Make(rect.right(), center_pt_y + scroll_button_size);
        }
        break;
        case DOWN:
        {
            int center_pt_x, center_pt_y;
            center_pt_x = rect.x() + std::ceil(rect.width() / 2);
            center_pt_y = rect.y() + std::ceil(rect.height() - scroll_button_size) / 2 + scroll_button_size;

            pts[0] = SkPoint::Make(center_pt_x, center_pt_y);

            pts[1] = SkPoint::Make(rect.x(), center_pt_y - scroll_button_size);

            pts[2] = SkPoint::Make(rect.right(), center_pt_y - scroll_button_size);
        }
        break;
        case LEFT:
        {
            int center_pt_x, center_pt_y;
            center_pt_x = rect.x() + std::ceil(rect.width() - scroll_button_size) / 2;
            center_pt_y = rect.y() + std::ceil(rect.height() / 2);

            pts[0] = SkPoint::Make(center_pt_x, center_pt_y);

            pts[1] = SkPoint::Make(center_pt_x + scroll_button_size, rect.y());

            pts[2] = SkPoint::Make(center_pt_x + scroll_button_size, rect.bottom());
        }
        break;
        case RIGHT:
        {
            int center_pt_x, center_pt_y;
            center_pt_x = rect.x() + std::ceil(rect.width() - scroll_button_size) / 2 + scroll_button_size;
            center_pt_y = rect.y() + std::ceil(rect.height() / 2);

            pts[0] = SkPoint::Make(center_pt_x, center_pt_y);

            pts[1] = SkPoint::Make(center_pt_x - scroll_button_size, rect.y());

            pts[2] = SkPoint::Make(center_pt_x - scroll_button_size, rect.bottom());
        }
        break;
        default:
            break;
        }
        SkPath path;
        path.addPoly(pts, 3, true);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(clrThumb);
        canvas->DrawPath(path, paint);
    }
}

bool FloatingScrollBarButton::OnMousePressed(const ui::MouseEvent& event)
{
    if (visible_bounds().Contains(event.location()))
    {
        SetState(ThumbState_PRESSED);
        int inc = kFloatingScrollbarPressedIncrement * scroll_bar_->GetMaxPosition();
        if (is_prev_)
        {
            scroll_view_->ScrollToPosition(!scroll_bar_->is_horiz(), scroll_bar_->GetPosition() - inc, ButtonPress);
        }
        else
        {
            scroll_view_->ScrollToPosition(!scroll_bar_->is_horiz(), scroll_bar_->GetPosition() + inc, ButtonPress);
        }
    }
    return false;
}


// FloatingScrollBarThumb
FloatingScrollBarThumb::FloatingScrollBarThumb(FloatingScrollBar *scroll_bar)
    : FloatingScrollBarBaseCtrl(scroll_bar, CT_THUMB)
{
    thumb_colors_[ThumbState_NORMAL] = SkColorSetA(SK_ColorBLACK, 0.3f * 255);
    thumb_colors_[ThumbState_HOVERED] = SkColorSetA(SK_ColorBLACK, 0.4f * 255);
    thumb_colors_[ThumbState_PRESSED] = SkColorSetA(SK_ColorBLACK, 0.5f * 255);
}

void FloatingScrollBarThumb::SetColor(ThumbState state, SkColor color) {
    DCHECK(state < ThumbState_COUNT);
    thumb_colors_[state] = color;
}

void FloatingScrollBarThumb::SetThumbBoundsRect(const gfx::Rect& thumb_bound)
{
    if (scroll_bar_->is_horiz())
    {
        int ori_y = thumb_bound.bottom()
            - scroll_bar_->GetScrollBarBorderThickness()
            - scroll_bar_->GetScrollBarVisibleHeight();
        thumb_bounds_.SetRect(
            thumb_bound.x(),
            ori_y,
            thumb_bound.width(),
            scroll_bar_->GetScrollBarVisibleHeight());
    }
    else
    {
        int ori_x = thumb_bound.right()
            - scroll_bar_->GetScrollBarBorderThickness()
            - scroll_bar_->GetScrollBarVisibleWidth();
        thumb_bounds_.SetRect(
            ori_x,
            thumb_bound.y(),
            scroll_bar_->GetScrollBarVisibleWidth(),
            thumb_bound.height());
    }
}

void FloatingScrollBarThumb::OnPaint(gfx::Canvas* canvas)
{
    SkColor clrThumb = SK_ColorRED;
    SkColor clrBk = scrollbar_hoverd_bk_color_;
    switch (state_)
    {
    case ThumbState_OUTOFFCLT:
        // 鼠标不在窗口内不显示滚动条
        break;
    case ThumbState_NORMAL:
        clrThumb = thumb_colors_[ThumbState_NORMAL];
        break;
    case ThumbState_HOVERED:
        clrThumb = thumb_colors_[ThumbState_HOVERED];
        break;
    case ThumbState_PRESSED:
        clrThumb = thumb_colors_[ThumbState_PRESSED];
        break;
    default:
        break;
    }
    if (state_ != ThumbState_OUTOFFCLT)
    {
        gfx::Rect bar_rect = scroll_view_->ConvertRectToParent(visible_bounds());
        gfx::Rect thumb_rect = scroll_view_->ConvertRectToParent(thumb_bounds());

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(clrThumb);
        if (scroll_bar_->is_horiz())
        {
            bililive::DrawCircleRect(canvas,
                thumb_rect.x(), thumb_rect.y(), thumb_rect.width(), thumb_rect.height(), paint);
        }
        else
        {
            bililive::DrawCircleRect(canvas,
                thumb_rect.x(), thumb_rect.y(), thumb_rect.width(), thumb_rect.height(), paint, false);
        }
    }
}

bool FloatingScrollBarThumb::OnMousePressed(const ui::MouseEvent& event)
{
    // 点在滚动滑块上就return true，否则false
    if (thumb_bounds().Contains(event.location()))
    {
        SetState(ThumbState_PRESSED);
        scroll_view_->SchedulePaintInRect(bounds());
        return true;
    }
    else
    {
        if (visible_bounds().Contains(event.location()))
        {
            int inc = kFloatingScrollbarPressedIncrement * scroll_bar_->GetMaxPosition();
            if (event.location().y() < thumb_bounds().y())
            {
                scroll_view_->ScrollToPosition(!scroll_bar_->is_horiz(), scroll_bar_->GetPosition() - inc, ThumbPress);
            }
            else if (event.location().y() > thumb_bounds().bottom())
            {
                scroll_view_->ScrollToPosition(!scroll_bar_->is_horiz(), scroll_bar_->GetPosition() + inc, ThumbPress);
            }
        }
    }
    return false;
}


// FloatingScrollBar
FloatingScrollBar::FloatingScrollBar(bool is_horiz,bool is_bar_narrow)
    : is_horiz_(is_horiz)
    , visible_(false)
    , max_pos_(0)
    , current_pos_(0)
    , scroll_view_(nullptr)
    , viewport_size_(0)
    , content_size_(0)
    , prev_button_(new FloatingScrollBarButton(this, true, is_bar_narrow))
    , thumb_(new FloatingScrollBarThumb(this))
    , next_button_(new FloatingScrollBarButton(this, false, is_bar_narrow))
    , is_bar_narrow_(is_bar_narrow)
{
    ResetState(ThumbState_OUTOFFCLT);
}

FloatingScrollBar::~FloatingScrollBar()
{
    delete prev_button_;
    delete thumb_;
    delete next_button_;
}

void FloatingScrollBar::SetColor(ThumbState state, SkColor color) {
    prev_button_->SetColor(state, color);
    thumb_->SetColor(state, color);
    next_button_->SetColor(state, color);
}

void FloatingScrollBar::SetScrollView(ScrollViewWithFloatingScrollbar *scroll_view)
{
    scroll_view_ = scroll_view;
    prev_button_->SetScrollView(scroll_view_);
    thumb_->SetScrollView(scroll_view_);
    next_button_->SetScrollView(scroll_view_);
}

void FloatingScrollBar::ResetState(ThumbState state)
{
    prev_button_->SetState(state);
    thumb_->SetState(state);
    next_button_->SetState(state);
}

ThumbState FloatingScrollBar::thumb_state() const
{
    return thumb_->state();
}

int FloatingScrollBar::thumb_range() const
{
    return is_horiz_
        ? (thumb_->width() - thumb_->thumb_bounds().width())
        : (thumb_->height() - thumb_->thumb_bounds().height());
}

void FloatingScrollBar::SetVisible(bool visible)
{
    visible_ = visible;
}

void FloatingScrollBar::SetBounds(int x, int y, int width, int height)
{
    SetBoundsRect(gfx::Rect(x, y, width, height));
}

void FloatingScrollBar::SetBoundsRect(const gfx::Rect& bounds)
{
    bounds_ = bounds;

    gfx::Rect prev, middle, next;
    GetOperateRect(bounds_, &prev, &middle, &next);
    prev_button_->SetBoundsRect(prev);
    thumb_->SetBoundsRect(middle);
    next_button_->SetBoundsRect(next);
}

void FloatingScrollBar::GetOperateRect(const gfx::Rect &bound,
    gfx::Rect *prev, gfx::Rect *middle, gfx::Rect *next) const
{
    if (visible_)
    {
        if (is_horiz_)
        {
            *middle = gfx::Rect(bound.x() + GetScrollBarWidth(), bound.y(),
                bound.width() - GetScrollBarWidth() * 2, GetScrollBarHeight());
            *prev = gfx::Rect(bound.x(), bound.y(),
                GetScrollBarWidth(), GetScrollBarHeight());
            *next = gfx::Rect(bound.right() - GetScrollBarWidth(), bound.y(),
                GetScrollBarWidth(), GetScrollBarHeight());
        }
        else
        {
            *middle = gfx::Rect(bound.right() - GetScrollBarWidth(), bound.y() + GetScrollBarHeight(),
                GetScrollBarWidth(), bound.height() - GetScrollBarHeight() * 2);
            *prev = gfx::Rect(bound.right() - GetScrollBarWidth(), bound.y(),
                GetScrollBarWidth(), GetScrollBarHeight());
            *next = gfx::Rect(bound.right() - GetScrollBarWidth(), bound.bottom() - GetScrollBarHeight(),
                GetScrollBarWidth(), GetScrollBarHeight());
        }
    }
}

void FloatingScrollBar::Update(int viewport_size, int content_size, int current_pos)
{
    max_pos_ = std::max(0, content_size - viewport_size);
    current_pos_ = current_pos;
    viewport_size_ = viewport_size;
    content_size_ = content_size;

    gfx::Rect thumb;
    if (is_horiz_)
    {
        int thumb_size = std::min(1.0f, viewport_size_ * 1.0f / content_size_) * thumb_->width();
        thumb_size = std::max(kMinimumSize, thumb_size);
        thumb = gfx::Rect(thumb_->x() + GetPosition() * 1.0f / GetMaxPosition() * (thumb_->width() - thumb_size), thumb_->y(),
            thumb_size, GetScrollBarHeight());
    }
    else
    {
        int thumb_size = std::min(1.0f, viewport_size_ * 1.0f / content_size_) * thumb_->height();
        thumb_size = std::max(kMinimumSize, thumb_size);
        thumb = gfx::Rect(thumb_->x(), thumb_->y() + GetPosition() * 1.0f / GetMaxPosition() * (thumb_->height() - thumb_size),
            GetScrollBarWidth(), thumb_size);
    }
    thumb_->SetThumbBoundsRect(thumb);

    scroll_view_->SchedulePaintInRect(bounds());
}

int FloatingScrollBar::GetMaxPosition() const
{
    return max_pos_;
}

int FloatingScrollBar::GetMinPosition() const
{
    return 0;
}

int FloatingScrollBar::GetPosition() const
{
    return current_pos_;
}

int FloatingScrollBar::GetScrollBarWidth() const {
    return kFloatingScrollbarWidth;
}

int FloatingScrollBar::GetScrollBarHeight() const {
    return kFloatingScrollbarHeight;
}

int FloatingScrollBar::GetScrollBarVisibleWidth() const {
    return is_bar_narrow_ ? kFloatingMinScrollbarVisibleWidth : kFloatingScrollbarVisibleWidth;
}

int FloatingScrollBar::GetScrollBarVisibleHeight() const {
    return is_bar_narrow_ ? kFloatingMinScrollbarVisibleHeight : kFloatingScrollbarVisibleHeight;
}

int FloatingScrollBar::GetScrollBarBorderThickness() const {
    return nFloatingScrollbarBorderThickness;
}

void FloatingScrollBar::OnPaint(gfx::Canvas* canvas)
{
    if (visible())
    {
        prev_button_->OnPaint(canvas);
        thumb_->OnPaint(canvas);
        next_button_->OnPaint(canvas);
    }
}

bool FloatingScrollBar::OnMousePressed(const ui::MouseEvent& event)
{
    if (thumb_->OnMousePressed(event))
    {
        drag_start_pos_ = GetPosition();
        drag_thumb_down_point_ = event.location();
        return true;
    }
    else
    {
        prev_button_->OnMousePressed(event);
        next_button_->OnMousePressed(event);
    }
    return false;
}

bool FloatingScrollBar::OnMouseDragged(const ui::MouseEvent& event)
{
    gfx::Point point = event.location();
    if (visible() && (thumb_state() == ThumbState_PRESSED))
    {
        int px_offset = 0;
        if (is_horiz_)
        {
            px_offset = point.x() - drag_thumb_down_point_.x();
        }
        else
        {
            px_offset = point.y() - drag_thumb_down_point_.y();
        }
        int max_drag_range = thumb_range();
        int scale_offset = -std::round((px_offset * 1.0f / max_drag_range) * GetMaxPosition());
        // 下滚负数，上滚正数
        scroll_view_->ScrollToPosition(!is_horiz_, drag_start_pos_ - scale_offset, Drag);
    }
    return true;
}

void FloatingScrollBar::OnMouseMoved(const ui::MouseEvent& event)
{
    bool request_paint = false;
    request_paint |= prev_button_->OnMouseMoved(event);
    request_paint |= thumb_->OnMouseMoved(event);
    request_paint |= next_button_->OnMouseMoved(event);

    if (request_paint)
    {
        scroll_view_->SchedulePaintInRect(bounds());
    }
}


// Viewport contains the contents View of the ScrollViewWithFloatingScrollbar.
class ScrollViewWithFloatingScrollbar::Viewport : public views::View
{
public:
    Viewport()
    {
        set_notify_enter_exit_on_child(true);
    };
    virtual ~Viewport() {}

protected:
    // View
    virtual const char* GetClassName() const OVERRIDE{
        return "ScrollViewWithFloatingScrollbar::Viewport";
    }

    virtual void ScrollRectToVisible(const gfx::Rect& rect) OVERRIDE
    {
        if (!has_children() || !parent())
        {
            return;
        }

        View* contents = child_at(0);
        gfx::Rect scroll_rect(rect);
        scroll_rect.Offset(-contents->x(), -contents->y());
        static_cast<ScrollViewWithFloatingScrollbar*>(parent())->ScrollContentsRegionToBeVisible(scroll_rect);
    }

    virtual void ChildPreferredSizeChanged(View* child) OVERRIDE
    {
        InvalidateLayout();
        PreferredSizeChanged();
    }

private:
    DISALLOW_COPY_AND_ASSIGN(Viewport);
};


// ScrollViewWithFloatingScrollbar
char ScrollViewWithFloatingScrollbar::kViewClassName[] = "ScrollViewWithFloatingScrollbar";

ScrollViewWithFloatingScrollbar::ScrollViewWithFloatingScrollbar(views::View *contents, bool is_bar_narrow)
    : hide_horizontal_scrollbar_(false)
    , header_viewport_(new Viewport())
    , header_(nullptr)
    , contents_viewport_(new Viewport())
    , contents_(contents)
    , horiz_sb_(new FloatingScrollBar(true, is_bar_narrow))
    , vert_sb_(new FloatingScrollBar(false, is_bar_narrow))
    , scroll_reason_(NoMove)
{
    horiz_sb_->SetScrollView(this);
    vert_sb_->SetScrollView(this);

    AddChildView(header_viewport_);
    AddChildView(contents_viewport_);

    set_notify_enter_exit_on_child(true);
    //set_focusable(true);
    //set_focus_border(nullptr);
}

ScrollViewWithFloatingScrollbar::~ScrollViewWithFloatingScrollbar()
{
    if (horiz_sb_)
    {
        delete horiz_sb_;
    }
    if (vert_sb_)
    {
        delete vert_sb_;
    }
}

void ScrollViewWithFloatingScrollbar::SetScrollThumbColor(ThumbState state, SkColor color)
{
    horiz_sb_->SetColor(state, color);
    vert_sb_->SetColor(state, color);
    SchedulePaint();
}

void ScrollViewWithFloatingScrollbar::SetScrollPositionChangeListener(ScrollPositionChangeListener* l) {
    listener_ = l;
}

void ScrollViewWithFloatingScrollbar::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this && details.is_add)
    {
        if (!contents_->parent())
        {
            views::View *tmpview = contents_;
            contents_ = nullptr;
            SetContents(tmpview);
        }
    }
}

gfx::Size ScrollViewWithFloatingScrollbar::GetPreferredSize()
{
    int w = 0;
    int h = 0;
    gfx::Size size;
    if (header_)
    {
        size = header_->GetPreferredSize();
        w = size.width();
        h = size.height();
    }
    if (contents_)
    {
        size = contents_->GetPreferredSize();
        w = std::max(w, size.width());
        h += size.height();
    }
    if (border())
    {
        w += border()->GetInsets().width();
        h += border()->GetInsets().height();
    }
    size.SetSize(w, h);
    return size;
}

void ScrollViewWithFloatingScrollbar::Layout()
{
    // Most views will want to auto-fit the available space. Most of them want to
    // use all available width (without overflowing) and only overflow in
    // height. Examples are HistoryView, MostVisitedView, DownloadTabView, etc.
    // Other views want to fit in both ways. An example is PrintView. To make both
    // happy, assume a vertical scrollbar but no horizontal scrollbar. To override
    // this default behavior, the inner view has to calculate the available space,
    // used ComputeScrollBarsVisibility() to use the same calculation that is done
    // here and sets its bound to fit within.
    gfx::Rect viewport_bounds = GetContentsBounds();
    if (viewport_bounds.IsEmpty()) {
        // There's nothing to layout.
        return;
    }
    const int contents_x = viewport_bounds.x();
    const int contents_y = viewport_bounds.y();

    const int header_height = std::min(viewport_bounds.height(), header_ ? header_->GetHeightForWidth(viewport_bounds.width()) : 0);
    viewport_bounds.set_height(std::max(0, viewport_bounds.height() - header_height));
    viewport_bounds.set_y(viewport_bounds.y() + header_height);
    // viewport_size is the total client space available.
    gfx::Size viewport_size = viewport_bounds.size();
    // Assumes a vertical scrollbar since most of the current views are designed
    // for this.
    int horiz_sb_height = GetScrollBarHeight();
    int vert_sb_width = GetScrollBarWidth();
    // Update the bounds right now so the inner views can fit in it.
    contents_viewport_->SetBoundsRect(viewport_bounds);

    // Give |contents_| a chance to update its bounds if it depends on the
    // viewport.
    if (contents_)
    {
        contents_->Layout();
    }

    bool horiz_sb_required = false;
    bool vert_sb_required = false;
    if (contents_)
    {
        gfx::Size content_size = contents_->size();
        ComputeScrollBarsVisibility(
            viewport_size,
            content_size,
            &horiz_sb_required,
            &vert_sb_required);
    }
    bool resize_corner_is_shown = /*resize_corner_ && */horiz_sb_required && vert_sb_required;
    // Take action.
    SetControlVisibility(false, horiz_sb_required);
    SetControlVisibility(true, vert_sb_required);
    //SetControlVisibility(resize_corner_, resize_corner_required);

    if (horiz_sb_required) {
        horiz_sb_->SetBounds(
            viewport_bounds.x(),
            viewport_bounds.bottom() - horiz_sb_height,
            viewport_bounds.width() - (vert_sb_required ? vert_sb_width : 0),
            horiz_sb_height);
    }
    if (vert_sb_required) {
        vert_sb_->SetBounds(
            viewport_bounds.right() - vert_sb_width,
            viewport_bounds.y(),
            vert_sb_width,
            viewport_bounds.height() - (horiz_sb_required ? horiz_sb_height : 0));
    }
    //if (resize_corner_is_shown) {
    //    // Show the resize corner.
    //    resize_corner_rect_.SetRect(
    //        viewport_bounds.right() - vert_sb_width,
    //        viewport_bounds.bottom() - horiz_sb_height,
    //        vert_sb_width,
    //        horiz_sb_height);
    //}

    header_viewport_->SetBounds(contents_x, contents_y, viewport_bounds.width(), header_height);
    if (header_)
    {
        //header_->Layout();
        // header不需要滚动，就不需要像content一样自己setsize来让ScrollView计算滚动条了
        // 只需要在ScrollView里直接设置它的尺寸即可
        header_->SetSize(header_viewport_->size());
    }

    // Update to the real client size with the visible scrollbars.
    contents_viewport_->SetBoundsRect(viewport_bounds);

    CheckScrollBounds(header_viewport_, header_);
    CheckScrollBounds(contents_viewport_, contents_);
    UpdateScrollBarPositions();
}

void ScrollViewWithFloatingScrollbar::Paint(gfx::Canvas* canvas)
{
    __super::Paint(canvas);

    if (IsDrawn())
    {
        if (vert_sb_->visible())
        {
            vert_sb_->OnPaint(canvas);
        }
        if (horiz_sb_->visible())
        {
            horiz_sb_->OnPaint(canvas);
        }
    }
}

views::View* ScrollViewWithFloatingScrollbar::GetEventHandlerForPoint(const gfx::Point& point)
{
    if (vert_sb_->visible())
    {
        if (vert_sb_->bounds().Contains(point))
        {
            return this;
        }
    }
    if (horiz_sb_->visible())
    {
        if (horiz_sb_->bounds().Contains(point))
        {
            return this;
        }
    }
    return __super::GetEventHandlerForPoint(point);
}

void ScrollViewWithFloatingScrollbar::OnMouseEntered(const ui::MouseEvent& event)
{
    vert_sb_->ResetState(ThumbState_NORMAL);
    horiz_sb_->ResetState(ThumbState_NORMAL);
    if (vert_sb_->visible())
    {
        SchedulePaintInRect(vert_sb_->bounds());
    }
    if (horiz_sb_->visible())
    {
        SchedulePaintInRect(horiz_sb_->bounds());
    }
}

void ScrollViewWithFloatingScrollbar::OnMouseMoved(const ui::MouseEvent& event)
{
    if (horiz_sb_->visible())
    {
        horiz_sb_->OnMouseMoved(event);
    }
    if (vert_sb_->visible())
    {
        vert_sb_->OnMouseMoved(event);
    }
}

void ScrollViewWithFloatingScrollbar::OnMouseExited(const ui::MouseEvent& event)
{
    vert_sb_->ResetState(ThumbState_OUTOFFCLT);
    horiz_sb_->ResetState(ThumbState_OUTOFFCLT);
    if (vert_sb_->visible())
    {
        SchedulePaintInRect(vert_sb_->bounds());
    }
    if (horiz_sb_->visible())
    {
        SchedulePaintInRect(horiz_sb_->bounds());
    }
}

void ScrollViewWithFloatingScrollbar::ChildPreferredSizeChanged(views::View* child)
{
    InvalidateLayout();
    Layout();
}

bool ScrollViewWithFloatingScrollbar::OnMousePressed(const ui::MouseEvent& event)
{
    if (vert_sb_->visible())
    {
        if (vert_sb_->bounds().Contains(event.location()))
        {
            return vert_sb_->OnMousePressed(event);
        }
    }
    if (horiz_sb_->visible())
    {
        if (horiz_sb_->bounds().Contains(event.location()))
        {
            return horiz_sb_->OnMousePressed(event);
        }
    }
    return false;
}

bool ScrollViewWithFloatingScrollbar::OnMouseDragged(const ui::MouseEvent& event)
{
    gfx::Point point = event.location();
    if (vert_sb_->visible() && (vert_sb_->thumb_state() == ThumbState_PRESSED))
    {
        return vert_sb_->OnMouseDragged(event);
    }
    else if (horiz_sb_->visible() && (horiz_sb_->thumb_state() == ThumbState_PRESSED))
    {
        return horiz_sb_->OnMouseDragged(event);
    }
    return false;
}

void ScrollViewWithFloatingScrollbar::OnMouseReleased(const ui::MouseEvent& event)
{
    OnMouseMoved(event);
}

bool ScrollViewWithFloatingScrollbar::OnMouseWheel(const ui::MouseWheelEvent& event)
{
    bool handled = false;
    // 优先处理垂直滚动条
    if (vert_sb_->visible())
    {
        if (vert_sb_->thumb_state() != ThumbState_PRESSED)
        {
            // 下滚负数，上滚正数
            ScrollToPosition(true, vert_sb_->GetPosition() - event.y_offset(), Wheel);
        }
        handled = true;
    }
    else if (horiz_sb_->visible())
    {
        if (horiz_sb_->thumb_state() != ThumbState_PRESSED)
        {
            // 下滚负数，上滚正数
            ScrollToPosition(false, horiz_sb_->GetPosition() - event.y_offset(), Wheel);
        }
        handled = true;
    }
    return handled;
}

bool ScrollViewWithFloatingScrollbar::OnKeyPressed(const ui::KeyEvent& event)
{
    bool ret = false;
    switch (event.key_code())
    {
    case ui::VKEY_UP:
    {
        if (vert_sb_->visible())
        {
            int inc = kFloatingScrollbarPressedIncrement * vert_sb_->GetMaxPosition();
            ScrollToPosition(true, vert_sb_->GetPosition() - inc, Key);
            ret = true;
        }
    }
    break;
    case ui::VKEY_DOWN:
    {
        if (vert_sb_->visible())
        {
            int inc = kFloatingScrollbarPressedIncrement * vert_sb_->GetMaxPosition();
            ScrollToPosition(true, vert_sb_->GetPosition() + inc, Key);
            ret = true;
        }
    }
    break;
    case ui::VKEY_LEFT:
    {
        if (horiz_sb_->visible())
        {
            int inc = kFloatingScrollbarPressedIncrement * horiz_sb_->GetMaxPosition();
            ScrollToPosition(false, horiz_sb_->GetPosition() - inc, Key);
            ret = true;
        }
    }
    break;
    case ui::VKEY_RIGHT:
    {
        if (horiz_sb_->visible())
        {
            int inc = kFloatingScrollbarPressedIncrement * horiz_sb_->GetMaxPosition();
            ScrollToPosition(false, horiz_sb_->GetPosition() + inc, Key);
            ret = true;
        }
    }
    break;
    case ui::VKEY_HOME:
    {
        if (vert_sb_->visible())
        {
            ScrollToPosition(true, vert_sb_->GetMinPosition(), Key);
            ret = true;
        }
        else if (horiz_sb_->visible())
        {
            ScrollToPosition(false, horiz_sb_->GetMinPosition(), Key);
            ret = true;
        }
    }
    break;
    case ui::VKEY_END:
    {
        if (vert_sb_->visible())
        {
            ScrollToPosition(true, vert_sb_->GetMaxPosition(), Key);
            ret = true;
        }
        else if (horiz_sb_->visible())
        {
            ScrollToPosition(false, horiz_sb_->GetMaxPosition(), Key);
            ret = true;
        }
    }
    break;
    case ui::VKEY_PRIOR:
    {
        if (vert_sb_->visible())
        {
            //ScrollByContentsOffset(true, contents_viewport_->height(), Key);
            ret = true;
        }
        else if (horiz_sb_->visible())
        {
            //ScrollByContentsOffset(false, contents_viewport_->width(), Key);
            ret = true;
        }
    }
    break;
    case ui::VKEY_NEXT:
    {
        if (vert_sb_->visible())
        {
            //ScrollByContentsOffset(true, -contents_viewport_->height(), Key);
            ret = true;
        }
        else if (horiz_sb_->visible())
        {
            //ScrollByContentsOffset(false, -contents_viewport_->width(), Key);
            ret = true;
        }
    }
    break;
    default:
        break;
    }
    return ret;
}

// ScrollBarController
void ScrollViewWithFloatingScrollbar::ScrollToPosition(bool vert, int position, ScrollReason reason)
{
    if (!contents_)
        return;

    scroll_reason_ = reason;

    if (!vert && horiz_sb_->visible())
    {
        position = AdjustPosition(contents_->x(), position, contents_->width(),
            contents_viewport_->width());
        if (-contents_->x() == position)
            return;
        contents_->SetX(-position);
        if (header_)
        {
            header_->SetX(-position);
            //header_->SchedulePaintInRect(header_->GetVisibleBounds());
        }
    }
    else if (vert && vert_sb_->visible())
    {
        position = AdjustPosition(contents_->y(), position, contents_->height(),
            contents_viewport_->height());
        if (-contents_->y() == position)
            return;
        contents_->SetY(-position);
    }
    //contents_->SchedulePaintInRect(contents_->GetVisibleBounds());

    UpdateScrollBarPositions();

    if (listener_) {
        listener_->OnScrollPositionChanged(vert, position, reason);
    }
}

int ScrollViewWithFloatingScrollbar::GetScrollIncrement(bool vert, bool is_page, bool is_positive)
{
    bool is_horizontal = !vert;
    /*int amount = 0;
    if (contents_) {
        if (is_page) {
            amount = contents_->GetPageScrollIncrement(
                this, is_horizontal, is_positive);
        }
        else {
            amount = contents_->GetLineScrollIncrement(
                this, is_horizontal, is_positive);
        }
        if (amount > 0)
            return amount;
    }*/
    // No view, or the view didn't return a valid amount.
    if (is_page)
    {
        return is_horizontal ? contents_viewport_->width() :
            contents_viewport_->height();
    }
    return is_horizontal ? contents_viewport_->width() / 5 :
        contents_viewport_->height() / 5;
}

void ScrollViewWithFloatingScrollbar::SetHorizontalScrollBar(FloatingScrollBar* horiz_sb)
{
    FloatingScrollBar* old_bar = horiz_sb_;
    if (old_bar)
    {
        horiz_sb->SetVisible(old_bar->visible());
        horiz_sb->SetBoundsRect(old_bar->bounds());
        delete old_bar;
    }
    horiz_sb_ = horiz_sb;
}

void ScrollViewWithFloatingScrollbar::SetVerticalScrollBar(FloatingScrollBar* vert_sb)
{
    FloatingScrollBar* old_bar = vert_sb_;
    if (old_bar)
    {
        vert_sb->SetVisible(old_bar->visible());
        vert_sb->SetBoundsRect(old_bar->bounds());
        delete old_bar;
    }
    vert_sb_ = vert_sb;
}

// ScrollView
void ScrollViewWithFloatingScrollbar::SetContents(views::View* content, bool delete_old/* = true*/)
{
    SetHeaderOrContents(contents_viewport_, content, &contents_, delete_old);
}

void ScrollViewWithFloatingScrollbar::SetHeader(views::View* header, bool delete_old/* = true*/)
{
    SetHeaderOrContents(header_viewport_, header, &header_, delete_old);
}

void ScrollViewWithFloatingScrollbar::SetHeaderOrContents(Viewport* parent, views::View* new_view, views::View** member, bool delete_old)
{
    if (*member == new_view)
    {
        return;
    }

    parent->RemoveAllChildViews(delete_old);
    *member = new_view;
    if (*member)
    {
        parent->AddChildView(*member);
    }
    Layout();
}

void ScrollViewWithFloatingScrollbar::ComputeScrollBarsVisibility(
    const gfx::Size& vp_size,
    const gfx::Size& content_size,
    bool* horiz_is_shown,
    bool* vert_is_shown)
{
    // Try to fit both ways first, then try vertical bar only, then horizontal
    // bar only, then defaults to both shown.
    if (content_size.width() <= vp_size.width() &&
        content_size.height() <= vp_size.height()) {
        *horiz_is_shown = false;
        *vert_is_shown = false;
    }
    else if (content_size.width() <= vp_size.width()) {
        *horiz_is_shown = false;
        *vert_is_shown = true;
    }
    else if (content_size.height() <= vp_size.height()) {
        *horiz_is_shown = true;
        *vert_is_shown = false;
    }
    else {
        *horiz_is_shown = true;
        *vert_is_shown = true;
    }

    if (hide_horizontal_scrollbar_)
        *horiz_is_shown = false;
}

void ScrollViewWithFloatingScrollbar::ScrollContentsRegionToBeVisible(const gfx::Rect& rect)
{
    if (!contents_ || (!horiz_sb_->visible() && !vert_sb_->visible()))
        return;

    // Figure out the maximums for this scroll view.
    const int contents_max_x =
        std::max(contents_viewport_->width(), contents_->width());
    const int contents_max_y =
        std::max(contents_viewport_->height(), contents_->height());

    // Make sure x and y are within the bounds of [0,contents_max_*].
    int x = std::max(0, std::min(contents_max_x, rect.x()));
    int y = std::max(0, std::min(contents_max_y, rect.y()));

    // Figure out how far and down the rectangle will go taking width
    // and height into account.  This will be "clipped" by the viewport.
    const int max_x = std::min(contents_max_x,
        x + std::min(rect.width(), contents_viewport_->width()));
    const int max_y = std::min(contents_max_y,
        y + std::min(rect.height(), contents_viewport_->height()));

    // See if the rect is already visible. Note the width is (max_x - x)
    // and the height is (max_y - y) to take into account the clipping of
    // either viewport or the content size.
    const gfx::Rect vis_rect = GetVisibleRect();
    if (vis_rect.Contains(gfx::Rect(x, y, max_x - x, max_y - y)))
        return;

    // Shift contents_'s X and Y so that the region is visible. If we
    // need to shift up or left from where we currently are then we need
    // to get it so that the content appears in the upper/left
    // corner. This is done by setting the offset to -X or -Y.  For down
    // or right shifts we need to make sure it appears in the
    // lower/right corner. This is calculated by taking max_x or max_y
    // and scaling it back by the size of the viewport.
    const int new_x =
        (vis_rect.x() > x) ? x : std::max(0, max_x - contents_viewport_->width());
    const int new_y =
        (vis_rect.y() > y) ? y : std::max(0, max_y - contents_viewport_->height());

    scroll_reason_ = Api;

    contents_->SetX(-new_x);
    if (header_)
        header_->SetX(-new_x);
    contents_->SetY(-new_y);
    UpdateScrollBarPositions();
}

void ScrollViewWithFloatingScrollbar::UpdateScrollBarPositions()
{
    if (!contents_)
        return;

    if (horiz_sb_->visible()) {
        int vw = contents_viewport_->width();
        int cw = contents_->width();
        int origin = contents_->x();
        horiz_sb_->Update(vw, cw, -origin);
    }
    if (vert_sb_->visible()) {
        int vh = contents_viewport_->height();
        int ch = contents_->height();
        int origin = contents_->y();
        vert_sb_->Update(vh, ch, -origin);
    }

    scroll_reason_ = NoMove;
}

void ScrollViewWithFloatingScrollbar::SetControlVisibility(bool vert, bool should_show)
{
    FloatingScrollBar *control = vert ? vert_sb_ : horiz_sb_;
    if (!control)
        return;

    if (should_show)
    {
        if (!control->visible())
        {
            control->SetVisible(true);
        }
    }
    else
    {
        control->SetVisible(false);
    }
}

gfx::Rect ScrollViewWithFloatingScrollbar::GetVisibleRect() const
{
    if (!contents_)
    {
        return gfx::Rect();
    }

    return gfx::Rect(-contents_->x(), -contents_->y(),
        contents_viewport_->width(), contents_viewport_->height());
}

int ScrollViewWithFloatingScrollbar::GetScrollBarWidth() const
{
    if (vert_sb_) {
        return vert_sb_->GetScrollBarWidth();
    }
    return kFloatingScrollbarWidth;
}

int ScrollViewWithFloatingScrollbar::GetScrollBarHeight() const
{
    if (horiz_sb_) {
        return horiz_sb_->GetScrollBarHeight();
    }
    return kFloatingScrollbarHeight;
}

ThumbState ScrollViewWithFloatingScrollbar::current_scrollbar_state(bool vert) const
{
    if (vert)
    {
        if (vert_sb_->visible())
        {
            return vert_sb_->thumb_state();
        }
    }
    else
    {
        if (horiz_sb_->visible())
        {
            return horiz_sb_->thumb_state();
        }
    }
    return ThumbState_NORMAL;
}


// BililiveViewWithFloatingScrollbar
BililiveViewWithFloatingScrollbar::BililiveViewWithFloatingScrollbar(bool hide_horiz /*= true*/)
{
    scroll_view_ = new ScrollViewWithFloatingScrollbar(this,true);
    scroll_view_->set_hide_horizontal_scrollbar(hide_horiz);

    set_notify_enter_exit_on_child(true);
}

void BililiveViewWithFloatingScrollbar::Layout()
{
    if (scroll_view_)
    {
        gfx::Rect scroll_bounds = scroll_view_->GetContentsBounds();

        // 滚动条出现就为滚动条区域腾出位置，否则内容区会出现折断
        //if (sbt_ & SBT_VERTICAL)
        {
            int pref_height = GetHeightForWidth(scroll_bounds.width());
            SetSize(gfx::Size(scroll_bounds.width(), pref_height));
        }
    }

    __super::Layout();
}

void BililiveViewWithFloatingScrollbar::ChildPreferredSizeChanged(views::View* child)
{
    PreferredSizeChanged();
}

void BililiveViewWithFloatingScrollbar::PreferredSizeChanged()
{
    __super::PreferredSizeChanged();
}
