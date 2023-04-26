#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_THEMES_V3_SPECIAL_CONSTANTS_H
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_THEMES_V3_SPECIAL_CONSTANTS_H

#include "SkColor.h"

extern const unsigned kHoverMaskAlpha;
extern const unsigned kInactiveAlpha;

// 3.0版本使用的特定的颜色
const SkColor clrMainViewTitleGradientBegin = SkColorSetRGB(0x00, 0x00, 0x00);
const SkColor clrMainViewTitleGradientEnd = SkColorSetRGB(0x00, 0x00, 0x00);
const SkColor clrMainViewTitleGuideBk = SkColorSetRGB(0xf2, 0xf7, 0xfa);
const SkColor clrMainViewTitleTextNormal = SK_ColorWHITE;
const SkColor clrMainViewTitleTextHover = SK_ColorWHITE;
const SkColor clrMainViewTitleCtrlBorderNormal = SkColorSetA(SK_ColorWHITE, kHoverMaskAlpha);
const SkColor clrMainViewTitleCtrlBorderHover = SkColorSetA(SK_ColorWHITE, kInactiveAlpha);

const SkColor clrListHeaderBk = SkColorSetRGB(0xed, 0xf1, 0xf5);
const SkColor clrListItemBk = SK_ColorWHITE;
const SkColor clrListItemBkHover = SkColorSetRGB(0xf7, 0xf9, 0xfa);

const SkColor clrRecordButtonText = SkColorSetRGB(0x53, 0x67, 0x77);
const SkColor clrRecordButtonBk = SkColorSetRGB(0xe9, 0xeb, 0xed);

const SkColor clrIntlButtonText = SK_ColorWHITE;
const SkColor clrIntlGrayButtonBk = SkColorSetRGB(0x5c, 0x5e, 0x70);
const SkColor clrIntlBlueButtonBk = SkColorSetRGB(0x4c, 0x93, 0xFF);

const SkColor clrVolumeDisable = SkColorSetRGB(0xe7, 0xe7, 0xe7);
const SkColor clrVolumeNormalGradientBegin = SkColorSetRGB(0xb3, 0xb3, 0xb3);
const SkColor clrVolumeNormalGradientEnd = SkColorSetRGB(0xdd, 0xdd, 0xdd);
const SkColor clrVolumeHoverGradientBegin = SkColorSetRGB(0x0e, 0xbe, 0xff);
const SkColor clrVolumeHoverGradientEnd = SkColorSetRGB(0xdd, 0xe4, 0xeb);

//直播工具互动玩法/基础功能
const SkColor toolGradientBegin = SkColorSetARGB(0.5f * 255, 0xEA, 0xEA, 0xEA);
const SkColor toolGradientEnd = SkColorSetARGB(0.05f * 255, 0xEA, 0xEA, 0xEA);

#endif