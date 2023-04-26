#include "source_audio_base_view.h"

#include "base/strings/stringprintf.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_hover_tip_button.h"
#include "bililive/bililive/ui/views/controls/linear_layout.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/sources_properties/source_camera_property_presenter_impl.h"

#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"


namespace
{
    static obs_proxy::SceneCollection* CurrentSceneCollection()
    {
        return OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    }
};

SourceAudioBaseView::SourceAudioBaseView(obs_proxy::SceneItem* scene_item) :
    presenter_(std::make_unique<SourceCameraPropertyPresenterImpl>(scene_item, this))
{
}

SourceAudioBaseView::~SourceAudioBaseView()
{
}

void SourceAudioBaseView::ViewHierarchyChanged(const ViewHierarchyChangedDetails& details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            OBSProxyService::GetInstance().obs_ui_proxy()->AddObserver(this);
            InitView();
            InitData();
        }
        else
        {
            OBSProxyService::GetInstance().obs_ui_proxy()->RemoveObserver(this);
        }
    }
}

void SourceAudioBaseView::OnPaint(gfx::Canvas* canvas)
{
    if (audio_setting_checkbox_->tag() == 1)
    {
        int y = audio_setting_checkbox_->bounds().bottom();
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

void SourceAudioBaseView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (sender == audio_setting_checkbox_)
    {
        SetCheck(!audio_setting_checkbox_->tag());
    }
    else if (sender == mic_mute_btn_)
    {
        if (presenter_->GetSceneItem())
        {
            auto vol_ctrl = CurrentSceneCollection()->GetVolumeControllerForAudioSource(presenter_->GetSceneItem());
            if (vol_ctrl)
            {
                bool mute = vol_ctrl->IsMuted();
                mute = !mute;
                vol_ctrl->SetMuted(mute);
                SetMuteBtnState(mute);
            }
        }
    }
}

void SourceAudioBaseView::SliderValueChanged(views::Slider* sender, float value, float old_value, views::SliderChangeReason reason)
{
    if (sender == volume_slider_)
    {
        if(presenter_->GetSceneItem())
        {
            auto vol_ctrl = CurrentSceneCollection()->GetVolumeControllerForAudioSource(presenter_->GetSceneItem());
            if (vol_ctrl)
            {
                if (reason == views::SliderChangeReason::VALUE_CHANGED_BY_USER)
                {
                    if (vol_ctrl->IsMuted())
                    {
                        vol_ctrl->SetMuted(false);
                    }
                }
                vol_ctrl->SetDeflection(MapFloatToInt(sender->value()) / 100.0f);
                SetVolume(MapFloatFromInt(std::round(vol_ctrl->GetDeflection() * 100.0f)));
            }
        }
        else
        {
            need_listening_ = true;
        }
    }
}

void SourceAudioBaseView::OnBililiveComboboxExDropDown(BililiveComboboxEx* combobox)
{
    if (combobox == audio_device_combox_)
    {
        auto select_data = audio_device_combox_->GetSelectedData<std::string>();
        RefreshAudioDeviceList();
        int selected_index = audio_device_combox_->FindItemData(select_data);
        if (selected_index == -1)
        {
            selected_index = 0;
        }
        audio_device_combox_->SetSelectedIndex(selected_index);
    }
}

void SourceAudioBaseView::OnSelectedIndexChanged(BililiveComboboxEx* combobox)
{
    if (combobox == audio_device_combox_)
    {
        EffectiveImmediately(CAMERA_AUDIO_DEVICE);
    }
    else if (combobox == audio_output_combox_)
    {
        EffectiveImmediately(CAMERA_AUDIO_OUTPUT);
    }
    else if (combobox == monitor_combobox_)
    {
        if (presenter_->GetSceneItem())
        {
            auto vol_ctrl = CurrentSceneCollection()->GetVolumeControllerForAudioSource(presenter_->GetSceneItem());
            if (vol_ctrl)
            {
                vol_ctrl->SetMonitoringType(monitor_combobox_->selected_index());
            }
        }
    }
}

void SourceAudioBaseView::RefreshResolutionListCallback()
{
}

void SourceAudioBaseView::OnAudioSourceActivate(obs_proxy::VolumeController* audio_source)
{
    if (presenter_->GetSceneItem())
    {
        auto vol_ctrl = CurrentSceneCollection()->GetVolumeControllerForAudioSource(presenter_->GetSceneItem());
        if (vol_ctrl)
        {
            SetVolume(MapFloatFromInt(std::round(vol_ctrl->GetDeflection() * 100.0f)));
            monitor_combobox_->SetSelectedIndex(vol_ctrl->GetMonitoringType());
            SetMuteBtnState(vol_ctrl->IsMuted());
        }
    }
}

void SourceAudioBaseView::InitView()
{
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();

    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);
    layout->SetInsets(0,0, GetLengthByDPIScale(32),0);

    auto column_set = layout->AddColumnSet(0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

    column_set = layout->AddColumnSet(1);
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0.0f, GetLengthByDPIScale(12));
    column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);

    auto label = new BililiveLabel(rb.GetLocalizedString(IDS_CAMERA_BEAUTY_AUDIO_SETTING));
    label->SetFont(ftFourteen);
    label->SetTextColor(GetColor(LivehimeColorType::TextTitle));

    audio_setting_checkbox_ = new BililiveImageButton(this);
    audio_setting_checkbox_->SetAllStateImage(rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_RADIO_OFF));
    audio_setting_checkbox_->set_tag(0);

    layout->StartRow(0, 1);
    layout->AddView(label);
    layout->AddView(audio_setting_checkbox_);

    main_view_ = new BililiveHideAwareView();
    auto main_view_layout = new views::GridLayout(main_view_);
    main_view_->SetLayoutManager(main_view_layout);

    layout->StartRow(1.0f, 0);
    layout->AddView(main_view_);

    main_view_layout->SetInsets(GetLengthByDPIScale(12), GetLengthByDPIScale(32), 0, 0);

    auto main_column_set = main_view_layout->AddColumnSet(0);
    main_column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);

    main_column_set = main_view_layout->AddColumnSet(1);
    main_column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.2125f, views::GridLayout::FIXED, 0, 0);
    main_column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.7875f, views::GridLayout::FIXED, 0, 0);
    main_column_set->AddPaddingColumn(0.0f, GetLengthByDPIScale(7));
    main_column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0.0f, views::GridLayout::USE_PREF, 0, 0);
    main_column_set->AddPaddingColumn(0.4f, 0);

    main_column_set = main_view_layout->AddColumnSet(2);
    main_column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.2125f, views::GridLayout::FIXED, 0, 0);
    main_column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0.049f, views::GridLayout::FIXED, 0, 0);
    main_column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.6485f, views::GridLayout::FIXED, 0, 0);
    main_column_set->AddColumn(views::GridLayout::TRAILING, views::GridLayout::CENTER, 0.09f, views::GridLayout::FIXED, 0, 0);
    main_column_set->AddPaddingColumn(0.4f, 0);

    //main_view_layout->StartRowWithPadding(1.0f, 0, 0, GetLengthByDPIScale(10));
    //label = new BililiveLabel(L"外接麦克风设置");
    //label->SetFont(ftFourteenBold);
    //label->SetTextColor(GetColor(LabelTitle));
    //main_view_layout->AddView(label);

    main_view_layout->StartRowWithPadding(1.0f, 1, 0, GetLengthByDPIScale(16));
    label = new BililiveLabel(rb.GetLocalizedString(IDS_SRCPROP_CAMERA_AUDIO_INPUT_DEVICE));
    label->SetFont(ftFourteen);
    label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    label->SetTextColor(GetColor(LivehimeColorType::TextTitle));
    main_view_layout->AddView(label);

    audio_device_combox_ = new LivehimeSrcPropCombobox();
    audio_device_combox_->set_listener(this);
    main_view_layout->AddView(audio_device_combox_);

    main_view_layout->StartRowWithPadding(1.0f, 2, 0, GetLengthByDPIScale(16));
    label = new BililiveLabel(rb.GetLocalizedString(IDS_SRCPROP_CAMERA_AUDIO_INPUT_VOLUME));
    label->SetFont(ftFourteen);
    label->SetTextColor(GetColor(LivehimeColorType::TextTitle));
    label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    main_view_layout->AddView(label);

    mic_mute_btn_ = new BililiveImageButton(this);
    mic_mute_btn_->set_id(0);
    mic_mute_btn_->SetPreferredSize(GetSizeByDPIScale({ 12, 12 }));
    mic_mute_btn_->SetAllStateImage(rb.GetImageSkiaNamed(IDR_PRESET_MATERIAL_MIC));
    main_view_layout->AddView(mic_mute_btn_);

    volume_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
    volume_slider_->SetKeyboardIncrement(1.0f / 100.f);
    main_view_layout->AddView(volume_slider_);

    volume_label_ = new BililiveLabel(L"0");
    volume_label_->SetFont(ftFourteen);
    volume_label_->SetPreferredSize(GetSizeByDPIScale({30, 14}));
    volume_label_->SetTextColor(SkColorSetRGB(97, 102, 109));
    volume_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    main_view_layout->AddView(volume_label_);

    //main_view_layout->StartRowWithPadding(1.0f, 1, 0, GetLengthByDPIScale(20));
    //label = new BililiveLabel(L"高级属性");
    //label->SetFont(ftFourteenBold);
    //label->SetTextColor(GetColor(LabelTitle));
    //label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    //main_view_layout->AddView(label);

    main_view_layout->StartRowWithPadding(1.0f, 1, 0, GetLengthByDPIScale(16));
    label = new BililiveLabel(rb.GetLocalizedString(IDS_SRCPROP_CAMERA_AUDIO_INPUT_SETTING));
    label->SetFont(ftFourteen);
    label->SetTextColor(GetColor(LivehimeColorType::TextTitle));
    label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    main_view_layout->AddView(label);

    monitor_combobox_ = new LivehimeSrcPropCombobox();
    monitor_combobox_->set_listener(this);
    monitor_combobox_->AddItem(rb.GetLocalizedString(IDS_SRCPROP_AUDIO_DEVICE_MONITER_NONE));
    monitor_combobox_->AddItem(rb.GetLocalizedString(IDS_SRCPROP_AUDIO_DEVICE_MONITER_ONLY));
    monitor_combobox_->AddItem(rb.GetLocalizedString(IDS_SRCPROP_AUDIO_DEVICE_MONITER_AND_OUTPUT));
    monitor_combobox_->SetSelectedIndex(0);
    main_view_layout->AddView(monitor_combobox_);

    auto bit_rate_tip = new LivehimeHoverTipButton(GetLocalizedString(IDS_SRCPROP_COMMON_TIPS_TITLE),
        rb.GetLocalizedString(IDS_SRCPROP_CAMERA_AUDIO_INPUT_SETTING_TIP));
    main_view_layout->AddView(bit_rate_tip);


    //main_view_layout->StartRowWithPadding(1.0f, 1, 0, GetLengthByDPIScale(20));
    //label = new BililiveLabel(L"音频输出");
    //label->SetFont(ftFourteenBold);
    //label->SetTextColor(GetColor(LabelTitle));
    //label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    //main_view_layout->AddView(label);


    main_view_layout->StartRowWithPadding(1.0f, 1, 0, GetLengthByDPIScale(10));
    label = new BililiveLabel(rb.GetLocalizedString(IDS_SRCPROP_CAMERA_AUDIO_OUTPUT));
    label->SetFont(ftFourteen);
    label->SetTextColor(GetColor(LivehimeColorType::TextTitle));
    label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    main_view_layout->AddView(label);

    audio_output_combox_ = new LivehimeSrcPropCombobox();
    audio_output_combox_->set_listener(this);
    main_view_layout->AddView(audio_output_combox_);

    main_view_->SetVisible(false);
}

void SourceAudioBaseView::InitData()
{
    RefreshAudioDeviceList();
    RefreshAudioOutputList();
    audio_output_combox_->SetSelectedData(presenter_->GetSelectedAudioOutput());

    if (presenter_->IsCustomAudioDevice())
    {
        audio_device_combox_->SetSelectedData(presenter_->SelectedAudioDevice());
    }
    else
    {
        audio_device_combox_->SetSelectedIndex(0);
    }

    if (presenter_->GetSceneItem())
    {
        auto vol_ctrl = CurrentSceneCollection()->GetVolumeControllerForAudioSource(presenter_->GetSceneItem());
        if (vol_ctrl)
        {
            SetVolume(MapFloatFromInt(std::round(vol_ctrl->GetDeflection() * 100.0f)));
            monitor_combobox_->SetSelectedIndex(vol_ctrl->GetMonitoringType());
            SetMuteBtnState(vol_ctrl->IsMuted());
        }
    }

    Layout();
}

void SourceAudioBaseView::SetCheck(bool check)
{
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();

    audio_setting_checkbox_->set_tag(check);
    main_view_->SetVisible(audio_setting_checkbox_->tag());

    if (check)
    {
        audio_setting_checkbox_->SetAllStateImage(rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_RADIO_ON));
    }
    else
    {
        audio_setting_checkbox_->SetAllStateImage(rb.GetImageSkiaNamed(IDR_CAMERA_BEAUTY_RADIO_OFF));
    }

    if (GetWidget() && GetWidget()->GetContentsView())
    {
        GetWidget()->GetContentsView()->Layout();
    }
}

bool SourceAudioBaseView::Checked()
{
    return audio_setting_checkbox_->tag();
}

void SourceAudioBaseView::RefreshAudioDeviceList()
{
    auto list = presenter_->CustomAudioDeviceList();
    list.insert(list.begin(), {GetLocalizedString(IDS_SRCPROP_CAMERA_AUDIO_DEFAULT_DEVICE), "0", true});
    audio_device_combox_->LoadList<std::string>(list);
}

void SourceAudioBaseView::RefreshAudioOutputList()
{
    PropertyList<int64> list;

    list.push_back(std::make_tuple(GetLocalizedString(IDS_SRCPROP_CAMERA_AUDIOOUTPUT_STREAM), 0LL, true));
    list.push_back(std::make_tuple(GetLocalizedString(IDS_SRCPROP_CAMERA_AUDIOOUTPUT_DIRECTSOUND), 1LL, true));
    list.push_back(std::make_tuple(GetLocalizedString(IDS_SRCPROP_CAMERA_AUDIOOUTPUT_WAVEOUT), 2LL, true));

    audio_output_combox_->LoadList<int64>(list);
}

void SourceAudioBaseView::SetVolume(float val)
{
    volume_label_->SetText(base::StringPrintf(L"%d", MapFloatToInt(val)));
    volume_slider_->SetValue(val);
}

void SourceAudioBaseView::EffectiveImmediately(CameraEffectiveControl effetive_control)
{
    switch (effetive_control)
    {
    case CAMERA_AUDIO_OUTPUT:
        presenter_->SetSelectedAudioOutput(audio_output_combox_->GetSelectedData<int64>());
        break;

    case CAMERA_AUDIO_DEVICE:
    {
        auto dev_str = audio_device_combox_->GetSelectedData<std::string>();

        if (dev_str == "0")
        {
            //默认设备
            presenter_->SetCustomAudioDevice(false);
        }
        else
        {
            presenter_->SelectedAudioDevice(audio_device_combox_->GetSelectedData<std::string>());
            presenter_->SetCustomAudioDevice(true);
        }
        break;
    }
    default:
        break;
    }

    presenter_->Update();
}

void SourceAudioBaseView::SetMuteBtnState(bool mute)
{
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();
	int normal_id = mute ? IDR_PRESET_MATERIAL_MUTE_MIC : IDR_PRESET_MATERIAL_MIC;
	int hover_id = mute ? IDR_PRESET_MATERIAL_MUTE_MIC_HOVER : IDR_PRESET_MATERIAL_MIC_HOVER;
	mic_mute_btn_->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(normal_id));
	mic_mute_btn_->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(hover_id));
	mic_mute_btn_->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(hover_id));
}

