#pragma once

#include "ui/views/view.h"
#include "ui/views/background.h"

class RoundRectBackground : 
    public views::Background
{
public:
    enum BackgroundCornerType
    {
        TOP_LEFT = 0x01,
        TOP_RIGHT = 0x02,
        BOTTOM_LEFT = 0x04,
        BOTTOM_RIGHT = 0x08,
        TOP = 0x03,
        BOTTOM = 0x0c,
        ALL = 0x0f,
    };

    explicit RoundRectBackground(int radius, int corner, SkColor color);

    void Paint(gfx::Canvas* canvas, views::View* view) const override;

private:
    int radius_ = 6;
    SkColor fillColor_ = 0;
    BackgroundCornerType corner_ = BackgroundCornerType::ALL;
    DISALLOW_COPY_AND_ASSIGN(RoundRectBackground);
};

