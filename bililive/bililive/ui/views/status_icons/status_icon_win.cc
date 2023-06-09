#include "status_icon_win.h"

#include "base/strings/string_number_conversions.h"
#include "base/win/metro.h"
#include "base/win/windows_version.h"

#include "ui/base/win/dpi.h"
#include "ui/gfx/icon_util.h"
#include "ui/gfx/point.h"
#include "ui/views/controls/menu/menu_item_view.h"
#include "ui/views/controls/menu/menu_runner.h"

#include "third_party/skia/include/core/SkBitmap.h"


////////////////////////////////////////////////////////////////////////////////
// StatusIconWin, public:

StatusIconWin::StatusIconWin(UINT id, HWND window, UINT message)
    : icon_id_(id),
      window_(window),
      message_id_(message),
      menu_model_(NULL),
      menu_view_(nullptr)
{
    NOTIFYICONDATA icon_data;
    InitIconData(&icon_data);
    icon_data.uFlags = NIF_MESSAGE;
    icon_data.uCallbackMessage = message_id_;
    BOOL result = Shell_NotifyIcon(NIM_ADD, &icon_data);
    // This can happen if the explorer process isn't running when we try to
    // create the icon for some reason (for example, at startup).
    if (!result)
    {
        LOG(WARNING) << "Unable to create status tray icon.";
    }
}

StatusIconWin::~StatusIconWin()
{
    // Remove our icon.
    NOTIFYICONDATA icon_data;
    InitIconData(&icon_data);
    Shell_NotifyIcon(NIM_DELETE, &icon_data);
}

void StatusIconWin::HandleClickEvent(const gfx::Point &cursor_pos,
                                     bool left_mouse_click)
{
    // Pass to the observer if appropriate.
    if (left_mouse_click && HasObservers())
    {
        DispatchClickEvent();
        return;
    }

    if (!menu_model_ && !menu_view_)
    {
        return;
    }

    // Set our window as the foreground window, so the context menu closes when
    // we click away from it.
    if (!SetForegroundWindow(window_))
    {
        return;
    }

    if (!menu_runner_)
    {
        if (menu_model_)
        {
            menu_runner_.reset(new views::MenuRunner(menu_model_));
        }
        else if (menu_view_)
        {
            menu_runner_.reset(new views::MenuRunner(menu_view_));
        }
    }
    if (menu_runner_)
    {
        ignore_result(menu_runner_->RunMenuAt(NULL, NULL,
            gfx::Rect(cursor_pos, gfx::Size()), views::MenuItemView::TOPLEFT,
            ui::MENU_SOURCE_MOUSE, views::MenuRunner::HAS_MNEMONICS));
    }
}

void StatusIconWin::HandleBalloonClickEvent()
{
    if (HasObservers())
    {
        DispatchBalloonClickEvent();
    }
}

void StatusIconWin::ResetIcon()
{
    NOTIFYICONDATA icon_data;
    InitIconData(&icon_data);
    // Delete any previously existing icon.
    Shell_NotifyIcon(NIM_DELETE, &icon_data);
    InitIconData(&icon_data);
    icon_data.uFlags = NIF_MESSAGE;
    icon_data.uCallbackMessage = message_id_;
    icon_data.hIcon = icon_.Get();
    // If we have an image, then set the NIF_ICON flag, which tells
    // Shell_NotifyIcon() to set the image for the status icon it creates.
    if (icon_data.hIcon)
    {
        icon_data.uFlags |= NIF_ICON;
    }
    // Re-add our icon.
    BOOL result = Shell_NotifyIcon(NIM_ADD, &icon_data);
    if (!result)
    {
        LOG(WARNING) << "Unable to re-create status tray icon.";
    }
}

void StatusIconWin::SetImage(const gfx::ImageSkia &image)
{
    // Create the icon.
    NOTIFYICONDATA icon_data;
    InitIconData(&icon_data);
    icon_data.uFlags = NIF_ICON;
    icon_.Set(IconUtil::CreateHICONFromSkBitmap(*image.bitmap()));
    icon_data.hIcon = icon_.Get();
    BOOL result = Shell_NotifyIcon(NIM_MODIFY, &icon_data);
    if (!result)
    {
        LOG(WARNING) << "Error setting status tray icon image";
    }
}

void StatusIconWin::SetPressedImage(const gfx::ImageSkia &image)
{
    // Ignore pressed images, since the standard on Windows is to not highlight
    // pressed status icons.
}

void StatusIconWin::SetToolTip(const string16 &tool_tip)
{
    // Create the icon.
    NOTIFYICONDATA icon_data;
    InitIconData(&icon_data);
    icon_data.uFlags = NIF_TIP;
    wcscpy_s(icon_data.szTip, tool_tip.c_str());
    BOOL result = Shell_NotifyIcon(NIM_MODIFY, &icon_data);
    if (!result)
    {
        LOG(WARNING) << "Unable to set tooltip for status tray icon";
    }
}

void StatusIconWin::DisplayBalloon(const gfx::ImageSkia &icon,
                                   const string16 &title,
                                   const string16 &contents)
{
    NOTIFYICONDATA icon_data;
    InitIconData(&icon_data);
    icon_data.uFlags = NIF_INFO;
    icon_data.dwInfoFlags = NIIF_INFO;
    wcscpy_s(icon_data.szInfoTitle, title.c_str());
    wcscpy_s(icon_data.szInfo, contents.c_str());
    icon_data.uTimeout = 0;

    base::win::Version win_version = base::win::GetVersion();
    if (!icon.isNull() && win_version != base::win::VERSION_PRE_XP)
    {
        balloon_icon_.Set(IconUtil::CreateHICONFromSkBitmap(*icon.bitmap()));
        if (win_version >= base::win::VERSION_VISTA)
        {
            icon_data.hBalloonIcon = balloon_icon_.Get();
            icon_data.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
        }
        else
        {
            icon_data.hIcon = balloon_icon_.Get();
            icon_data.uFlags |= NIF_ICON;
            icon_data.dwInfoFlags = NIIF_USER;
        }
    }

    BOOL result = Shell_NotifyIcon(NIM_MODIFY, &icon_data);
    if (!result)
    {
        LOG(WARNING) << "Unable to create status tray balloon.";
    }
}

////////////////////////////////////////////////////////////////////////////////
// StatusIconWin, private:

void StatusIconWin::UpdatePlatformContextMenu(ui::MenuModel *model)
{
    // |menu_model_| is about to be destroyed. Destroy the menu (which closes it)
    // so that it doesn't attempt to continue using |menu_model_|.
    menu_runner_.reset();
    DCHECK(model);
    menu_model_ = model;
}

void StatusIconWin::UpdatePlatformContextMenu(views::MenuItemView *menu)
{
    DCHECK(menu);
    menu_runner_.reset();
    menu_view_ = menu;
}

void StatusIconWin::InitIconData(NOTIFYICONDATA *icon_data)
{
    if (base::win::GetVersion() >= base::win::VERSION_VISTA)
    {
        memset(icon_data, 0, sizeof(NOTIFYICONDATA));
        icon_data->cbSize = sizeof(NOTIFYICONDATA);
    }
    else
    {
        memset(icon_data, 0, NOTIFYICONDATA_V3_SIZE);
        icon_data->cbSize = NOTIFYICONDATA_V3_SIZE;
    }

    icon_data->hWnd = window_;
    icon_data->uID = icon_id_;
}