#ifndef BILILIVE_BILILIVE_UI_VIEWS_TABAREA_LIVEHIME_MODULE_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_TABAREA_LIVEHIME_MODULE_VIEW_H

#include "ui/views/view.h"


namespace {
    class ModuleAreaView;
}

class LivehimeModuleView
    : public views::View
{
public:
    LivehimeModuleView();
    virtual ~LivehimeModuleView();

    static int GetMinimumHeight();

    gfx::Size GetPreferredSize() override;

protected:
    // views::View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) override;

private:
    void InitViews();

private:
    base::WeakPtrFactory<LivehimeModuleView> weakptr_factory_;

    ModuleAreaView* module_area_view_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(LivehimeModuleView);
};
#endif
