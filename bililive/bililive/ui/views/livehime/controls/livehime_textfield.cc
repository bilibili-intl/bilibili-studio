#include "livehime_textfield.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"


LivehimeTextfield::LivehimeTextfield(views::Textfield::StyleFlags style/* = views::Textfield::STYLE_DEFAULT*/)
    : BililiveTextfield(style)
{
    SetCommonStyle();
}

LivehimeTextfield::LivehimeTextfield(views::View *view, CooperateDirection dir/* = BD_RIGHT*/,
    views::Textfield::StyleFlags style/* = views::Textfield::STYLE_DEFAULT*/)
    : BililiveTextfield(view, dir, style)
{
    SetCommonStyle();
}

LivehimeTextfield::~LivehimeTextfield()
{
}

gfx::Size LivehimeTextfield::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.set_height(kBorderCtrlHeight);
    return size;
}

int LivehimeTextfield::GetHeightForWidth(int w)
{
    return GetPreferredSize().height();
}

void LivehimeTextfield::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    BililiveTextfield::ViewHierarchyChanged(details);
}

void LivehimeTextfield::SetCommonStyle()
{
    //SetBackgroundColor(clrWindowsContent);

    SetBackgroundColor(SkColorSetRGB(44, 45, 62)); 
    SetBorder(false);

    SetBorderColor(clrCtrlBorderNor, clrCtrlBorderHov);
    SetTextColor(clrLabelContent);
    SetFont(ftPrimary);
    set_placeholder_text_color(clrPlaceholderText);
    SetHorizontalMargins(kCtrlLeftInset, kCtrlRightInset);
}
