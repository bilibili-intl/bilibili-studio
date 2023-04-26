#include "bililive_frame_titlebar.h"


const char BililiveFrameTitleBar::kDevViewClassName[] = "BililiveFrameTitleBar";

BililiveFrameTitleBar::BililiveFrameTitleBar()
    : delegate_(nullptr)
    , active_(true)
{
}

int BililiveFrameTitleBar::NonClientHitTest(const gfx::Point &point)
{
    for (int i = 0; i < child_count(); i++)
    {
        if (child_at(i)->bounds().Contains(point))
        {
            gfx::Point pt(point);
            ConvertPointToTarget(this, child_at(i), &pt);
            return child_at(i)->HitTestRect(gfx::Rect(pt, gfx::Size(1, 1))) ? HTNOWHERE : HTCAPTION;
        }
    }
    return HTCAPTION;
}

void BililiveFrameTitleBar::SetTitle(const base::string16& title)
{
}

void BililiveFrameTitleBar::SetActive(bool active)
{
    if(active_ != active)
    {
        active_ = active;
        OnActiveChanged(!active_);
    }
}

