#pragma once

#include "skia/include/core/SkColor.h"


enum class DanmakuThemeType { kDark = 0, kWhite = 1, kViddup = 2 };
enum class DanmakuShowType { kMerged, kSplited };
enum class DanmakuButtonType { kColse, kSetting, kLock, kPin, kPop, kHighEnergy, kGiftBill, kSpeaker ,kFunction, kBlockEffect,kGiftPanel};
enum class DanmakuThemeViewType { kText, kTextHighLight, kBackground, kBackgroundHighLight, kGiftFilterBtnFocus, kSendBtn };

class DanmakuThemeInterface
{
public:
    DanmakuThemeInterface();
    virtual ~DanmakuThemeInterface();

    void SwitchTheme(DanmakuThemeType theme);
    DanmakuThemeType GetTheme();

protected:
    virtual void SwitchThemeImpl() = 0;

private:
    DanmakuThemeType    theme_ = DanmakuThemeType::kViddup;
};

class DanmakuShowTypeInterface
{
public:
    DanmakuShowTypeInterface();
    virtual ~DanmakuShowTypeInterface();

    void SwitchShowType(DanmakuShowType show_type);
    DanmakuShowType GetShowType();

protected:
    virtual void SwitchShowTypeImpl() = 0;

private:
    DanmakuShowType     show_type_ = DanmakuShowType::kMerged;
};

SkColor GetDanmakuThemeColor(DanmakuThemeType theme, DanmakuThemeViewType type);
