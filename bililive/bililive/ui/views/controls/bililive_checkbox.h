#ifndef BILILIVE_CHECKBOX_H_
#define BILILIVE_CHECKBOX_H_

#include "ui/views/controls/button/checkbox.h"

#include "bililive/bililive/livehime/help_center/help_center.h"

class BililiveCheckbox : 
    public views::Checkbox,
    public livehime::HelpSupport
{
public:
	explicit BililiveCheckbox(const base::string16& label);

protected:
    // LabelButton
    void StateChanged() override;

private:
	void InitParams();
};

#endif
