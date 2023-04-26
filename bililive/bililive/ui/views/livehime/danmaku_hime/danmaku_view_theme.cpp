#include "danmaku_view_theme.h"

DanmakuThemeInterface::DanmakuThemeInterface()
{
}

DanmakuThemeInterface::~DanmakuThemeInterface()
{
}

void DanmakuThemeInterface::SwitchTheme(DanmakuThemeType theme)
{
    if (theme_ == theme)
    {
        return;
    }

    theme_ = theme;


    SwitchThemeImpl();
}

DanmakuThemeType DanmakuThemeInterface::GetTheme()
{
    return theme_;
}

DanmakuShowTypeInterface::DanmakuShowTypeInterface()
{
}

DanmakuShowTypeInterface::~DanmakuShowTypeInterface()
{
}

void DanmakuShowTypeInterface::SwitchShowType(DanmakuShowType show_type)
{
    if (show_type_ == show_type) 
    {
        return;
    }

    show_type_ = show_type;

    SwitchShowTypeImpl();
}

DanmakuShowType DanmakuShowTypeInterface::GetShowType()
{
    return show_type_;
}

SkColor GetDanmakuThemeColor(DanmakuThemeType theme, DanmakuThemeViewType type)
{
    if (theme == DanmakuThemeType::kWhite)
    {
        switch (type)
        {
        case DanmakuThemeViewType::kText:
            return SkColorSetARGB(153, 83, 103, 119);
            break;

        case DanmakuThemeViewType::kTextHighLight:
            return SkColorSetRGB(0x53, 0x67, 0x77);
            break;

        case DanmakuThemeViewType::kBackground:
            return SkColorSetARGB(0xff, 0xf9, 0xf9, 0xf9);//255, 244, 244, 244
            break;

        case DanmakuThemeViewType::kBackgroundHighLight:
            return SkColorSetARGB(255, 249, 249, 249);
            break;
        }
    }
    else if (theme == DanmakuThemeType::kDark)
    {
        switch (type)
        {
            case DanmakuThemeViewType::kText:
                return SkColorSetARGB(153, 211, 226, 238);
            break;

            case DanmakuThemeViewType::kTextHighLight:
                return SkColorSetARGB(255, 211, 226, 238);
            break;

            case DanmakuThemeViewType::kBackground:
                return SkColorSetRGB(0x1B, 0x1B, 0x1B);
            break;

            case DanmakuThemeViewType::kBackgroundHighLight:
                return SkColorSetRGB(0x1B, 0x1B, 0x1B);
            break;
        }
    }
    else if (theme == DanmakuThemeType::kViddup)
    {
        switch (type)
        {
        case DanmakuThemeViewType::kText:
            return SkColorSetARGB(153, 83, 103, 119);
            break;

        case DanmakuThemeViewType::kTextHighLight:
            return SkColorSetRGB(0x53, 0x67, 0x77);
            break;

        case DanmakuThemeViewType::kBackground:
            return SkColorSetARGB(0xff, 0x15, 0x16, 0x20);
            break;

        case DanmakuThemeViewType::kBackgroundHighLight:
            return SkColorSetARGB(255, 0x15, 0x16, 0x20);
            break;
        }
    }

    switch (type)
    {
        case DanmakuThemeViewType::kGiftFilterBtnFocus:
        case DanmakuThemeViewType::kSendBtn:
            return SkColorSetARGB(255, 14, 190, 255);
        break;
    }


    return 0;
}


