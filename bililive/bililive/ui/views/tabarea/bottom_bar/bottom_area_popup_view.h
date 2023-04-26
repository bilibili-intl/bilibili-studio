#ifndef BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_AREA_POPUP_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_TABAREA_BOTTOM_AREA_POPUP_VIEW_H

#include "ui/views/view.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include <map>

enum TabareaBottomFunction
{
    TabareaBottomFunction_Banned,
    TabareaBottomFunction_NameList,
    TabareaBottomFunction_Search,
    TabareaBottomFunction_Refresh,
    TabareaBottomFunction_COUNT,
};

extern const std::map<TabareaBottomFunction, bililive::NotificationType> kTabareaCloseTable;
extern const std::map<TabareaBottomFunction, bililive::NotificationType> kTabareaShowTable;

bool ShowTabAreaBottomBarFunctionView(TabareaBottomFunction function);

#endif