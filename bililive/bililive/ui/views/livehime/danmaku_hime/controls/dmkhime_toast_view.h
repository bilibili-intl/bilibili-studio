#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_CONTROLS_DMKHIME_TOAST_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_CONTROLS_DMKHIME_TOAST_VIEW_H_

#include "base/strings/string16.h"

#include "ui/gfx/rect.h"


namespace views {
    class Widget;
}

namespace dmkhime {

    enum class DmkToastType {
        Default,
        Warning,
    };

    void ShowDmkhimeToast(
        views::Widget* parent,
        const gfx::Rect& parent_bounds,
        DmkToastType type, const base::string16 &text);
    void CloseDmkhimeToast();

}

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_DANMAKU_HIME_CONTROLS_DMKHIME_TOAST_VIEW_H_