#pragma once

#include "bililive/bililive/livehime/sources_properties/source_camera_property_contract.h"

#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/tab_bar_view.h"
#include "bililive/bililive/ui/views/controls/tab_select_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_image_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/custom_style_button.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/base_setup_view.h"
//#include "bililive/bililive/ui/views/livehime/sources_properties/beauty_camera_hover_btn.h"
#include <bililive/bililive/ui/views/livehime/sources_properties/beauty_base_item_view.h>
#include <unordered_map>
//#include "bililive/bililive/livehime/sources_properties/beauty_camera_controller.h"

// ª˘¥°√¿—’…Ë÷√“≥
class BeautyBaseView :
    public BaseSetupView,
    public views::ButtonListener,
    public TabBarViewListener
{
    //using BeautyEffectiveControl = BeautyCameraController::BeautyEffectiveControl;

public:
    explicit BeautyBaseView();

    virtual ~BeautyBaseView();

    //void UpdateIcon(int id, const gfx::ImageSkia& nor_icon, const gfx::ImageSkia& sel_icon);

    //void RefreshUi(const BeautyViewData& data);

    //void ReloadUi(const BeautyViewData& data);

    bool UpdateSelect(bool report = true);

    void InitView() override;

    void InitData() override;

    bool CheckSetupValid() override;

    bool SaveSetupChange() override;

    //bool Cancel() override;

protected:
    //TabBarViewListener
    bool TabItemSelect(int index, TabBarView* tab_view) override;
    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

private:
    bool                                                                    is_loading_ = false;
    //BeautyCameraHoverButton*                                                reset_btn_ = nullptr;
    //BeautyCameraHoverButton*                                                beauty_btn_ = nullptr;
    TabSelectView*                                                          beauty_tab_select_view_ = nullptr;
    TabBarView*                                                             tab_view_ = nullptr;
    TabBarView*                                                             shrink_content_view_ = nullptr;
    livehime::MessageBoxEndDialogSignalHandler                              reset_handler_;

    //std::map<BeautyEffectiveControl, std::string>                           control_text_map_;
    std::unordered_map<int, BeautyBaseItemView*>                            mp_beauty_item_;
    base::WeakPtrFactory<BeautyBaseView>                                    wpf_;

    DISALLOW_COPY_AND_ASSIGN(BeautyBaseView);

    void MessageBoxCallback(const base::string16& button_text, void* data);
    void ClearShrinkModeValue();
};
