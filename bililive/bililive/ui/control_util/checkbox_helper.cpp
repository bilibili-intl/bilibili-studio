#include "checkbox_helper.h"

BililiveCheckboxHelper::BililiveCheckboxHelper(const base::string16& text)
    : BililiveCheckbox(text)
{
    set_listener(this);
}

void BililiveCheckboxHelper::ButtonPressed(Button* sender, const ui::Event& event)
{
    if (clicked_handler_)
        clicked_handler_(this, event);
}

void BililiveCheckboxHelper::SetOnClicked(const CheckboxClickedHandler& handler)
{
    clicked_handler_ = handler;
}
