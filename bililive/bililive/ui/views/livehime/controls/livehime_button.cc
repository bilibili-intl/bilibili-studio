#include "livehime_button.h"

#include "bilibase/scope_guard.h"

#include "bililive/bililive/ui/views/livehime/colive/colive_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"

#include "ui/base/resource/resource_bundle.h"
#include "grit/theme_resources.h"
#include "grit/generated_resources.h"


namespace {

    const int kButtonHeightPadding = GetLengthByDPIScale(10);

}

LivehimeLabelButtonStyleRegister livehime_button_reg_obj;

LivehimeLabelButtonStyleRegister::LivehimeLabelButtonStyleRegister()
{
    BililiveLabelButton::ColorStruct clrs;
    // 这里不能 Get Color, 就直接写死了

    // 这里不能 GetColor
    // 海外样式
    clrs.set_disable = false;
    
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = SkColorSetRGB(76, 147, 255);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = SkColorSetRGB(76, 147, 255);
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetRGB(76, 147, 255);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetRGB(76, 147, 255);
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetRGB(76, 147, 255);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetRGB(76, 147, 255);
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = SK_ColorWHITE;

    //clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_ButtonDisabledColor);
    //clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_ButtonBackgroundColor);
    //clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_ButtonDisabledColor);
    BililiveLabelButton::RegisterButtonStyles(LivehimeButtonStyle_ActionButtonPositive, clrs);
    BililiveLabelButton::RegisterButtonStyles(LivehimeGeneralMsgBox_Button_Positive, clrs);
    //
    clrs.set_disable = false;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = SkColorSetRGB(92, 94, 112);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = SkColorSetRGB(92, 94, 112);
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetRGB(92, 94, 112);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetRGB(92, 94, 112);
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetRGB(92, 94, 112);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetRGB(92, 94, 112);
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = SK_ColorWHITE;

    //clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = clrFrameAndButtonBk;
    //clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = clrFrameAndButtonBk;
    //clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = clrFrameAndButtonBk;
    BililiveLabelButton::RegisterButtonStyles(LivehimeGeneralMsgBox_Button_Negative, clrs);
    clrs.set_disable = true;
    clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = SK_ColorWHITE;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = SK_ColorWHITE;
    clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = clrLabelText;
    BililiveLabelButton::RegisterButtonStyles(LivehimeButtonStyle_ActionButtonNegative, clrs);
    //
    clrs.set_disable = false;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = SkColorSetRGB(64, 65, 85);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = SkColorSetRGB(64, 65, 85);
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetRGB(64, 65, 85);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetRGB(64, 65, 85);
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetRGB(64, 65, 85);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetRGB(64, 65, 85);
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = SK_ColorWHITE;
    BililiveLabelButton::RegisterButtonStyles(LivehimeButtonStyle_FunctionButton, clrs);

    //
    clrs.set_disable = true;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = clrColiveCMBBorder;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = clrColiveOBSBackground;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = clrTextPrimary;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = clrCtrlBorderHov;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = clrColiveOBSBackground;
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = clrCtrlBorderHov;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = clrCtrlBorderPre;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = clrColiveOBSBackground;
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = clrCtrlBorderPre;

    clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(clrCtrlBorderNor, disable_alpha);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = clrColiveOBSBackground;
    clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(clrTextPrimary, disable_alpha);
    BililiveLabelButton::RegisterButtonStyles(LivehimeButtonStyle_ColiveButtonCancelMatching, clrs);

    //
    clrs.set_disable = false;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = clrButtonPositive;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorWHITE;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = clrButtonPositive;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = clrButtonPositiveHov;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = SK_ColorWHITE;
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = clrButtonPositiveHov;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = clrButtonPositivePre;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = SK_ColorWHITE;
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = clrButtonPositivePre;
    BililiveLabelButton::RegisterButtonStyles(LivehimeButtonStyle_ColiveButtonInvite, clrs);

    //
    clrs.set_disable = true;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = clrRecordButtonBk;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = clrRecordButtonBk;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = clrRecordButtonText;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetA(clrRecordButtonBk, disable_alpha);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetA(clrRecordButtonBk, disable_alpha);
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = clrRecordButtonText;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetA(clrRecordButtonBk, disable_alpha);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetA(clrRecordButtonBk, disable_alpha);
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = clrRecordButtonText;

    clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(clrRecordButtonBk, disable_alpha);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(clrRecordButtonBk, disable_alpha);
    clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(clrRecordButtonText, disable_alpha);
    BililiveLabelButton::RegisterButtonStyles(LivehimeButtonStyle_V3_RecordButton, clrs);

    //
    clrs.set_disable = true;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = SkColorSetA(SK_ColorWHITE, kHoverMaskAlpha);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorTRANSPARENT;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetA(SK_ColorWHITE, kInactiveAlpha);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetA(SK_ColorWHITE, kHoverMaskAlpha);
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetA(SK_ColorWHITE, kInactiveAlpha);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetA(SK_ColorWHITE, kHoverMaskAlpha);
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(SK_ColorWHITE, kHoverMaskAlpha);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = SK_ColorTRANSPARENT;
    clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(SK_ColorWHITE, kHoverMaskAlpha);
    BililiveLabelButton::RegisterButtonStyles(LivehimeButtonStyle_V3_TitleButton, clrs);


    clrs.set_disable = true;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = clrCtrlBorderNor;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = clrWindowsContent;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = clrTextCover;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = clrCtrlBorderHov;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = clrWindowsContent;
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = clrCtrlBorderHov;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = clrCtrlBorderPre;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = clrWindowsContent;
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = clrCtrlBorderPre;

    clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(clrCtrlBorderNor, disable_alpha);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(SK_ColorWHITE, disable_alpha);
    clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(clrTextCover, disable_alpha);

    BililiveLabelButton::RegisterButtonStyles(LivehimeButtonStyle_CoverButton, clrs);

    //
    clrs.set_disable = true;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = clrTextBorder;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = clrTextBG;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = clrTextALL;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = clrTextBorder;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = clrTextBG;
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = clrTextALLHov;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = clrTextBorder;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = clrTextBG;
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = clrTextALLHov;

    /*clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(clrCtrlBorderNor, disable_alpha);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(SK_ColorWHITE, disable_alpha);
    clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(clrTextCover, disable_alpha);*/
    BililiveLabelButton::RegisterButtonStyles(LivehimeButtonStyle_V4_CoverButton, clrs);

    //
    clrs.set_disable = false;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = clrNewBtnNor;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = clrNewBtnNor;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = clrNewBtnHov;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = clrNewBtnHov;
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = clrNewBtnNor;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = clrNewBtnNor;
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = SK_ColorWHITE;

    //clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_ButtonDisabledColor);
    //clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_ButtonBackgroundColor);
    //clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_ButtonDisabledColor);
    BililiveLabelButton::RegisterButtonStyles(LivehimeButtonStyle_V4_LiveButton, clrs);

    clrs.set_disable = false;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = SkColorSetA(clrRecordButtonText, 0.1f * 255);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = clrRecordButtonBk;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = clrRecordButtonText;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetA(clrRecordButtonText, 0.1f * 255);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetA(clrRecordButtonBk, disable_alpha);
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = clrRecordButtonText;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetA(clrRecordButtonText, 0.1f * 255);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetA(clrRecordButtonBk, disable_alpha);
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = clrRecordButtonText;

    clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(clrRecordButtonText, 0.1f * 255);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(clrRecordButtonBk, disable_alpha);
    clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(clrRecordButtonText, disable_alpha);

    //clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_ButtonDisabledColor);
    //clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_ButtonBackgroundColor);
    //clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_ButtonDisabledColor);
    BililiveLabelButton::RegisterButtonStyles(LivehimeButtonStyle_V4_RecordButton, clrs);

}

// LivehimeLinkButton
LivehimeLinkButton::LivehimeLinkButton(views::ButtonListener* listener, const string16& text)
    : BililiveLabelButton(listener, text)
{
    SetStyle(STYLE_DEFAULT);
    SetFont(ftPrimary);
    SetTextColor(views::Button::STATE_NORMAL, clrLinkButtonNormal);
    SetTextColor(views::Button::STATE_HOVERED, clrLinkButtonPressed);
    SetTextColor(views::Button::STATE_PRESSED, clrLinkButtonPressed);
    SetTextColor(views::Button::STATE_DISABLED, clrLinkButtonDisable);
    SetCursor(::LoadCursor(nullptr, IDC_HAND));
}

// LivehimeSmallTitleLinkButton
LivehimeSmallTitleLinkButton::LivehimeSmallTitleLinkButton(views::ButtonListener* listener, const string16& text)
    : BililiveLabelButton(listener, text)
{
    SetStyle(STYLE_DEFAULT);
    SetFont(ftTwelve);
    SetTextColor(views::Button::STATE_NORMAL, clrTextTitle);
    SetTextColor(views::Button::STATE_HOVERED, clrLinkButtonPressed);
    SetTextColor(views::Button::STATE_PRESSED, clrLinkButtonPressed);
    SetTextColor(views::Button::STATE_DISABLED, SkColorSetA(clrTextTitle, disable_alpha));
}

gfx::NativeCursor LivehimeSmallTitleLinkButton::GetCursor(const ui::MouseEvent& event)
{
    return ::LoadCursor(nullptr, IDC_HAND);
}

// LivehimeLabelButton
LivehimeLabelButton::LivehimeLabelButton(views::ButtonListener* listener, const string16& text, /*LivehimeButtonStyle*/int lbs)
    : BililiveLabelButton(listener, text)
{
    SetStyle(lbs);
    switch (lbs)
    {
    case LivehimeButtonStyle_ActionButtonPositive:
    case LivehimeButtonStyle_ActionButtonNegative:
        SetPreferredSize(LivehimeActionLabelButton::GetButtonSize());
        SetFont(ftPrimary);
        break;
    case LivehimeButtonStyle_FunctionButton:
        SetPreferredSize(LivehimeFunctionLabelButton::GetButtonSize());
        SetFont(ftPrimary);
        break;
    case LivehimeButtonStyle_ColiveButtonCancelMatching:
        SetPreferredSize(LivehimeColiveLabelButton::GetButtonSize());
        SetFont(ftPrimary);
        break;
    case LivehimeButtonStyle_ColiveButtonInvite:
        SetPreferredSize(LivehimeColiveInviteButton::GetButtonSize());
        SetFont(ftPrimary);
        break;
    case LivehimeButtonStyle_V3_RecordButton:
        SetFont(ftPrimary);
        break;
    case LivehimeGeneralMsgBox_Button_Positive:
    case LivehimeGeneralMsgBox_Button_Negative:
        SetFont(ftFourteen);
        SetPreferredSize(LivehimeGeneralMsgBoxButton::GetButtonSize());
        break;
	case LivehimeButtonStyle_V4_LiveButton:
	case LivehimeButtonStyle_V4_RecordButton:
		SetFont(ftPrimary);
        SetPreferredSize(GetSizeByDPIScale(gfx::Size(88, 32)));
		break;
    case LivehimeButtonStyle_CoverButton:
        SetFont(ftTwelve);
        break;
    case LivehimeButtonStyle_V4_CoverButton:
		SetFont(ftTwelve);
        SetPreferredSize(GetSizeByDPIScale(gfx::Size(88, 18)));
		break;
    default:
        break;
    }
}


// LivehimeActionLabelButton
const char LivehimeActionLabelButton::kViewClassName[] = "LivehimeActionLabelButton";

gfx::Size LivehimeActionLabelButton::GetButtonSize()
{
    static gfx::Size size = GetSizeByDPIScale(gfx::Size(80, 30));
    return size;
}

//LivehimeGeneralMsgBoxButton
const char LivehimeGeneralMsgBoxButton::kViewClassName[] = "LivehimeGeneralMsgButton";
gfx::Size LivehimeGeneralMsgBoxButton::GetButtonSize()
{
    static gfx::Size size = GetSizeByDPIScale(gfx::Size(88, 32));
    return size;
}

// LivehimeFunctionLabelButton
const char LivehimeFunctionLabelButton::kViewClassName[] = "LivehimeFunctionLabelButton";

gfx::Size LivehimeFunctionLabelButton::GetButtonSize()
{
    static gfx::Size size = GetSizeByDPIScale(gfx::Size(80, 30));
    return size;
}


// LivehimeVerticalLabelButton
LivehimeVerticalLabelButton::LivehimeVerticalLabelButton(views::ButtonListener* listener, const string16& text)
    : views::CustomButton(listener)
    , is_center_(false)
{
    colors_[views::Button::STATE_NORMAL] = clrTextPrimary;
    colors_[views::Button::STATE_HOVERED] = clrLivehime;
    colors_[views::Button::STATE_PRESSED] = clrLivehime;
    colors_[views::Button::STATE_DISABLED] = SkColorSetA(clrTextPrimary, disable_alpha);

    image_ = new views::ImageView();
    image_->set_interactive(false);
    image_->SetVerticalAlignment(views::ImageView::Alignment::CENTER);
    image_->SetHorizontalAlignment(views::ImageView::Alignment::CENTER);
    label_ = new BililiveLabel(text);
    AddChildView(image_);
    AddChildView(label_);
    SetFont(ftPrimary);
}

void LivehimeVerticalLabelButton::SetImage(views::Button::ButtonState state, const gfx::ImageSkia* image)
{
    images_[state] = image ? *image : gfx::ImageSkia();
    if (state == Button::STATE_HOVERED)
    {
        if (images_[Button::STATE_PRESSED].isNull())
        {
            images_[Button::STATE_PRESSED] = images_[state];
        }
    }
    image_->SetImage(images_[this->state()]);
    PreferredSizeChanged();
    Layout();
}

void LivehimeVerticalLabelButton::SetColor(views::Button::ButtonState state, const SkColor clr)
{
    colors_[state] = clr;
    label_->SetTextColor(colors_[this->state()]);
}

const gfx::ImageSkia& LivehimeVerticalLabelButton::GetImage(views::Button::ButtonState state) const {
    return images_[state];
}

gfx::Size LivehimeVerticalLabelButton::GetPreferredSize()
{
    gfx::Size img_size = image_->GetPreferredSize();
    gfx::Size lab_size = label_->GetPreferredSize();
    gfx::Size pref_size;
    pref_size.SetToMax(lab_size);
    pref_size.set_width(std::max(img_size.width(), lab_size.width()));
    pref_size.set_height(img_size.height() + GetLengthByDPIScale(5) + lab_size.height());
    return pref_size;
}

void LivehimeVerticalLabelButton::Layout()
{
    gfx::Size img_size = image_->GetPreferredSize();
    gfx::Size lab_size = label_->GetPreferredSize();
    if (is_center_){
		label_->SetBounds((width() - lab_size.width()) / 2 + 3 , (height() - lab_size.height())/2,
			lab_size.width(), lab_size.height());
		image_->SetBounds((width() - img_size.width()) / 2, (height() - img_size.height()) / 2,
			img_size.width(), img_size.height());
    }
    else {
		label_->SetBounds((width() - lab_size.width()) / 2, height() - lab_size.height(),
			lab_size.width(), lab_size.height());
		image_->SetBounds((width() - img_size.width()) / 2, label_->y() - GetLengthByDPIScale(5) - img_size.height(),
			img_size.width(), img_size.height());
    }
   
}

void LivehimeVerticalLabelButton::NotifyClick(const ui::Event& event)
{
    // crash发现个现象：在有些view里（用户信息面板）点击按钮跳转系统浏览器时，
    // 可能会因为窗口丢失焦点而直接关闭（UI线程不排队等待mouserelease事件处理完毕？）
    // 从而引起接下来的ReportBehaviorEvent使用到已被释放的资源引发崩溃。
    // 所以这里改成ReportBehaviorEvent置于__super::NotifyClick之前，看看效果。
    // 注意！由于这里把埋点置于按钮事件触发之前，所以如果是在点击时上报动态参数的场景，
    // 就不要采用按钮自动上报的方式，应该改用在按钮listener里进行埋点

    ReportBehaviorEvent();

    __super::NotifyClick(event);
}

void LivehimeVerticalLabelButton::StateChanged()
{
    image_->SetImage(images_[state()]);
    label_->SetTextColor(colors_[state()]);
}

void LivehimeVerticalLabelButton::SetFont(const gfx::Font &font)
{
    label_->SetFont(font);
    PreferredSizeChanged();
}

void LivehimeVerticalLabelButton::SetText(const base::string16 &text)
{
    label_->SetText(text);
    PreferredSizeChanged();
}


// LivehimeImageRightButton
LivehimeImageRightButton::LivehimeImageRightButton(views::ButtonListener* listener, const string16& text)
	: views::CustomButton(listener)
{
    space_ = GetLengthByDPIScale(2);
	colors_[views::Button::STATE_NORMAL] = clrTextPrimary;
	colors_[views::Button::STATE_HOVERED] = clrLivehime;
	colors_[views::Button::STATE_PRESSED] = clrLivehime;
	colors_[views::Button::STATE_DISABLED] = SkColorSetA(clrTextPrimary, disable_alpha);

	image_ = new views::ImageView();
	image_->set_interactive(false);
	image_->SetVerticalAlignment(views::ImageView::Alignment::CENTER);
	image_->SetHorizontalAlignment(views::ImageView::Alignment::CENTER);
	label_ = new BililiveLabel(text);
    label_->SetVerticalAlignment(gfx::VerticalAlignment::ALIGN_VCENTER);
	AddChildView(image_);
	AddChildView(label_);
	SetFont(ftPrimary);
}

void LivehimeImageRightButton::SetImage(views::Button::ButtonState state, const gfx::ImageSkia* image)
{
	images_[state] = image ? *image : gfx::ImageSkia();
	if (state == Button::STATE_HOVERED)
	{
		if (images_[Button::STATE_PRESSED].isNull())
		{
			images_[Button::STATE_PRESSED] = images_[state];
		}
	}
	image_->SetImage(images_[this->state()]);
	PreferredSizeChanged();
	Layout();
}

void LivehimeImageRightButton::SetColor(views::Button::ButtonState state, const SkColor clr)
{
	colors_[state] = clr;
	label_->SetTextColor(colors_[this->state()]);
}

const gfx::ImageSkia& LivehimeImageRightButton::GetImage(views::Button::ButtonState state) const {
	return images_[state];
}

gfx::Size LivehimeImageRightButton::GetPreferredSize()
{
	gfx::Size img_size = image_->GetPreferredSize();
	gfx::Size lab_size = label_->GetPreferredSize();
	gfx::Size pref_size;
	pref_size.SetToMax(gfx::Size(lab_size.width() + img_size.width(), std::max(lab_size.height(),img_size.height())));
	pref_size.set_width(img_size.width() + space_ + lab_size.width());
	pref_size.set_height(std::max(img_size.height(), lab_size.height()));
	return pref_size;
}

void LivehimeImageRightButton::Layout()
{
	gfx::Size img_size = image_->GetPreferredSize();
	gfx::Size lab_size = label_->GetPreferredSize();
	int left = (width() - lab_size.width() - space_ - img_size.width()) / 2;
    left = left > 0 ? left : 0;
	label_->SetBounds(left,(height() - lab_size.height())/2,
		lab_size.width(), lab_size.height());
	image_->SetBounds(left + lab_size.width() + space_, (height() - img_size.height()) / 2,
		img_size.width(), img_size.height());
}

void LivehimeImageRightButton::StateChanged()
{
	image_->SetImage(images_[state()]);
	label_->SetTextColor(colors_[state()]);
}

void LivehimeImageRightButton::SetFont(const gfx::Font& font)
{
	label_->SetFont(font);
	PreferredSizeChanged();
}

void LivehimeImageRightButton::SetText(const base::string16& text)
{
	label_->SetText(text);
	PreferredSizeChanged();
}

gfx::NativeCursor LivehimeImageRightButton::GetCursor(const ui::MouseEvent& event)
{
	if (!enabled())
		return gfx::kNullCursor;

	if (!cursor_)
	{
		static HCURSOR g_hand_cursor = LoadCursor(NULL, IDC_ARROW);
		return g_hand_cursor;
	}
	else
	{
		return cursor_;
	}
}

// LivehimeColorRadioButton
LivehimeColorRadioButton::LivehimeColorRadioButton(int group_id, SkColor color)
    : BililiveRadioButton(L"", group_id)
    , color_(color)
{
    set_focus_border(nullptr);
    SetCheckedImageSkia(ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_SELECT_STATE));
    UpdateImage();
}

LivehimeColorRadioButton::~LivehimeColorRadioButton()
{
}

void LivehimeColorRadioButton::SetCheckedImageSkia(gfx::ImageSkia *image)
{
    check_image_ = *image;
}

void LivehimeColorRadioButton::SetColor(SkColor color)
{
    color_ = color;
}

gfx::Size LivehimeColorRadioButton::GetPreferredSize()
{
    return GetSizeByDPIScale({18, 18}); //gfx::Size(LivehimePaddingCharWidth(ftPrimary) * 4, ftPrimary.GetHeight() + kButtonHeightPadding);
}

int LivehimeColorRadioButton::GetHeightForWidth(int w)
{
    return GetPreferredSize().height();
}

void LivehimeColorRadioButton::OnPaint(gfx::Canvas* canvas)
{
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(color_);
    paint.setStyle(SkPaint::kFill_Style);
    canvas->DrawRoundRect(GetLocalBounds(), 4, paint);

    if (checked() && !check_image_.isNull())
    {
        canvas->DrawImageInt(check_image_, width() - check_image_.width(), 0);
    }
}

const gfx::ImageSkia& LivehimeColorRadioButton::GetImage(ButtonState for_state)
{
    return null_image_;
}

void LivehimeColorRadioButton::SetChecked(bool checked)
{
    __super::SetChecked(checked);

    SchedulePaint();
}


// LivehimeFilterRadioButton
LivehimeFilterRadioButton::LivehimeFilterRadioButton(int group_id)
    : BililiveRadioButton(L"", group_id)
{
    set_focus_border(nullptr);
    SetFont(ftTwelve);
    UpdateImage();
}

LivehimeFilterRadioButton::~LivehimeFilterRadioButton()
{

}

void LivehimeFilterRadioButton::SetImageAndText(gfx::ImageSkia *image,
    const base::string16 &text)
{
    main_image_ = *image;
    text_ = text;
}

gfx::NativeCursor LivehimeFilterRadioButton::GetCursor(const ui::MouseEvent& event)
{
    if (!enabled())
        return gfx::kNullCursor;

    static HCURSOR g_hand_cursor = LoadCursor(nullptr, IDC_HAND);
    return g_hand_cursor;
}

gfx::Size LivehimeFilterRadioButton::GetPreferredSize()
{
    return GetSizeByDPIScale({65, 88});
}

int LivehimeFilterRadioButton::GetHeightForWidth(int w)
{
    return GetPreferredSize().height();
}

void LivehimeFilterRadioButton::OnPaint(gfx::Canvas* canvas)
{
    gfx::Rect img_rect;
    img_rect.set_width(GetLengthByDPIScale(65));
    img_rect.set_height(GetLengthByDPIScale(65));

    if (!main_image_.isNull())
    {
        canvas->DrawImageInt(main_image_, 
            0, 
            0, 
            main_image_.width(), 
            main_image_.height(),
            img_rect.x(), 
            img_rect.y(), 
            img_rect.width(), 
            img_rect.height(), 
            true);
    }

    if (!text_.empty())
    {
        gfx::Rect content_rect = GetContentsBounds();
        int text_width = GetFont().GetStringWidth(text_);

        gfx::Rect text_rect;
        text_rect.set_width(text_width);
        text_rect.set_height(GetLengthByDPIScale(17));
        text_rect.set_x((content_rect.width() - text_width) / 2.0);
        text_rect.set_y(GetLengthByDPIScale(71));

        SkColor text_color{};
        if (checked()) 
        {
            text_color = SkColorSetRGB(116, 215, 255);
        }
        else 
        {
            text_color = SkColorSetRGB(141, 154, 164);
        }

        canvas->DrawStringInt(text_, GetFont(), text_color, text_rect);
    }

    if (checked()) 
    {
        SkPaint painter;
        painter.setStyle(SkPaint::kStroke_Style);
        painter.setStrokeWidth(2.0f);
        painter.setColor(SkColorSetRGB(14, 190, 255));

        bililive::DrawRoundRect(canvas, img_rect.x(), img_rect.y(), img_rect.width(), img_rect.height(), GetLengthByDPIScale(12), painter);
    }
}

void LivehimeFilterRadioButton::OnFocus()
{
    Checkbox::OnFocus();
}

const gfx::ImageSkia& LivehimeFilterRadioButton::GetImage(ButtonState for_state)
{
    return null_image_;
}

void LivehimeFilterRadioButton::SetChecked(bool checked)
{
    if (checked != views::RadioButton::checked())
    {
        __super::SetChecked(checked);

        SchedulePaint();
    }
}

void LivehimeFilterRadioButton::NotifyClick(const ui::Event& event)
{
    if (!checked())
    {
        SetChecked(true);
    }
    else
    {
        SetChecked(false);
    }
    RequestFocus();
    LabelButton::NotifyClick(event);
}


// LivehimeCheckbox
LivehimeCheckbox::LivehimeCheckbox(const base::string16& label)
    : BililiveCheckbox(label)
{
    InitStyle();
}

LivehimeCheckbox::LivehimeCheckbox(UIViewsStyle style, const base::string16& label) : BililiveCheckbox(label)
{
    InitStyle();
    if (style == UIViewsStyle::Style_4_10)
    {
		SetTextColor(Button::STATE_NORMAL, GetColor(LabelTitle));
		SetTextColor(Button::STATE_HOVERED, GetColor(LabelTitle));
		SetTextColor(Button::STATE_PRESSED, GetColor(LabelTitle));
		SetTextColor(Button::STATE_DISABLED, SkColorSetA(GetColor(LabelTitle), disable_alpha));
    }
    if (style == UIViewsStyle::Style_Model_MsgBox) 
    {
        SetFont(ftTwelve);
        SetTextColor(Button::STATE_NORMAL, SkColorSetRGB(0x75, 0x85, 0x92));
        SetTextColor(Button::STATE_HOVERED, SkColorSetRGB(0x75, 0x85, 0x92));
        SetTextColor(Button::STATE_PRESSED, SkColorSetRGB(0x75, 0x85, 0x92));
        SetTextColor(Button::STATE_DISABLED, SkColorSetA(SkColorSetRGB(0x75, 0x85, 0x92), disable_alpha));
        gfx::ImageSkia unselc = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEMAIN_MSGBOX_GEN_TOOLBAR_CHECKBOX);
        gfx::ImageSkia unselc_hov = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEMAIN_MSGBOX_GEN_TOOLBAR_CHECKBOX);
        gfx::ImageSkia selec = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEMAIN_MSGBOX_GEN_TOOLBAR_CHECKBOX_CHECKED);
        gfx::ImageSkia selec_dis = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEMAIN_MSGBOX_GEN_TOOLBAR_CHECKBOX_CHECKED);
        SetCustomImage(false, false, views::Button::STATE_NORMAL, unselc);
        SetCustomImage(true, false, views::Button::STATE_NORMAL, selec);
        SetCustomImage(false, false, views::Button::STATE_HOVERED, unselc_hov);
        SetCustomImage(true, false, views::Button::STATE_HOVERED, selec);
        SetCustomImage(false, false, views::Button::STATE_PRESSED, unselc);
        SetCustomImage(true, false, views::Button::STATE_PRESSED, selec);
        SetCustomImage(false, false, views::Button::STATE_DISABLED, unselc);
        SetCustomImage(true, false, views::Button::STATE_DISABLED, selec_dis);
    }
}

void LivehimeCheckbox::InitStyle()
{
	SetFont(ftPrimary);

    SetTextColor(Button::STATE_NORMAL, SkColorSetRGB(255, 255, 255));
    SetTextColor(Button::STATE_HOVERED, SkColorSetRGB(255, 255, 255));
    SetTextColor(Button::STATE_PRESSED, SkColorSetRGB(255, 255, 255));
    SetTextColor(Button::STATE_DISABLED, SkColorSetA(clrOptionButtonNor, disable_alpha));

	static gfx::ImageSkia unsel, unsel_hv, selected, selected_dis;
	static bool init = false;
	if (!init)
	{
		init = true;
		unsel = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_CHECKBOX);
		unsel_hv = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_CHECKBOX_HV);
		selected = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_CHECKBOX_SELECT);
		selected_dis = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_CHECKBOX_SELECT_DIS);
	}

	SetCustomImage(false, false, views::Button::STATE_NORMAL, unsel);
	SetCustomImage(false, true, views::Button::STATE_NORMAL, unsel);
	SetCustomImage(false, false, views::Button::STATE_HOVERED, unsel_hv);
	SetCustomImage(false, true, views::Button::STATE_HOVERED, unsel_hv);
	SetCustomImage(false, false, views::Button::STATE_PRESSED, unsel_hv);
	SetCustomImage(false, true, views::Button::STATE_PRESSED, unsel_hv);
	SetCustomImage(false, false, views::Button::STATE_DISABLED, unsel);
	SetCustomImage(false, true, views::Button::STATE_DISABLED, unsel);

	SetCustomImage(true, false, views::Button::STATE_NORMAL, selected);
	SetCustomImage(true, true, views::Button::STATE_NORMAL, selected);
	SetCustomImage(true, false, views::Button::STATE_HOVERED, selected);
	SetCustomImage(true, true, views::Button::STATE_HOVERED, selected);
	SetCustomImage(true, false, views::Button::STATE_PRESSED, selected);
	SetCustomImage(true, true, views::Button::STATE_PRESSED, selected);
	SetCustomImage(true, false, views::Button::STATE_DISABLED, selected_dis);
	SetCustomImage(true, true, views::Button::STATE_DISABLED, selected_dis);
}


// LivehimeRadioButton
LivehimeRadioButton::LivehimeRadioButton(const string16& label, int group_id, views::ButtonListener* listener/* = nullptr*/)
    : BililiveRadioButton(label, group_id)
{
    set_listener(listener);
    set_focus_border(nullptr);
    SetFont(ftPrimary);

    SetTextColor(views::Button::STATE_NORMAL, SkColorSetRGB(255, 255, 255));
    SetTextColor(views::Button::STATE_HOVERED, SkColorSetRGB(255, 255, 255));
    SetTextColor(views::Button::STATE_PRESSED, SkColorSetRGB(255, 255, 255));
    SetTextColor(views::Button::STATE_DISABLED, SkColorSetARGB(disable_alpha, 255, 255, 255));

    this->label()->SetHasFocusBorder(false);

    static gfx::ImageSkia unsel, unsel_hv, selected;
    static bool init = false;
    if (!init)
    {
        init = true;
        unsel = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_RADIO);
        unsel_hv = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_RADIO_HV);
        selected = *ResourceBundle::GetSharedInstance().GetImageSkiaNamed(IDR_LIVEHIME_RADIO_SELECT);
    }

    SetCustomImage(false, false, views::Button::STATE_NORMAL, unsel);
    SetCustomImage(false, true, views::Button::STATE_NORMAL, unsel);
    SetCustomImage(false, false, views::Button::STATE_HOVERED, unsel_hv);
    SetCustomImage(false, true, views::Button::STATE_HOVERED, unsel_hv);
    SetCustomImage(false, false, views::Button::STATE_PRESSED, unsel_hv);
    SetCustomImage(false, true, views::Button::STATE_PRESSED, unsel_hv);
    SetCustomImage(false, false, views::Button::STATE_DISABLED, unsel);
    SetCustomImage(false, true, views::Button::STATE_DISABLED, unsel);

    SetCustomImage(true, false, views::Button::STATE_NORMAL, selected);
    SetCustomImage(true, true, views::Button::STATE_NORMAL, selected);
    SetCustomImage(true, false, views::Button::STATE_HOVERED, selected);
    SetCustomImage(true, true, views::Button::STATE_HOVERED, selected);
    SetCustomImage(true, false, views::Button::STATE_PRESSED, selected);
    SetCustomImage(true, true, views::Button::STATE_PRESSED, selected);
    SetCustomImage(true, false, views::Button::STATE_DISABLED, selected);
    SetCustomImage(true, true, views::Button::STATE_DISABLED, selected);
}

void LivehimeRadioButton::SetAllStateTextColor(SkColor clr)
{
    SetTextColor(views::Button::STATE_NORMAL, clr);
    SetTextColor(views::Button::STATE_HOVERED, clr);
    SetTextColor(views::Button::STATE_PRESSED, clr);
    SetTextColor(views::Button::STATE_DISABLED, SkColorSetA(clr, disable_alpha));
}


// LivehimeCapsuleButton
gfx::Size LivehimeCapsuleButton::GetButtonSize()
{
    return gfx::Size(LivehimePaddingCharWidth(ftPrimary) * 8, ftPrimary.GetHeight() + kButtonHeightPadding);
}

LivehimeCapsuleButton::LivehimeCapsuleButton(views::ButtonListener* listener, const string16& text)
    : CustomButton(listener)
    , is_checked_(false)
{
    clrs_details_.set_disable = true;
    clrs_details_.border_clrs[views::Button::ButtonState::STATE_NORMAL] = clrCtrlBorderNor;
    clrs_details_.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorTRANSPARENT;
    clrs_details_.text_clrs[views::Button::ButtonState::STATE_NORMAL] = clrLabelContent;

    clrs_details_.border_clrs[views::Button::ButtonState::STATE_HOVERED] = clrButtonPositive;
    clrs_details_.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = SK_ColorTRANSPARENT;
    clrs_details_.text_clrs[views::Button::ButtonState::STATE_HOVERED] = clrLabelContent;

    clrs_details_.border_clrs[views::Button::ButtonState::STATE_PRESSED] = clrButtonPositive;
    clrs_details_.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = SK_ColorTRANSPARENT;
    clrs_details_.text_clrs[views::Button::ButtonState::STATE_PRESSED] = clrLabelContent;

    clrs_details_.border_clrs[views::Button::ButtonState::STATE_DISABLED] = clrButtonPositive;
    clrs_details_.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(SK_ColorWHITE, disable_alpha);
    clrs_details_.text_clrs[views::Button::ButtonState::STATE_DISABLED] = SkColorSetA(clrLabelContent, disable_alpha);

    font_ = ftTwelve;
    label_ = new BililiveLabel(text);
    label_->SetTextColor(clrLabelContent);
    label_->SetFont(font_);
    AddChildView(label_);
}

void LivehimeCapsuleButton::SetChecked(bool checked)
{
    if (is_checked_ != checked)
    {
        is_checked_ = checked;
        StateChanged();
        SchedulePaint();
    }
}

void LivehimeCapsuleButton::StateChanged()
{
    if (!is_checked_)
    {
        label_->SetTextColor(clrs_details_.text_clrs[state()]);
    }
    else
    {
        label_->SetTextColor(clrButtonPositive);
    }
}

void LivehimeCapsuleButton::OnPaintBackground(gfx::Canvas* canvas)
{
    SkColor clrBorder = clrCtrlBorderNor;
    SkColor clrBk = SK_ColorTRANSPARENT;
    if (is_checked_)
    {
        clrBorder = clrButtonPositive;
    }
    else
    {
        if (state() == Button::STATE_DISABLED)
        {
            //DCHECK(clrs_details_.set_disable);
        }

        clrBorder = clrs_details_.border_clrs[state()];
        clrBk = clrs_details_.bkg_clrs[state()];
    }

    SkPaint paint;
    paint.setAntiAlias(true);

    // 填充底色
    paint.setColor(clrBk);
    canvas->DrawRoundRect(GetLocalBounds(), height() / 2, paint);

    // 边框
    paint.setColor(clrBorder);
    // 直接用canvas的DrawRoundRect(setStyle(SkPaint::kStroke_Style) & setStrokeWidth(1))线条很难看，用雷喵喵大法
    bililive::DrawRoundRect(canvas, 0, 0, width(), height(), height() / 2, paint);
}

gfx::Size LivehimeCapsuleButton::GetPreferredSize()
{
    gfx::Size size;
    int height = label_->GetPreferredSize().height() + kButtonHeightPadding;
    size.SetSize(LivehimePaddingCharWidth(font_) * 6 + height, height);
    return size;
}

int LivehimeCapsuleButton::GetHeightForWidth(int w)
{
    return GetPreferredSize().height();
}

void LivehimeCapsuleButton::Layout()
{
    int thickness = (width()- LivehimePaddingCharWidth(font_) * 6 )/ 2;
    int label_height = label_->GetPreferredSize().height();
    label_->SetBounds(thickness, (height() - label_height) / 2, LivehimePaddingCharWidth(font_) * 6, label_height);
}

void LivehimeCapsuleButton::SetColorDetails(const BililiveLabelButton::ColorStruct& clrs)
{
    clrs_details_ = clrs;
}

void LivehimeCapsuleButton::SetText(const string16& text)
{
    label_->SetText(text);
}


// LivehimeColiveLabelButton
const char LivehimeColiveLabelButton::kViewClassName[] = "LivehimeColiveLabelButton";

gfx::Size LivehimeColiveLabelButton::GetButtonSize() {
    static gfx::Size size = GetSizeByDPIScale(gfx::Size(120, 30));
    return size;
}


// LivehimeColiveInviteButton
const char LivehimeColiveInviteButton::kViewClassName[] = "LivehimeColiveInviteButton";

gfx::Size LivehimeColiveInviteButton::GetButtonSize()
{
    static gfx::Size size = GetSizeByDPIScale(gfx::Size(60, 24));
    return size;
}


// LivehimeTitlebarButton
LivehimeTitlebarButton::LivehimeTitlebarButton(views::ButtonListener* listener, const string16& text)
    : BililiveLabelButton(listener, text)
{
    SetStyle(LivehimeButtonStyle_V3_TitleButton);
    SetFont(ftTwelve);
}

// LivehimeRoomInfoAreaButton
LivehimeRoomInfoAreaButton::LivehimeRoomInfoAreaButton(views::ButtonListener* listener, const string16& text)
    : BililiveLabelButton(listener, text)
{
    SetStyle(LivehimeButtonStyle_FunctionButton);
    SetFont(ftTwelve);
    SetTextColor(Button::STATE_NORMAL, SkColorSetRGB(0x2C,0x44,0x57));
}
//LivehimeSwitchModeButton
LivehimeSwitchModeButton::LivehimeSwitchModeButton(views::ButtonListener* listener, const string16& text)
    :BililiveLabelButton(listener, text)
{

	SetFont(ftTwelve);
	SetTextColor(Button::STATE_NORMAL, GetColor(LabelTitle));
    SetTextColor(Button::STATE_HOVERED, SkColorSetRGB(0x0E, 0xBE, 0xFF));
    SetTextColor(Button::STATE_PRESSED, SkColorSetRGB(0x0E, 0xBE, 0xFF));
    SetPreferredSize(GetSizeByDPIScale({ 45, 20 }));

}

LivehimeCoverModeButton::LivehimeCoverModeButton(views::ButtonListener* listener, CoverModeButtonType bt_type)
	: LivehimeLabelButton(listener, L"", LivehimeButtonStyle_V4_CoverButton)
{
    if (bt_type == BtType_SelectPartition)
    {
        SetText(GetLocalizedString(IDS_TOOLBAR_LIVE_PARTITION));
		SetImage(views::Button::STATE_NORMAL, *GetImageSkiaNamed(IDR_LIVEHIME_PARTITION_BAR));
		SetImage(views::Button::STATE_HOVERED, *GetImageSkiaNamed(IDR_LIVEHIME_PARTITION_BAR_HOV));
        set_focusable(true);
    } 
    else if(bt_type == BtType_TalkSubject)
    {
        SetText(L"#选择话题");
		SetImage(views::Button::STATE_NORMAL, *GetImageSkiaNamed(IDR_LIVEHIME_TOPIC_BAR));
		SetImage(views::Button::STATE_HOVERED, *GetImageSkiaNamed(IDR_LIVEHIME_TOPIC_BAR_HOV));
    }
	SetFont(ftTwelve);
	SetFixedBkColor(SkColorSetRGB(0xF4, 0xF4, 0xF4));
	set_round_corner(true);
	SetRadius(GetLengthByDPIScale(2));
}


//添加素材按钮
MaterialControlButton::MaterialControlButton(views::ButtonListener* listener,MaterialButtonType bt_type)
	: BililiveLabelButton(listener, L"")
{
	if (bt_type == BtType_Add)
	{
		material_image_normal_ = GetImageSkiaNamed(IDR_LIVEHIME_V4_ADD_NORMAL);
		material_image_hover_ = GetImageSkiaNamed(IDR_LIVEHIME_V4_ADD_HOV);
		material_image_press_ = GetImageSkiaNamed(IDR_LIVEHIME_V4_ADD_HOV);
        SetText(GetLocalizedString(IDS_TABAREA_SCENE_ADD_MATERIALS));
	}
	else if (bt_type == BtType_Clear)
	{
		material_image_normal_ = GetImageSkiaNamed(IDR_LIVEHIME_V4_CLEAN_NORMAL);
		material_image_hover_ = GetImageSkiaNamed(IDR_LIVEHIME_V4_CLEAN_HOV);
		material_image_press_ = GetImageSkiaNamed(IDR_LIVEHIME_V4_CLEAN_HOV);
        SetText(GetLocalizedString(IDS_TABAREA_SCENE_CLEAR_MATERIALS));
	}
	else if (bt_type == BtType_Plugin)
	{
		material_image_normal_ = GetImageSkiaNamed(IDR_LIVEHIME_V4_PLUGIN);
		material_image_hover_ = GetImageSkiaNamed(IDR_LIVEHIME_V4_PLUGIN_HOV);
		material_image_press_ = GetImageSkiaNamed(IDR_LIVEHIME_V4_PLUGIN_HOV);
		SetText(GetLocalizedString(IDS_TABAREA_SCENE_PLUGIN));
	}

	SetTextColor(Button::STATE_NORMAL, GetColor(LabelTitle));
	SetTextColor(Button::STATE_HOVERED, SkColorSetRGB(0x0E, 0xBE, 0xFF));
	SetTextColor(Button::STATE_PRESSED, SkColorSetRGB(0x0E, 0xBE, 0xFF));
	SetImage(Button::STATE_NORMAL, *material_image_normal_);
	SetImage(Button::STATE_HOVERED, *material_image_hover_);
	SetImage(Button::STATE_PRESSED, *material_image_press_);
	SetPreferredSize(GetSizeByDPIScale(gfx::Size(85, 14)));
	SetFont(ftPrimary);
	set_round_corner(false);
}


