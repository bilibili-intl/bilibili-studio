#include "bililive/bililive/ui/views/livehime/layout/user_info_layout.h"

#include "ui/views/view.h"


UserInfoLayout::UserInfoLayout() {

}

gfx::Size UserInfoLayout::GetPreferredSize(views::View* host) {
    gfx::Size result;
    for (int i = 0; i < host->child_count(); ++i) {
        auto child = host->child_at(i);
        if (!child->visible()) {
            continue;
        }
        auto param = params_[i];
        auto child_size = child->GetPreferredSize();
        if (child_size.height() > result.height()) {
            result.set_height(child_size.height());
        }
        result.Enlarge(child_size.width() + param.left_margin + param.right_margin, 0);
    }
    result.Enlarge(host->GetInsets().width(), host->GetInsets().height());
    return result;
}

int UserInfoLayout::GetPreferredHeightForWidth(views::View* host, int width) {
    return GetPreferredSize(host).height();
}

void UserInfoLayout::Layout(views::View* host) {
    gfx::Rect child_area(host->GetContentsBounds());

    auto pre_size = GetPreferredSize(host);
    int shrink_width = std::max(pre_size.width() - child_area.width(), 0);

    int x = child_area.x();
    int y = child_area.y();
    for (int i = 0; i < host->child_count(); ++i) {
        auto child = host->child_at(i);
        if (child->visible()) {
            auto child_size = child->GetPreferredSize();
            auto param = params_[i];
            if (param.need_shrink && shrink_width > 0) {
                if (child_size.width() >= shrink_width) {
                    child_size.Enlarge(-shrink_width, 0);
                    shrink_width = 0;
                } else {
                    shrink_width -= child_size.width();
                    child_size.set_width(0);
                }
            }
            gfx::Rect bounds(
                x + param.left_margin,
                y + (pre_size.height() - child_size.height()) / 2,
                child_size.width(), child_size.height());
            bounds.Intersect(child_area);
            child->SetBoundsRect(bounds);

            x += child_size.width() + param.left_margin + param.right_margin;
        }
    }
}

void UserInfoLayout::SetLayoutParams(
    int pos, int left_margin, int right_margin, bool need_shrink/* = false*/)
{
    LayoutParams params;
    params.left_margin = left_margin;
    params.right_margin = right_margin;
    params.need_shrink = need_shrink;

    params_[pos] = params;
}
