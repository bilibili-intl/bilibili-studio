#ifndef BILILIVE_BILILIVE_STATUS_ICONS_STATUS_ICON_H_
#define BILILIVE_BILILIVE_STATUS_ICONS_STATUS_ICON_H_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "base/strings/string16.h"

#include "ui/views/controls/menu/menu_item_view.h"

namespace gfx
{
    class ImageSkia;
}

namespace ui
{
    class MenuModel;
}

class StatusIconObserver;

class StatusIcon
{
public:
    StatusIcon();
    virtual ~StatusIcon();

    virtual void SetImage(const gfx::ImageSkia &image) = 0;

    virtual void SetPressedImage(const gfx::ImageSkia &image) = 0;

    virtual void SetToolTip(const string16 &tool_tip) = 0;

    virtual void DisplayBalloon(const gfx::ImageSkia &icon,
                                const string16 &title,
                                const string16 &contents) = 0;

    void SetContextMenu(ui::MenuModel *menu);
    void SetContextMenu(views::MenuItemView *menu_view);

    void AddObserver(StatusIconObserver *observer);
    void RemoveObserver(StatusIconObserver *observer);

    bool HasObservers() const;

    void DispatchClickEvent();
#if defined(OS_WIN)
    void DispatchBalloonClickEvent();
#endif

protected:
    virtual void UpdatePlatformContextMenu(ui::MenuModel *model) = 0;
    virtual void UpdatePlatformContextMenu(views::MenuItemView *menu) = 0;

private:
    ObserverList<StatusIconObserver> observers_;

    scoped_ptr<ui::MenuModel> context_menu_contents_;

    DISALLOW_COPY_AND_ASSIGN(StatusIcon);
};

#endif