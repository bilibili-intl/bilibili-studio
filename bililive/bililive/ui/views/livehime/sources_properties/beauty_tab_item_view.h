#pragma once

//#include "bililive/bililive/livehime/sources_properties/beauty_camera_controller.h"
#include "bililive/bililive/ui/views/controls/tab_bar_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"

//ÃÀÑÕÑ¡Ïî¿¨
class BeautyTabItemView :
    public TabBarItemView/*,
    public BeautyCameraItem*/
{
public:
    BeautyTabItemView(TabBarView* parent, const base::string16& text);
    ~BeautyTabItemView();

    const base::string16& GetTagText();
    void SetPreferredSize(const gfx::Size& size);
    gfx::Size GetPreferredSize() override;
    void OnPaint(gfx::Canvas* canvas) override;

protected:
    bool OnSelect() override;
    void OnSelectChanged(int select_index) override;

private:
    //BeautyClassTag tag_;
    bool select_ = false;
    base::string16 text_;
    gfx::Size perferred_size_ = GetSizeByDPIScale({ 60, 30 });;

};

