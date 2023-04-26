#include "bililive_views_delegate.h"
#include "base/memory/scoped_ptr.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/bililive/prefs/scoped_user_pref_update.h"
#include "bililive/public/common/pref_names.h"
#include "ui/base/ui_base_switches.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/screen.h"
#include "ui/views/widget/native_widget.h"
#include "ui/views/widget/widget.h"
#include <dwmapi.h>
#include "base/win/windows_version.h"
#include "ui/base/win/shell.h"
#include "bililive/app/bililive_dll_resource.h"
#include "bililive/public/common/bililive_constants.h"
#include "ui/gfx/icon_util.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "bililive/bililive/profiles/profile.h"


namespace
{
    PrefService *GetPrefsForWindow(const views::Widget *window)
    {
        Profile *profile = reinterpret_cast<Profile *>(
                               window->GetNativeWindowProperty(Profile::kProfileKey));
        if (!profile)
        {
            NULL;
        }

        return profile->GetPrefs();
    }

    int GetAppIconResourceId()
    {
        return IDR_MAINFRAME;
    }

    HICON GetAppIcon()
    {
        const int icon_id = GetAppIconResourceId();
        return LoadIcon(GetModuleHandle(bililive::kBililiveResourcesDll),
                        MAKEINTRESOURCE(icon_id));
    }

    scoped_ptr<SkBitmap> GetAppIconForSize(int size)
    {
        const int icon_id = GetAppIconResourceId();
        return IconUtil::CreateSkBitmapFromIconResource(
                   GetModuleHandle(bililive::kBililiveResourcesDll), icon_id, size);
    }

}


void BililiveViewsDelegate::SaveWindowPlacement(const views::Widget *window,
        const std::string &window_name,
        const gfx::Rect &bounds,
        ui::WindowShowState show_state)
{
    PrefService *prefs = GetPrefsForWindow(window);
    if (!prefs)
    {
        return;
    }

    DCHECK(prefs->FindPreference(window_name.c_str()));
    DictionaryPrefUpdate update(prefs, window_name.c_str());
    DictionaryValue *window_preferences = update.Get();
    window_preferences->SetInteger("left", bounds.x());
    window_preferences->SetInteger("top", bounds.y());
    window_preferences->SetInteger("right", bounds.right());
    window_preferences->SetInteger("bottom", bounds.bottom());
    window_preferences->SetBoolean("maximized",
                                   show_state == ui::SHOW_STATE_MAXIMIZED);
    gfx::Rect work_area(gfx::Screen::GetScreenFor(window->GetNativeView())->
                        GetDisplayNearestWindow(window->GetNativeView()).work_area());
    window_preferences->SetInteger("work_area_left", work_area.x());
    window_preferences->SetInteger("work_area_top", work_area.y());
    window_preferences->SetInteger("work_area_right", work_area.right());
    window_preferences->SetInteger("work_area_bottom", work_area.bottom());
}

bool BililiveViewsDelegate::GetSavedWindowPlacement(
    const std::string &window_name,
    gfx::Rect *bounds,
    ui::WindowShowState *show_state) const
{
    //PrefService* prefs = g_bililive_process->local_state();
    PrefService *prefs = GetBililiveProcess()->profile()->GetPrefs();
    if (!prefs)
    {
        return false;
    }

    DCHECK(prefs->FindPreference(window_name.c_str()));
    const DictionaryValue *dictionary = prefs->GetDictionary(window_name.c_str());
    int left, top, right, bottom;
    if (!dictionary || !dictionary->GetInteger("left", &left) ||
        !dictionary->GetInteger("top", &top) ||
        !dictionary->GetInteger("right", &right) ||
        !dictionary->GetInteger("bottom", &bottom))
    {
        return false;
    }

    bounds->SetRect(left, top, right - left, bottom - top);

    bool maximized = false;
    if (dictionary)
    {
        dictionary->GetBoolean("maximized", &maximized);
    }
    *show_state = maximized ? ui::SHOW_STATE_MAXIMIZED : ui::SHOW_STATE_NORMAL;

    return true;
}

void BililiveViewsDelegate::NotifyAccessibilityEvent(
    views::View *view, ui::AccessibilityTypes::Event event_type)
{
}

void BililiveViewsDelegate::NotifyMenuItemFocused(const string16 &menu_name,
        const string16 &menu_item_name,
        int item_index,
        int item_count,
        bool has_submenu)
{
}

HICON BililiveViewsDelegate::GetDefaultWindowIcon() const
{
    return GetAppIcon();
}

views::NonClientFrameView *BililiveViewsDelegate::CreateDefaultNonClientFrameView(
    views::Widget *widget)
{
    return NULL;
}

bool BililiveViewsDelegate::UseTransparentWindows() const
{
    return false;
}

void BililiveViewsDelegate::AddRef()
{
    GetBililiveProcess()->AddRefModule();
}

void BililiveViewsDelegate::ReleaseRef()
{
    GetBililiveProcess()->ReleaseModule();
}

void BililiveViewsDelegate::OnBeforeWidgetInit(
    views::Widget::InitParams *params,
    views::internal::NativeWidgetDelegate *delegate)
{
}

base::TimeDelta
BililiveViewsDelegate::GetDefaultTextfieldObscuredRevealDuration()
{
    return base::TimeDelta();
}