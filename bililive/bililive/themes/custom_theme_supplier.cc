#include "bililive/bililive/themes/custom_theme_supplier.h"

#include "base/memory/ref_counted_memory.h"
#include "ui/gfx/color_utils.h"
#include "ui/gfx/image/image.h"

CustomThemeSupplier::CustomThemeSupplier(ThemeType theme_type)
    : theme_type_(theme_type) {}

CustomThemeSupplier::~CustomThemeSupplier() {}

void CustomThemeSupplier::StartUsingTheme() {}

void CustomThemeSupplier::StopUsingTheme() {}

bool CustomThemeSupplier::GetTint(int id, color_utils::HSL *hsl) const
{
    return false;
}

bool CustomThemeSupplier::GetColor(int id, SkColor *color) const
{
    return false;
}

bool CustomThemeSupplier::GetDisplayProperty(int id, int *result) const
{
    return false;
}

gfx::Image CustomThemeSupplier::GetImageNamed(int id)
{
    return gfx::Image();
}

base::RefCountedMemory *CustomThemeSupplier::GetRawData(
    int idr_id,
    ui::ScaleFactor scale_factor) const
{
    return NULL;
}

bool CustomThemeSupplier::HasCustomImage(int id) const
{
    return false;
}