#ifndef BILILIVE_BILILIVE_UI_VIEWS_TABAREA_TOAST_VIEW_H
#define BILILIVE_BILILIVE_UI_VIEWS_TABAREA_TOAST_VIEW_H

#include "base/strings/string16.h"

enum TabAreaToastType
{
    TabAreaToastType_Default,
    TabAreaToastType_Warning,
};

void ShowTabAreaToast(TabAreaToastType type, const base::string16 &text);


#endif