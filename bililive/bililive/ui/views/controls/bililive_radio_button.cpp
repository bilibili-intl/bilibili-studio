#include "bililive_radio_button.h"

volatile long BililiveRadioButton::inc_group_id_ = 0;

int BililiveRadioButton::new_groupid()
{
    return ::InterlockedIncrement(&inc_group_id_);
}

BililiveRadioButton::BililiveRadioButton(const base::string16& label, int group_id)
    : views::RadioButton(label, group_id)
{
}
