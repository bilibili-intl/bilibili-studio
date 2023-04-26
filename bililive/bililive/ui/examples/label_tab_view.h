#ifndef BILILIVE_BILILIVE_UI_EXAMPLES_LABEL_TAB_VIEW_H_
#define BILILIVE_BILILIVE_UI_EXAMPLES_LABEL_TAB_VIEW_H_

#include "ui/views/view.h"


namespace examples {

    class LabelTabView : public views::View {
    public:
        LabelTabView() = default;

        // views::View
        void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails& details) override;

    private:
        void InitViews();

        DISALLOW_COPY_AND_ASSIGN(LabelTabView);
    };

}

#endif  // BILILIVE_BILILIVE_UI_EXAMPLES_LABEL_TAB_VIEW_H_