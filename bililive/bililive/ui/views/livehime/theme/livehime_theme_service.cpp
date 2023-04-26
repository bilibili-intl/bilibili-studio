#include "livehime_theme_service.h"
#include "livehime_theme_pref_names.h"

#include "bilibase/basic_types.h"

#include "base/prefs/pref_service.h"

#include "ui/base/win/shell.h"

#include "ui/views/widget/widget.h"

#include "bililive/bililive/profiles/profile.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/public/bililive/bililive_process.h"

namespace
{
    BOOL CALLBACK ThemeChangedChildWindowCallbackProc(HWND hwnd, LPARAM lParam)
    {
        views::Widget* widget = views::Widget::GetWidgetForNativeView(hwnd);
        if (widget && widget->is_secondary_widget())
        {
            if (widget->widget_delegate() &&
                (strcmp(widget->widget_delegate()->GetWidgetDelegateClassName(),
                        BililiveWidgetDelegate::kWidgetDelegateClassName) == 0)
                )
            {
                widget->ThemeChanged();

                //::EnumChildWindows(hwnd, ThemeChangedChildWindowCallbackProc, 0);

                ::InvalidateRect(hwnd, nullptr, FALSE);
            }
        }
        return TRUE;
    }

    BOOL CALLBACK ThemeChangedWindowCallbackProc(HWND hwnd, LPARAM lParam)
    {
        views::Widget* widget = views::Widget::GetWidgetForNativeView(hwnd);
        if (widget && widget->is_secondary_widget())
        {
            if(widget->widget_delegate() && 
                (strcmp(widget->widget_delegate()->GetWidgetDelegateClassName(), 
                        BililiveWidgetDelegate::kWidgetDelegateClassName) == 0)
               )
            {
                widget->ThemeChanged();

                ::EnumChildWindows(hwnd, ThemeChangedChildWindowCallbackProc, 0);

                ::InvalidateRect(hwnd, nullptr, FALSE);
            }
        }
        return TRUE;
    }

}  // namespace

void LivehimeThemeService::RegisterProfilePrefs(PrefRegistrySimple *registry)
{
    registry->RegisterIntegerPref(
        prefs::kCurrentThemeType,
        bilibase::enum_cast(LivehimeThemeType::Viddup));
}

LivehimeThemeService *LivehimeThemeService::Create()
{
    std::unique_ptr<LivehimeThemeService> theme_service(new LivehimeThemeService());
    return theme_service.release();
}

LivehimeThemeService::LivehimeThemeService()
    : rb_(ResourceBundle::GetSharedInstance())
    , theme_type_(LivehimeThemeType::Unknown)
    , default_theme_colors_(&g_default_theme_colors)
{
}

LivehimeThemeService::~LivehimeThemeService()
{
}

void LivehimeThemeService::Init(Profile *profile)
{
    int type = profile->GetPrefs()->GetInteger(prefs::kCurrentThemeType);
    theme_type_ = (LivehimeThemeType)type;

    if (theme_type_ <= LivehimeThemeType::Unknown || theme_type_ >= LivehimeThemeType::Count)
    {
        theme_type_ = LivehimeThemeType::Black;
    }
    LoadTheme();    
}

void LivehimeThemeService::LoadTheme()
{
    switch (theme_type_)
    {
    case LivehimeThemeType::Black:
        theme_colors_ = &g_black_theme_colors;
        break;
    case LivehimeThemeType::Blue:
        theme_colors_ = &g_blue_theme_colors;
        break;
    case LivehimeThemeType::Viddup:
        theme_colors_ = &g_viddup_theme_colors;
        break;
    default:
        NOTREACHED() << "invalid theme type";
        break;
    }
}

void LivehimeThemeService::NotifyThemeChanged()
{
    ::EnumThreadWindows(::GetCurrentThreadId(), ThemeChangedWindowCallbackProc, 0);
}

LivehimeThemeType LivehimeThemeService::GetThemeType() const
{
    return theme_type_;
}

void LivehimeThemeService::ChangeTheme(LivehimeThemeType type)
{
    DCHECK(type > LivehimeThemeType::Unknown && type < LivehimeThemeType::Count);
    if(theme_type_ != type)
    {
        theme_type_ = type;
        LoadTheme();
        NotifyThemeChanged();

        GetBililiveProcess()->profile()->GetPrefs()->SetInteger(prefs::kCurrentThemeType, bilibase::enum_cast(theme_type_));
    }
}

gfx::ImageSkia* LivehimeThemeService::GetImageSkiaNamed(int id) const
{
    return rb_.GetImageSkiaNamed(id);
}

SkColor LivehimeThemeService::GetColor(int id) const
{
    DCHECK(id >= LivehimeColorType::Theme && id < LivehimeColorType::Count);

    if(theme_colors_ && (theme_colors_->find((LivehimeColorType)id) != theme_colors_->end()))
    {
        return theme_colors_->at((LivehimeColorType)id);
    }
    else if (default_theme_colors_ && (default_theme_colors_->find((LivehimeColorType)id) != default_theme_colors_->end()))
    {
        return default_theme_colors_->at((LivehimeColorType)id);
    }

    NOTREACHED() << "not define color";
    return SK_ColorWHITE;
}

bool LivehimeThemeService::GetDisplayProperty(int id, int *result) const
{
    return false;
}

bool LivehimeThemeService::ShouldUseNativeFrame() const
{
    return ui::win::IsAeroGlassEnabled();
}

bool LivehimeThemeService::HasCustomImage(int id) const
{
    return false;
}

base::RefCountedMemory* LivehimeThemeService::GetRawData(int id, ui::ScaleFactor scale_factor) const
{
    return rb_.LoadDataResourceBytesForScale(id, scale_factor);
}
