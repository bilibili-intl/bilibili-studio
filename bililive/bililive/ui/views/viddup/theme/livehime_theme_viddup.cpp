#include "bililive/bililive/ui/views/livehime/theme/livehime_theme_constants.h"


namespace
{
    // ֱ����ȫ������ɫ->[
    const U8CPU viddup_disable_alpha = 0.3f * 255;

    // ����ֱ������ɫ
    const SkColor clrViddupTheme = SkColorSetRGB(0x0c, 0x0d, 0x13);
    
    // ����ɫͬɫ����hoverɫ
    const SkColor clrViddupThemeLight = SkColorSetRGB(0x32, 0x32, 0x32);
    
    // ���߿�Ŀؼ��ı߿���ͨɫ
    const SkColor clrViddupControlBorder = SkColorSetRGB(0x00, 0x00, 0x00);
    
    // ���ڱ߿�ɫ
    const SkColor clrViddupWindowsBorder = SkColorSetRGB(0x00, 0x00, 0x00);
    
    // ������߿�ɫ
    const SkColor clrViddupDroplistBorder = SkColorSetRGB(0xeb, 0xeb, 0xeb);
    
    // ���ڱ������Ϳͻ�����ɫ
    const SkColor clrViddupWindowsGrayBk = SkColorSetRGB(0x22, 0x23, 0x31);
    const SkColor clrViddupWindowsTitle = SkColorSetRGB(0x22, 0x23, 0x31);
    const SkColor clrViddupWindowsContent = SkColorSetRGB(0x15, 0x16, 0x20);
    
    // ��ɫ
    const SkColor clrViddupTextPrimary = SkColorSetRGB(0x99, 0x99, 0x99);// rgba(153,153,153,1)
    const SkColor clrViddupTextSecondary = SkColorSetRGB(0xa1, 0xb2, 0xbf);// 161,178,191
    const SkColor clrViddupTextTitle = SkColorSetRGB(0xFF, 0xFF, 0xFF);// rgba(255,255,255,1)
    const SkColor clrViddupTextTip = SkColorSetRGB(0x2c, 0x44, 0x57);
    const SkColor clrViddupTextTipWarn = SkColorSetRGB(0xfb, 0x72, 0x99); // ����վ�ۺ�ɫ
    const SkColor clrViddupTextCover = SkColorSetRGB(0x2C, 0x44, 0x57);
    
    //V4.0 �汾����Ҫ��ɫ�Լ�͸����
    const SkColor clrViddupTextALL = SkColorSetRGB(0x53, 0x67, 0x77);  //�����ӵ���Ҫ������ɫ
    const SkColor clrViddupTextALLHov = SkColorSetRGB(0x0E, 0xBE, 0xFF);  //�����ӵ���Ҫ������ɫ
    const SkColor clrViddupTextBG = SkColorSetRGB(0xF4, 0xF4, 0xF4);  //������ť������ɫ
    const SkColor clrViddupTextBorder = SkColorSetRGB(0xF9, 0xF9, 0xF9);  //�߿���ɫ
    
    //ֱ����ť����ʽ��ɫ
    const SkColor clrViddupNewBtnHov = SkColorSetRGB(0xfb, 0x72, 0x99);
    const SkColor clrViddupNewBtnNor = SkColorSetRGB(0xFF, 0x66, 0x66);
    const unsigned kMaskViddupAlphaThree = 0.3f * 256;
    const unsigned kMaskViddupAlphaSix = 0.6f * 256;
    const unsigned kMaskViddupAlphaFour = 0.4f * 256;
    //ֱ�����ڿ�ʼֱ����ť

    // ��ǩҳѡ�е�ɫ
    const SkColor clrViddupTabbedBkSelected = SkColorSetRGB(74, 75, 97);
    // placehoder_text_color
    const SkColor clrViddupPlaceholderText = clrViddupControlBorder;
    // ]<-ֱ����ȫ������ɫ

    // ���߿�Ŀؼ��ı߿���̬ɫ
    const SkColor clrViddupCtrlBorderNor = clrViddupControlBorder;
    const SkColor clrViddupCtrlBorderHov = clrViddupTheme;
    const SkColor clrViddupCtrlBorderPre = clrViddupTheme;

    // ���������İ�ť��̬ɫ
    const SkColor clrViddupButtonPositive = clrViddupTheme;
    const SkColor clrViddupButtonPositiveHov = clrViddupThemeLight;
    const SkColor clrViddupButtonPositivePre = clrViddupThemeLight;

    // label��ɫ
    const SkColor clrViddupLabelTitle = clrViddupTextTitle;
    const SkColor clrViddupLabelContent = clrViddupTextPrimary;
    const SkColor clrViddupLabelDescribe = clrViddupTextSecondary;
    const SkColor clrViddupLabelTip = clrViddupTextTip;

    // tabbed��̬��ɫ
    const SkColor clrViddupTabbedTextNor = clrViddupTextPrimary;
    const SkColor clrViddupTabbedTextHov = clrViddupTextPrimary;
    const SkColor clrViddupTabbedTextPre = clrViddupTheme;

    // tabbed��̬��ɫ
    const SkColor clrViddupTabbedBkNor = clrViddupWindowsContent;
    const SkColor clrViddupTabbedBkHov = clrViddupWindowsGrayBk;
    const SkColor clrViddupTabbedBkPre = clrViddupTabbedBkSelected;

    // ��������slider
    const SkColor clrViddupProgressFull = clrViddupTheme;
    const SkColor clrViddupProgressEmpty = clrViddupControlBorder;

    // checkbox��radio
    const SkColor clrViddupOptionButtonNor = clrViddupTextPrimary;
    const SkColor clrViddupOptionButtonHov = clrViddupTextPrimary;
    const SkColor clrViddupOptionButtonPre = clrViddupTextPrimary;

    // link
    const SkColor clrViddupLinkButtonNormal = clrViddupTextPrimary;
    const SkColor clrViddupLinkButtonPressed = clrViddupTheme;
    const SkColor clrViddupLinkButtonDisable = clrViddupControlBorder;

    // ����������ɫ
    const SkColor clrViddupVolumeBack = SkColorSetRGB(0xeb, 0xeb, 0xeb);

    // ��������ɫ
    const SkColor clrViddupVolumeNormal = SkColorSetRGB(0xa6, 0xda, 0xf6);

    // ������������ɫ
    const SkColor clrViddupVolumeLoud = SkColorSetRGB(0xff, 0xa0, 0xa0);

    // ���ڵı�������ɫ
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
    
    // ��������slider
    { ProgressEmpty, clrViddupControlBorder },
    { ProgressFull, clrViddupTheme },

    // ������
    { VolumeEmpty, clrViddupVolumeBack },
    { VolumeFull, clrViddupVolumeNormal },
    { VolumeLoud, clrViddupVolumeLoud },

    { LabelTitle, clrViddupLabelTitle },

    { DropBorder, SkColorSetRGB(49, 50, 64) },
};