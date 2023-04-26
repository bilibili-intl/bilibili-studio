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
    Viddup,  // ����ֱ��������ʽ
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

    // ��������slider
    ProgressEmpty,
    ProgressFull,

    // ������
    VolumeEmpty,
    VolumeFull,
    VolumeLoud,

    // �ı�
    LabelTitle,

    DropBorder,

    Count,
};


// ֱ����ȫ������ɫ->[
const U8CPU disable_alpha = 0.3f * 255;

// ֱ������ɫ
const SkColor clrLivehime = SkColorSetRGB(0x0e, 0xbe, 0xff);
// ����ɫͬɫ����hoverɫ
const SkColor clrLivehimeFocus = SkColorSetRGB(0, 0x9b, 0xd5);
// ���߿�Ŀؼ��ı߿���ͨɫ
const SkColor clrControlBorder = SkColorSetRGB(0xd7, 0xd7, 0xd7);
// ���ڱ߿�ɫ
const SkColor clrWindowsBorder = SkColorSetRGB(0xd0, 0xd7, 0xdd);
// ������߿�ɫ
const SkColor clrDroplistBorder = SkColorSetRGB(0xeb, 0xeb, 0xeb);
// ���ڱ������Ϳͻ�����ɫ
const SkColor clrWindowsGrayBk = SkColorSetRGB(0xf5, 0xf5, 0xf5);
const SkColor clrWindowsTitle = clrLivehime;
const SkColor clrWindowsContent = SK_ColorWHITE;
// ��ɫ
const SkColor clrTextPrimary = SkColorSetRGB(0x8d, 0x9a, 0xa4);// 141,154,164
const SkColor clrTextSecondary = SkColorSetRGB(0xa1, 0xb2, 0xbf);// 161,178,191
const SkColor clrTextTitle = SkColorSetRGB(0x53, 0x67, 0x77);// 83,103,119
const SkColor clrTextTip = SkColorSetRGB(0x2c, 0x44, 0x57);
const SkColor clrTextTipWarn = SkColorSetRGB(0xfb, 0x72, 0x99); // ����վ�ۺ�ɫ
const SkColor clrTextCover = SkColorSetRGB(0x2C, 0x44, 0x57);
//V4.0 �汾����Ҫ��ɫ�Լ�͸����
const SkColor clrTextALL  = SkColorSetRGB(0x53, 0x67, 0x77); //�����ӵ���Ҫ������ɫ
const SkColor clrTextALLHov = SkColorSetRGB(0x0E, 0xBE, 0xFF); //�����ӵ���Ҫ������ɫ
const SkColor clrTextBG = SkColorSetRGB(0xF4, 0xF4, 0xF4); //������ť������ɫ
const SkColor clrTextBorder = SkColorSetRGB(0xF9, 0xF9, 0xF9);//�߿���ɫ
//ֱ����ť����ʽ��ɫ
const SkColor clrNewBtnHov = SkColorSetRGB(0xfb, 0x72, 0x99);
const SkColor clrNewBtnNor = SkColorSetRGB(0xFF, 0x66, 0x66);
const unsigned kMaskAlphaThree = 0.3f * 256;
const unsigned kMaskAlphaSix  = 0.6f * 256;
const unsigned kMaskAlphaFour = 0.4f * 256;

//ֱ�����ڿ�ʼֱ����ť


// ��ǩҳѡ�е�ɫ
const SkColor clrTabbedBkSelected = SkColorSetRGB(234, 248, 255);
// placehoder_text_color
const SkColor clrPlaceholderText = clrControlBorder;

// ]<-ֱ����ȫ������ɫ

// ���߿�Ŀؼ��ı߿���̬ɫ
const SkColor clrCtrlBorderNor = clrControlBorder;
const SkColor clrCtrlBorderHov = clrLivehime;
const SkColor clrCtrlBorderPre = clrLivehime;

// ���������İ�ť��̬ɫ
const SkColor clrButtonPositive = clrLivehime;
const SkColor clrButtonPositiveHov = clrLivehimeFocus;
const SkColor clrButtonPositivePre = clrLivehimeFocus;

// label��ɫ
const SkColor clrLabelTitle = clrTextTitle;
const SkColor clrLabelContent = clrTextPrimary;
const SkColor clrLabelDescribe = clrTextSecondary;
const SkColor clrLabelTip = clrTextTip;

// tabbed��̬��ɫ
const SkColor clrTabbedTextNor = clrTextPrimary;
const SkColor clrTabbedTextHov = clrTextPrimary;
const SkColor clrTabbedTextPre = clrLivehime;

// tabbed��̬��ɫ
const SkColor clrTabbedBkNor = clrWindowsContent;
const SkColor clrTabbedBkHov = clrWindowsGrayBk;
const SkColor clrTabbedBkPre = clrTabbedBkSelected;

// ��������slider
const SkColor clrProgressFull = clrLivehime;
const SkColor clrProgressEmpty = clrControlBorder;

// checkbox��radio
const SkColor clrOptionButtonNor = clrTextPrimary;
const SkColor clrOptionButtonHov = clrTextPrimary;
const SkColor clrOptionButtonPre = clrTextPrimary;

// link
const SkColor clrLinkButtonNormal = clrTextPrimary;
const SkColor clrLinkButtonPressed = clrLivehime;
const SkColor clrLinkButtonDisable = clrControlBorder;

// ����������ɫ
const SkColor clrVolumeBack = SkColorSetRGB(0xeb, 0xeb, 0xeb);
// ��������ɫ
const SkColor clrVolumeNormal = SkColorSetRGB(0xa6, 0xda, 0xf6);
// ������������ɫ
const SkColor clrVolumeLoud = SkColorSetRGB(0xff, 0xa0, 0xa0);

// Themes
extern const std::map<LivehimeColorType, SkColor> g_default_theme_colors;
extern const std::map<LivehimeColorType, SkColor> g_blue_theme_colors;
extern const std::map<LivehimeColorType, SkColor> g_black_theme_colors;
extern const std::map<LivehimeColorType, SkColor> g_viddup_theme_colors;

#endif