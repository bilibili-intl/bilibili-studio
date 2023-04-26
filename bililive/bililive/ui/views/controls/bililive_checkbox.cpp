#include "bililive_checkbox.h"
#include "bililive_theme_common.h"

BililiveCheckbox::BililiveCheckbox(const base::string16& label)
    : Checkbox(label)
{
    InitParams();
}

void BililiveCheckbox::StateChanged()
{
    // 这里不做非hovered态下的直接关闭，不然用户没有机会将鼠标划到帮助bubble上bubble就关闭了
    if (enable_help() && state() == Button::STATE_HOVERED)
    {
        ShowHelp(this);
    }
    __super::StateChanged();
}

void BililiveCheckbox::InitParams()
{
    set_focus_border(nullptr);
    SetTextColor(Button::STATE_NORMAL, clrLabelText);
    SetTextColor(Button::STATE_DISABLED, clrLabelDisableText);
}
