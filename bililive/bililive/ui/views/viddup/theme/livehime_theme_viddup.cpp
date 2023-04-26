#include "bililive/bililive/ui/views/livehime/theme/livehime_theme_constants.h"


namespace
{
    // 直播姬全部的配色->[
    const U8CPU viddup_disable_alpha = 0.3f * 255;

    // 海外直播姬主色
    const SkColor clrViddupTheme = SkColorSetRGB(0x0c, 0x0d, 0x13);
    
    // 与主色同色调的hover色
    const SkColor clrViddupThemeLight = SkColorSetRGB(0x32, 0x32, 0x32);
    
    // 带边框的控件的边框普通色
    const SkColor clrViddupControlBorder = SkColorSetRGB(0x00, 0x00, 0x00);
    
    // 窗口边框色
    const SkColor clrViddupWindowsBorder = SkColorSetRGB(0x00, 0x00, 0x00);
    
    // 下拉框边框色
    const SkColor clrViddupDroplistBorder = SkColorSetRGB(0xeb, 0xeb, 0xeb);
    
    // 窗口标题栏和客户区底色
    const SkColor clrViddupWindowsGrayBk = SkColorSetRGB(0x22, 0x23, 0x31);
    const SkColor clrViddupWindowsTitle = SkColorSetRGB(0x22, 0x23, 0x31);
    const SkColor clrViddupWindowsContent = SkColorSetRGB(0x15, 0x16, 0x20);
    
    // 字色
    const SkColor clrViddupTextPrimary = SkColorSetRGB(0x99, 0x99, 0x99);// rgba(153,153,153,1)
    const SkColor clrViddupTextSecondary = SkColorSetRGB(0xa1, 0xb2, 0xbf);// 161,178,191
    const SkColor clrViddupTextTitle = SkColorSetRGB(0xFF, 0xFF, 0xFF);// rgba(255,255,255,1)
    const SkColor clrViddupTextTip = SkColorSetRGB(0x2c, 0x44, 0x57);
    const SkColor clrViddupTextTipWarn = SkColorSetRGB(0xfb, 0x72, 0x99); // 类主站粉红色
    const SkColor clrViddupTextCover = SkColorSetRGB(0x2C, 0x44, 0x57);
    
    //V4.0 版本的主要颜色以及透明度
    const SkColor clrViddupTextALL = SkColorSetRGB(0x53, 0x67, 0x77);  //新增加的主要字体颜色
    const SkColor clrViddupTextALLHov = SkColorSetRGB(0x0E, 0xBE, 0xFF);  //新增加的主要字体颜色
    const SkColor clrViddupTextBG = SkColorSetRGB(0xF4, 0xF4, 0xF4);  //分区按钮背景颜色
    const SkColor clrViddupTextBorder = SkColorSetRGB(0xF9, 0xF9, 0xF9);  //边框颜色
    
    //直播按钮新样式红色
    const SkColor clrViddupNewBtnHov = SkColorSetRGB(0xfb, 0x72, 0x99);
    const SkColor clrViddupNewBtnNor = SkColorSetRGB(0xFF, 0x66, 0x66);
    const unsigned kMaskViddupAlphaThree = 0.3f * 256;
    const unsigned kMaskViddupAlphaSix = 0.6f * 256;
    const unsigned kMaskViddupAlphaFour = 0.4f * 256;
    //直播姬在开始直播按钮

    // 标签页选中底色
    const SkColor clrViddupTabbedBkSelected = SkColorSetRGB(74, 75, 97);
    // placehoder_text_color
    const SkColor clrViddupPlaceholderText = clrViddupControlBorder;
    // ]<-直播姬全部的配色

    // 带边框的控件的边框三态色
    const SkColor clrViddupCtrlBorderNor = clrViddupControlBorder;
    const SkColor clrViddupCtrlBorderHov = clrViddupTheme;
    const SkColor clrViddupCtrlBorderPre = clrViddupTheme;

    // 积极操作的按钮三态色
    const SkColor clrViddupButtonPositive = clrViddupTheme;
    const SkColor clrViddupButtonPositiveHov = clrViddupThemeLight;
    const SkColor clrViddupButtonPositivePre = clrViddupThemeLight;

    // label字色
    const SkColor clrViddupLabelTitle = clrViddupTextTitle;
    const SkColor clrViddupLabelContent = clrViddupTextPrimary;
    const SkColor clrViddupLabelDescribe = clrViddupTextSecondary;
    const SkColor clrViddupLabelTip = clrViddupTextTip;

    // tabbed三态字色
    const SkColor clrViddupTabbedTextNor = clrViddupTextPrimary;
    const SkColor clrViddupTabbedTextHov = clrViddupTextPrimary;
    const SkColor clrViddupTabbedTextPre = clrViddupTheme;

    // tabbed三态底色
    const SkColor clrViddupTabbedBkNor = clrViddupWindowsContent;
    const SkColor clrViddupTabbedBkHov = clrViddupWindowsGrayBk;
    const SkColor clrViddupTabbedBkPre = clrViddupTabbedBkSelected;

    // 进度条、slider
    const SkColor clrViddupProgressFull = clrViddupTheme;
    const SkColor clrViddupProgressEmpty = clrViddupControlBorder;

    // checkbox、radio
    const SkColor clrViddupOptionButtonNor = clrViddupTextPrimary;
    const SkColor clrViddupOptionButtonHov = clrViddupTextPrimary;
    const SkColor clrViddupOptionButtonPre = clrViddupTextPrimary;

    // link
    const SkColor clrViddupLinkButtonNormal = clrViddupTextPrimary;
    const SkColor clrViddupLinkButtonPressed = clrViddupTheme;
    const SkColor clrViddupLinkButtonDisable = clrViddupControlBorder;

    // 音量条背景色
    const SkColor clrViddupVolumeBack = SkColorSetRGB(0xeb, 0xeb, 0xeb);

    // 音量条颜色
    const SkColor clrViddupVolumeNormal = SkColorSetRGB(0xa6, 0xda, 0xf6);

    // 音量条爆音颜色
    const SkColor clrViddupVolumeLoud = SkColorSetRGB(0xff, 0xa0, 0xa0);

    // 窗口的标题字颜色
    const SkColor clrViddupWindowTitleText = SkColorSetRGB(159, 161, 178);
}

const std::map<LivehimeColorType, SkColor> g_viddup_theme_colors {
    { Theme, clrViddupTheme },
    { ThemeLight, clrViddupThemeLight },

    { WindowTitle, clrViddupWindowsTitle },
    { WindowTitleText, clrViddupWindowTitleText },
    { WindowChildTitleText, clrViddupTextTitle },
    { WindowClient, clrViddupWindowsContent },
    { WindowStatus, clrViddupWindowsContent },
    { WindowBorder, clrViddupWindowsBorder },

    { TextTitle, clrViddupTextTitle },
    { TextContent, clrViddupTextPrimary },
    { TextDescribe, clrViddupTextSecondary },
    { TextTip, clrViddupTextTip },
    { TextWarning, clrViddupTextTipWarn },
    { TextPlaceholder, clrViddupPlaceholderText },

    { SeparatorLine, clrViddupWindowsGrayBk },

    { CtrlBorderNor, clrViddupControlBorder },
    { CtrlBorderHov, clrViddupTheme },
    { CtrlBorderPre, clrViddupTheme },
    //{ CtrlBorderDis, clrViddupTheme },

    { ButtonPositive, clrViddupTheme },
    { ButtonPositiveHov, clrViddupThemeLight },
    { ButtonPositivePre, clrViddupThemeLight },
    //{ ButtonPositiveDis, clrViddupTheme },
    { ButtonNegative, clrViddupControlBorder },
    { ButtonNegativeHov, clrViddupThemeLight },
    { ButtonNegativePre, clrViddupThemeLight },
    //{ ButtonNegativeDis, clrViddupTheme },
    
    { OptionButtonNor, clrViddupTextPrimary },
    { OptionButtonHov, clrViddupTextPrimary },
    { OptionButtonPre, clrViddupTextPrimary },
    //{ OptionButtonDis, clrViddupTheme },

    { LinkButtonNor, clrViddupTextPrimary },
    { LinkButtonHov, clrViddupTheme },
    { LinkButtonPre, clrViddupTheme },
    //{ LinkButtonDis, clrViddupTheme },

    // tabbed
    { TabbedTextNor, SK_ColorWHITE},
    { TabbedTextHov, SK_ColorWHITE},
    { TabbedTextPre, SK_ColorWHITE},
    { TabbedBkNor, SkColorSetRGB(34, 35, 49) },
    { TabbedBkHov, SkColorSetRGB(34, 35, 49) },
    { TabbedBkPre, SkColorSetRGB(34, 35, 49) },
    { TabbedBkSel, clrViddupTabbedBkSelected },

    // listview/gridview
    { ListHeaderBk, clrListHeaderBk },
    { ListItemBkNor, clrListItemBk },
    { ListItemBkHov, clrListItemBkHover },
    { ListItemBkPre, clrListHeaderBk },
    
    // 进度条、slider
    { ProgressEmpty, clrViddupControlBorder },
    { ProgressFull, clrViddupTheme },

    // 音量条
    { VolumeEmpty, clrViddupVolumeBack },
    { VolumeFull, clrViddupVolumeNormal },
    { VolumeLoud, clrViddupVolumeLoud },

    { LabelTitle, clrViddupLabelTitle },

    { DropBorder, SkColorSetRGB(49, 50, 64) },
};