#include "livehime_combobox.h"

#include "bililive/bililive/ui/views/controls/bililive_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

#include "ui/base/resource/resource_bundle.h"
#include "grit/theme_resources.h"
#include "grit/generated_resources.h"

LivehimeCombobox::LivehimeCombobox(bool editable/* = false*/)
    : BililiveComboboxEx(editable)
{

    SetBackgroundColor(SkColorSetRGB(44, 45, 62));
    SetBorderColor(SkColorSetRGB(56, 57, 70), SkColorSetRGB(56, 57, 70), SkColorSetRGB(56, 57, 70));
    SetTextColor(SkColorSetRGB(255, 255, 255), SkColorSetRGB(255, 255, 255));

    SetFont(ftPrimary);
}

LivehimeCombobox::~LivehimeCombobox()
{
}

gfx::Size LivehimeCombobox::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.set_height(kBorderCtrlHeight);
    return size;
}

int LivehimeCombobox::GetHeightForWidth(int w)
{
    return GetPreferredSize().height();
}
