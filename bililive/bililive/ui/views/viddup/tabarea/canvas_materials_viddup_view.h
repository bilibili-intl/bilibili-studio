#ifndef BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TABAREA_CANVAS_MATERIALS_WIDGET_H_
#define BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TABAREA_CANVAS_MATERIALS_WIDGET_H_

#include "ui/views/controls/button/button.h"
#include "ui/views/widget/widget_observer.h"

#include "bililive/bililive/livehime/live_model/live_model_controller.h"

namespace {
    class MaterialsButtonView;
}

class BililiveBubbleView;

class CanvasMaterialsViddupView
    : public views::View
    , LiveModelControllerObserver

{
public:
    CanvasMaterialsViddupView(views::View* parent,bool land_model);

protected:
    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails &details) override;
    void OnPaintBackground(gfx::Canvas* canvas) override;
    void Layout() override;

    // LiveModelControllerObserver
    void OnLiveLayoutModelChanged(bool user_invoke) override;

private:
    void InitViews();

    void LayoutPortraitVtuber();
    void LayoutOthers();

    views::View* area_delegate_ = nullptr;
    bool is_land_model_;
    bool is_vtuber_opened_ = false;
 

    MaterialsButtonView* add_btn_ = nullptr;
    MaterialsButtonView* window_btn_ = nullptr;
    MaterialsButtonView* proj_btn_ = nullptr;
    MaterialsButtonView* cam_btn_ = nullptr;
    MaterialsButtonView* vtb_btn_ = nullptr;
    MaterialsButtonView* more_btn_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(CanvasMaterialsViddupView);
};

#endif  //BILILIVE_BILILIVE_UI_VIEWS_VIDDUP_TABAREA_CANVAS_MATERIALS_WIDGET_H_