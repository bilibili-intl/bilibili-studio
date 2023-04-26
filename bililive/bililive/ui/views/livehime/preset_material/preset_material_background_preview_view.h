#pragma once

#include "bililive/bililive/livehime/preset_material/preset_material_ui_presenter.h"
#include "bililive/bililive/ui/views/controls/bililive_floating_scroll_view.h"
#include "bililive/bililive/ui/views/livehime/controls/custom_style_radio_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"

class PresetMaterialBackgroundPreviewObserver
{
public:
    ~PresetMaterialBackgroundPreviewObserver() {}
    virtual void OnBkImageSelected(const BkImageInfo& bk_info) = 0;
    virtual void OnBkImageInitialized() = 0;

protected:
    PresetMaterialBackgroundPreviewObserver() {}
};

class PresetMaterialBackgroundPreviewView :
    public BililiveViewWithFloatingScrollbar,
    public views::ButtonListener,
    public PresetMaterialUIObserver
{
public:
    PresetMaterialBackgroundPreviewView(PresetMaterialBackgroundPreviewObserver* observer, bool is_horiz, bool is_vtuber_preset);
    ~PresetMaterialBackgroundPreviewView();

    void Layout() override;
    gfx::Size GetPreferredSize() override;
    //gfx::Size GetMinimumSize() override;
    
    bool SelectCheckBox(int index);
    BkImageInfo GetSelectedBkImageInfo();

protected:
    //views
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;
    
    //PresetMaterialUIObserver
    void OnPresetMaterialInitialized() override;
    void OnPresetMaterialBkUpdate(int index, const BkImageInfo& info) override;

    //ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

private:
    void InitView();
    void LoadBkInfo(int group_id);
    void SelectCheckBoxInternal(int index);
    base::string16 GetBackgroundText(int index);
    gfx::Size GetBtnSize(bool is_vtuber_preset);
    gfx::Size GetBtnImageSize(bool is_vtuber_preset);
    float GetBtnSpan(bool is_vtuber_preset);


    bool is_vtuber_preset_ = false;
    CustomStyleRadioButton* CreateBkButton(int group_id, const base::string16& text);
    PresetMaterialBackgroundPreviewObserver* observer_ = nullptr;
    bool is_horiz_ = false;
    int group_id_{};
    std::vector<CustomStyleRadioButton*> vec_bg_checkbox;
};

