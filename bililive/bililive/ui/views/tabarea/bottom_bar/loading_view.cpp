#include "bililive/bililive/ui/views/tabarea/bottom_bar/loading_view.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"

LoadingView::LoadingView() {
}

LoadingView::~LoadingView() {
}

void LoadingView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) {
    if (details.child == this) {
        if (details.is_add) {
            InitView();
        }
    }
}

void LoadingView::InitView() {
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);
    views::ColumnSet *columnset = layout->AddColumnSet(0);
    columnset->AddPaddingColumn(1.0f, 0);
    columnset->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    columnset->AddPaddingColumn(1.0f, 0);

    loading_label_ = new LivehimeContentLabel(rb.GetLocalizedString(IDS_DANMUKU_BANNED_LOADING));

    layout->AddPaddingRow(1.0f, 0);
    layout->StartRow(0, 0);
    layout->AddView(loading_label_);
    layout->AddPaddingRow(0, kPaddingRowHeightEnds);
    layout->AddPaddingRow(1.0f, 0);
}

void LoadingView::set_label_text(const string16& text) {
    loading_label_->SetText(text);
}