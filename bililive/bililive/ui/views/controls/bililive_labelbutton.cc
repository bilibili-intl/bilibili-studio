#include "bililive_labelbutton.h"

#include "bililive/bililive/ui/views/uploader/bililive_uploader_type.h"
#include "bililive/bililive/ui/views/controls/bililive_theme_common.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/views/layout/fill_layout.h"
#include "ui/gfx/canvas.h"

#include "third_party/skia/include/core/SkPaint.h"

#include "ui/base/resource/resource_bundle.h"
#include "grit/theme_resources.h"


// BililiveLabelButton

namespace
{
    bool AccessStyleSets(int style, BililiveLabelButton::ColorStruct &clrs, bool set_mode)
    {
        static std::map<int, BililiveLabelButton::ColorStruct> style_map_;

        if (set_mode)
        {
            if (style_map_.find(style) == style_map_.end())
            {
                style_map_[style] = clrs;
                return true;
            }
            else
            {
                NOTREACHED() << "style should registed only once";
            }
        }
        else
        {
            if (style_map_.find(style) != style_map_.end())
            {
                clrs = style_map_[style];
                return true;
            }
        }
        return false;
    }
}

// static
void BililiveLabelButton::RegisterButtonStyles(int style, const ColorStruct& clrs)
{
    AccessStyleSets(style, const_cast<ColorStruct&>(clrs), true);
}

bool BililiveLabelButton::GetButtonStyles(int style, ColorStruct &clrs)
{
    return AccessStyleSets(style, clrs, false);
}

BililiveLabelButton::BililiveLabelButton(views::ButtonListener* listener, const string16& text)
    : views::LabelButton(listener, text)
    , round_corner_(true)
    , is_fixed_bk_(false)
    , is_set_all_text_clr_(false)
    , enlarge_cx_(0)
    , enlarge_cy_(0)
    , radius_(4)
    , cursor_(NULL)
    , draw_underline_(false)
{
    SetStyle(STYLE_BLUETEXT_WHITEBK);
    SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_CENTER);
    set_focus_border(nullptr);
    set_focusable(true);
}

void BililiveLabelButton::SetStyle(int style, bool repaint/* = false*/)
{
    is_fixed_bk_ = is_set_all_text_clr_ = false;
    bililive_btn_style_ = style;
    if (BililiveLabelButton::STYLE_DEFAULT != bililive_btn_style_)
    {
        bool bRet = GetButtonStyles(bililive_btn_style_, color_details_);
        DCHECK(bRet);

        SetStyle(color_details_, repaint);
        bililive_btn_style_ = style;
    }
    else
    {
        set_border(nullptr);
    }
    if (repaint)
    {
        SchedulePaint();
    }
}

void BililiveLabelButton::SetStyle(const ColorStruct& color_details, bool repaint /*= false*/)
{
    bililive_btn_style_ = BililiveLabelButton::STYLE_UNREGISTERED;
    color_details_ = color_details;

    SetTextColor(Button::STATE_NORMAL, color_details_.text_clrs[Button::STATE_NORMAL]);
    SetTextColor(Button::STATE_HOVERED, color_details_.text_clrs[Button::STATE_HOVERED]);
    SetTextColor(Button::STATE_PRESSED, color_details_.text_clrs[Button::STATE_PRESSED]);
    if (!color_details_.set_disable)
    {
        SetTextColor(Button::STATE_DISABLED, GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_ButtonDisabledColor));
    }
    else
    {
        SetTextColor(Button::STATE_DISABLED, color_details_.text_clrs[Button::STATE_DISABLED]);
    }
    if (repaint)
    {
        SchedulePaint();
    }
}

void BililiveLabelButton::SetTextColorEx(views::Button::ButtonState for_state, const SkColor& color)
{
    color_details_.text_clrs[for_state] = color;
    SetTextColor(for_state, color);
}

void BililiveLabelButton::Enlarge(int width, int height)
{
    enlarge_cx_ = width;
    enlarge_cy_ = height;
}

gfx::Size BililiveLabelButton::GetPreferredSize()
{
    set_min_size(gfx::Size());
    gfx::Size si = __super::GetPreferredSize();
    si.Enlarge(enlarge_cx_, enlarge_cy_);
    si.SetToMax(pref_size_);
    return si;
}

gfx::NativeCursor BililiveLabelButton::GetCursor(const ui::MouseEvent& event)
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

void BililiveLabelButton::SetCustomInsets(bool is_use, const gfx::Insets& insets)
{
    is_use_custom_insets_ = is_use;
    custom_insets_ = insets;
}

gfx::Insets BililiveLabelButton::GetInsets() const 
{
    if (is_use_custom_insets_)
    {
        return custom_insets_;
    }
    return __super::GetInsets();
}

void BililiveLabelButton::NotifyClick(const ui::Event& event)
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

void BililiveLabelButton::ChildPreferredSizeChanged(views::View* child)
{
    InvalidateLayout();
    Layout();
}

void BililiveLabelButton::OnPaint(gfx::Canvas* canvas)
{
    if (bililive_btn_style_ == STYLE_DEFAULT)
    {
        __super::OnPaint(canvas);
        if (draw_underline_)
        {
			auto bound = GetContentsBounds();
			views::Button::ButtonState st = state();
			SkColor clr = color_details_.text_clrs[st];
			SkPaint paint;
			paint.setColor(clr);
			canvas->DrawLine(gfx::Point(0, bound.height() - 1), gfx::Point(bound.width(), bound.height() - 1), paint);
        }
        return;
    }

    OnPaintBackground(canvas);

    views::Button::ButtonState st = state();
    SkColor clrBk = color_details_.bkg_clrs[st];
    SkColor clrBorder = color_details_.border_clrs[st];

    if (st == views::Button::STATE_DISABLED)
    {
        if (!color_details_.set_disable)
        {
            clrBorder = GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_ButtonDisabledColor);
            clrBk = GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_ButtonBackgroundColor);
        }
    }

    if (is_fixed_bk_)
    {
        clrBk = fixed_bk_color_;
    }

    gfx::Rect rect = GetLocalBounds();
    SkPaint paint;
    paint.setAntiAlias(true);
    if (round_corner_)
    {
        int radius = GetLengthByDPIScale(radius_);
        paint.setColor(opacity_ == 255 ? clrBk : SkColorSetA(clrBk, opacity_)); //背景颜色
        bililive::FillRoundRect(canvas, 0, 0, width(), height(), radius, paint);

        //绘制边线
        if (is_draw_border_)
        {
			paint.setColor(opacity_ == 255 ? clrBorder : SkColorSetA(clrBorder, opacity_)); //线框颜色
            bililive::DrawRoundRect(canvas, 0, 0, width(), height(), radius, paint);
        }
    }
    /*else
    {
        canvas->FillRect(rect, clrBk);
        rect.Inset(0, 0, 1, 1);
        canvas->DrawRect(rect, clrBorder);
    }*/
}

void BililiveLabelButton::SetImageSize(const gfx::Size& size) {
    image_view()->SetImageSize(size);
}

void BililiveLabelButton::SetAllStateImage(const gfx::ImageSkia& image)
{
    SetImage(Button::STATE_NORMAL, image);
    SetImage(Button::STATE_HOVERED, image);
    SetImage(Button::STATE_PRESSED, image);
    SetImage(Button::STATE_DISABLED, image);
}

void BililiveLabelButton::SetAllStateTextColor(SkColor color)
{
    is_set_all_text_clr_ = true;
    all_text_color_ = color;
    SetTextColor(Button::STATE_NORMAL, color);
    SetTextColor(Button::STATE_HOVERED, color);
    SetTextColor(Button::STATE_PRESSED, color);
    SetTextColor(Button::STATE_DISABLED, color);
}

void BililiveLabelButton::SetFixedBkColor(SkColor bkColor)
{
    is_fixed_bk_ = true;
    fixed_bk_color_ = bkColor;
    SchedulePaint();
}

SkColor BililiveLabelButton::GetNormalBkColor()
{
    if (is_fixed_bk_)
    {
        return fixed_bk_color_;
    }
    if (bililive_btn_style_ != STYLE_DEFAULT)
    {
        return color_details_.bkg_clrs[views::Button::ButtonState::STATE_NORMAL];
    }
    return SK_ColorWHITE;
}



// BililiveLabelButtonStyleRegister
BililiveLabelButtonStyleRegister labelbutton_reg_obj;

BililiveLabelButtonStyleRegister::BililiveLabelButtonStyleRegister()
{
    BililiveLabelButton::ColorStruct clrs;
    //
    clrs.set_disable = false;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = clrFrameAndButtonBk;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = clrFrameAndButtonBk;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = clrButtonHoverBk;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = clrButtonHoverBk;
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = clrButtonPressBk;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = clrButtonPressBk;
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = SK_ColorWHITE;

    //clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_ButtonDisabledColor);
    //clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_ButtonBackgroundColor);
    //clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = GetNativeTheme()->GetSystemColor(ui::NativeTheme::kColorId_ButtonDisabledColor);
    //style_map_[STYLE_WHITETEXT_BLUEBK] = clrs;
    BililiveLabelButton::RegisterButtonStyles(BililiveLabelButton::STYLE_WHITETEXT_BLUEBK, clrs);

    //
    clrs.set_disable = false;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = clrFrameAndButtonBk;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorWHITE;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = clrFrameAndButtonBk;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = clrButtonHoverBk;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = SK_ColorWHITE;
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = clrButtonHoverBk;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = clrButtonPressBk;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = SK_ColorWHITE;
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = clrButtonPressBk;

    //clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = clrFrameAndButtonBk;
    //clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = clrFrameAndButtonBk;
    //clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = clrFrameAndButtonBk;
    //style_map_[STYLE_BLUETEXT_WHITEBK] = clrs;
    BililiveLabelButton::RegisterButtonStyles(BililiveLabelButton::STYLE_BLUETEXT_WHITEBK, clrs);

    //
    clrs.set_disable = false;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = clrLabelTipText;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorWHITE;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = clrLabelTipText;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = clrLabelText;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = SK_ColorWHITE;
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = clrLabelText;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = clrLabelOperateText;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = SK_ColorWHITE;
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = clrLabelOperateText;

    //clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = clrFrameAndButtonBk;
    //clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = clrFrameAndButtonBk;
    //clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = clrFrameAndButtonBk;
    //style_map_[STYLE_GRAYTEXT_WHITEBK] = clrs;
    BililiveLabelButton::RegisterButtonStyles(BililiveLabelButton::STYLE_GRAYTEXT_WHITEBK, clrs);

    //
    clrs.set_disable = false;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = bililive_uploader::clrPink;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = bililive_uploader::clrPink;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = bililive_uploader::clrHoverPink;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = bililive_uploader::clrHoverPink;
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = bililive_uploader::clrHoverPink;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = bililive_uploader::clrHoverPink;
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = SK_ColorWHITE;
    //clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = clrFrameAndButtonBk;
    //clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = clrFrameAndButtonBk;
    //clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = clrFrameAndButtonBk;
    //style_map_[STYLE_WHITETEXT_PINKBK] = clrs;
    BililiveLabelButton::RegisterButtonStyles(BililiveLabelButton::STYLE_WHITETEXT_PINKBK, clrs);

    //
    clrs.set_disable = false;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = bililive_uploader::clrPink;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorWHITE;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = bililive_uploader::clrPink;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = bililive_uploader::clrHoverPink;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = SK_ColorWHITE;
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = bililive_uploader::clrHoverPink;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = bililive_uploader::clrHoverPink;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = SK_ColorWHITE;
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = bililive_uploader::clrHoverPink;
    //clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = clrFrameAndButtonBk;
    //clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = clrFrameAndButtonBk;
    //clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = clrFrameAndButtonBk;
    //style_map_[STYLE_PINKTEXT_WHITEBK] = clrs;
    BililiveLabelButton::RegisterButtonStyles(BililiveLabelButton::STYLE_PINKTEXT_WHITEBK, clrs);

    //
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = bililive_uploader::clrButtonGrayBorder;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = bililive_uploader::clrButtonGrayBk;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = bililive_uploader::clrGrayText;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = bililive_uploader::clrButtonGrayBorder;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = bililive_uploader::clrButtonHoverGrayBk;
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = bililive_uploader::clrGrayText;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = bililive_uploader::clrButtonGrayBorder;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = bililive_uploader::clrButtonHoverGrayBk;
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = bililive_uploader::clrGrayText;

    clrs.set_disable = true;
    clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = bililive_uploader::clrButtonGrayBorderDis;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = bililive_uploader::clrButtonGrayBkDis;
    clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = bililive_uploader::clrButtonGrayTextDis;
    //style_map_[STYLE_GRAYTEXT_GRAYBK] = clrs;
    BililiveLabelButton::RegisterButtonStyles(BililiveLabelButton::STYLE_GRAYTEXT_GRAYBK, clrs);

    //
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = bililive_uploader::clrButtonGrayBorder;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = bililive_uploader::clrButtonGrayBk;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = bililive_uploader::clrBlackText;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = bililive_uploader::clrButtonGrayBorder;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = bililive_uploader::clrButtonHoverGrayBk;
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = bililive_uploader::clrBlackText;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = bililive_uploader::clrButtonGrayBorder;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = bililive_uploader::clrButtonHoverGrayBk;
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = bililive_uploader::clrBlackText;

    clrs.set_disable = true;
    clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = bililive_uploader::clrButtonGrayBorderDis;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = bililive_uploader::clrButtonGrayBkDis;
    clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = bililive_uploader::clrButtonGrayTextDis;
    //style_map_[STYLE_BLACKTEXT_GRAYBK] = clrs;
    BililiveLabelButton::RegisterButtonStyles(BililiveLabelButton::STYLE_BLACKTEXT_GRAYBK, clrs);

    //
    clrs.set_disable = false;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = bililive_uploader::clrButtonGrayBorder;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorWHITE;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = bililive_uploader::clrBlackText;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = bililive_uploader::clrButtonGrayBorder;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = SK_ColorWHITE;
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = bililive_uploader::clrBlackText;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = bililive_uploader::clrButtonGrayBorder;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = SK_ColorWHITE;
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = bililive_uploader::clrBlackText;

    clrs.set_disable = true;
    clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = bililive_uploader::clrButtonGrayBorderDis;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = bililive_uploader::clrButtonGrayBkDis;
    clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = bililive_uploader::clrButtonGrayTextDis;

    //style_map_[STYLE_BLACKTEXT_WHITEBK] = clrs;
    BililiveLabelButton::RegisterButtonStyles(BililiveLabelButton::STYLE_BLACKTEXT_WHITEBK, clrs);

    clrs.set_disable = false;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = clrIntlGrayButtonBk;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = clrIntlGrayButtonBk;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetA(clrIntlGrayButtonBk, 0.9f * 255);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetA(clrIntlGrayButtonBk, 0.9f * 255);
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetA(clrIntlGrayButtonBk, 0.9f * 255);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetA(clrIntlGrayButtonBk, 0.9f * 255);
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = SK_ColorWHITE;

    clrs.set_disable = true;
    clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = clrIntlGrayButtonBk;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = clrIntlGrayButtonBk;
    clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = SK_ColorWHITE;

    //style_map_[STYLE_BLACKTEXT_WHITEBK] = clrs;
    BililiveLabelButton::RegisterButtonStyles(BililiveLabelButton::STYLE_INTL_WHITETEXT_GRAYBK, clrs);

    clrs.set_disable = false;
    clrs.border_clrs[views::Button::ButtonState::STATE_NORMAL] = clrIntlBlueButtonBk;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_NORMAL] = clrIntlBlueButtonBk;
    clrs.text_clrs[views::Button::ButtonState::STATE_NORMAL] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetA(clrIntlBlueButtonBk, 0.9f*255);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_HOVERED] = SkColorSetA(clrIntlBlueButtonBk, 0.9f * 255);
    clrs.text_clrs[views::Button::ButtonState::STATE_HOVERED] = SK_ColorWHITE;

    clrs.border_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetA(clrIntlBlueButtonBk, 0.9f * 255);
    clrs.bkg_clrs[views::Button::ButtonState::STATE_PRESSED] = SkColorSetA(clrIntlBlueButtonBk, 0.9f * 255);
    clrs.text_clrs[views::Button::ButtonState::STATE_PRESSED] = SK_ColorWHITE;

    clrs.set_disable = true;
    clrs.border_clrs[views::Button::ButtonState::STATE_DISABLED] = clrIntlBlueButtonBk;
    clrs.bkg_clrs[views::Button::ButtonState::STATE_DISABLED] = clrIntlBlueButtonBk;
    clrs.text_clrs[views::Button::ButtonState::STATE_DISABLED] = SK_ColorWHITE;

    //style_map_[STYLE_BLACKTEXT_WHITEBK] = clrs;
    BililiveLabelButton::RegisterButtonStyles(BililiveLabelButton::STYLE_INTL_WHITETEXT_BLUEBK, clrs);
}

