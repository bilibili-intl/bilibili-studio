#include "status_tray.h"

#include <algorithm>

#include "status_icon.h"

StatusTray::~StatusTray()
{
}

StatusIcon *StatusTray::CreateStatusIcon(StatusIconType type,
        const gfx::ImageSkia &image,
        const string16 &tool_tip)
{
    StatusIcon *icon = CreatePlatformStatusIcon(type, image, tool_tip);
    if (icon)
    {
        status_icons_.push_back(icon);
    }
    return icon;
}

void StatusTray::RemoveStatusIcon(StatusIcon *icon)
{
    StatusIcons::iterator i(
        std::find(status_icons_.begin(), status_icons_.end(), icon));

    if (i == status_icons_.end())
    {
        NOTREACHED();
        return;
    }

    status_icons_.erase(i);
}

StatusTray::StatusTray()
{
}