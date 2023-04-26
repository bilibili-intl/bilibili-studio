
#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_SLIDER_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_SLIDER_H

#include "base/basictypes.h"
#include "ui/views/controls/slider.h"
#include "ui/views/controls/label.h"

#include "bililive/bililive/livehime/help_center/help_center.h"



class BililiveSlider : 
    public views::Slider,
    public livehime::HelpSupport
{
public:
    enum SliderBarAlignment
    {
        SBA_LEADING,
        SBA_CENTER,
        SBA_TRAILING,
    };

    BililiveSlider(views::SliderListener* listener, Orientation orientation, SliderBarAlignment align = SBA_CENTER);
    virtual ~BililiveSlider(){}

    void SetMouseWheelIncrement(float increment){ mousewheel_increment_ = increment; }
    void SetAcceptKeyPress(bool accept){ accept_key_press_ = accept; }
    void SetAcceptMouseWheel(bool accept){ accept_mouse_wheel_ = accept; }
    void SetScrollLength(unsigned int length){ scroll_line_length_ = length; }
    void SetAssociationLabel(views::Label *label){ association_label_ = label; }
    views::Label* GetAssociationLabel() const { return association_label_; }
    void SetAssociationName(const base::string16 &name){ association_name_ = name; }
    base::string16 GetAssociationName() const { return association_name_; }
    void SetBarDetails(int barsize, SkColor clrFull, SkColor clrEmpty, bool repaint = true);
    void SetThumbImage(const gfx::ImageSkia *thumb_nor, const gfx::ImageSkia *thumb_hv, const gfx::ImageSkia *thumb_dis);
    void SetDrawInsets(const gfx::Insets &insets);

protected:
    // views::View overrides:
    void OnPaint(gfx::Canvas* canvas) override;
    void OnMouseEntered(const ui::MouseEvent& event) override;
    void OnMouseExited(const ui::MouseEvent& event) override;
    bool OnMousePressed(const ui::MouseEvent& event) override;
    bool OnMouseWheel(const ui::MouseWheelEvent& event) override;
    bool OnKeyPressed(const ui::KeyEvent& event) override;
    gfx::Size GetPreferredSize() override;
    bool HitTestRect(const gfx::Rect& rect) const override;
    void OnEnabledChanged() override;

    gfx::Rect GetContentsBoundsImp() const;
    Orientation orientation() const;

protected:
    SliderBarAlignment alignment_;
    const gfx::ImageSkia* thumb_nor_;
    const gfx::ImageSkia* thumb_hv_;
    const gfx::ImageSkia* thumb_dis_;
    float mousewheel_increment_;
    views::Label *association_label_;
    base::string16 association_name_;
    bool accept_key_press_;
    bool accept_mouse_wheel_;
    unsigned int scroll_line_length_;
    int scrollbar_thickness_;
    SkColor full_color_;
    SkColor empty_color_;
    gfx::Insets draw_insets_;

private:
    bool mouse_enter_;

    DISALLOW_COPY_AND_ASSIGN(BililiveSlider);
};


#endif  // BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_COMBOX_BILILIVE_COMBOBOX_H
