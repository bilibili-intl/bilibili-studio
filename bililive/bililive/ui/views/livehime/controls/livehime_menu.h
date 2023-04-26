#ifndef LIVEHIME_MENU_H
#define LIVEHIME_MENU_H

#include "bililive/bililive/ui/views/controls/bililive_menuitemview.h"

class LivehimeMenuItemView : public BililiveMenuItemView
{
public:
    explicit LivehimeMenuItemView(views::MenuDelegate* delegate);
    virtual ~LivehimeMenuItemView();

protected:
    // View
    virtual gfx::Size GetPreferredSize() OVERRIDE;
    virtual int GetHeightForWidth(int w) OVERRIDE;

    // MenuItemView
    virtual const views::MenuConfig& GetMenuConfig() const override;

protected:
    // Creates a MenuItemView. This is used by the various AddXXX methods.
    explicit LivehimeMenuItemView(views::MenuItemView* parent, int command, views::MenuItemView::Type type);

    virtual views::MenuItemView* CreateCurrentDevrivedInstance(
        BililiveMenuItemView* parent, int command, views::MenuItemView::Type type) override;

private:
    DISALLOW_COPY_AND_ASSIGN(LivehimeMenuItemView);
};

#endif
