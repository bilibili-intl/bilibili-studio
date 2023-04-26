#ifndef BILILIVE_BILILIVE_UI_EXAMPLES_EXAMPLES_FRAME_VIEW_H_
#define BILILIVE_BILILIVE_UI_EXAMPLES_EXAMPLES_FRAME_VIEW_H_

#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"


class NavigationBar;

namespace views {
    class Widget;
    class NonClientFrameView;
}

namespace examples {

    class ExamplesFrameView : public BililiveWidgetDelegate {
    public:
        ExamplesFrameView();
        ~ExamplesFrameView();

        static void ShowWindow();

        // views::WidgetDelegate
        views::View* GetContentsView() override { return this; }
        bool CanResize() const override { return true; }
        bool CanMaximize() const override { return true; }
        views::NonClientFrameView* CreateNonClientFrameView(views::Widget* widget) override;

        // views::View
        void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;
        gfx::Size GetPreferredSize() override;

    private:
        void InitViews();
        void UninitViews();

        static ExamplesFrameView* instance_;

        NavigationBar* tab_pane_;
        std::vector<int> tab_ids_;

        DISALLOW_COPY_AND_ASSIGN(ExamplesFrameView);
    };

}

#endif  // BILILIVE_BILILIVE_UI_EXAMPLES_EXAMPLES_FRAME_VIEW_H_