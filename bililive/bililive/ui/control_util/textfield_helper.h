#ifndef CONTROL_UTILS_TEXTFIELD_HELPER_H_
#define CONTROL_UTILS_TEXTFIELD_HELPER_H_

#include "bililive/bililive/ui/views/controls/bililive_textfield.h"
#include "button_helper.h"

#include <string>
#include <functional>

class BililiveTextfieldHelper;

using TextfieldChangedHandler = std::function < void(BililiveTextfieldHelper* sender, const base::string16& new_contents) > ;
using TextfieldClickedHandler = std::function < void(BililiveTextfieldHelper* sender, const ui::Event& event) >;

class BililiveTextfieldHelper
    : public BililiveTextfield
    , public views::TextfieldController
    , public views::ButtonListener
{
public:
    BililiveTextfieldHelper();
    BililiveTextfieldHelper(views::Textfield::StyleFlags style);

    void SetOnButtonClicked(const TextfieldClickedHandler& handler);
    void SetOnTextChanged(const TextfieldChangedHandler& handler);

protected:
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;
    void ContentsChanged(views::Textfield* sender, const base::string16& new_contents) override;

protected:
    TextfieldClickedHandler button_clicked_handler_;
    TextfieldChangedHandler text_changed_handler_;
};

#endif

