#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_THEMES_CONSTANTS_H
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_THEMES_CONSTANTS_H

#include <map>

#include "SkColor.h"

#include "livehime_theme_v3_special_constants.h"

enum class LivehimeThemeType
{
    Unknown,
    Blue,
    Black,
    Viddup,  // 海外直播主题样式
    Count,
};

enum LivehimeColorType
{
    Theme,
    ThemeLight,

    WindowTitle,
    WindowTitleText,
    WindowChildTitleText,
    WindowClient,
    WindowStatus,
    WindowBorder,

    TextTitle,
    TextContent,
    TextDescribe,
    TextTip,
    TextWarning,
    TextPlaceholder,

    SeparatorLine,

    CtrlBorderNor,
    CtrlBorderHov,
    CtrlBorderPre,
    //CtrlBorderDis,

    ButtonPositive,
    ButtonPositiveHov,
    ButtonPositivePre,
    //ButtonPositiveDis,
    ButtonNegative,
    ButtonNegativeHov,
    ButtonNegativePre,
    //ButtonNegativeDis,

    OptionButtonNor,
    OptionButtonHov,
    OptionButtonPre,
    //OptionButtonDis,

    LinkButtonNor,
    LinkButtonHov,
    LinkButtonPre,
    //LinkButtonDis,

    // tabbed
    TabbedTextNor,
    TabbedTextHov,
    TabbedTextPre,
    TabbedBkNor,
    TabbedBkHov,
    TabbedBkPre,
    TabbedBkSel,

    // listview/gridview
    ListHeaderBk,
    ListItemBkNor,
    ListItemBkHov,
    ListItemBkPre,

    // 进度条、slider
    ProgressEmpty,
    ProgressFull,

    // 音量条
    VolumeEmpty,
    VolumeFull,
    VolumeLoud,

    // 文本
    LabelTitle,

    DropBorder,

    Count,
};


// 直播姬全部的配色->[
const U8CPU disable_alpha = 0.3f * 255;

// 直播姬主色
const SkColor clrLivehime = SkColorSetRGB(0x0e, 0xbe, 0xff);
// 与主色同色调的hover色
const SkColor clrLivehimeFocus = SkColorSetRGB(0, 0x9b, 0xd5);
// 带边框的控件的边框普通色
const SkColor clrControlBorder = SkColorSetRGB(0xd7, 0xd7, 0xd7);
// 窗口边框色
const SkColor clrWindowsBorder = SkColorSetRGB(0xd0, 0xd7, 0xdd);
// 下拉框边框色
const SkColor clrDroplistBorder = SkColorSetRGB(0xeb, 0xeb, 0xeb);
// 窗口标题栏和客户区底色
const SkColor clrWindowsGrayBk = SkColorSetRGB(0xf5, 0xf5, 0xf5);
const SkColor clrWindowsTitle = clrLivehime;
const SkColor clrWindowsContent = SK_ColorWHITE;
// 字色
const SkColor clrTextPrimary = SkColorSetRGB(0x8d, 0x9a, 0xa4);// 141,154,164
const SkColor clrTextSecondary = SkColorSetRGB(0xa1, 0xb2, 0xbf);// 161,178,191
const SkColor clrTextTitle = SkColorSetRGB(0x53, 0x67, 0x77);// 83,103,119
const SkColor clrTextTip = SkColorSetRGB(0x2c, 0x44, 0x57);
const SkColor clrTextTipWarn = SkColorSetRGB(0xfb, 0x72, 0x99); // 类主站粉红色
const SkColor clrTextCover = SkColorSetRGB(0x2C, 0x44, 0x57);
//V4.0 版本的主要颜色以及透明度
const SkColor clrTextALL  = SkColorSetRGB(0x53, 0x67, 0x77); //新增加的主要字体颜色
const SkColor clrTextALLHov = SkColorSetRGB(0x0E, 0xBE, 0xFF); //新增加的主要字体颜色
const SkColor clrTextBG = SkColorSetRGB(0xF4, 0xF4, 0xF4); //分区按钮背景颜色
const SkColor clrTextBorder = SkColorSetRGB(0xF9, 0xF9, 0xF9);//边框颜色
//直播按钮新样式红色
const SkColor clrNewBtnHov = SkColorSetRGB(0xfb, 0x72, 0x99);
const SkColor clrNewBtnNor = SkColorSetRGB(0xFF, 0x66, 0x66);
const unsigned kMaskAlphaThree = 0.3f * 256;
const unsigned kMaskAlphaSix  = 0.6f * 256;
const unsigned kMaskAlphaFour = 0.4f * 256;

//直播姬在开始直播按钮


// 标签页选中底色
const SkColor clrTabbedBkSelected = SkColorSetRGB(234, 248, 255);
// placehoder_text_color
const SkColor clrPlaceholderText = clrControlBorder;

// ]<-直播姬全部的配色

// 带边框的控件的边框三态色
const SkColor clrCtrlBorderNor = clrControlBorder;
const SkColor clrCtrlBorderHov = clrLivehime;
const SkColor clrCtrlBorderPre = clrLivehime;

// 积极操作的按钮三态色
const SkColor clrButtonPositive = clrLivehime;
const SkColor clrButtonPositiveHov = clrLivehimeFocus;
const SkColor clrButtonPositivePre = clrLivehimeFocus;

// label字色
const SkColor clrLabelTitle = clrTextTitle;
const SkColor clrLabelContent = clrTextPrimary;
const SkColor clrLabelDescribe = clrTextSecondary;
const SkColor clrLabelTip = clrTextTip;

// tabbed三态字色
const SkColor clrTabbedTextNor = clrTextPrimary;
const SkColor clrTabbedTextHov = clrTextPrimary;
const SkColor clrTabbedTextPre = clrLivehime;

// tabbed三态底色
const SkColor clrTabbedBkNor = clrWindowsContent;
const SkColor clrTabbedBkHov = clrWindowsGrayBk;
const SkColor clrTabbedBkPre = clrTabbedBkSelected;

// 进度条、slider
const SkColor clrProgressFull = clrLivehime;
const SkColor clrProgressEmpty = clrControlBorder;

// checkbox、radio
const SkColor clrOptionButtonNor = clrTextPrimary;
const SkColor clrOptionButtonHov = clrTextPrimary;
const SkColor clrOptionButtonPre = clrTextPrimary;

// link
const SkColor clrLinkButtonNormal = clrTextPrimary;
const SkColor clrLinkButtonPressed = clrLivehime;
const SkColor clrLinkButtonDisable = clrControlBorder;

// 音量条背景色
const SkColor clrVolumeBack = SkColorSetRGB(0xeb, 0xeb, 0xeb);
// 音量条颜色
const SkColor clrVolumeNormal = SkColorSetRGB(0xa6, 0xda, 0xf6);
// 音量条爆音颜色
const SkColor clrVolumeLoud = SkColorSetRGB(0xff, 0xa0, 0xa0);

// Themes
extern const std::map<LivehimeColorType, SkColor> g_default_theme_colors;
extern const std::map<LivehimeColorType, SkColor> g_blue_theme_colors;
extern const std::map<LivehimeColorType, SkColor> g_black_theme_colors;
extern const std::map<LivehimeColorType, SkColor> g_viddup_theme_colors;

#endif