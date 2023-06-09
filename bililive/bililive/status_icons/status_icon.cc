#include "status_icon.h"

#include "status_icon_observer.h"
#include "ui/base/models/menu_model.h"

StatusIcon::StatusIcon()
{
}

StatusIcon::~StatusIcon()
{
}

void StatusIcon::AddObserver(StatusIconObserver *observer)
{
    observers_.AddObserver(observer);
}

void StatusIcon::RemoveObserver(StatusIconObserver *observer)
{
    observers_.RemoveObserver(observer);
}

bool StatusIcon::HasObservers() const
{
    return observers_.size() > 0;
}

void StatusIcon::DispatchClickEvent()
{
    FOR_EACH_OBSERVER(StatusIconObserver, observers_, OnStatusIconClicked());
}

#if defined(OS_WIN)
void StatusIcon::DispatchBalloonClickEvent()
{
    FOR_EACH_OBSERVER(StatusIconObserver, observers_, OnBalloonClicked());
}
#endif

void StatusIcon::SetContextMenu(ui::MenuModel *menu)
{
    // The UI may been showing a menu for the current model, don't destroy it
    // until we've notified the UI of the change.
    scoped_ptr<ui::MenuModel> old_menu = context_menu_contents_.Pass();
    context_menu_contents_.reset(menu);
    UpdatePlatformContextMenu(menu);
}

void StatusIcon::SetContextMenu(views::MenuItemView *menu_view)
{
    UpdatePlatformContextMenu(menu_view);
}
