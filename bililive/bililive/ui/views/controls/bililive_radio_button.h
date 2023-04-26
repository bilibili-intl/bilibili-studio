#ifndef BILILIVE_RADIO_BUTTON_H_
#define BILILIVE_RADIO_BUTTON_H_

#include "ui/views/controls/button/radio_button.h"

class BililiveRadioButton : public views::RadioButton
{
public:
    explicit BililiveRadioButton(const base::string16& label, int group_id);

    static int new_groupid();

protected:

private:
    static volatile long inc_group_id_;
};

#endif
