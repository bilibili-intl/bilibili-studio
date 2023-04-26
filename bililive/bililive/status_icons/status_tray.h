#ifndef BILILIVE_BILILIVE_STATUS_ICONS_STATUS_TRAY_H_
#define BILILIVE_BILILIVE_STATUS_ICONS_STATUS_TRAY_H_

#include "base/basictypes.h"
#include "base/gtest_prod_util.h"
#include "base/memory/scoped_vector.h"
#include "base/strings/string16.h"

namespace gfx
{
    class ImageSkia;
}

class StatusIcon;

class StatusTray
{
public:
    enum StatusIconType
    {
        NOTIFICATION_TRAY_ICON = 0,
        MEDIA_STREAM_CAPTURE_ICON,
        BACKGROUND_MODE_ICON,
        OTHER_ICON,
        NAMED_STATUS_ICON_COUNT
    };

    static StatusTray *Create();

    virtual ~StatusTray();

    StatusIcon *CreateStatusIcon(StatusIconType type,
                                 const gfx::ImageSkia &image,
                                 const string16 &tool_tip);

    void RemoveStatusIcon(StatusIcon *icon);

protected:
    typedef ScopedVector<StatusIcon> StatusIcons;

    StatusTray();

    virtual StatusIcon *CreatePlatformStatusIcon(StatusIconType type,
            const gfx::ImageSkia &image,
            const string16 &tool_tip) = 0;

    const StatusIcons &status_icons() const
    {
        return status_icons_;
    }

private:
    StatusIcons status_icons_;

    DISALLOW_COPY_AND_ASSIGN(StatusTray);
};

#endif