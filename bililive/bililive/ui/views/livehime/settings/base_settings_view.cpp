#include "base_settings_view.h"


int BaseSettingsView::radio_button_groupid_ = 0;

// View
void BaseSettingsView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) {
    if (details.child == this) {
        if (details.is_add)
        {
            InitViews();
            InitData();
        }
        else
        {
            UninitViews();
        }
    }
};