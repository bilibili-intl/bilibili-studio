#include "bililive/bililive/ui/views/controls/average_layout.h"

#include "ui/views/view.h"


namespace bililive {

    AverageLayout::AverageLayout(
        Orientation orientation,
        bool average_child,
        int between_child_spacing)
        : orientation_(orientation),
          average_child_(average_child),
          between_child_spacing_(between_child_spacing)
    {}

    gfx::Size AverageLayout::GetPreferredSize(views::View* host) {
        int child_cot = host->child_count();
        if (child_cot == 0) {
            return {};
        }

        if (orientation_ == VERTICAL) {
            int child_width = 0;
            for (int i = 0; i < child_cot; ++i) {
                if (host->child_at(i)->visible()) {
                    child_width = std::max(host->child_at(i)->GetPreferredSize().width(), child_width);
                }
            }

            int total_height = 0;
            for (int i = 0; i < child_cot; ++i) {
                if (host->child_at(i)->visible()) {
                    total_height += host->child_at(i)->GetHeightForWidth(child_width) + between_child_spacing_;
                }
            }

            auto insets = host->GetInsets();
            return gfx::Size(child_width + insets.width(), total_height + insets.height());
        } else {
            int total_width = 0;
            int child_height = 0;
            for (int i = 0; i < child_cot; ++i) {
                if (host->child_at(i)->visible()) {
                    auto child_size = host->child_at(i)->GetPreferredSize();
                    child_height = std::max(child_size.height(), child_height);
                    total_width += child_size.width() + between_child_spacing_;
                }
            }

            auto insets = host->GetInsets();
            return gfx::Size(total_width + insets.width(), child_height + insets.height());
        }
    }

    void AverageLayout::Layout(views::View* host) {
        int child_cot = host->child_count();
        int child_vcnt = 0;
        for (int i = 0; i < child_cot; ++i) {
            if (host->child_at(i)->visible()) {
                ++child_vcnt;
            }
        }
        if (child_vcnt == 0) {
            return;
        }

        auto bounds = host->GetContentsBounds();

        int x = bounds.x();
        int y = bounds.y();

        if (average_child_) {
            int debug_total_spacing = 0;
            if (orientation_ == VERTICAL) {
                int cur_y = y;
                int spacing = bounds.height() % child_vcnt;
                int child_height = bounds.height() / child_vcnt;
                for (int i = 0; i < child_cot; ++i) {
                    if (host->child_at(i)->visible()) {
                        int cur_child_height = child_height + (needOneSpace(i, child_vcnt, spacing) ? 1 : 0);
                        debug_total_spacing += needOneSpace(i, child_vcnt, spacing) ? 1 : 0;
                        host->child_at(i)->SetBounds(x, cur_y, bounds.width(), cur_child_height);
                        cur_y += cur_child_height + between_child_spacing_;
                    }
                }
                DCHECK(debug_total_spacing == spacing);
            } else {
                int cur_x = x;
                int spacing = bounds.width() % child_vcnt;
                int child_width = bounds.width() / child_vcnt;
                for (int i = 0; i < child_cot; ++i) {
                    if (host->child_at(i)->visible()) {
                        int cur_child_width = child_width + (needOneSpace(i, child_vcnt, spacing) ? 1 : 0);
                        debug_total_spacing += needOneSpace(i, child_vcnt, spacing) ? 1 : 0;
                        host->child_at(i)->SetBounds(cur_x, y, cur_child_width, bounds.height());
                        cur_x += cur_child_width + between_child_spacing_;
                    }
                }
                DCHECK(debug_total_spacing == spacing);
            }
        } else {
            if (orientation_ == VERTICAL) {
                int cur_y = y;
                for (int i = 0; i < child_cot; ++i) {
                    if (host->child_at(i)->visible()) {
                        int child_height = host->child_at(i)->GetHeightForWidth(bounds.width());
                        host->child_at(i)->SetBounds(x, cur_y, bounds.width(), child_height);
                        cur_y += child_height + between_child_spacing_;
                    }
                }
            } else {
                int cur_x = x;
                for (int i = 0; i < child_cot; ++i) {
                    if (host->child_at(i)->visible()) {
                        int child_width = host->child_at(i)->GetPreferredSize().width();
                        host->child_at(i)->SetBounds(cur_x, y, child_width, bounds.height());
                        cur_x += child_width + between_child_spacing_;
                    }
                }
            }
        }
    }

    bool AverageLayout::needOneSpace(int idx, int size, int spacing) const {
        if (size <= 1 || spacing <= 0) return false;

        int end = size - 1;
        int center = size / 2;
        int radius = spacing / 2;
        if (size % 2 && spacing % 2) {
            // [center-radius, center+radius]
            if (idx >= center - radius && idx <= center + radius) {
                return true;
            }
        } else if (!(size % 2) && !(spacing % 2)) {
            // [center-radius, center+radius)
            if (idx >= center - radius && idx < center + radius) {
                return true;
            }
        } else if (size % 2 && !(spacing % 2)) {
            // [0, radius)U(end-radius, end]
            if ((idx >= 0 && idx < radius) || (idx > end - radius && idx <= end)) {
                return true;
            }
        } else if (!(size % 2) && spacing % 2) {
            // [0, radius]U(end-radius, end]
            if ((idx >= 0 && idx <= radius) || (idx > end - radius && idx <= end)) {
                return true;
            }
        }
        return false;
    }

}