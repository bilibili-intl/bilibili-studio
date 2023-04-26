#include "tab_select_view.h"


TabSelectView::TabSelectView()
{
}

TabSelectView::~TabSelectView()
{

}

gfx::Size TabSelectView::GetPreferredSize()
{
    if (child_count() <= 0)
    {
        return {};
    }
    else
    {
        return child_at(select_index_)->GetPreferredSize();
    }
}

void TabSelectView::Layout()
{
    int ch_count = child_count();
    for (int i = 0; i < ch_count; i++)
    {
        auto ch_view = child_at(i);
        if (select_index_ == i)
        {
            ch_view->SetBoundsRect(GetContentsBounds());
            ch_view->SetVisible(true);
        }
        else
        {
            ch_view->SetVisible(false);
        }
    }
}

void TabSelectView::SelectView(int index)
{
    if (index < 0 ||
        index >= static_cast<int>(child_count()))
    {
        return;
    }

    select_index_ = index;
    Layout();
    if (parent())
    {
        parent()->Layout();
    }
}

int TabSelectView::GetSelect()
{
    return select_index_;
}


