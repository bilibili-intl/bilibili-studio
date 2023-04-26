#ifndef BILILIVE_BILILIVE_UI_VIEW_LIVEHIME_MAIN_VIEW_FAUX_AUDIENT_EFFECT_H_
#define BILILIVE_BILILIVE_UI_VIEW_LIVEHIME_MAIN_VIEW_FAUX_AUDIENT_EFFECT_H_

#include "ui/views/view.h"
#include "ui/views/controls/button/button.h"

namespace
{
    class FauxAudientEffectImageView;
}

class LivehimeOBSPreviewControl;
class BililiveLabelButton;
class BililiveFloatingView;

class FauxAudientEffectView : 
    public views::View,
    views::ButtonListener
{
public:
    FauxAudientEffectView();

    void SetOrientation(bool landscape);

protected:
    // View
    void ViewHierarchyChanged(
        const ViewHierarchyChangedDetails &details) override;
    void Layout() override;
    void VisibilityChanged(View* starting_from, bool is_visible) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

private:
    void InitViews();

private:
    using super = View;

    bool landscape_ = true;

    LivehimeOBSPreviewControl* obs_preview_control_ = nullptr;
    BililiveFloatingView* floating_view_ = nullptr;
    FauxAudientEffectImageView* faux_img_view_;
    BililiveLabelButton* layout_suggestion_button_ = nullptr;
};

#endif  // BILILIVE_BILILIVE_UI_VIEW_LIVEHIME_MAIN_VIEW_FAUX_AUDIENT_EFFECT_H_