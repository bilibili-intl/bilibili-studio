#include "source_filter_base_property_view.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/controls/button/button.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"


#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/bililive/ui/views/controls/bililive_widget_delegate/bililive_widget_delegate.h"
#include "bililive/bililive/livehime/sources_properties/source_filter_property_presenter.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_fliter_select_color_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_window_property_view.h"
#include <bililive/secret/public/event_tracking_service.h>


SourceFilterBaseView::SourceFilterBaseView(obs_proxy::SceneItem* scene_item, int64_t plugin_id)
        :label(nullptr),
        background_switch_tip_(nullptr),
        background_switch_checkbox_(nullptr),
        similarity_slider_(nullptr),
        similarity_label_(nullptr),
        smoothness_slider_(nullptr),
        smoothness_label_(nullptr),
        overflow_slider_(nullptr),
        overflow_label_(nullptr),
        alpha_slider_(nullptr),
        alpha_label_(nullptr),
        video_set_tip_(nullptr),
        custom_button_(nullptr),
        switch_button_(nullptr),
        window_detail_view_(nullptr),
        main_view_(nullptr),
        plugin_id_(plugin_id),
        presenter_(std::make_unique<SourcePublicFilterPropertyPresenterImp>(scene_item)) {

}

SourceFilterBaseView::~SourceFilterBaseView() {

}

void SourceFilterBaseView::SetCheck(bool check)
{
    UpdateSwitchButtonStatus(check);
    AdjustMainViewVisible();
    //CheckSwitch();
    //EffectiveImmediately(FRAMES_IS_USE_COLORKEY);
}

bool SourceFilterBaseView::Checked()
{
    return erase_background_checkbox_->tag();
}

void SourceFilterBaseView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details){
    if (details.child == this){
        if (details.is_add)
        {
            InitView();
            InitData();
        }
    }
};

void SourceFilterBaseView::InitView()
{
    auto& rb = ResourceBundle::GetSharedInstance();

    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    layout->SetInsets(0, 0, GetLengthByDPIScale(16), 0);

    auto column_set = layout->AddColumnSet(0);
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0.0f,GetLengthByDPIScale(12));
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);

    auto label = new BililiveLabel(rb.GetLocalizedString(IDS_CAMERA_BEAUTY_ERASE_BACKGROUND));
    label->SetFont(ftFourteen);
    label->SetTextColor(GetColor(LivehimeColorType::TextTitle));

    erase_background_checkbox_ = new BililiveImageButton(this);
    erase_background_checkbox_->SetAllStateImage(rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_RADIO_OFF));
    erase_background_checkbox_->set_tag(0);

    layout->StartRow(0, 0);
    layout->AddView(label);
    layout->AddView(erase_background_checkbox_);

    column_set = layout->AddColumnSet(1);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::USE_PREF, 0, 0);

    main_view_ = new BililiveHideAwareView();
    {
        views::GridLayout* layout = new views::GridLayout(main_view_);
        main_view_->SetLayoutManager(layout);
        layout->SetInsets(0, GetLengthByDPIScale(32), GetLengthByDPIScale(16), 0);

        column_set = layout->AddColumnSet(1);
        column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);

        column_set->AddPaddingColumn(0.f, GetLengthByDPIScale(10));
        column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0.f, GetLengthByDPIScale(10));
        column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0.f, GetLengthByDPIScale(10));
        column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0.f, GetLengthByDPIScale(20));
        column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0.4f, 0);


        column_set = layout->AddColumnSet(2);
        column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0, GetLengthByDPIScale(12));
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 1.0f, views::GridLayout::FIXED, 0, 0);
        column_set->AddPaddingColumn(0, GetLengthByDPIScale(24));
        column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
        column_set->AddPaddingColumn(0.4f, 0);

        background_switch_tip_ = new LivehimeHoverTipButton(GetLocalizedString(IDS_SRCPROP_COMMON_TIPS_TITLE),
            GetLocalizedString(IDS_SRCPROP_CAMERA_SETTING_TIP));

        label = new LivehimeContentLabel(GetLocalizedString(IDS_SRCPROP_FILTER_BASE_PROPERTY_VIEW_FILTER_COLOR));
        label->SetTextColor(GetColor(LivehimeColorType::TextTitle));
        label->SetPreferredSize(GetSizeByDPIScale({57, 20}));
        label->SetFont(ftFourteen);

        custom_button_ = new LivehimeFunctionLabelButton(this, GetLocalizedString(IDS_SRCPROP_FILTER_BASE_PROPERTY_VIEW_CUSTOM));
        custom_button_->SetFont(ftTwelve);
        custom_button_->SetPreferredSize(GetSizeByDPIScale({52, 20}));

        color_button_groupid_ = BililiveRadioButton::new_groupid();
        color_buttons_[0] = new LivehimeColorRadioButton(color_button_groupid_, SkColorSetRGB(183, 235, 164));
        color_buttons_[1] = new LivehimeColorRadioButton(color_button_groupid_, SkColorSetRGB(101, 196, 255));
        color_buttons_[2] = new LivehimeColorRadioButton(color_button_groupid_, SkColorSetRGB(255, 179, 186));

        for (int i = 0; i < 3; i++) {
            color_buttons_[i]->set_listener(this);
        }

        layout->AddPaddingRow(0, GetLengthByDPIScale(16));
        layout->StartRow(0, 1);
        layout->AddView(label);

        layout->AddView(color_buttons_[0]);
        layout->AddView(color_buttons_[1]);
        layout->AddView(color_buttons_[2]);
        layout->AddView(custom_button_);

        //相似度
        label = new LivehimeContentLabel(GetLocalizedString(IDS_SRCPROP_FILTER_BASE_PROPERTY_VIEW_SIMILARITY));
        label->SetPreferredSize(GetSizeByDPIScale({ 57, 20 }));
        label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
        label->SetTextColor(GetColor(LivehimeColorType::TextTitle));
        label->SetFont(ftFourteen);
        similarity_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
        similarity_label_ = new LivehimeContentLabel(L"1");
        similarity_label_->SetFont(ftFourteen);
        similarity_label_->SetTextColor(SkColorSetRGB(97, 102, 109));
        similarity_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
        similarity_label_->SetPreferredSize(gfx::Size(GetLengthByDPIScale(40), similarity_label_->GetPreferredSize().height()));

        layout->AddPaddingRow(0, GetLengthByDPIScale(16));
        layout->StartRow(0, 2);
        layout->AddView(label);
        layout->AddView(similarity_slider_);
        layout->AddView(similarity_label_);

        // 平滑度
        label = new LivehimeContentLabel(GetLocalizedString(IDS_SRCPROP_FILTER_BASE_PROPERTY_VIEW_SMOOTHNESS));
        label->SetPreferredSize(GetSizeByDPIScale({ 57, 20 }));
        label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
        label->SetTextColor(GetColor(LivehimeColorType::TextTitle));
        label->SetFont(ftFourteen);
        smoothness_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
        smoothness_label_ = new LivehimeContentLabel(L"1");
        smoothness_label_->SetFont(ftFourteen);
        smoothness_label_->SetTextColor(SkColorSetRGB(97, 102, 109));
        smoothness_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
        smoothness_label_->SetPreferredSize(gfx::Size(GetLengthByDPIScale(40), smoothness_label_->GetPreferredSize().height()));

        layout->AddPaddingRow(0, GetLengthByDPIScale(16));
        layout->StartRow(0, 2);
        layout->AddView(label);
        layout->AddView(smoothness_slider_);
        layout->AddView(smoothness_label_);

        // 透明度
        label = new LivehimeContentLabel(GetLocalizedString(IDS_SRCPROP_FILTER_BASE_PROPERTY_VIEW_TRANSPARENCY));
        label->SetPreferredSize(GetSizeByDPIScale({ 57, 20 }));
        label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
        label->SetTextColor(GetColor(LivehimeColorType::TextTitle));
        label->SetFont(ftFourteen);
        alpha_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
        alpha_label_ = new LivehimeContentLabel(L"0");
        alpha_label_->SetFont(ftFourteen);
        alpha_label_->SetTextColor(SkColorSetRGB(97, 102, 109));
        alpha_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
        alpha_label_->SetPreferredSize(gfx::Size(GetLengthByDPIScale(40), alpha_label_->GetPreferredSize().height()));

        layout->AddPaddingRow(0, GetLengthByDPIScale(16));
        layout->StartRow(0, 2);
        layout->AddView(label);
        layout->AddView(alpha_slider_);
        layout->AddView(alpha_label_);

        // 主色溢出
        label = new LivehimeContentLabel(GetLocalizedString(IDS_SRCPROP_FILTER_BASE_PROPERTY_VIEW_MAIN_COLOR_OVERFLOW));
        label->SetPreferredSize(GetSizeByDPIScale({ 57, 20 }));
        label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
        label->SetTextColor(GetColor(LivehimeColorType::TextTitle));
        label->SetFont(ftFourteen);
        overflow_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
        overflow_label_ = new LivehimeContentLabel(L"1");
        overflow_label_->SetFont(ftFourteen);
        overflow_label_->SetTextColor(SkColorSetRGB(97, 102, 109));
        overflow_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
        overflow_label_->SetPreferredSize(gfx::Size(GetLengthByDPIScale(40), overflow_label_->GetPreferredSize().height()));

        layout->AddPaddingRow(0, GetLengthByDPIScale(16));
        layout->StartRow(0, 2);
        layout->AddView(label);
        layout->AddView(overflow_slider_);
        layout->AddView(overflow_label_);

    }
    layout->StartRow(0, 1);
    layout->AddView(main_view_);

    main_view_->SetVisible(false);
    view_height_ = GetLengthByDPIScale(270);
}

void SourceFilterBaseView::SetWindowProDetailView(WindowPropDetailView* view) {
    window_detail_view_ = view;
}

void SourceFilterBaseView::InitData()
{
    bool orig_is_use_colorkey = presenter_->GetIsUseColorKey();
    UpdateSwitchButtonStatus(orig_is_use_colorkey);

    similarity_slider_->SetValue(presenter_->GetColorKeySimilar());
    smoothness_slider_->SetValue(presenter_->GetColorKeySmooth());
    alpha_slider_->SetValue(presenter_->GetColorKeyTransparent());
    overflow_slider_->SetValue(presenter_->GetColorKeySpill());

    if (erase_background_checkbox_->tag())
    {
        SetSelectedColorCheck(presenter_->GetColorKeyColorVal());
    }
    presenter_->Snapshot();   //保存初始配置
    AdjustMainViewVisible();
    data_loaded_ = true;
}

int SourceFilterBaseView::GetSwitchButtonTagVal()
{
    return erase_background_checkbox_->tag();
}

void SourceFilterBaseView::RecoveryConfig()
{
    presenter_->Restore();     //恢复初始配置
}

void SourceFilterBaseView::UpdateSwitchButtonStatus(bool turn_on)
{
    auto& rb = ResourceBundle::GetSharedInstance();

    erase_background_checkbox_->set_tag(turn_on);

    if (turn_on)
    {
        erase_background_checkbox_->SetAllStateImage(rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_RADIO_ON));
    }
    else
    {
        erase_background_checkbox_->SetAllStateImage(rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_RADIO_OFF));
    }

    //int dst_tag = turn_on ? 1 : 0;
    //switch_button_->SetAllStateImage(GetImageSkiaNamed(
    //    turn_on ? IDR_LIVEHIME_V3_ACTIVITY_DANMAKU_ON : IDR_LIVEHIME_V3_ACTIVITY_DANMAKU_OFF));
    //switch_button_->set_tag(dst_tag);
}

void SourceFilterBaseView::SetColorButtonUnCheck() {
    int color_size = sizeof(color_buttons_) / sizeof(color_buttons_[0]);
    for (int i = 0; i < color_size; ++i) {
        color_buttons_[i]->SetChecked(false);
    }
}

void SourceFilterBaseView::SetSelectedColorCheck(const std::tuple<std::string, SkColor>& t) {
    int button_id = 0;
    std::string key = std::get<0>(t);
    SkColor color = std::get<1>(t);

    if (key == kKeyGreen) {
        button_id = 0;
    }
    else if (key == kKeyBlue) {
        button_id = 1;
    }
    else if (key == kKeyRed) {
        button_id = 2;
    }
    else if (key == kKeyCustom) {
        button_id = 3;
        SetColorButtonUnCheck();
        custom_button_->SetFixedBkColor(color);
    }

    if (button_id < 3) {
        color_buttons_[button_id]->SetChecked(true);
    }
}

bool SourceFilterBaseView::IsColorButton(views::Button* sender) {
    bool ret = false;
    int color_size = sizeof(color_buttons_) / sizeof(color_buttons_[0]);
    for (int i = 0; i < color_size; ++i) {
        if (sender == color_buttons_[i]) {
            ret = true;
            break;
        }
    }
    return ret;
}

SkColor SourceFilterBaseView::GetSelectedColor() {
    if (color_buttons_[0]->checked()) {
        return kGreen;
    }
    else if (color_buttons_[1]->checked()) {
        return kBlue;
    }
    else if (color_buttons_[2]->checked()) {
        return kRed;
    }
    else {
        return current_color_;
    }
}

void SourceFilterBaseView::EffectiveImmediately(FramesEffectiveControl effetive_control) {
    if (!data_loaded_ || effetive_control == FRAMES_INVALID) {
        return;
    }

    switch (effetive_control) {
    case FRAMES_IS_USE_COLORKEY: {
        presenter_->SetIsUseColorKey(erase_background_checkbox_->tag());

        std::tuple<std::string, SkColor> default(kKeyGreen, kGreen);
        presenter_->SetColorKeyColorVal(default);

        if (erase_background_checkbox_->tag()) {
            SetSelectedColorCheck(presenter_->GetColorKeyColorVal());
            presenter_->SetColorKeyColorVal(presenter_->GetColorKeyColorVal());
            presenter_->SetColorKeySimilar(similarity_slider_->value());
            presenter_->SetColorKeySmooth(smoothness_slider_->value());
            presenter_->SetColorKeySpill(overflow_slider_->value());
            presenter_->SetColorKeyTransparent(alpha_slider_->value());
        }
    }
        break;
    case FRAMES_COLOR:
        presenter_->SetColorKeyColorVal(std::tuple<std::string,SkColor>(current_color_type_,current_color_));
        break;
    case FRAMES_SIMILARITY:
        presenter_->SetColorKeySimilar(similarity_slider_->value());
        break;
    case FRAMES_SMOOTHNESS:
        presenter_->SetColorKeySmooth(smoothness_slider_->value());
        break;
    case FRAMES_OVERFLOW:
        presenter_->SetColorKeySpill(overflow_slider_->value());
        break;
    case FRAMES_ALPHA:
        presenter_->SetColorKeyTransparent(alpha_slider_->value());
        break;
    default:
        break;
    }
    presenter_->UpdateFilters();
}

void SourceFilterBaseView::ButtonPressed(views::Button* sender, const ui::Event& event) {
    if (sender == erase_background_checkbox_)
    {
        erase_background_checkbox_->set_tag(!erase_background_checkbox_->tag());
        UpdateSwitchButtonStatus(erase_background_checkbox_->tag());

        if (plugin_id_ > 0) {
            std::vector<std::string> vec_outcome;
            vec_outcome.push_back(base::StringPrintf("plugin_id:%d", plugin_id_));
            vec_outcome.push_back(base::StringPrintf("switch_type:%d", erase_background_checkbox_->tag()));
            std::string outcome = JoinString(vec_outcome, ";");
            livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::BrowserBackgroundClick, outcome);
        }

        AdjustMainViewVisible();
        EffectiveImmediately(FRAMES_IS_USE_COLORKEY);
        ReportAction();
    }
    else if (sender == custom_button_) {
        //pick color
        SetColorButtonUnCheck();
        SourceFliterSelectColorView::ShowWindow(this);

        if (plugin_id_ >0) {
            livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::BrowserCustomizedClick, base::StringPrintf("plugin_id:%d", plugin_id_));
        }
    }
    else if (IsColorButton(sender)) {
        if (sender ==color_buttons_[0]) {
            current_color_type_ = kKeyGreen;
            current_color_ = kGreen;
        }
        else if(sender == color_buttons_[1]) {
            current_color_type_ = kKeyBlue;
            current_color_ = kBlue;
        }
        else if (sender == color_buttons_[2]) {
            current_color_type_ = kKeyRed;
            current_color_ = kRed;
        }
        custom_button_->SetFixedBkColor(SkColorSetRGB(255, 255, 255));
        EffectiveImmediately(FRAMES_COLOR);
    }
}

void SourceFilterBaseView::UpdateFilterCustomColor(const SkColor& color) {
    int r = SkColorGetR(color);
    int g = SkColorGetG(color);
    int b = SkColorGetB(color);

    custom_button_->SetFixedBkColor(SkColorSetRGB(r, g, b));
    custom_button_->SchedulePaint();

    current_color_ = SkColorSetRGB(r, g, b);
    current_color_type_ = kKeyCustom;

    EffectiveImmediately(FRAMES_COLOR);
}

void SourceFilterBaseView::SetMaterialType(size_t type) {
    material_type_ = type;
}

size_t SourceFilterBaseView::GetMaterialType() {
    return material_type_;
}

void SourceFilterBaseView::OnPaint(gfx::Canvas* canvas)
{
    if (erase_background_checkbox_->tag() == 1)
    {
        int y = erase_background_checkbox_->bounds().bottom();
        y += GetLengthByDPIScale(9);

        auto rt = GetContentsBounds();
        rt.set_y(y);
        rt.set_height(rt.height() - y - GetLengthByDPIScale(16));

        SkPaint p;
        p.setStyle(SkPaint::kFill_Style);

        // rgb(23, 25, 35)
        p.setColor(SkColorSetRGB(23, 25, 35));

        p.setAntiAlias(true);
        canvas->DrawRoundRect(rt, GetLengthByDPIScale(8), p);
    }

    __super::OnPaint(canvas);
}

// SliderListener
void SourceFilterBaseView::SliderValueChanged(views::Slider* sender,
    float value,
    float old_value,
    views::SliderChangeReason reason) {
    if (data_loaded_ && reason == views::SliderChangeReason::VALUE_CHANGED_BY_API) {
        ;
    }
    else {
        FramesEffectiveControl effetive_control = FRAMES_INVALID;
        if (sender == similarity_slider_) {
            similarity_label_->SetText(base::StringPrintf(L"%d", MapFloatToInt(value, kChromaKeySimilarMin, kChromaKeySimilarMax)));
            effetive_control = FRAMES_SIMILARITY;
        }
        else if (sender == smoothness_slider_) {
            smoothness_label_->SetText(base::StringPrintf(L"%d", MapFloatToInt(value, kChromaKeySmoothMin, kChromaKeySmoothMax)));
            effetive_control = FRAMES_SMOOTHNESS;
        }
        else if (sender == overflow_slider_) {
            overflow_label_->SetText(base::StringPrintf(L"%d", MapFloatToInt(value, kChromaKeySpillMin, kChromaKeySpillMax)));
            effetive_control = FRAMES_OVERFLOW;
        }
        else if (sender == alpha_slider_) {
            alpha_label_->SetText(base::StringPrintf(L"%d", MapFloatToInt(value)));
            effetive_control = FRAMES_ALPHA;
        }
        EffectiveImmediately(effetive_control);
    }
}

void SourceFilterBaseView::ReportAction() {
    base::StringPairs data;
    std::pair<std::string, std::string> pair1("material_type", std::to_string(GetMaterialType()));
    std::pair<std::string, std::string> pair2("switch", std::to_string(erase_background_checkbox_->tag()));
    data.push_back(pair1);
    data.push_back(pair2);

    livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::SubtractSwitchClick, data);
}

void SourceFilterBaseView::AdjustMainViewVisible() {
    if (erase_background_checkbox_->tag())
    {
        main_view_->SetVisible(true);
    }
    else
    {
        main_view_->SetVisible(false);
    }

    if (GetWidget() && GetWidget()->GetContentsView())
    {
        GetWidget()->GetContentsView()->InvalidateLayout();
        GetWidget()->GetContentsView()->Layout();
    }
}
