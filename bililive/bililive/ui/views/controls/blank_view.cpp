#include "blank_view.h"

BlankView::BlankView()
{

}

BlankView::~BlankView()
{

}

void BlankView::SetPreferredSize(gfx::Size size)
{
    preferred_size_ = size;
}

gfx::Size BlankView::GetPreferredSize()
{
    return preferred_size_;
}
