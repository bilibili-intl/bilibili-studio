#include "button_helper.h"

BililiveButtonHelper::BililiveButtonHelper(const base::string16& text)
    : BililiveLabelButton(this, text)
{
}

void BililiveButtonHelper::ButtonPressed(Button* sender, const ui::Event& event)
{
    if (clicked_handler_)
        clicked_handler_(this, event);
}

void BililiveButtonHelper::SetOnClicked(const ButtonClickedHandler& handler)
{
    clicked_handler_ = handler;
}
