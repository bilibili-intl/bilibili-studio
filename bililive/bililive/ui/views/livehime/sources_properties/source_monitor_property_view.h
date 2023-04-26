#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_MONITOR_PROPERTY_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_MONITOR_PROPERTY_VIEW_H_

#include "bililive/bililive/ui/views/livehime/controls/livehime_combobox.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_hover_tip_button.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_property_base_view.h"

class MonitorPropDetailView
    : public livehime::BasePropertyView,
      public views::ButtonListener,
      public BililiveComboboxExListener
{
public:
    explicit MonitorPropDetailView(obs_proxy::SceneItem* scene_item);

    ~MonitorPropDetailView() OVERRIDE;

protected:
	// BililiveComboboxExListener
    void OnSelectedIndexChanged(BililiveComboboxEx* combobox) override;
	void OnBililiveComboboxPressed(BililiveComboboxEx* combobox)override;

private:
    void InitView() override;

    void InitData() override;

    bool CheckSetupValid() override;

    bool SaveSetupChange() override;

    bool Cancel() override;

    gfx::ImageSkia* GetSkiaIcon() override;

    std::wstring GetCaption() override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    void RefreshList();

private:
    MonitorSceneItemHelper item_;

    LivehimeSrcPropCombobox* monitor_combobox_;
    BililiveCheckbox* capture_mouse_checkbox_;
    DISALLOW_COPY_AND_ASSIGN(MonitorPropDetailView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_MONITOR_PROPERTY_VIEW_H_