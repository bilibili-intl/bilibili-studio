#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_CONTROLS_LIVEHIME_THEME_COMMON_H
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_CONTROLS_LIVEHIME_THEME_COMMON_H

#include "base/memory/ref_counted_memory.h"

#include "ui/base/win/dpi.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/platform_font.h"
#include "ui/gfx/font.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"

#include "bililive/bililive/ui/views/livehime/theme/livehime_theme_constants.h"


const int LivehimePaddingCharWidth(const gfx::Font &font);

// 可支持任意 DPI 的缩放方法。
gfx::Font GetPointFontByDPIScale(int point_size);
gfx::Font GetPointFont(int point_size);
int GetLengthByDPIScale(int len);
float GetLengthByDPIScaleF(int len);
gfx::Size GetSizeByDPIScale(const gfx::Size &size);
gfx::Rect GetRectByDPIScale(const gfx::Rect &rect);

// 与固定资源图合用的缩放方法。注意：只能配合框架来用
// 目前的资源图有 100%、150% 和 200% 缩放。
int GetLengthByImgDPIScale(int len);
gfx::Size GetSizeByImgDPIScale(const gfx::Size &size);
gfx::Rect GetRectByImgDPIScale(const gfx::Rect &rect);
gfx::Insets GetInsetsByImgDPIScale(const gfx::Insets &insets);

// 当想要对图片的长度进行 DPI 缩放时使用该方法。
int GetImageLengthByDPIScale(int len);

SkColor GetUserLevelColor(int level);
base::string16 GetLocalizedString(int resource_id);
gfx::ImageSkia* GetImageSkiaNamed(int resource_id);
base::RefCountedStaticMemory* LoadDataResourceBytes(int resource_id);
SkColor GetColor(LivehimeColorType id);

extern const int kHitTestThickness;             // 主界面边框检测
extern const int kBorderCtrlHeight;             // 编辑框、下拉框这种带边框的控件高度
extern const int kMessageBoxWidth;              // 信息弹出框的宽度
extern const int kMainWndTabAreaWidth;          // 主界面右侧面板宽度
extern const int kMainWndTabAreaStripHeight;    // 主界面右侧面板标签头高度
extern const gfx::Size kLeftStripPosStripSize;  // 靠左排列的tab标签的项尺寸
extern const int kMsgboxTitleBarHeight;         // 普通弹框标题栏高度
extern const int kDataTitleHeight;              // 列表标题栏高度
extern const int kDataHeaderHeight;               // 列表头高度
extern const int kDataItemHeight;               // 列表项高度
extern const int kGiftboxItemWidth;               // 礼物盒子礼物项宽度
extern const int kGiftboxItemHeight;               // 礼物盒子礼物项高度
extern const int kGiftboxIconWidth;               // 礼物项礼物图标宽度
extern const int kLuckGiftRecordWidth;
extern const int kLuckGiftRecordHeight;

extern const int kPaddingRowHeightEnds;           // 界面顶部和底部距离边界的高度
extern const int kPaddingRowHeightForGroupCtrls;  // 同一组控件之间的行间距
extern const int kPaddingRowHeightForCtrlTips;    // 某个控件与其tip控件的行间距
extern const int kPaddingRowHeightForGroupTips;   // 某组控件与其tip控件的行间距
extern const int kPaddingRowHeightForDiffGroups;  // 同界面不同分组的控件间行高，例如上组最后一个和下组标题间的行高
extern const int kPaddingColWidthEnds;            // 列集合两端的留白
extern const int kPaddingColWidthForGroupCtrls;   // 同一列同组控件的列间距
extern const int kPaddingColWidthForCtrlTips;     // 某个控件与其tip控件的列间距
extern const int kPaddingColWidthForActionButton; // 确定、取消这类的动作按钮的间距
extern const int kPaddingColWidthEndsSmall;            // 列集合两端的留白
extern const int kUrlEditMinHeight;
extern const int kPaddingColWidthEndsInviting;

extern const int kCtrlLeftInset;                // 控件内部内容的左右缩进
extern const int kCtrlRightInset;               // 控件内部内容的左右缩进

extern const int kPermanentAppCount;            // 常驻应用数

// 主界面录制/直播按钮尺寸
extern const int kRecordButtonWidth;
extern const int kRecordButtonHeight;
extern const int kLiveButtonWidth;
extern const int kLiveButtonHeight;

extern const gfx::Font ftSix;
extern const gfx::Font ftEight;
extern const gfx::Font ftEightBold;
extern const gfx::Font ftNine;
extern const gfx::Font ftTen;
extern const gfx::Font ftTenBold;
extern const gfx::Font ftEleven;
extern const gfx::Font ftElevenBold;
extern const gfx::Font ftTwelve;
extern const gfx::Font ftTwelveBold;
extern const gfx::Font ftThirteen;
extern const gfx::Font ftThirteenBold;
extern const gfx::Font ftFourteen;
extern const gfx::Font ftFourteenBold;
extern const gfx::Font ftFifteen;
extern const gfx::Font ftFifteenBold;
extern const gfx::Font ftSixteen;
extern const gfx::Font ftSixteenBold;
extern const gfx::Font ftSeventeen;
extern const gfx::Font ftEighteen;
extern const gfx::Font ftEighteenBold;
extern const gfx::Font ftNineteen;
extern const gfx::Font ftTwenty;
extern const gfx::Font ftTwentyBold;
extern const gfx::Font ftTwentyTwo;
extern const gfx::Font ftTwentyTwoBold;
extern const gfx::Font ftPrimary;  // 主要字体，标题、文字描述、button字体
extern const gfx::Font ftPrimaryBold;
extern const gfx::Font ftTwentyEight;
extern const gfx::Font ftTwentyEightBold;
extern const gfx::Font ftThirty;
extern const gfx::Font ftThirtyBold;
extern const gfx::Font ftThirtyTwo;
extern const gfx::Font ftThirtySix;
extern const gfx::Font ftThirtySixBold;
extern const gfx::Font ftTwentyFour;
extern const gfx::Font ftTwentyFourBold;

const unsigned kAlphaOpaqueOneTenths = 0.1f * 256;
const unsigned kAlphaOpaqueTwoTenths = 0.2f * 256;
const unsigned kAlphaOpaqueThreeTenths = 0.3f * 256;
const unsigned kAlphaOpaqueFourTenths = 0.4f * 256;
const unsigned kAlphaOpaqueFiveTenths = 0.5f * 256;
const unsigned kAlphaOpaqueSixTenths = 0.6f * 256;
const unsigned kAlphaOpaqueSevenTenths = 0.7f * 256;
const unsigned kAlphaOpaqueEightTenths = 0.8f * 256;
const unsigned kAlphaOpaqueNineTenths = 0.9f * 256;
const unsigned kAlphaOpaque = 255;

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_CONTROLS_LIVEHIME_THEME_COMMON_H
