#include "bililive_util_views.h"

#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "ui/views/layout/fill_layout.h"

#include "third_party/skia/include/core/SkPaint.h"

// BililiveSingleChildShowContainerView
void BililiveSingleChildShowContainerView::Layout()
{
    for (int i = 0; i < child_count(); i++)
    {
        child_at(i)->SetBounds(0, 0, width(), height());
    }
}

void BililiveSingleChildShowContainerView::SetPreferredSize(const gfx::Size& pref_size)
{
    pref_size_ = pref_size;
}

gfx::Size BililiveSingleChildShowContainerView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();

    for (int i = 0; i < child_count(); ++i)
    {
        size.SetToMax(child_at(i)->GetPreferredSize());
    }

    if (!pref_size_.IsEmpty())
    {
        size.SetToMax(pref_size_);
        size.set_width(std::min(pref_size_.width(), size.width()));
    }

    return size;
}

int BililiveSingleChildShowContainerView::GetHeightForWidth(int w)
{
    int h = 0;
    for (int i = 0; i < child_count(); ++i)
    {
        h = std::max(h, child_at(i)->GetHeightForWidth(w));
    }
    return h;
}

void BililiveSingleChildShowContainerView::ChildPreferredSizeChanged(views::View* child)
{
    PreferredSizeChanged();
}

BililiveMutilWidthContainerView::BililiveMutilWidthContainerView(int space, int max_hight)
    : space_(space),
    max_hight_(max_hight)

{
}

void BililiveMutilWidthContainerView::Layout()
{
    int px = 0;
    for (int i = 0; i < child_count(); i++)
    {
        if (child_at(i)->visible()) {
            auto child_size = child_at(i)->GetPreferredSize();
            if (max_hight_ != 0) {
                child_at(i)->SetBounds(px, (max_hight_ - child_size.height()) / 2, child_size.width(), child_size.height());
            }
            else {
                child_at(i)->SetBounds(px, 0, child_size.width(), child_size.height());
            }
            auto size = child_at(i)->GetPreferredSize();
            px = size.width();
            if (i < (child_count() - 1)) {
                px = px + space_;
            }
        }
    }
}

gfx::Size BililiveMutilWidthContainerView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    int width = 0;
    for (int i = 0; i < child_count(); ++i)
    {
        if (child_at(i)->visible())
        {
            auto child_size = child_at(i)->GetPreferredSize();
            size.set_height(std::max(size.height(), child_size.height()));
            width = width + child_size.width() + space_;
        }
    }
    size.set_width(width);
    return size;
}

void BililiveMutilWidthContainerView::ChildPreferredSizeChanged(views::View* child)
{
    PreferredSizeChanged();
}

gfx::Size BililiveSingleChildHightShowContainerView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();

    for (int i = 0; i < child_count(); ++i)
    {
        if (child_at(i)->visible())
        {
            size.SetToMax(child_at(i)->GetPreferredSize());
        }
    }

    if (!pref_size_.IsEmpty())
    {
        size.SetToMax(pref_size_);
        size.set_height(std::min(pref_size_.height(), size.height()));
    }

    return size;
}

void BililiveSingleChildHightShowContainerView::Layout()
{
    for (int i = 0; i < child_count(); i++)
    {
        child_at(i)->SetBounds(0, 0, width(), height());
    }
}

void BililiveSingleChildHightShowContainerView::ChildPreferredSizeChanged(views::View* child)
{
    PreferredSizeChanged();
}

void BililiveSingleChildHightShowContainerView::SetPreferredSize(const gfx::Size& pref_size)
{
    pref_size_ = pref_size;
}


// BililiveRecursivePreferredSizeChangedContainerView
void BililiveRecursivePreferredSizeChangedContainerView::ChildPreferredSizeChanged(views::View* child)
{
    PreferredSizeChanged();
}


// BililiveHideAwareView
BililiveHideAwareView::BililiveHideAwareView(bool fill_layout, bool auto_hide)
    : auto_hide_(auto_hide)
{
    if (fill_layout)
    {
        SetLayoutManager(new views::FillLayout());
    }
}

void BililiveHideAwareView::SetPlaceholderSize(const gfx::Size& placeholder_size)
{
    placeholder_size_ = placeholder_size;
}

gfx::Size BililiveHideAwareView::GetPreferredSize()
{
    gfx::Size size = visible() ? View::GetPreferredSize() : placeholder_size_;
    return size;
}

int BililiveHideAwareView::GetHeightForWidth(int w)
{
    return visible() ? View::GetHeightForWidth(w) : placeholder_size_.height();
}

gfx::Insets BililiveHideAwareView::GetInsets() const
{
    gfx::Insets pref_insets = __super::GetInsets();
    pref_insets.Set(
        pref_insets.top() + insets_.top(),
        pref_insets.left() + insets_.left(),
        pref_insets.bottom() + insets_.bottom(),
        pref_insets.right() + insets_.right());
    return pref_insets;
}

void BililiveHideAwareView::SetInsets(const gfx::Insets& insets)
{
    insets_ = insets;
}

void BililiveHideAwareView::ChildVisibilityChanged(views::View* child)
{
    if (auto_hide_)
    {
        bool more_than_one_visible = false;
        for (int i = 0; i < child_count(); ++i)
        {
            if (child_at(i)->visible())
            {
                more_than_one_visible = true;
                break;
            }
        }
        SetVisible(more_than_one_visible);
        PreferredSizeChanged();
    }
}

void BililiveHideAwareView::VisibilityChanged(View* starting_from, bool is_visible)
{
    PreferredSizeChanged();
}
