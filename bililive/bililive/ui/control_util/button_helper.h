#ifndef CONTROL_UTILS_BUTTON_HELPER_H_
#define CONTROL_UTILS_BUTTON_HELPER_H_

#include <functional>
#include "bililive/bililive/ui/views/controls/bililive_labelbutton.h"

class BililiveButtonHelper;
using ButtonClickedHandler = std::function<void(BililiveButtonHelper* sender, const ui::Event& event)>;

class BililiveButtonHelper : public BililiveLabelButton, public views::ButtonListener
{
    ButtonClickedHandler clicked_handler_;

protected:
    void ButtonPressed(Button* sender, const ui::Event& event) override;

public:
    BililiveButtonHelper(const base::string16& text);
    void SetOnClicked(const ButtonClickedHandler& handler);
};

#endif

