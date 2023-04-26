#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_AVERAGE_LAYOUT_H_
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_AVERAGE_LAYOUT_H_

#include "ui/views/layout/layout_manager.h"


namespace bililive {

    class AverageLayout : public views::LayoutManager {
    public:
        enum Orientation {
            HORIZONTAL,
            VERTICAL,
        };

        explicit AverageLayout(
            Orientation orientation = HORIZONTAL,
            bool average_child = true,
            int between_child_spacing = 0);

        // views::LayoutManager
        gfx::Size GetPreferredSize(views::View* host) override;
        void Layout(views::View* host) override;

    private:
        bool needOneSpace(int idx, int size, int spacing) const;

        Orientation orientation_;
        bool average_child_;
        int between_child_spacing_;
    };

}

#endif  // BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_AVERAGE_LAYOUT_H_