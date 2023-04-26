#include "bililive_checkbox.h"
#include "bililive_theme_common.h"

BililiveCheckbox::BililiveCheckbox(const base::string16& label)
    : Checkbox(label)
{
    InitParams();
}

void BililiveCheckbox::StateChanged()
{
    // ���ﲻ����hovered̬�µ�ֱ�ӹرգ���Ȼ�û�û�л��Ὣ��껮������bubble��bubble�͹ر���
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
