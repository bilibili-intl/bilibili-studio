#pragma once

#include "ui/views/view.h"

//�հ׵�view����������preferredSize�������հס�
class BlankView : 
    public views::View
{
public:
    BlankView();
    ~BlankView();

    void SetPreferredSize(gfx::Size size);

protected:
    gfx::Size GetPreferredSize() override;

private:
    gfx::Size preferred_size_;

};

