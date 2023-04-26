#pragma once

#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
//#include "bililive/bililive/livehime/sources_properties/beauty_camera_controller.h"
#include "ui/views/view.h"

class BeautySlideView;

class BeautyCameraControlPreviewView :
    public views::View,
    public views::ButtonListener
{
public:
    BeautyCameraControlPreviewView();
    ~BeautyCameraControlPreviewView();

    void ShowButton(bool show);
    void ShowSlider(bool show);
    //void BindMaterial(const BeautyClassTag& tag);

protected:
    //view
    void VisibilityChanged(View* starting_from, bool is_visible) override;

    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

private:
    //BeautyClassTag          tag_;
    BeautySlideView*        slider_view_ = nullptr;
    BililiveImageButton*    contrast_btn_ = nullptr;

    void InitView();
};
