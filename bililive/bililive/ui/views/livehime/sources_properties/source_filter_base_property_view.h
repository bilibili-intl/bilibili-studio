#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCE_FLITER_BASE_PROPERTY_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SOURCE_FLITER_BASE_PROPERTY_H_

#include "bililive/bililive/ui/views/livehime/sources_properties/source_camera_base_settings_view.h"

class SourcePublicFilterPropertyPresenterImp;
class LivehimeContentLabel;
class BililiveLabelButton;
class LivehimeSlider;
class SourceFliterSelectColorView;
class BililiveImageButton;
class WindowPropDetailView;
class BililiveHideAwareView;

class SourceFilterBaseView
    : public views::View,
    views::ButtonListener,
    views::SliderListener {

public:
    enum CommonButtonId {
        BACKGROUND_SWITCH_CHECKBOX = 1
    };

    enum FramesEffectiveControl {
        FRAMES_INVALID = -1,
        FRAMES_IS_USE_COLORKEY,
        FRAMES_COLOR,
        FRAMES_SIMILARITY,
        FRAMES_SMOOTHNESS,
        FRAMES_OVERFLOW,
        FRAMES_ALPHA,
    };

    const SkColor kGreen = 0xB7EBA4;
    const SkColor kBlue = 0x65C4FF;
    const SkColor kRed = 0xFFB3BA;

    const std::string kKeyGreen = "kGreen";
    const std::string kKeyBlue = "kBlue";
    const std::string kKeyRed = "kRed";
    const std::string kKeyCustom = "kCustom";

public:
    SourceFilterBaseView(obs_proxy::SceneItem* scene_item,int64_t plugin_id = 0);
    virtual ~SourceFilterBaseView();

    void SetCheck(bool check);
    bool Checked();

    void SetWindowProDetailView(WindowPropDetailView* view);
    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;
    // SliderListener
    void SliderValueChanged(views::Slider* sender,
        float value, float old_value, views::SliderChangeReason reason) override;

    // View
    void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) override;

    void InitView() ;
    void InitData() ;

    int GetSwitchButtonTagVal();
    void RecoveryConfig();
    void UpdateFilterCustomColor(const SkColor& color);
    void SetMaterialType(size_t type);
    size_t GetMaterialType();

    void OnPaint(gfx::Canvas* canvas) override;

private:
    bool IsColorButton(views::Button* sender);
    void UpdateSwitchButtonStatus(bool turn_on);
    void EffectiveImmediately(FramesEffectiveControl effetive_control);

    void SetSelectedColorCheck(const std::tuple<std::string, SkColor>& t);
    void SetColorButtonUnCheck();

    SkColor GetSelectedColor();
    void ReportAction();
    void AdjustMainViewVisible();

private:
    bool data_loaded_ = false;
    int color_button_groupid_ = 0;
    size_t material_type_ = 0;
    int window_init_height_ = 0;
    int view_height_ = 0;
    int64_t plugin_id_ = 0;

    SkColor current_color_ = kGreen;
    std::string current_color_type_ = kKeyGreen;

    LivehimeContentLabel* label = nullptr;
    LivehimeHoverTipButton* background_switch_tip_ = nullptr;
    BililiveImageButton* erase_background_checkbox_ = nullptr;
    LivehimeCheckbox* background_switch_checkbox_ = nullptr;
    BililiveImageButton* switch_button_ = nullptr;


    BililiveHideAwareView* main_view_;
    LivehimeColorRadioButton* color_buttons_[3];
    BililiveLabelButton* custom_button_;
    LivehimeSlider* similarity_slider_;
    LivehimeContentLabel* similarity_label_;
    LivehimeSlider* smoothness_slider_;
    LivehimeContentLabel* smoothness_label_;
    LivehimeSlider* overflow_slider_;
    LivehimeContentLabel* overflow_label_;
    LivehimeSlider* alpha_slider_;
    LivehimeContentLabel* alpha_label_;
    LivehimeHoverTipButton* video_set_tip_;

    std::unique_ptr<SourcePublicFilterPropertyPresenterImp> presenter_;
    WindowPropDetailView* window_detail_view_;

    DISALLOW_COPY_AND_ASSIGN(SourceFilterBaseView);
};

#endif