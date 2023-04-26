#ifndef BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_WIDGET_DELEGATE_BILILIVE_MASK_WIDGET_H_
#define BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_WIDGET_DELEGATE_BILILIVE_MASK_WIDGET_H_

#include "ui/gfx/image/image_skia.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_observer.h"


class BililiveMaskWidget
    : public views::Widget
{
public:
    BililiveMaskWidget();
    virtual ~BililiveMaskWidget();

    static BililiveMaskWidget* ShowMaskWidget(HWND parWnd);

protected:

private:
    DISALLOW_COPY_AND_ASSIGN(BililiveMaskWidget);
};



#endif  // BILILIVE_BILILIVE_UI_VIEWS_CONTROLS_BILILIVE_WIDGET_DELEGATE_BILILIVE_WIDGET_DELEGATE_H_