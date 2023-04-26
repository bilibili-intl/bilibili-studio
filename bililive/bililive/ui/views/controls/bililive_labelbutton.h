
#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_LABEL_BUTTON_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_LABEL_BUTTON_H

#include "base/basictypes.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/button/label_button.h"

#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/bililive/ui/views/controls/bililive_theme_common.h"



class BililiveLabelButton :
    public views::LabelButton,
    public livehime::SupportsEventReportV2
{
public:
    struct ColorStruct
    {
        bool set_disable = false;
        SkColor border_clrs[views::Button::STATE_COUNT];
        SkColor bkg_clrs[views::Button::STATE_COUNT];
        SkColor text_clrs[views::Button::STATE_COUNT];
    };

    enum BililiveButtonStyle {
        STYLE_UNREGISTERED = -2,
        STYLE_DEFAULT = -1,
        STYLE_WHITETEXT_BLUEBK,
        STYLE_BLUETEXT_WHITEBK,
        STYLE_GRAYTEXT_WHITEBK,

        STYLE_WHITETEXT_PINKBK,
        STYLE_PINKTEXT_WHITEBK,
        STYLE_GRAYTEXT_GRAYBK,
        STYLE_BLACKTEXT_GRAYBK,
        STYLE_BLACKTEXT_WHITEBK,

        STYLE_INTL_WHITETEXT_GRAYBK,
        STYLE_INTL_WHITETEXT_BLUEBK,


        STYLE_LIVEHIME = 0x10,
        STYLE_UGC = 0x20,
        STYLE_BILILIVE = 0x30,
    };

    static void RegisterButtonStyles(int style, const ColorStruct &clrs);
    static bool GetButtonStyles(int style, ColorStruct &clrs);

    BililiveLabelButton(views::ButtonListener* listener, const string16& text);
    virtual ~BililiveLabelButton(){}

    // 单纯依靠代码绘制
    void Enlarge(int width, int height);
    void SetStyle(int style, bool repaint = false);
    void SetStyle(const ColorStruct& color_details, bool repaint = false);
    const ColorStruct& GetStyle() const { return color_details_; }
    void set_round_corner(bool round){ round_corner_ = round; }
    void SetRadius(int radius) { radius_ = radius; }
   
    void SetCursor(gfx::NativeCursor cursor){ cursor_ = cursor; }
    void SetPreferredSize(const gfx::Size &size){ pref_size_ = size; }
    void SetFixedBkColor(SkColor bkColor);
    SkColor GetNormalBkColor(); //获取待机时按钮的背景色
    void SetAllStateTextColor(SkColor color);

    void SetImageSize(const gfx::Size& size);
    void SetAllStateImage(const gfx::ImageSkia& image);

    void set_no_subpixel_rendering(bool no_subpixel_rendering) {
        label()->set_no_subpixel_rendering(no_subpixel_rendering);
    }
    void SetOpacity(int opacity) {opacity_ = opacity;};
    // View
    virtual gfx::Size GetPreferredSize() override;
    void SetDrawUnderline(bool draw) { draw_underline_ = draw; }
    void SetTextColorEx(views::Button::ButtonState for_state, const SkColor &color);
    void SetCustomInsets(bool is_use, const gfx::Insets & insets);
    void SetDrawBorder(bool is_draw) { is_draw_border_ = is_draw; };
protected:
    // View
    void ChildPreferredSizeChanged(views::View* child) override;
    void OnPaint(gfx::Canvas* canvas) override;
    gfx::NativeCursor GetCursor(const ui::MouseEvent& event) override;
    gfx::Insets GetInsets() const override;
    // Button
    void NotifyClick(const ui::Event& event) override;

private:
    gfx::NativeCursor cursor_;

    int bililive_btn_style_;
    ColorStruct color_details_;
    bool round_corner_;

    gfx::Size pref_size_;
    int enlarge_cx_;
    int enlarge_cy_;
    //radius_ 设置DPI值
    int radius_;
    int opacity_ = 255;
    bool is_fixed_bk_;
    SkColor fixed_bk_color_;

    bool is_set_all_text_clr_;
    SkColor all_text_color_;
    bool draw_underline_;

    bool is_use_custom_insets_ = false;
    gfx::Insets custom_insets_ = gfx::Insets(0,0,0,0);
    bool is_draw_border_ = true;

    DISALLOW_COPY_AND_ASSIGN(BililiveLabelButton);
};

class BililiveLabelButtonStyleRegister
{
public:
    BililiveLabelButtonStyleRegister();
    ~BililiveLabelButtonStyleRegister(){}
};

extern BililiveLabelButtonStyleRegister labelbutton_reg_obj;

#endif  // BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_LABEL_BUTTON_H
