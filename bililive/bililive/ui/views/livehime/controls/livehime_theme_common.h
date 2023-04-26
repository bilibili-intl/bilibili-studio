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

// ��֧������ DPI �����ŷ�����
gfx::Font GetPointFontByDPIScale(int point_size);
gfx::Font GetPointFont(int point_size);
int GetLengthByDPIScale(int len);
float GetLengthByDPIScaleF(int len);
gfx::Size GetSizeByDPIScale(const gfx::Size &size);
gfx::Rect GetRectByDPIScale(const gfx::Rect &rect);

// ��̶���Դͼ���õ����ŷ�����ע�⣺ֻ����Ͽ������
// Ŀǰ����Դͼ�� 100%��150% �� 200% ���š�
int GetLengthByImgDPIScale(int len);
gfx::Size GetSizeByImgDPIScale(const gfx::Size &size);
gfx::Rect GetRectByImgDPIScale(const gfx::Rect &rect);
gfx::Insets GetInsetsByImgDPIScale(const gfx::Insets &insets);

// ����Ҫ��ͼƬ�ĳ��Ƚ��� DPI ����ʱʹ�ø÷�����
int GetImageLengthByDPIScale(int len);

SkColor GetUserLevelColor(int level);
base::string16 GetLocalizedString(int resource_id);
gfx::ImageSkia* GetImageSkiaNamed(int resource_id);
base::RefCountedStaticMemory* LoadDataResourceBytes(int resource_id);
SkColor GetColor(LivehimeColorType id);

extern const int kHitTestThickness;             // ������߿���
extern const int kBorderCtrlHeight;             // �༭�����������ִ��߿�Ŀؼ��߶�
extern const int kMessageBoxWidth;              // ��Ϣ������Ŀ��
extern const int kMainWndTabAreaWidth;          // �������Ҳ������
extern const int kMainWndTabAreaStripHeight;    // �������Ҳ�����ǩͷ�߶�
extern const gfx::Size kLeftStripPosStripSize;  // �������е�tab��ǩ����ߴ�
extern const int kMsgboxTitleBarHeight;         // ��ͨ����������߶�
extern const int kDataTitleHeight;              // �б�������߶�
extern const int kDataHeaderHeight;               // �б�ͷ�߶�
extern const int kDataItemHeight;               // �б���߶�
extern const int kGiftboxItemWidth;               // ���������������
extern const int kGiftboxItemHeight;               // �������������߶�
extern const int kGiftboxIconWidth;               // ����������ͼ����
extern const int kLuckGiftRecordWidth;
extern const int kLuckGiftRecordHeight;

extern const int kPaddingRowHeightEnds;           // ���涥���͵ײ�����߽�ĸ߶�
extern const int kPaddingRowHeightForGroupCtrls;  // ͬһ��ؼ�֮����м��
extern const int kPaddingRowHeightForCtrlTips;    // ĳ���ؼ�����tip�ؼ����м��
extern const int kPaddingRowHeightForGroupTips;   // ĳ��ؼ�����tip�ؼ����м��
extern const int kPaddingRowHeightForDiffGroups;  // ͬ���治ͬ����Ŀؼ����иߣ������������һ��������������и�
extern const int kPaddingColWidthEnds;            // �м������˵�����
extern const int kPaddingColWidthForGroupCtrls;   // ͬһ��ͬ��ؼ����м��
extern const int kPaddingColWidthForCtrlTips;     // ĳ���ؼ�����tip�ؼ����м��
extern const int kPaddingColWidthForActionButton; // ȷ����ȡ������Ķ�����ť�ļ��
extern const int kPaddingColWidthEndsSmall;            // �м������˵�����
extern const int kUrlEditMinHeight;
extern const int kPaddingColWidthEndsInviting;

extern const int kCtrlLeftInset;                // �ؼ��ڲ����ݵ���������
extern const int kCtrlRightInset;               // �ؼ��ڲ����ݵ���������

extern const int kPermanentAppCount;            // ��פӦ����

// ������¼��/ֱ����ť�ߴ�
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
extern const gfx::Font ftPrimary;  // ��Ҫ���壬���⡢����������button����
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
