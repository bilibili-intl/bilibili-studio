#include "textfield_helper.h"

BililiveTextfieldHelper::BililiveTextfieldHelper()
{
    SetController(this);
}

BililiveTextfieldHelper::BililiveTextfieldHelper(views::Textfield::StyleFlags style)
    : BililiveTextfield(style)
{
    SetController(this);
}

void BililiveTextfieldHelper::SetOnButtonClicked(const TextfieldClickedHandler& handler)
{
    button_clicked_handler_ = handler;
}

void BililiveTextfieldHelper::SetOnTextChanged(const TextfieldChangedHandler& handler)
{
    text_changed_handler_ = handler;
}

void BililiveTextfieldHelper::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (button_clicked_handler_)
        button_clicked_handler_(this, event);
}

void BililiveTextfieldHelper::ContentsChanged(views::Textfield* sender, const base::string16& new_contents)
{
    if (text_changed_handler_)
        text_changed_handler_(this, new_contents);
}
