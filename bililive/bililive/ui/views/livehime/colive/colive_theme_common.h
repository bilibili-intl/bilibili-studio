#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_COLIVE_COLIVE_THEME_COMMON_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_COLIVE_COLIVE_THEME_COMMON_H_

#include "ui/gfx/canvas.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"


// ���󴰿�������ɫ

// ���ڱ�����
const SkColor clrColiveTitleBar = SkColorSetRGB(0x2c, 0x30, 0x33);
// �����²�������
const SkColor clrColiveToolBar = SkColorSetRGB(0x3a, 0x3f, 0x41);
// OBS��������
const SkColor clrColiveOBSBackground = SkColorSetRGB(24, 30, 32);
// �ü�������ɫ
const SkColor clrColiveCropShade = SkColorSetARGB(0x80, 0x0b, 0x0b, 0x0b);
// ȡ��ƥ�䰴ť�߿���ɫ
const SkColor clrColiveCMBBorder = SkColorSetRGB(0x3a, 0x3f, 0x41);

// PK�б������ɫ
const SkColor clrPKListBackground = SkColorSetRGB(0xF7, 0xF9, 0xFA);
const SkColor clrPKListSelected = SkColorSetRGB(0xDA, 0xE6, 0xEC);

// ��ɫ
const SkColor clrColiveTextTip = SkColorSetRGB(0xa1, 0xb2, 0xbf);
const SkColor clrColiveTextPrimary = SkColorSetRGB(0x8d, 0x9a, 0xa4);
const SkColor clrColiveTextSecondary = SkColorSetRGB(0x53, 0x67, 0x77);
const SkColor clrColiveTextWhite = SK_ColorWHITE;
const SkColor clrColiveTextBlue = SkColorSetRGB(21, 191, 254);

const SkColor clrColiveTextScore = SK_ColorWHITE;
const SkColor clrColiveTextTopic = SK_ColorWHITE;
const SkColor clrColiveTextCountdown = SK_ColorYELLOW;

// ʤ������
const SkColor kClrAssistorBk = SkColorSetRGB(255, 248, 237);
const SkColor kClrAssistor = SkColorSetRGB(171, 106, 50);
const SkColor kClrCrownHalo = SkColorSetRGB(255, 177, 3);
const SkColor kClrVictoryBackground = SK_ColorWHITE;

// PK�и��ֿؼ�
const SkColor kClrBarPlaceholder = SkColorSetRGB(0x23, 0x23, 0x23);
const SkColor kClrBarBackground = SkColorSetARGB(0xcc, 0x17, 0x17, 0x17);
const SkColor kClrNicknameBackground = SkColorSetARGB(0x7f, 0, 0, 0);
const SkColor kClrNicknameColor = SkColorSetRGB(0xff, 0xff, 0xff);

extern const int kVictoryBorderInset;
extern const int kVictoryHeadshotSize;

extern const int kPkRecordViewWidth;
extern const int kPkRecordItemHeight;
extern const int kPkInviteListViewWidth;
extern const int kPkInviteListItemHeight;
extern const int kColiveFrameWidth;
extern const int kColiveFrameHeight;
extern const int kVictorViewWidth;
extern const int kVictorViewHeight;

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_COLIVE_COLIVE_THEME_COMMON_H_