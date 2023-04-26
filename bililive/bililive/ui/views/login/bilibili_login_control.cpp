#include "bilibili_login_control.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/common/bililive_context.h"

#include "ui/base/resource/resource_bundle.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"

// BilibiliLoginLabelButton
BilibiliLoginLabelButton::BilibiliLoginLabelButton(views::ButtonListener* listener, const string16& text)
    : BililiveLabelButton(listener, text)
{
    static bool isreg = false;
    if (!isreg)
    {
        isreg = true;
        BililiveLabelButton::ColorStruct clrs;

        clrs.set_disable = false;
        clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = clrButtonPositive;
        clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = clrButtonPositive;
        clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorWHITE;

        clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = clrButtonPositiveHov;
        clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = clrButtonPositiveHov;
        clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = SK_ColorWHITE;

        clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = clrButtonPositivePre;
        clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = clrButtonPositivePre;
        clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = SK_ColorWHITE;

        BililiveLabelButton::RegisterButtonStyles(BililiveButtonStyle::STYLE_BILILIVE, clrs);
    }

    SetStyle(BililiveButtonStyle::STYLE_BILILIVE);
    SetPreferredSize(gfx::Size(LivehimePaddingCharWidth(ftFourteen) * 15, ftFourteen.GetHeight() * 1.5f));
    SetFont(ftPrimary);

}

//LivehimeTextfield
LoginTextfield::LoginTextfield(StyleFlags style) : BilibiliNativeEditView(style)
{
}

LoginTextfield::LoginTextfield(views::View *view, CooperateDirection dir, StyleFlags style)
    : BilibiliNativeEditView(view, dir, style)
{
}

LoginTextfield::~LoginTextfield()
{
}

gfx::Size LoginTextfield::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    size.set_height(kBorderCtrlHeight);
    return size;
}

int LoginTextfield::GetHeightForWidth(int w)
{
    return GetPreferredSize().height();
}

void LoginTextfield::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.is_add && details.child == this)
    {
        SetCommonStyle();
    }

    BilibiliNativeEditView::ViewHierarchyChanged(details);
}

void LoginTextfield::SetCommonStyle()
{
    SetBorderColor(clrCtrlBorderNor, clrCtrlBorderHov);
    SetTextColor(clrTextTip);
    SetFont(ftTwelve);
    SetHorizontalMargins(kCtrlLeftInset, kCtrlRightInset);
}