#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_WINDOW_PROPERTY_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_WINDOW_PROPERTY_VIEW_H_

#include "bililive/bililive/ui/views/livehime/controls/livehime_combobox.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_native_edit.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_property_base_view.h"

class SourceFilterBaseView;
class BililiveLabelButton;
class SourceWindowPropertyPresenterImp;
class WindowPropDetailView;

class WindowCaptureDetailView : public livehime::BasePropertyView {
public:
    WindowCaptureDetailView(obs_proxy::SceneItem* scene_item,bool preset_setting = false);//preset_setting «∑Ò‘§…ËÀÿ≤ƒ…Ë÷√
    ~WindowCaptureDetailView();
private:
    void InitView() override;
    bool SaveSetupChange() override;
    bool Cancel() override;

    gfx::ImageSkia* GetSkiaIcon() override;
    std::wstring GetCaption() override;

    gfx::Size GetPreferredSize() override;
    bool CheckSetupValid() override;
private:
    bool preset_setting_ = false;
    obs_proxy::SceneItem* scene_item_;
    std::unique_ptr<WindowPropDetailView> main_view_;
};

class WindowPropDetailView
    : public livehime::BaseScrollbarView,
      public views::ButtonListener,
      public BililiveComboboxExListener
{
public:
    explicit WindowPropDetailView(obs_proxy::SceneItem* scene_item,bool preset_setting = false);

    ~WindowPropDetailView();

    bool CheckSetupValid() override;

    bool SaveSetupChange() override;

    void PostSaveSetupChange(obs_proxy::SceneItem* scene_item) override;

    bool Cancel() override;

    gfx::ImageSkia* GetSkiaIcon() override;

    std::wstring GetCaption() override;

    void ReportAdvanceSettingInfoAction();
private:
    void InitView() override;

    void InitData() override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    void OnSelectedIndexChanged(BililiveComboboxEx* combobox) override;
    void OnBililiveComboboxPressed(BililiveComboboxEx* combobox)override;

    void RefreshList();

    void ObsModelNameToLivehime(PropertyList<std::string>& list);

private:
    void UpdateSwitchButtonStatus(bool turn_on);
    void AdjustFilterVisible();
    void ReportAction(secret::LivehimePolarisBehaviorEvent event,
                      bool switch_on);
    void ReportCaptureMethodAction(secret::LivehimePolarisBehaviorEvent event,const std::string& val);

private:
    int advance_view_height_ = 0;
    int window_init_height_ = 0;
    bool preset_setting_ = false;
    bool aero_enable_ = false;
    std::unique_ptr<SourceWindowPropertyPresenterImp> presenter_;
    //WindowSceneItemHelper item_;
    obs_proxy::SceneItem* scene_item_;

    LivehimeStyleLabel* label_warning_;
    LivehimeSrcPropCombobox* window_combobox_;
    LivehimeSrcPropCombobox* model_combobox_ = nullptr;
    BililiveCheckbox* capture_mouse_checkbox_;
    BililiveCheckbox* compatible_mode_checkbox_;
    BililiveCheckbox* sli_mode_checkbox_;
    BililiveCheckbox* aero_mode_checkbox_;

    SourceFilterBaseView* source_filter_base_view_;
    BililiveLabelButton* advance_switch_;

    views::View* advance_view_;

    base::WeakPtrFactory<WindowPropDetailView> weak_ptr_factory_;
    DISALLOW_COPY_AND_ASSIGN(WindowPropDetailView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCES_PROPERTIES_SOURCE_WINDOW_PROPERTY_VIEW_H_