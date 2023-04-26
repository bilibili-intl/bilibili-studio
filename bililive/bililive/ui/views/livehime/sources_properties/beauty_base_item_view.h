#pragma once

#include "bililive/bililive/ui/views/controls/tab_bar_view.h"
//#include "bililive/bililive/livehime/sources_properties/beauty_camera_controller.h"

#include "ui/gfx/image/image_skia.h"

//基础美颜效果 item
class BeautyBaseItemView :
    public TabBarItemView/*,
    public BeautyCameraItem*/
{
public:
    BeautyBaseItemView(TabBarView* parent);
    ~BeautyBaseItemView();

    gfx::Size GetPreferredSize() override;

    void SetNew(bool new_effect);
    void SetSideRound(bool is_round);

    void SetImage(const gfx::ImageSkia& image);
    void SetSelImage(const gfx::ImageSkia& image);
    void SetText(const base::string16& text);
    void SetPercent(float percent);

protected:
    void OnPaint(gfx::Canvas* canvas);
    bool OnSelect();
    void OnSelectChanged(int select_index);

private:
    gfx::ImageSkia          img_;
    gfx::ImageSkia          sel_img_;
    gfx::ImageSkia          new_img_;
    base::string16          text_;
    const gfx::Size         img_size_;
    bool                    is_round_ = true;
    float                   percent_ = 0.0f;
    bool                    is_select_ = false;
    bool                    is_new_ = false;
};
