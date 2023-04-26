#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmakuhime_theme_common.h"

#include "ui/gfx/platform_font.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"


// Danmakuhime color themes
namespace dmkhime {

    // Black & White of Sorceress
    namespace theme_sorceress {
        const SkColor clrDanmakuUILabel = SkColorSetRGB(0xff, 0xff, 0xff);
        const SkColor clrDanmakuUITitle = SkColorSetRGB(0xeb, 0xeb, 0xeb);
        const SkColor clrDanmakuUIBk = SkColorSetRGB(0x15, 0x15, 0x15);
        const SkColor clrDanmakuUITitleBarBk = SkColorSetRGB(0x27, 0x27, 0x27);
        const SkColor clrDanmakuUIBorder = SkColorSetRGB(0x27, 0x27, 0x27);
        const SkColor clrUIGiftTabSplitter = SkColorSetARGB(38, 0xff, 0xff, 0xff);

        const SkColor clrDanmakuCommonText = SkColorSetRGB(0xff, 0xff, 0xff);
        const SkColor clrDanmakuUsernameText = SkColorSetRGB(0xad, 0xbc, 0xd9);
        const SkColor clrDanmakuSpeakText = SkColorSetARGB(0xB2, 0xff, 0xff, 0xff);  //70%

        const SkColor clrFlagNormal = SkColorSetRGB(0xef, 0x8e, 0x00);
        const SkColor clrFlagSVIP = SkColorSetRGB(0xfb, 0x72, 0x99);

        const SkColor clrDanmakuEnter = SkColorSetARGB(0x4C, 0xff, 0xff, 0xff);
        const SkColor clrDanmakuGiftStrong = SkColorSetRGB(0xff, 0xdb, 0x79);
        const SkColor clrDanmakuAnnouncement = SkColorSetRGB(0xff, 0xa5, 0xa5);
        const SkColor clrDanmakuMeleeAnnoc = SkColorSetRGB(0xd4, 0x8f, 0xff);

        const SkColor clrSChatBorder = SkColorSetARGB(0x80, 0xff, 0xff, 0xff);
        const SkColor clrSChatSplitter = SkColorSetARGB(0x4C, 0xff, 0xff, 0xff);

        const SkColor clrLiveAsistantBackground = SkColorSetRGB(0x2F, 0x1A, 0x22);
    }

    // Blue of Deep-Sea Girl
    namespace theme_deepseagirl {
        const SkColor clrDanmakuUILabel = SkColorSetRGB(0x74, 0xd7, 0xff);
        const SkColor clrDanmakuUITitle = SkColorSetRGB(0x74, 0xd7, 0xff);
        const SkColor clrDanmakuUIBk = SkColorSetRGB(0x15, 0x15, 0x15);
        const SkColor clrDanmakuUITitleBarBk = SkColorSetRGB(0x27, 0x27, 0x27);
        const SkColor clrDanmakuUIBorder = SkColorSetRGB(0x27, 0x27, 0x27);
        const SkColor clrUIGiftTabSplitter = SkColorSetARGB(38, 0xff, 0xff, 0xff);

        const SkColor clrDanmakuCommonText = SkColorSetRGB(0x0e, 0xbe, 0xff);
        const SkColor clrDanmakuUsernameText = SkColorSetRGB(0xad, 0xbc, 0xd9);
        const SkColor clrDanmakuSpeakText = SkColorSetRGB(0x0e, 0xbe, 0xff);

        const SkColor clrFlagNormal = SkColorSetRGB(0xef, 0x8e, 0x00);
        const SkColor clrFlagSVIP = SkColorSetRGB(0xfb, 0x72, 0x99);

        const SkColor clrDanmakuEnter = SkColorSetARGB(0x4C, 0xff, 0xff, 0xff);
        const SkColor clrDanmakuGiftStrong = SkColorSetRGB(0x50, 0xe3, 0xc2);
        const SkColor clrDanmakuAnnouncement = SkColorSetRGB(0xff, 0xa5, 0xe0);
        const SkColor clrDanmakuMeleeAnnoc = SkColorSetRGB(0xd4, 0x8f, 0xff);

        const SkColor clrSChatBorder = SkColorSetARGB(0x80, 0xff, 0xff, 0xff);
        const SkColor clrSChatSplitter = SkColorSetARGB(0x4C, 0xff, 0xff, 0xff);

        const SkColor clrLiveAsistantBackground = SkColorSetRGB(0xFF, 0xEC, 0xF1);
    }

    // Boundless Snow & Autumn Fruits
    namespace theme_snowfruit {
        const SkColor clrDanmakuUILabel = SkColorSetRGB(0x66, 0x66, 0x66);
        const SkColor clrDanmakuUITitle = SkColorSetRGB(0x66, 0x66, 0x66);
        const SkColor clrDanmakuUIBk = SkColorSetRGB(0xff, 0xff, 0xff);
        const SkColor clrDanmakuUITitleBarBk = SkColorSetRGB(0xf5, 0xf5, 0xf5);
        const SkColor clrDanmakuUIBorder = SkColorSetRGB(0xf5, 0xf5, 0xf5);
        const SkColor clrUIGiftTabSplitter = SkColorSetARGB(0xFF, 0xe7, 0xe7, 0xe7);

        const SkColor clrDanmakuCommonText = SkColorSetRGB(0x4a, 0x4a, 0x4a);
        const SkColor clrDanmakuUsernameText = SkColorSetRGB(0x8d, 0x9a, 0xa4);
        const SkColor clrDanmakuSpeakText = SkColorSetRGB(0x4a, 0x4a, 0x4a);

        const SkColor clrFlagNormal = SkColorSetRGB(0xef, 0x8e, 0x00);
        const SkColor clrFlagSVIP = SkColorSetRGB(0xfb, 0x72, 0x99);

        const SkColor clrDanmakuEnter = SkColorSetRGB(0x99, 0x99, 0x99);
        const SkColor clrDanmakuGiftStrong = SkColorSetRGB(0xff, 0x7d, 0x4e);
        const SkColor clrDanmakuAnnouncement = SkColorSetRGB(0xf0, 0x8d, 0xa9);
        const SkColor clrDanmakuMeleeAnnoc = SkColorSetRGB(0xd4, 0x8f, 0xff);

        const SkColor clrSChatBorder = SkColorSetARGB(0xFF, 0xe7, 0xe7, 0xe7);
        const SkColor clrSChatSplitter = SkColorSetARGB(0xFF, 0xe7, 0xe7, 0xe7);

        const SkColor clrLiveAsistantBackground = SkColorSetRGB(0xFF, 0xEC, 0xF1);
    }

    SkColor GetThemeColor(Theme theme, ColorType type) {
        switch (theme) {
        case SORCERESS:
        {
            switch (type) {
            case ColorType::UI_LABEL:
                return theme_sorceress::clrDanmakuUILabel;
            case ColorType::UI_TITLE:
                return theme_sorceress::clrDanmakuUITitle;
            case ColorType::UI_BK:
                return theme_sorceress::clrDanmakuUIBk;
            case ColorType::UI_TITLEBAR_BK:
                return theme_sorceress::clrDanmakuUITitleBarBk;
            case ColorType::UI_BORDER:
                return theme_sorceress::clrDanmakuUIBorder;
            case ColorType::UI_GIFT_TAB_SPLITTER:
                return theme_sorceress::clrUIGiftTabSplitter;
            case ColorType::COMMON_TEXT:
                return theme_sorceress::clrDanmakuCommonText;
            case ColorType::USERNAME_TEXT:
                return theme_sorceress::clrDanmakuUsernameText;
            case ColorType::SPEAK_TEXT:
                return theme_sorceress::clrDanmakuSpeakText;
            case ColorType::FLAG_SPECIAL:
                return theme_sorceress::clrFlagNormal;
            case ColorType::FLAG_NORMAL:
                return theme_sorceress::clrFlagSVIP;
            case ColorType::ENTER:
                return theme_sorceress::clrDanmakuEnter;
            case ColorType::STRONG_GIFT:
                return theme_sorceress::clrDanmakuGiftStrong;
            case ColorType::ANNOUNCEMENT:
                return theme_sorceress::clrDanmakuAnnouncement;
            case ColorType::MELEE_ANNOC:
                return theme_sorceress::clrDanmakuMeleeAnnoc;
            case ColorType::SCHAT_BORDER:
                return theme_sorceress::clrSChatBorder;
            case ColorType::SCHAT_SPLITTER:
                return theme_sorceress::clrSChatSplitter;
            case ColorType::LiveAsistantBackground:
                return theme_sorceress::clrLiveAsistantBackground;
            default:
                NOTREACHED();
                break;
            }
            break;
        }
        case DEEP_SEA_GIRL:
        {
            switch (type) {
            case ColorType::UI_LABEL:
                return theme_deepseagirl::clrDanmakuUILabel;
            case ColorType::UI_TITLE:
                return theme_deepseagirl::clrDanmakuUITitle;
            case ColorType::UI_BK:
                return theme_deepseagirl::clrDanmakuUIBk;
            case ColorType::UI_TITLEBAR_BK:
                return theme_deepseagirl::clrDanmakuUITitleBarBk;
            case ColorType::UI_BORDER:
                return theme_deepseagirl::clrDanmakuUIBorder;
            case ColorType::UI_GIFT_TAB_SPLITTER:
                return theme_deepseagirl::clrUIGiftTabSplitter;
            case ColorType::COMMON_TEXT:
                return theme_deepseagirl::clrDanmakuCommonText;
            case ColorType::USERNAME_TEXT:
                return theme_deepseagirl::clrDanmakuUsernameText;
            case ColorType::SPEAK_TEXT:
                return theme_deepseagirl::clrDanmakuSpeakText;
            case ColorType::FLAG_SPECIAL:
                return theme_deepseagirl::clrFlagNormal;
            case ColorType::FLAG_NORMAL:
                return theme_deepseagirl::clrFlagSVIP;
            case ColorType::ENTER:
                return theme_deepseagirl::clrDanmakuEnter;
            case ColorType::STRONG_GIFT:
                return theme_deepseagirl::clrDanmakuGiftStrong;
            case ColorType::ANNOUNCEMENT:
                return theme_deepseagirl::clrDanmakuAnnouncement;
            case ColorType::MELEE_ANNOC:
                return theme_deepseagirl::clrDanmakuMeleeAnnoc;
            case ColorType::SCHAT_BORDER:
                return theme_deepseagirl::clrSChatBorder;
            case ColorType::SCHAT_SPLITTER:
                return theme_deepseagirl::clrSChatSplitter;
			case ColorType::LiveAsistantBackground:
				return theme_deepseagirl::clrLiveAsistantBackground;
            default:
                NOTREACHED();
                break;
            }
            break;
        }
        case SNOW_FRUIT:
        {
            switch (type) {
            case ColorType::UI_LABEL:
                return theme_snowfruit::clrDanmakuUILabel;
            case ColorType::UI_TITLE:
                return theme_snowfruit::clrDanmakuUITitle;
            case ColorType::UI_BK:
                return theme_snowfruit::clrDanmakuUIBk;
            case ColorType::UI_TITLEBAR_BK:
                return theme_snowfruit::clrDanmakuUITitleBarBk;
            case ColorType::UI_BORDER:
                return theme_snowfruit::clrDanmakuUIBorder;
            case ColorType::UI_GIFT_TAB_SPLITTER:
                return theme_snowfruit::clrUIGiftTabSplitter;
            case ColorType::COMMON_TEXT:
                return theme_snowfruit::clrDanmakuCommonText;
            case ColorType::USERNAME_TEXT:
                return theme_snowfruit::clrDanmakuUsernameText;
            case ColorType::SPEAK_TEXT:
                return theme_snowfruit::clrDanmakuSpeakText;
            case ColorType::FLAG_SPECIAL:
                return theme_snowfruit::clrFlagNormal;
            case ColorType::FLAG_NORMAL:
                return theme_snowfruit::clrFlagSVIP;
            case ColorType::ENTER:
                return theme_snowfruit::clrDanmakuEnter;
            case ColorType::STRONG_GIFT:
                return theme_snowfruit::clrDanmakuGiftStrong;
            case ColorType::ANNOUNCEMENT:
                return theme_snowfruit::clrDanmakuAnnouncement;
            case ColorType::MELEE_ANNOC:
                return theme_snowfruit::clrDanmakuMeleeAnnoc;
            case ColorType::SCHAT_BORDER:
                return theme_snowfruit::clrSChatBorder;
            case ColorType::SCHAT_SPLITTER:
                return theme_snowfruit::clrSChatSplitter;
			case ColorType::LiveAsistantBackground:
				return theme_snowfruit::clrLiveAsistantBackground;
            default:
                NOTREACHED();
                break;
            }
            break;
        }
        }

        NOTREACHED();
        return 0;
    }


    // Danmaku margins
    const int kDanmakuBgPaddingX = 8;
    const int kDanmakuBgPaddingY = 8;
    const int kDanmakuBgSmallPaddingY = 4;
    const int kDanmakuMarginX = 16;
    const int kDanmakuMarginY = 10;
    const int kCommonNoticeDanmakuBtnMargin = 10;
    const int kCommonNoticeDanmakuBackgroundRadius = 4;
    const int kliveAsistantBackgroundRadius = GetLengthByDPIScale(5);
    const int kUserFlagPaddingHori = GetLengthByDPIScale(6);
    const int kUserFlagMarginHori = GetLengthByDPIScale(6);
    const int kUserMedalPaddingHori = GetLengthByDPIScale(4);
    const int kDanmakuCommonNoticeBgPaddingX = GetLengthByDPIScale(25);

    // Danmakuhime measurements
    const int kGiftTitleHeight = GetLengthByDPIScale(34);
    const int kMinGiftViewHeight = GetLengthByDPIScale(120);
    const int kMinDanmakuViewHeight = GetLengthByDPIScale(120);
    const int kResizeDetectHeight = GetLengthByDPIScale(6);
    const int kSpliterHeight = GetLengthByDPIScale(14);
    const int kBorderSize = GetLengthByDPIScale(1);
    const int kSplitWindowWidth = GetLengthByDPIScale(393);
    const int kMinSplitWindowHeight = kMinGiftViewHeight + kMinDanmakuViewHeight + kSpliterHeight;
    const int kDanmakuMainViewWidth = GetLengthByDPIScale(270);
    const int kDanmakuViewPopDefWidth = GetLengthByDPIScale(270);
    const int kDanmakuGiftViewPopDefWidth = GetLengthByDPIScale(300);
    const int kDanmakuGiftViewPopDefHeight = GetLengthByDPIScale(200);
    const int kDanmakuInteractionViewPopDefHeight = GetLengthByDPIScale(366);
	const int kDanmakuGiftViewPopMinHeight = GetLengthByDPIScale(170);
	const int kDanmakuInteractionViewPopMinHeight = GetLengthByDPIScale(260);
    //活动and任务区
	const int kActivityAndTaskViewDefaultWidth = GetLengthByDPIScale(280);
    const int kActivityAndTaskViewTitleHeight = GetLengthByDPIScale(34);
    const int kAnchorTaskBrowserViewHeight = GetLengthByDPIScale(48);
	const int kPenantTaskBrowserViewHeight = GetLengthByDPIScale(35);
    const int kActivityTaskViewPopMinHeight = kActivityAndTaskViewTitleHeight + kAnchorTaskBrowserViewHeight + kPenantTaskBrowserViewHeight;// GetLengthByDPIScale(110);
    const int kActivityTaskViewPopPopDefHeight = kActivityAndTaskViewTitleHeight + kAnchorTaskBrowserViewHeight + kPenantTaskBrowserViewHeight;//GetLengthByDPIScale(110);
    // Danmaku fonts
    const int kMinFontSize = 10;
    const int kMaxFontSize = 20;
}