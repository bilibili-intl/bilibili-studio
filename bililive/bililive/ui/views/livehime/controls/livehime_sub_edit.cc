#include "livehime_sub_edit.h"
#include "livehime_hover_tip_button.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"


LivehimeSubEditView::LivehimeSubEditView(StyleFlags style)
    : BilibiliNativeEditView(style)
{
    SetCommonStyle();
}

LivehimeSubEditView::LivehimeSubEditView(views::View *view,
    CooperateDirection dir, StyleFlags style)
    : BilibiliNativeEditView(view, CooperateDirection::BD_CORNER, style)
{
    set_notify_enter_exit_on_child(false);
    SetCommonStyle();
    SetSkipDefaultKeyEventProcessing(true);
    LimitText(80);
    SetMultiLine(true, false, true);
}

LivehimeSubEditView::~LivehimeSubEditView()
{
}

gfx::Size LivehimeSubEditView::GetPreferredSize()
{
    gfx::Size size = __super::GetPreferredSize();
    return size;
}

int LivehimeSubEditView::GetHeightForWidth(int w)
{
    return GetPreferredSize().height();
}

void LivehimeSubEditView::SetCommonStyle()
{
    SetBackgroundColor(clrWindowsContent);
    SetBorderColor(clrCtrlBorderNor, clrCtrlBorderHov);
    SetTextColor(clrLabelContent);
    SetFont(ftPrimary);
    set_placeholder_text_color(clrPlaceholderText);
    SetHorizontalMargins(kCtrlLeftInset, kCtrlRightInset);
}

