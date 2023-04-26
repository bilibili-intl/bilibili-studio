#include "sound_reverb_view.h"

#include "base/strings/stringprintf.h"

#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"
#include "bililive/bililive/ui/views/controls/linear_layout.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"

#include "ui/views/layout/grid_layout.h"
#include "ui/views/widget/widget.h"
#include <base/strings/utf_string_conversions.h>

const int kSliderRangeMin = 0;
const int kSliderRangeMax = 100;

SoundReverbView::SoundReverbView(views::ButtonListener* listener) :
    sound_effect_listener_(listener)
{
}

SoundReverbView::~SoundReverbView()
{
}

views::View* SoundReverbView::GetMixerMainView()
{
    return main_view_;
}

LivehimeCheckbox* SoundReverbView::GetEnableCheckBox()
{
    return enable_mixer_checkbox_;
}

void SoundReverbView::SetPresenter(contracts::SoundEffectPropertyPresenter* presenter)
{
    presenter_ = presenter;

    InitData();
}

void SoundReverbView::OnEnableCheckBoxChanged()
{
    main_view_->SetVisible(enable_mixer_checkbox_->checked());
    if (GetWidget()) 
    {
        GetWidget()->GetContentsView()->Layout();
    }
}

void SoundReverbView::OnSaveChanged()
{
    if (!presenter_)
    {
        return;
    }

    presenter_->EndPreview();
    presenter_->SetIsReverbEnabled(enable_mixer_checkbox_->checked());
    presenter_->SetRoomSize(room_size_slider_->value());
    presenter_->SetDamping(damp_slider_->value());
    presenter_->SetWetLevel(wet_slider_->value());
    presenter_->SetDryLevel(dry_slider_->value());
    presenter_->SetWidth(width_slider_->value());
    presenter_->SetFreezeMode(freeze_mode_slider_->value());
    presenter_->UpdateFilters();
}

void SoundReverbView::InitMicVolumeController(contracts::BililiveVolumeControllersContract* controller)
{
    if (controller && controller->ControllerIsValid())
    {
        volume_controllers_ = controller;
        volume_controllers_->RegisterVolumeLevelUpdatedHandler(
            std::bind(&BililiveVolumeBar::VolumeLevelUpdate, volume_bar_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        volume_bar_->SetMuted(volume_controllers_->GetMuted());
    }
}

void SoundReverbView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this && details.is_add) 
    {
        InitView();
    }
}

void SoundReverbView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (!presenter_)
    {
        return;
    }

    if (sender == reset_btn_) 
    {
        presenter_->SetAllValueToDefault();
        UpdateControlStateAndData();
    }
    else if (sender == preview_btn_) 
    {
        if (preview_btn_->checked()) 
        {
            volume_bar_->SetVolumeState(true);
            presenter_->StartPreview();
        }
        else 
        {
            volume_bar_->SetVolumeState(false);
            presenter_->EndPreview();
        }
    }
}

void SoundReverbView::SliderValueChanged(views::Slider* sender, float value, float old_value, views::SliderChangeReason reason)
{
    if (!presenter_) 
    {
        return;
    }

    if (sender == room_size_slider_)
    {
        room_size_label_->SetText(
            base::StringPrintf(L"%d",
            MapFloatToInt(room_size_slider_->value(),
            kSliderRangeMin, kSliderRangeMax)));

        presenter_->SetRoomSize(value);
    }
    else if (sender == damp_slider_)
    {
        damp_label_->SetText(
            base::StringPrintf(L"%d",
            MapFloatToInt(damp_slider_->value(),
            kSliderRangeMin, kSliderRangeMax)));

        presenter_->SetDamping(value);
    }
    else if (sender == wet_slider_)
    {
        wet_label_->SetText(
            base::StringPrintf(L"%d",
            MapFloatToInt(wet_slider_->value(),
            kSliderRangeMin, kSliderRangeMax)));

        presenter_->SetWetLevel(value);
    }
    else if (sender == dry_slider_)
    {
        dry_label_->SetText(
            base::StringPrintf(L"%d",
            MapFloatToInt(dry_slider_->value(),
            kSliderRangeMin, kSliderRangeMax)));

        presenter_->SetDryLevel(value);
    }
    else if (sender == width_slider_)
    {
        width_label_->SetText(
            base::StringPrintf(L"%d",
            MapFloatToInt(width_slider_->value(),
            kSliderRangeMin, kSliderRangeMax)));

        presenter_->SetWidth(value);
    }
    else if (sender == freeze_mode_slider_)
    {
        freeze_mode_label_->SetText(
            base::StringPrintf(L"%d",
            MapFloatToInt(freeze_mode_slider_->value(),
            kSliderRangeMin, kSliderRangeMax)));

        presenter_->SetFreezeMode(value);
    }
}

void SoundReverbView::InitView()
{
    views::GridLayout* layout = new views::GridLayout(this);
    this->SetLayoutManager(layout);

    auto col_set = layout->AddColumnSet(0);
    col_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0.5f, views::GridLayout::FIXED, 0, 0);
    col_set->AddColumn(views::GridLayout::TRAILING, views::GridLayout::CENTER, 0.5f, views::GridLayout::FIXED, 0, 0);

    col_set = layout->AddColumnSet(1);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

	col_set = layout->AddColumnSet(2);
	col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.255f, views::GridLayout::FIXED, 0, 0);
	col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.745f, views::GridLayout::FIXED, 0, 0);

    enable_mixer_checkbox_ = new LivehimeCheckbox(GetLocalizedString(IDS_CONFIG_AUDIO_SOUND_REVERB_TITLE));
    enable_mixer_checkbox_->set_listener(sound_effect_listener_);
    enable_mixer_checkbox_->SetFont(ftFourteen);
    enable_mixer_checkbox_->SetTextColor(views::Button::STATE_NORMAL, GetColor(LabelTitle));
    enable_mixer_checkbox_->SetTextColor(views::Button::STATE_HOVERED, GetColor(LabelTitle));
    enable_mixer_checkbox_->SetTextColor(views::Button::STATE_PRESSED, GetColor(LabelTitle));

    reset_btn_ = new CustomStyleButton(this);
    reset_btn_->SetPreferredSize(GetSizeByDPIScale({ 100, 20 }));
    reset_btn_->SetText(GetLocalizedString(IDS_CONFIG_AUDIO_SOUND_REVERB_RESET));
    reset_btn_->SetAllStateBorder(true);
    reset_btn_->SetAllStateBorderLineWidth(GetLengthByDPIScale(1.0f));
    reset_btn_->SetAllStateBorderRoundRatio(GetLengthByDPIScale(4.0f));
    reset_btn_->SetTextColor(views::Button::STATE_NORMAL, GetColor(LabelTitle));
    reset_btn_->SetTextColor(views::Button::STATE_HOVERED, SkColorSetRGB(14, 190, 255));
    reset_btn_->SetTextColor(views::Button::STATE_PRESSED, SkColorSetRGB(14, 190, 255));
    reset_btn_->SetBorderColor(views::Button::STATE_NORMAL, GetColor(LabelTitle));
    reset_btn_->SetBorderColor(views::Button::STATE_HOVERED, SkColorSetRGB(14, 190, 255));
    reset_btn_->SetBorderColor(views::Button::STATE_PRESSED, SkColorSetRGB(14, 190, 255));

    layout->StartRow(1.0f, 0);
    layout->AddView(enable_mixer_checkbox_);
    layout->AddView(reset_btn_);

    auto label1 = new BililiveLabel(GetLocalizedString(IDS_CONFIG_AUDIO_SOUND_REVERB_ROOM), ftTwelve);
    label1->SetTextColor(GetColor(LabelTitle));
    label1->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);

    auto label2 = new BililiveLabel(GetLocalizedString(IDS_CONFIG_AUDIO_SOUND_REVERB_WET), ftTwelve);
    label2->SetTextColor(GetColor(LabelTitle));
    label2->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);

    auto label3 = new BililiveLabel(GetLocalizedString(IDS_CONFIG_AUDIO_SOUND_REVERB_DRY), ftTwelve);
    label3->SetTextColor(GetColor(LabelTitle));
    label3->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);

    auto label4 = new BililiveLabel(GetLocalizedString(IDS_CONFIG_AUDIO_SOUND_REVERB_WIDTH), ftTwelve);
    label4->SetTextColor(GetColor(LabelTitle));
    label4->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);

    auto label5 = new BililiveLabel(GetLocalizedString(IDS_CONFIG_AUDIO_SOUND_REVERB_DAMP), ftTwelve);
    label5->SetTextColor(GetColor(LabelTitle));
    label5->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);

    auto label6 = new BililiveLabel(GetLocalizedString(IDS_CONFIG_AUDIO_SOUND_REVERB_FREEZE), ftTwelve);
    label6->SetTextColor(GetColor(LabelTitle));
    label6->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);

    room_size_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
    room_size_label_ = new BililiveLabel(L"0", ftFourteen);
    room_size_label_->SetTextColor(GetColor(LabelTitle));
    room_size_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_RIGHT);

    wet_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
    wet_label_ = new BililiveLabel(L"0", ftFourteen);
    wet_label_->SetTextColor(GetColor(LabelTitle));
    wet_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_RIGHT);

    dry_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
    dry_label_ = new BililiveLabel(L"0", ftFourteen);
    dry_label_->SetTextColor(GetColor(LabelTitle));
    dry_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_RIGHT);

    width_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
    width_label_ = new BililiveLabel(L"0", ftFourteen);
    width_label_->SetTextColor(GetColor(LabelTitle));
    width_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_RIGHT);

    damp_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
    damp_label_ = new BililiveLabel(L"0", ftFourteen);
    damp_label_->SetTextColor(GetColor(LabelTitle));
    damp_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_RIGHT);

    freeze_mode_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
    freeze_mode_label_ = new BililiveLabel(L"0", ftFourteen);
    freeze_mode_label_->SetTextColor(GetColor(LabelTitle));
    freeze_mode_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_RIGHT);

    preview_btn_ = new CustomStyleCheckbox(this, GetLocalizedString(IDS_CONFIG_AUDIO_SOUND_REVERB_CHECK));
    preview_btn_->SetPreferredSize(GetSizeByDPIScale({ 150, 20 }));
    preview_btn_->SetFont(ftTwelve);
    preview_btn_->SetAllStateTextColor(GetColor(LabelTitle));
    preview_btn_->SetAllStateBorder(true);
    preview_btn_->SetAllStateBorderLineWidth(GetLengthByDPIScale(1.0f));
    preview_btn_->SetAllStateBorderRoundRatio(GetLengthByDPIScale(4.0f));
    preview_btn_->SetAllStateBorderColor(GetColor(LabelTitle));
    preview_btn_->SetCheckedBorder(true);
    preview_btn_->SetCheckedTextColor(SkColorSetRGB(14, 190, 255));
    preview_btn_->SetCheckedBorderColor(SkColorSetRGB(14, 190, 255));
    preview_btn_->SetCheckBorderRoundRatio(GetLengthByDPIScale(4.0f));

    volume_bar_ = new BililiveVolumeBar();

    auto volume_tip = new LivehimeHoverTipButton(GetLocalizedString(IDS_SRCPROP_COMMON_TIPS_TITLE),
        GetLocalizedString(IDS_CONFIG_AUDIO_SOUND_REVERB_CHECK_TIP));

    auto sub_view = CreateSubView(preview_btn_, volume_tip, GetLengthByDPIScale(4));

    main_view_ = new BililiveHideAwareView();
    auto main_layout = new views::GridLayout(main_view_);
    main_view_->SetLayoutManager(main_layout);

    col_set = main_layout->AddColumnSet(0);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.1675f, views::GridLayout::FIXED, 0, 0);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.7175f, views::GridLayout::FIXED, 0, 0);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.115f, views::GridLayout::FIXED, 0, 0);

    col_set = main_layout->AddColumnSet(1);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.255f, views::GridLayout::FIXED, 0, 0);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.745f, views::GridLayout::FIXED, 0, 0);


    main_layout->StartRow(1.0f, 0);
    main_layout->AddView(label1);
    main_layout->AddView(room_size_slider_);
    main_layout->AddView(room_size_label_);

    main_layout->StartRowWithPadding(1.0f, 0, 0, GetLengthByDPIScale(16));
    main_layout->AddView(label2);
    main_layout->AddView(wet_slider_);
    main_layout->AddView(wet_label_);

    main_layout->StartRowWithPadding(1.0f, 0, 0, GetLengthByDPIScale(16));
    main_layout->AddView(label3);
    main_layout->AddView(dry_slider_);
    main_layout->AddView(dry_label_);

    main_layout->StartRowWithPadding(1.0f, 0, 0, GetLengthByDPIScale(16));
    main_layout->AddView(label4);
    main_layout->AddView(width_slider_);
    main_layout->AddView(width_label_);

    main_layout->StartRowWithPadding(1.0f, 0, 0, GetLengthByDPIScale(16));
    main_layout->AddView(label5);
    main_layout->AddView(damp_slider_);
    main_layout->AddView(damp_label_);

    main_layout->StartRowWithPadding(1.0f, 0, 0, GetLengthByDPIScale(16));
    main_layout->AddView(label6);
    main_layout->AddView(freeze_mode_slider_);
    main_layout->AddView(freeze_mode_label_);

    layout->StartRowWithPadding(1.0f, 1, 0.0f, GetLengthByDPIScale(10));
    layout->AddView(main_view_);
    main_view_->SetVisible(false);

    layout->StartRowWithPadding(1.0f, 2, 0.0f, GetLengthByDPIScale(10));
    layout->AddView(sub_view);
    layout->AddView(volume_bar_);
}

void SoundReverbView::InitData()
{
    UpdateControlStateAndData();
}

void SoundReverbView::UpdateControlStateAndData()
{
    if (!presenter_) 
    {
        return;
    }

    room_size_slider_->SetValue(presenter_->GetRoomSize());
    wet_slider_->SetValue(presenter_->GetWetLevel());
    dry_slider_->SetValue(presenter_->GetDryLevel());
    width_slider_->SetValue(presenter_->GetWidth());
    damp_slider_->SetValue(presenter_->GetDamping());
    freeze_mode_slider_->SetValue(presenter_->GetFreezeMode());
}

//创建一个由标签加按钮的组成子view
views::View* SoundReverbView::CreateSubView(views::View* label, views::View* buuton, int padding)
{
    views::View* sub_view = new views::View();
    bililive::LinearLayout* layout = new bililive::LinearLayout(bililive::LinearLayout::kHoirzontal, sub_view);
    sub_view->SetLayoutManager(layout);
    layout->AddView(label, bililive::LinearLayout::kLeading, bililive::LinearLayout::kCenter);
    layout->AddView(buuton, bililive::LinearLayout::kLeading, bililive::LinearLayout::kCenter, padding);

    return sub_view;
}
