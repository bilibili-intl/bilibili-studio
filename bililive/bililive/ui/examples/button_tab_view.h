#ifndef BILILIVE_BILILIVE_UI_EXAMPLES_BUTTON_TAB_VIEW_H_
#define BILILIVE_BILILIVE_UI_EXAMPLES_BUTTON_TAB_VIEW_H_

#include "ui/views/view.h"

#include "ui/views/controls/button/button.h"


class LivehimeCheckbox;

namespace examples {

    class ButtonTabView :
        public views::View,
        public views::ButtonListener {
    public:
        ButtonTabView();

        // views::View
        void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;

        // views::ButtonListener
        void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    private:
        void InitViews();

        bool using_dwrite_;
        LivehimeCheckbox* checkbox_;

        DISALLOW_COPY_AND_ASSIGN(ButtonTabView);
    };

}

#endif  // BILILIVE_BILILIVE_UI_EXAMPLES_BUTTON_TAB_VIEW_H_