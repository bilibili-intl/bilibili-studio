#ifndef CONTROL_UTILS_CHECKBOX_HELPER_H_
#define CONTROL_UTILS_CHECKBOX_HELPER_H_

#include "button_helper.h"
#include "bililive/bililive/ui/views/controls/bililive_checkbox.h"

class BililiveCheckboxHelper;
using CheckboxClickedHandler = std::function<void(BililiveCheckboxHelper*, const ui::Event& event)>;

class BililiveCheckboxHelper : public BililiveCheckbox, public views::ButtonListener
{
    CheckboxClickedHandler clicked_handler_;

protected:
    void ButtonPressed(Button* sender, const ui::Event& event) override;

public:
    explicit BililiveCheckboxHelper(const base::string16& label);
    void SetOnClicked(const CheckboxClickedHandler& handler);
};

#endif
