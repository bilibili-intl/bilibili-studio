#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_COLIVE_COLIVE_SPLITTER_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_COLIVE_COLIVE_SPLITTER_VIEW_H_

#include "SkPaint.h"
#include "ui/views/view.h"


class ColiveSplitterView : public views::View {
public:
    ColiveSplitterView(int thickness, SkColor color);

    void SetColor(SkColor color);

    gfx::Size GetPreferredSize() override;
    void OnPaint(gfx::Canvas* canvas) override;

private:
    using super = View;

    SkPaint paint_;
    int thickness_;
    SkColor color_;
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_COLIVE_COLIVE_SPLITTER_VIEW_H_