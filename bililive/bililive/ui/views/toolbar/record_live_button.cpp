#include "record_live_button.h"

#include "base/strings/stringprintf.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/base/resource/resource_bundle.h"


static base::string16 LocalStr(int res_id) {
    return ui::ResourceBundle::GetSharedInstance().GetLocalizedString(res_id);
}

RecordLiveButton::RecordLiveButton(views::ButtonListener* listener, const string16& text, bool positive)
    : LivehimeLabelButton(listener, text,
        positive ? LivehimeButtonStyle_V4_LiveButton : LivehimeButtonStyle_V4_RecordButton)
    , is_on_hover_(false)
    , is_starting_(false)
{
    record_time_ = L"  00:00:00";
    if (positive)
    {
        //直播按钮
        mouse_entered_image_ = new gfx::ImageSkia();
        mouse_entered_text_ = LocalStr(IDS_TOOLBAR_STOP_LIVE);
        mouse_exited_image_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_LIVE_END);
        not_starting_text_ = LocalStr(IDS_TOOLBAR_START_LIVE);
        not_starting_image_ = new gfx::ImageSkia();
    }
    else
    {
        //录制按钮
        mouse_entered_image_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_RECORD_RED);
        mouse_entered_text_ = LocalStr(IDS_TOOLBAR_STOP_RECORDING);
        mouse_exited_image_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_RECORD_TIME);
        not_starting_text_ = LocalStr(IDS_TOOLBAR_START_RECORDING);
        not_starting_image_ = GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_RECORD);
    }
}

void RecordLiveButton::OnPaint(gfx::Canvas* canvas)
{
    __super::OnPaint(canvas);

    if (is_starting_ && !is_on_hover_)
    {
        gfx::Rect rect = this->bounds();
        canvas->DrawLine(gfx::Point(rect.width() * 0.32, rect.height() * 0.3),
            gfx::Point(rect.width() * 0.32, rect.height() * 0.7), SkColorSetRGB(0xCC, 0xD2, 0xD9));
    }

}

void RecordLiveButton::OnMouseEntered(const ui::MouseEvent& event)
{
    is_on_hover_ = true;
    if (is_starting_)
    {
        SetAllStateImage(*mouse_entered_image_);
        SetText(mouse_entered_text_);
    }

    __super::OnMouseEntered(event);
}

void RecordLiveButton::OnMouseExited(const ui::MouseEvent& event)
{
    is_on_hover_ = false;
    if (is_starting_)
    {
        SetAllStateImage(*mouse_exited_image_);
        SetText(record_time_);
    }

    __super::OnMouseExited(event);
}

void RecordLiveButton::SetRecordOrLiveTime(const base::string16& record_time)
{
    record_time_ = record_time;
    if (!is_starting_)
        return;
    if (!is_on_hover_)
    {
        SetText(record_time_);
    }
}

void RecordLiveButton::SetStartingStatus(bool running)
{
    is_starting_ = running;
}

void RecordLiveButton::SetFirstButtonStatus()
{
    if (this->IsMouseHovered())
    {
        is_on_hover_ = true;
    }
    else
    {
        is_on_hover_ = false;
    }
}

void RecordLiveButton::SetButtonStatus()
{
    if (is_starting_)
    {
        if (is_on_hover_)
        {
            SetAllStateImage(*mouse_entered_image_);
            SetText(mouse_entered_text_);
        }
        else
        {
            SetAllStateImage(*mouse_exited_image_);
            SetText(record_time_);
        }
    }
    else
    {
        SetAllStateImage(*not_starting_image_);
        SetText(not_starting_text_);
        record_time_ = L"  00:00:00";
    }
}

