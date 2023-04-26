#ifndef BILILIVE_BILILIVE_THEMES_THEME_PROPERTIES_H
#define BILILIVE_BILILIVE_THEMES_THEME_PROPERTIES_H


#include <set>
#include <string>

#include "base/basictypes.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/gfx/color_utils.h"

class ThemeProperties
{
public:

    enum OverwritableByUserThemeProperty
    {
        COLOR_FRAME,
        COLOR_FRAME_INACTIVE,
        COLOR_FRAME_INCOGNITO,
        COLOR_FRAME_INCOGNITO_INACTIVE,
        COLOR_TOOLBAR,
        COLOR_TAB_TEXT,
        COLOR_BACKGROUND_TAB_TEXT,
        COLOR_BOOKMARK_TEXT,
        COLOR_NTP_BACKGROUND,
        COLOR_NTP_TEXT,
        COLOR_NTP_LINK,
        COLOR_NTP_LINK_UNDERLINE,
        COLOR_NTP_HEADER,
        COLOR_NTP_SECTION,
        COLOR_NTP_SECTION_TEXT,
        COLOR_NTP_SECTION_LINK,
        COLOR_NTP_SECTION_LINK_UNDERLINE,
        COLOR_BUTTON_BACKGROUND,

        TINT_BUTTONS,
        TINT_FRAME,
        TINT_FRAME_INACTIVE,
        TINT_FRAME_INCOGNITO,
        TINT_FRAME_INCOGNITO_INACTIVE,
        TINT_BACKGROUND_TAB,

        NTP_BACKGROUND_ALIGNMENT,
        NTP_BACKGROUND_TILING,
        NTP_LOGO_ALTERNATE
    };

    enum Alignment
    {
        ALIGN_CENTER = 0,
        ALIGN_LEFT   = 1 << 0,
        ALIGN_TOP    = 1 << 1,
        ALIGN_RIGHT  = 1 << 2,
        ALIGN_BOTTOM = 1 << 3,
    };

    enum Tiling
    {
        NO_REPEAT = 0,
        REPEAT_X = 1,
        REPEAT_Y = 2,
        REPEAT = 3
    };

    enum NotOverwritableByUserThemeProperty
    {
        COLOR_CONTROL_BACKGROUND = 1000,
        COLOR_TOOLBAR_SEPARATOR,

        COLOR_NTP_SECTION_HEADER_TEXT,
        COLOR_NTP_SECTION_HEADER_TEXT_HOVER,
        COLOR_NTP_SECTION_HEADER_RULE,
        COLOR_NTP_SECTION_HEADER_RULE_LIGHT,
        COLOR_NTP_TEXT_LIGHT,
        COLOR_MANAGED_USER_LABEL,
        COLOR_MANAGED_USER_LABEL_BACKGROUND,
        COLOR_MANAGED_USER_LABEL_BORDER,

#if defined(OS_MACOSX)
        COLOR_TOOLBAR_BEZEL,
        COLOR_TOOLBAR_STROKE,
        COLOR_TOOLBAR_STROKE_INACTIVE,
        COLOR_TOOLBAR_BUTTON_STROKE,
        COLOR_TOOLBAR_BUTTON_STROKE_INACTIVE,
        GRADIENT_FRAME_INCOGNITO,
        GRADIENT_FRAME_INCOGNITO_INACTIVE,
        GRADIENT_TOOLBAR,
        GRADIENT_TOOLBAR_INACTIVE,
        GRADIENT_TOOLBAR_BUTTON,
        GRADIENT_TOOLBAR_BUTTON_INACTIVE,
        GRADIENT_TOOLBAR_BUTTON_PRESSED,
        GRADIENT_TOOLBAR_BUTTON_PRESSED_INACTIVE
#endif
    };

    static int StringToAlignment(const std::string &alignment);

    static int StringToTiling(const std::string &tiling);

    static std::string AlignmentToString(int alignment);

    static std::string TilingToString(int tiling);

    static bool IsThemeableImage(int resource_id);

    static const std::set<int> &GetTintableToolbarButtons();

    static color_utils::HSL GetDefaultTint(int id);

    static SkColor GetDefaultColor(int id);

    static bool GetDefaultDisplayProperty(int id, int *result);

private:
    DISALLOW_IMPLICIT_CONSTRUCTORS(ThemeProperties);
};

#endif