#pragma once

#include "ui/views/view.h"

//空白的view，可以设置preferredSize用来填充空白。
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

