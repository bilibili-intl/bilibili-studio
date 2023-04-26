
#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_PROGRESSBAR_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_PROGRESSBAR_H

#include "base/basictypes.h"

#include "ui/views/controls/progress_bar.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"



class BililiveProgressBar : public views::ProgressBar
{
public:
    BililiveProgressBar();
    virtual ~BililiveProgressBar(){}

    void SetBarDetails(int cy, SkColor clrFull, SkColor clrEmpty);
    void SetBarDetails(int cw, int ch,SkColor clrFull, SkColor clrEmpty);

protected:
	// views::View overrides:
	virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;
private:
    bool is_set_details_;
    int bar_height_;
    int bat_wight_;
    SkColor full_color_;
    SkColor empty_color_;

    gfx::ImageSkia* progress_img_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(BililiveProgressBar);
};


#endif  // BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_COMBOX_BILILIVE_COMBOBOX_H
