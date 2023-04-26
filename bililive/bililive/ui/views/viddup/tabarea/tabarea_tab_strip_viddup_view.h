#ifndef BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TABAREA_TAB_STRIP_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TABAREA_TAB_STRIP_VIEW_H

#include "bililive/bililive/ui/views/livehime/controls/livehime_tabbed.h"

#include "ui/views/controls/menu/menu_delegate.h"
#include "ui/views/controls/button/image_button.h"


// Scene TAB TAB
class TabAreaTabStripViddupView
    : public LivehimeTopStripPosStripView
{
public:
    TabAreaTabStripViddupView(int index, const base::string16 &name, bool vmodel = false);
    virtual ~TabAreaTabStripViddupView();

    int scene_index() const { return scene_index_; }
    base::string16 scene_name() const { return scene_name_; }

protected:
    // View
    void OnPaintBackground(gfx::Canvas* canvas) override;
    gfx::Size GetPreferredSize() override;
    int GetHeightForWidth(int w) override;

    // LivehimeTopStripPosStripView
    void OnStateChanged() override;

private:
    int scene_index_ = -1;
    base::string16 scene_name_;

    base::WeakPtrFactory<TabAreaTabStripViddupView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(TabAreaTabStripViddupView);
};

#endif  //BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TABAREA_TAB_STRIP_VIEW_H