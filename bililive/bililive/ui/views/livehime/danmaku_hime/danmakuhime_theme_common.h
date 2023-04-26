#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_DANMAKUHIME_THEME_COMMON_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_DANMAKUHIME_THEME_COMMON_H_

#include "ui/gfx/canvas.h"


// Danmakuhime color themes
namespace dmkhime {

    enum Theme {
        SORCERESS = 0,
        DEEP_SEA_GIRL,
        SNOW_FRUIT
    };

    enum class ColorType {
        UI_LABEL,
        UI_TITLE,
        UI_BK,
        UI_TITLEBAR_BK,
        UI_BORDER,
        UI_GIFT_TAB_SPLITTER,

        COMMON_TEXT,
        USERNAME_TEXT,
        SPEAK_TEXT,

        FLAG_SPECIAL,
        FLAG_NORMAL,

        ENTER,
        STRONG_GIFT,
        ANNOUNCEMENT,
        MELEE_ANNOC,

        SCHAT_BORDER,
        SCHAT_SPLITTER,

        LiveAsistantBackground
    };

    SkColor GetThemeColor(Theme theme, ColorType type);


    // Danmaku margins
    extern const int kDanmakuBgPaddingX;
    extern const int kDanmakuBgPaddingY;
    extern const int kDanmakuBgSmallPaddingY;
    extern const int kDanmakuMarginX;
    extern const int kDanmakuMarginY;
    extern const int kCommonNoticeDanmakuBtnMargin;
    extern const int kCommonNoticeDanmakuBackgroundRadius;
    extern const int kliveAsistantBackgroundRadius;
    extern const int kUserFlagPaddingHori;
    extern const int kUserFlagMarginHori;
    extern const int kUserMedalPaddingHori;
    extern const int kDanmakuCommonNoticeBgPaddingX;


    // Danmakuhime measurements
    extern const int kGiftTitleHeight;
    extern const int kMinGiftViewHeight;
    extern const int kMinDanmakuViewHeight;
    extern const int kResizeDetectHeight;
    extern const int kSpliterHeight;
    extern const int kBorderSize;
    extern const int kMinSplitWindowHeight;
    extern const int kDanmakuMainViewWidth;
    extern const int kDanmakuViewPopDefWidth;
    extern const int kDanmakuGiftViewPopDefWidth;
    extern const int kDanmakuGiftViewPopDefHeight;
    extern const int kDanmakuInteractionViewPopDefHeight;
    extern const int kDanmakuGiftViewPopMinHeight;
    extern const int kDanmakuInteractionViewPopMinHeight;
    extern const int kActivityAndTaskViewDefaultWidth;
    extern const int kActivityAndTaskViewTitleHeight;
    extern const int kPenantTaskBrowserViewHeight;
    extern const int kAnchorTaskBrowserViewHeight;
    extern const int kActivityTaskViewPopMinHeight;
    extern const int kActivityTaskViewPopPopDefHeight;
    // Danmaku fonts
    extern const int kMinFontSize;
    extern const int kMaxFontSize;


}

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_DANMAKUHIME_THEME_COMMON_H_