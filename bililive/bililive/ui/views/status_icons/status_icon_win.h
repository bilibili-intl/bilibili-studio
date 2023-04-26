#ifndef BILILIVE_BILILIVE_UI_VIEWS_STATUS_ICONS_STATUS_ICON_WIN_H_
#define BILILIVE_BILILIVE_UI_VIEWS_STATUS_ICONS_STATUS_ICON_WIN_H_

#include <windows.h>
#include <shellapi.h>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/win/scoped_gdi_object.h"
#include "bililive/bililive/status_icons/status_icon.h"

namespace gfx
{
    class Point;
}

namespace views
{
    class MenuRunner;
}

class StatusIconWin : public StatusIcon
{
public:
    StatusIconWin(UINT id, HWND window, UINT message);
    virtual ~StatusIconWin();

    void HandleClickEvent(const gfx::Point &cursor_pos, bool left_button_click);

    void HandleBalloonClickEvent();

    void ResetIcon();

    UINT icon_id() const
    {
        return icon_id_;
    }
    UINT message_id() const
    {
        return message_id_;
    }

    virtual void SetImage(const gfx::ImageSkia &image) OVERRIDE;
    virtual void SetPressedImage(const gfx::ImageSkia &image) OVERRIDE;
    virtual void SetToolTip(const string16 &tool_tip) OVERRIDE;
    virtual void DisplayBalloon(const gfx::ImageSkia &icon,
                                const string16 &title,
                                const string16 &contents) OVERRIDE;

protected:
    virtual void UpdatePlatformContextMenu(ui::MenuModel *model) OVERRIDE;
    virtual void UpdatePlatformContextMenu(views::MenuItemView *menu) OVERRIDE;

private:
    void InitIconData(NOTIFYICONDATA *icon_data);

    UINT icon_id_;

    HWND window_;

    UINT message_id_;

    base::win::ScopedHICON icon_;

    base::win::ScopedHICON balloon_icon_;

    ui::MenuModel *menu_model_;
    views::MenuItemView *menu_view_;

    scoped_ptr<views::MenuRunner> menu_runner_;

    DISALLOW_COPY_AND_ASSIGN(StatusIconWin);
};

#endif