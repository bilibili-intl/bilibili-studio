#pragma once

#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"

#include "ui/views/controls/button/button.h"
#include "ui/views/view.h"


class RecordLiveButton : public LivehimeLabelButton
{
public:
    RecordLiveButton(views::ButtonListener* listener, const string16& text, bool positive);

    void OnPaint(gfx::Canvas* canvas);
    void OnMouseEntered(const ui::MouseEvent& event) override;
    void OnMouseExited(const ui::MouseEvent& event) override;
    void SetRecordOrLiveTime(const base::string16& record_time);
    void SetStartingStatus(bool running);
    void SetFirstButtonStatus();
    void SetButtonStatus();

private:
    bool is_on_hover_;
    base::string16 record_time_;
    bool is_starting_;
    gfx::ImageSkia* mouse_entered_image_;
    gfx::ImageSkia* mouse_exited_image_;
    gfx::ImageSkia* not_starting_image_;
    base::string16   mouse_entered_text_;
    base::string16   not_starting_text_;
};

