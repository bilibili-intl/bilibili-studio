#pragma once

#include <memory>

#include "ui/base/view_prop.h"


namespace livehime
{
    enum class UIPropType
    {
        Unknown = -1,
        AppMngBtn,
        CpmEntranceBtn,
    };

    std::unique_ptr<ui::ViewProp> SetUIProp(UIPropType ui_type, void* data);
    void* GetUIProp(UIPropType ui_type);
}
