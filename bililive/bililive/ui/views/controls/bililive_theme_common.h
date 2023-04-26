#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_THEME_COMMON_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_THEME_COMMON_H


#include "ui/gfx/canvas.h"
#include "ui/gfx/platform_font.h"
#include "ui/gfx/font.h"

#include "base/strings/string16.h"

#include "bililive/public/common/refcounted_dictionary.h"

enum class UIViewsStyle
{
	Style_4_10,//4.10版本后加入的新的样式
	Style_Model_MsgBox,
};


const SkColor clrGrayBorder = SkColorSetRGB(221, 223, 228);
const SkColor clrFrameTitle = SkColorSetRGB(255, 255, 255);
const SkColor clrFrameAndButtonBk = SkColorSetRGB(35, 174, 230);
const SkColor clrFrameBorder = SkColorSetRGB(79, 193, 233);
const SkColor clrClientAreaBk = SkColorSetRGB(255, 255, 255);
const SkColor clrBlurHideWidgetFrame = SkColorSetRGB(186, 186, 186);

const SkColor clrButtonHoverBk = SkColorSetRGB(91, 204, 249);
const SkColor clrButtonPressBk = SkColorSetRGB(32, 155, 205);

const SkColor clrComboboxBorder = SkColorSetRGB(238, 238, 238);
const SkColor clrLabelText = SkColorSetRGB(121, 140, 167);
const SkColor clrLabelDisableText = SkColorSetRGB(203, 203, 203);
const SkColor clrLabelTipText = SkColorSetRGB(175, 186, 202);
const SkColor clrLabelOperateText = SkColorSetRGB(100, 162, 197);
const SkColor clrLabelNormalText = SkColorSetRGB(83, 103, 119);

const SkColor clrTextfieldBorder = SkColorSetRGB(238, 238, 238);
const SkColor clrTextfieldFocusBorder = SkColorSetRGB(91, 204, 249);
const SkColor clrTextfieldText = SkColorSetRGB(0x79, 0x8C, 0xA7);

const SkColor clrHintBoxBorder = SkColorSetRGB(0x8f, 0xc5, 0xe3);
const SkColor clrHintBoxBk = SkColorSetRGB(0xea, 0xf8, 0xff);

const SkColor clrSliderEmptyArea = SkColorSetRGB(221, 221, 221);
const SkColor clrSliderFullArea = clrFrameBorder;

const SkColor clrNormalPink = SkColorSetRGB(242, 127, 158);
const SkColor clrHoverPink = SkColorSetRGB(245, 165, 187);
const SkColor clrPressPink = SkColorSetRGB(240, 111, 147);

extern const gfx::Font ftDefaultBold;
extern const gfx::Font ftLabel;
extern const gfx::Font ftNormal;
extern const gfx::Font ftNormalBold;
extern const gfx::Font ftNormal1;
extern const gfx::Font ftNormalBold1;
extern const gfx::Font ftNormal2;
extern const gfx::Font ftNormalBold2;
extern const gfx::Font ftNormal4;
extern const gfx::Font ftNormalBold4;

#endif