#ifndef BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_LOADING_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_LOADING_VIEW_H

#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"

class LoadingView
    : public views::View {
public:
    LoadingView();

    ~LoadingView();

    void set_label_text(const string16& text);

private:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;

    void InitView();

private:
    LivehimeContentLabel* loading_label_ = nullptr;
};

#endif //BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_BAR_LOADING_VIEW_H