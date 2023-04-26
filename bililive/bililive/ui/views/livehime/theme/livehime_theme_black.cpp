#include "livehime_theme_constants.h"

namespace
{
    const SkColor clrTheme = SkColorSetRGB(40, 40, 40);
    const SkColor clrThemeLight = SkColorSetRGB(50, 50, 50);
}

const std::map<LivehimeColorType, SkColor> g_black_theme_colors {
    { Theme, clrTheme },
    { ThemeLight, clrThemeLight },

    { WindowTitle, clrThemeLight },
    { WindowChildTitleText, clrLivehime },
    { WindowClient, clrTheme },
    { WindowStatus, clrTheme },
};