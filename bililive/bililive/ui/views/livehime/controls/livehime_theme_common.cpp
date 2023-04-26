#include "livehime_theme_common.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/insets.h"

#include "bililive/bililive/ui/views/livehime/theme/livehime_theme_service.h"
#include "bililive/public/bililive/bililive_process.h"


//namespace
//{

static std::string font_name(gfx::Font().GetFontName());
static std::map<int, gfx::Font> fonts;

gfx::Font GetPointFontByDPIScale(int point_size)
{
    if (fonts.find(point_size) == fonts.end())
    {
        gfx::Font ft_scale(font_name, point_size * ui::GetDPIScale());
        fonts[point_size] = ft_scale;
    }
    return fonts[point_size];
}

//}

gfx::Font GetPointFont(int point_size) 
{
    static std::string font_name(gfx::Font().GetFontName());
    static std::map<int, gfx::Font> fonts;
    if (fonts.find(point_size) == fonts.end())
    {
        gfx::Font ft_scale(font_name, point_size);
        fonts[point_size] = ft_scale;
    }
    return fonts[point_size];
}


const int LivehimePaddingCharWidth(const gfx::Font &font)
{
    static base::string16 str = ResourceBundle::GetSharedInstance().GetLocalizedString(IDS_PADDING_CHAR);
    return font.GetStringWidth(str);
}

//
int GetLengthByDPIScale(int len)
{
    return std::round(len * ui::GetDPIScale());
}

float GetLengthByDPIScaleF(int len) {
    return len * ui::GetDPIScale();
}

gfx::Size GetSizeByDPIScale(const gfx::Size &size)
{
    gfx::Size scale_size = size;
    scale_size.set_width(GetLengthByDPIScale(size.width()));
    scale_size.set_height(GetLengthByDPIScale(size.height()));
    return scale_size;
}

gfx::Rect GetRectByDPIScale(const gfx::Rect &rect)
{
    gfx::Rect scale_rect = rect;
    gfx::Size size = rect.size();
    scale_rect.set_size(GetSizeByDPIScale(size));
    return scale_rect;
}

int GetLengthByImgDPIScale(int len) {
    return std::floor(len * ui::win::GetDeviceScaleFactor());
}

gfx::Size GetSizeByImgDPIScale(const gfx::Size &size) {
    gfx::Size scale_size = size;
    scale_size.set_width(GetLengthByImgDPIScale(size.width()));
    scale_size.set_height(GetLengthByImgDPIScale(size.height()));
    return scale_size;
}

gfx::Rect GetRectByImgDPIScale(const gfx::Rect &rect) {
    gfx::Rect scale_rect = rect;
    gfx::Size size = rect.size();
    scale_rect.set_size(GetSizeByImgDPIScale(size));
    return scale_rect;
}

gfx::Insets GetInsetsByImgDPIScale(const gfx::Insets &insets) {
    return insets.Scale(ui::win::GetDeviceScaleFactor());
}

int GetImageLengthByDPIScale(int len) {
    float diff_scale = ui::GetDPIScale() / ui::win::GetDeviceScaleFactor();
    return len * diff_scale;
}

SkColor GetUserLevelColor(int level)
{
    if (level > 30)
    {
        return SkColorSetRGB(255, 134, 178);
    }
    else if (level > 20)
    {
        return SkColorSetRGB(160, 104, 241);
    }
    else if (level > 10)
    {
        return SkColorSetRGB(88, 150, 222);
    }
    return SkColorSetRGB(97, 192, 90);
}

base::string16 GetLocalizedString(int resource_id)
{
    return ui::ResourceBundle::GetSharedInstance().GetLocalizedString(resource_id);
}

gfx::ImageSkia* GetImageSkiaNamed(int resource_id)
{
    return ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(resource_id);
}

base::RefCountedStaticMemory* LoadDataResourceBytes(int resource_id)
{
    return ui::ResourceBundle::GetSharedInstance().LoadDataResourceBytes(resource_id);
}

SkColor GetColor(LivehimeColorType id)
{
    return GetBililiveProcess()->ThemeService()->GetColor(id);
}


const int kHitTestThickness = GetLengthByDPIScale(5);         // 主界面边框检测
// 控件尺寸
const int kBorderCtrlHeight = GetLengthByDPIScale(30);
const int kMessageBoxWidth = GetLengthByDPIScale(358);
const int kMainWndTabAreaWidth = GetLengthByDPIScale(255);
const int kMainWndTabAreaStripHeight = GetLengthByDPIScale(40);
const gfx::Size kLeftStripPosStripSize = GetSizeByDPIScale(gfx::Size(160, 40));
const int kMsgboxTitleBarHeight = GetLengthByDPIScale(30);
const int kDataTitleHeight = GetLengthByDPIScale(42);
const int kDataHeaderHeight = GetLengthByDPIScale(28);
const int kDataItemHeight = GetLengthByDPIScale(40);
const int kGiftboxItemWidth = GetLengthByDPIScale(560);
const int kGiftboxItemHeight = GetLengthByDPIScale(36);
const int kGiftboxIconWidth = GetLengthByDPIScale(30);
const int kLuckGiftRecordWidth = GetLengthByDPIScale(400);
const int kLuckGiftRecordHeight = GetLengthByDPIScale(60);
const int kUrlEditMinHeight = GetLengthByDPIScale(50);
// 间隔
// 基本原则是最大限度保证控件中的内容的呈现，防止间隔跟随拉伸后屏幕尺寸不够窗体的显示需要
const int kPaddingRowHeightEnds = GetLengthByDPIScale(24);           // 界面顶部和底部距离边界的高度
const int kPaddingRowHeightForGroupCtrls = GetLengthByDPIScale(12);  // 同一组控件之间的行间距
const int kPaddingRowHeightForCtrlTips = GetLengthByDPIScale(6);     // 某个控件与其tip控件的行间距
const int kPaddingRowHeightForGroupTips = GetLengthByDPIScale(9);    // 某组控件与其tip控件的行间距
const int kPaddingRowHeightForDiffGroups = GetLengthByDPIScale(21);  // 同界面不同分组的控件间行高，例如上组最后一个和下组标题间的行高
const int kPaddingColWidthEnds = GetLengthByDPIScale(52);            // 列集合两端的留白
const int kPaddingColWidthForGroupCtrls = GetLengthByDPIScale(12);   // 同一列同组控件的列间距
const int kPaddingColWidthForCtrlTips = GetLengthByDPIScale(6);      // 某个控件与其tip控件的列间距
const int kPaddingColWidthForActionButton = GetLengthByDPIScale(24); // 确定、取消这类的动作按钮的间距
const int kPaddingColWidthEndsSmall = GetLengthByDPIScale(21);       // 列集合两端的留白
const int kPaddingColWidthEndsInviting = GetLengthByDPIScale(10);       // 列集合两端的留白

const int kCtrlLeftInset = GetLengthByDPIScale(12);
const int kCtrlRightInset = GetLengthByDPIScale(12);

const int kPermanentAppCount = 5;

const int kRecordButtonWidth = GetLengthByDPIScale(200);
const int kLiveButtonWidth = GetLengthByDPIScale(200);

const int kRecordButtonHeight = GetLengthByDPIScale(32);
const int kLiveButtonHeight = GetLengthByDPIScale(32);

// 字体
const gfx::Font ftSix = GetPointFontByDPIScale(6);
const gfx::Font ftEight = GetPointFontByDPIScale(8);
const gfx::Font ftEightBold(ftEight.DeriveFont(0, gfx::Font::BOLD));
const gfx::Font ftNine = GetPointFontByDPIScale(9);
const gfx::Font ftTen = GetPointFontByDPIScale(10);
const gfx::Font ftTenBold(ftTen.DeriveFont(0, gfx::Font::BOLD));
const gfx::Font ftEleven = GetPointFontByDPIScale(11);
const gfx::Font ftElevenBold(ftEleven.DeriveFont(0, gfx::Font::BOLD));
const gfx::Font ftTwelve = GetPointFontByDPIScale(12);
const gfx::Font ftTwelveBold(ftTwelve.DeriveFont(0, gfx::Font::BOLD));
const gfx::Font ftThirteen = GetPointFontByDPIScale(13);
const gfx::Font ftThirteenBold(ftThirteen.DeriveFont(0, gfx::Font::BOLD));
const gfx::Font ftFourteen = GetPointFontByDPIScale(14);
const gfx::Font ftFourteenBold(ftFourteen.DeriveFont(0, gfx::Font::BOLD));
const gfx::Font ftFifteen = GetPointFontByDPIScale(15);
const gfx::Font ftFifteenBold(ftFifteen.DeriveFont(0, gfx::Font::BOLD));
const gfx::Font ftSixteen = GetPointFontByDPIScale(16);
const gfx::Font ftSixteenBold(ftSixteen.DeriveFont(0, gfx::Font::BOLD));
const gfx::Font ftSeventeen = GetPointFontByDPIScale(17);
const gfx::Font ftEighteen = GetPointFontByDPIScale(18);
const gfx::Font ftEighteenBold(ftEighteen.DeriveFont(0, gfx::Font::BOLD));
const gfx::Font ftNineteen = GetPointFontByDPIScale(19);
const gfx::Font ftTwenty = GetPointFontByDPIScale(20);
const gfx::Font ftTwentyBold(ftTwenty.DeriveFont(0, gfx::Font::BOLD));
const gfx::Font ftTwentyTwo = GetPointFontByDPIScale(22);
const gfx::Font ftTwentyTwoBold(ftTwentyTwo.DeriveFont(0, gfx::Font::BOLD));
const gfx::Font ftTwentyFour = GetPointFontByDPIScale(24);
const gfx::Font ftTwentyFourBold(ftTwentyFour.DeriveFont(0, gfx::Font::BOLD));
const gfx::Font ftPrimary = ftFourteen;
const gfx::Font ftPrimaryBold = ftFourteenBold;

const gfx::Font ftTwentyEight = GetPointFontByDPIScale(28);
const gfx::Font ftTwentyEightBold(ftTwentyEight.DeriveFont(0, gfx::Font::BOLD));

const gfx::Font ftThirty = GetPointFontByDPIScale(30);
const gfx::Font ftThirtyBold(ftThirty.DeriveFont(0, gfx::Font::BOLD));
const gfx::Font ftThirtyTwo = GetPointFontByDPIScale(32);
const gfx::Font ftThirtySix = GetPointFontByDPIScale(36);
const gfx::Font ftThirtySixBold(ftThirtySix.DeriveFont(0, gfx::Font::BOLD));
// 配色
//const SkColor clrWindowsTitle = SkColorSetRGB(50, 50, 50);
//const SkColor clrWindowsContent = SkColorSetRGB(40, 40, 40);