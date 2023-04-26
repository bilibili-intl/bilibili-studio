#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_COLIVE_PK_RECORD_POPUP_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_COLIVE_PK_RECORD_POPUP_VIEW_H

#include "ui/views/view.h"


class BililiveWidgetDelegate;

namespace bililive
{
    enum class PanelType {
        Left,
        Right
    };

    bool ShowPanelView(
        BililiveWidgetDelegate* main_window,
        views::Widget* pk_operate_widget,
        PanelType panel_type, views::View* content_view, int view_width);
}


#endif