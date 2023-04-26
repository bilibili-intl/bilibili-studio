#pragma once

#include "ui/views/view.h"

//�����л���ʾ��view��view
class TabSelectView :
    public views::View
{
public:
    TabSelectView();
    ~TabSelectView();

    gfx::Size GetPreferredSize() override;
    void Layout() override;
    void SelectView(int index);
    int GetSelect();

private:
    int select_index_ = 0;
};
