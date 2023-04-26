
#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_LINK_H
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_LINK_H

#include "base/basictypes.h"
#include "ui/views/controls/link.h"
#include "bililive/bililive/ui/views/controls/bililive_theme_common.h"



class BililiveLink : public views::Link
{
public:
    BililiveLink()
    {
        initParam();
    }
    explicit BililiveLink(const string16& text)
        : views::Link(text)
    {
        initParam();
    }
    virtual ~BililiveLink(){}

    void SetTextColor(SkColor clr);
    void SetVerticalAlignment(gfx::VerticalAlignment alignment){ vertical_alignment_ = alignment; }

protected:
    void initParam()
    {
        vertical_alignment_ = gfx::ALIGN_VCENTER;
        SetUnderline(false);
        SetAutoColorReadabilityEnabled(false);
        SetEnabledColor(clrLabelText);
        SetDisabledColor(clrLabelDisableText);
    }

    // view
    virtual void PaintText(gfx::Canvas* canvas,
        const string16& text,
        const gfx::Rect& text_bounds,
        int flags) OVERRIDE;

private:
    gfx::VerticalAlignment vertical_alignment_;

    DISALLOW_COPY_AND_ASSIGN(BililiveLink);
};


#endif  // BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_COMBOX_BILILIVE_COMBOBOX_H
