#include "bililive/bililive/ui/views/livehime/volume/bililive_sound_effect_view.h"

#include <functional>
#include <mutex>

#include "base/strings/utf_string_conversions.h"
#include "base/strings/stringprintf.h"

#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/command_updater_delegate.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/settings/settings_presenter_impl.h"
#include "bililive/bililive/livehime/volume/bililive_volume_controllers_devices_present_impl.h"
#include "bililive/bililive/livehime/volume/sound_effect_property_presenter_impl.h"
#include "bililive/bililive/ui/views/controls/linear_layout.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_hover_tip_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_slider.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/settings/base_settings_view.h"

#include "bililive/bililive/ui/views/livehime/settings/denoise_slider.h"
#include "bililive/bililive/ui/views/toolbar/toolbar_volume_ctrl.h"

#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"

#include "obs/obs_proxy/public/proxy/obs_proxy.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"

#include "grit/generated_resources.h"
#include "grit/theme_resources.h"

#define MIC_INIT        1
#define SPEAKER_INIT    2

enum SoundEffectViewColumnId{
    TITLE_COLUMN_ID,
    MIC_VOLUME_COLUMN_ID,
    DENOISE_SLIDER_COLUMN_ID,
    RADIO_COLUMN_ID,
    CHECKBOX_COLUMN_ID,
    SLIDER_COLUMN_ID,
    MIC_COLUMN_ID,
    RESTORE_COLUMN_ID
};

const int kVolThickness = 15;
const double kUpdateFrequencySecond = 1.0;

const int kSliderRangeMin = 0;
const int kSliderRangeMax = 100;

const int kDenoiseSliderId = 1013;

const int kAudioButtonStartId = 10031;
const int kAudioNoMixRadioButtonId = 10031;
const int kAudioLToDualRadioButtonId = 10032;
const int kAudioRToDualRadioButtonId = 10033;
const int kAudioMixRadioButtonId = 10034;


// BililiveSoundEffectView
BililiveSoundEffectView::BililiveSoundEffectView() :
      mic_controllers_(std::make_unique<BililiveVolumeControllersPresenterImpl>()),
      speaker_controllers_(std::make_unique<BililiveVolumeControllersPresenterImpl>()),
      audio_setting_presenter_(std::make_unique<SettingsPresenterImpl>(this)),
      denoise_slider_(nullptr),
      weakptr_factory_(this)
{
}

BililiveSoundEffectView::~BililiveSoundEffectView()
{
}

void BililiveSoundEffectView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {        
            InitViews();

            // 初始一律禁用，等obs底层成功添加了audio/video的时候通过回调决定是否启用
            SetVolumeBtnState(true, false);
            SetVolumeBtnState(false, false);
            SetVolumeBtnEnable(true, false);
            SetVolumeBtnEnable(false, false);

            // 界面创建时scenecolllection还没创建，不能拿到音量数据，此时InitData是失败的，
            // 若程序已正常运行了则成功，就不需要等到OnAudioSourceActivateHandler去进行init了
            if (OBSProxyService::GetInstance().obs_ui_proxy()->is_initialized()) 
            {
                InitVolumeControllerData(true);
                InitVolumeControllerData(false);
            }
            OBSProxyService::GetInstance().obs_ui_proxy()->AddObserver(this);

            InitData();

            notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_VOLUME_SYSTEM_DISABLE,
                base::NotificationService::AllSources());
            notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_VOLUME_SYSTEM_ENABLE,
                base::NotificationService::AllSources());
        }
        else 
        {
            if (OBSProxyService::GetInstance().obs_ui_proxy())
            {
                OBSProxyService::GetInstance().obs_ui_proxy()->RemoveObserver(this);
            }

            notifation_register_.RemoveAll();
        }
    }
}

void BililiveSoundEffectView::InitViews()
{
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);
    layout->SetInsets(insets_);

    auto col_set = layout->AddColumnSet(0);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);

    col_set = layout->AddColumnSet(1);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.2125f, views::GridLayout::FIXED, 0, 0);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.7875f, views::GridLayout::FIXED, 0, 0);

    col_set = layout->AddColumnSet(2);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.2125f, views::GridLayout::FIXED, 0, 0);
    col_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0.049f, views::GridLayout::FIXED, 0, 0);
    col_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0.5908f, views::GridLayout::FIXED, 0, 0);
    col_set->AddColumn(views::GridLayout::TRAILING, views::GridLayout::CENTER, 0.1477f, views::GridLayout::FIXED, 0, 0);

    auto label_color = GetColor(LabelTitle);
    auto label1 = new BililiveLabel(GetLocalizedString(IDS_CONFIG_AUDIO_TITLE), ftFourteenBold);
    label1->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    label1->SetTextColor(label_color);

    auto label2 = new BililiveLabel(GetLocalizedString(IDS_CONFIG_AUDIO_DEVICE), ftFourteen);
    label2->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    label2->SetTextColor(label_color);

    auto label3 = new BililiveLabel(GetLocalizedString(IDS_CONFIG_AUDIO_INPUT_VOLUME), ftFourteen);
    label3->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    label3->SetTextColor(label_color);

    auto label4 = new BililiveLabel(GetLocalizedString(IDS_CONFIG_AUDIO_SPEAKER_SETTING), ftFourteenBold);
    label4->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    label4->SetTextColor(label_color);

    auto label5 = new BililiveLabel(GetLocalizedString(IDS_CONFIG_AUDIO_DEVICE), ftFourteen);
    label5->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    label5->SetTextColor(label_color);

    auto label6 = new BililiveLabel(GetLocalizedString(IDS_CONFIG_AUDIO_OUTPUT_VOLUME), ftFourteen);
    label6->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    label6->SetTextColor(label_color);

    mic_adv_label_ = new BililiveLabel(GetLocalizedString(IDS_CONFIG_AUDIO_MAIN_MIC_ADV_SETTING), ftFourteenBold);
    mic_adv_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    mic_adv_label_->SetTextColor(label_color);

    mic_denoise_label_ = new BililiveLabel(GetLocalizedString(IDS_CONFIG_AUDIO_MICPHONE_DENOISE), ftFourteenBold);
    mic_denoise_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    mic_denoise_label_->SetTextColor(label_color);

    audio_rate_label_ = new BililiveLabel(GetLocalizedString(IDS_CONFIG_AUDIO_RATE), ftFourteen);
    audio_rate_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    audio_rate_label_->SetTextColor(label_color);

    sound_track_label_ = new BililiveLabel(GetLocalizedString(IDS_CONFIG_AUDIO_TRACE), ftFourteen);
    sound_track_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    sound_track_label_->SetTextColor(label_color);

    volume_tip_ = new LivehimeHoverTipButton(GetLocalizedString(IDS_SRCPROP_COMMON_TIPS_TITLE),
        GetLocalizedString(IDS_CONFIG_AUDIO_MONO_CAN_TRY));

    bit_rate_tip_ = new LivehimeHoverTipButton(GetLocalizedString(IDS_SRCPROP_COMMON_TIPS_TITLE),
        GetLocalizedString(IDS_CONFIG_AUDIO_RATE_TIP));

    mic_mute_btn_ = new BililiveImageButton(this);
    mic_mute_btn_->set_id(0);
    mic_mute_btn_->SetPreferredSize(GetSizeByDPIScale({12, 12}));
    mic_mute_btn_->SetAllStateImage(rb.GetImageSkiaNamed(IDR_PRESET_MATERIAL_MIC));

    speaker_mute_btn_ = new BililiveImageButton(this);
    speaker_mute_btn_->set_id(0);
    speaker_mute_btn_->SetPreferredSize(GetSizeByDPIScale({ 12, 12 }));
    speaker_mute_btn_->SetAllStateImage(rb.GetImageSkiaNamed(IDR_PRESET_MATERIAL_SPEAKER));

    denoise_slider_ = new DenoiseSlider(this);
    denoise_slider_->set_id(kDenoiseSliderId);

    mic_combobox_ = new LivehimeCombobox();
    mic_combobox_->SetDefaultItem(rb.GetLocalizedString(IDS_COMMON_NO_CAMERA), std::string());

    speaker_combobox_ = new LivehimeCombobox();
    speaker_combobox_->SetDefaultItem(rb.GetLocalizedString(IDS_COMMON_NO_SYS_VOICE), std::string());

    mic_volume_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
    speaker_volume_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);

    mic_volume_label_ = new BililiveLabel(L"0", ftFourteen);
    mic_volume_label_->SetPreferredSize(GetSizeByDPIScale({ 60, 18 }));
    mic_volume_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_RIGHT);
    mic_volume_label_->SetTextColor(label_color);

    speaker_volume_label_ = new BililiveLabel(L"0", ftFourteen);
    speaker_volume_label_->SetPreferredSize(GetSizeByDPIScale({60, 18}));
    speaker_volume_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_RIGHT);
    speaker_volume_label_->SetTextColor(label_color);

    bitrate_combobox_ = new LivehimeCombobox();

    track_combobox_ = new LivehimeCombobox();
    track_combobox_->set_listener(this);
    track_combobox_->AddItem(rb.GetLocalizedString(IDS_CONFIG_AUDIO_MIC_NO_MIX), kAudioNoMixRadioButtonId);
    track_combobox_->AddItem(rb.GetLocalizedString(IDS_CONFIG_AUDIO_MIC_L_TO_DUAL), kAudioLToDualRadioButtonId);
    track_combobox_->AddItem(rb.GetLocalizedString(IDS_CONFIG_AUDIO_MIC_R_TO_DUAL), kAudioRToDualRadioButtonId);
    track_combobox_->AddItem(rb.GetLocalizedString(IDS_CONFIG_AUDIO_MIC_MIX), kAudioMixRadioButtonId);

    reverb_view_ = new SoundReverbView(this);

	auto bit_rate_sub_view = CreateSubView(audio_rate_label_, bit_rate_tip_, GetLengthByDPIScale(4));
	auto volume_sub_view = CreateSubView(sound_track_label_, volume_tip_, GetLengthByDPIScale(4));

    layout->StartRow(1.0f, 0);
    layout->AddView(label1);

    layout->StartRowWithPadding(1.0f, 1, 0.0f, GetLengthByDPIScale(10));
    layout->AddView(label2);
    layout->AddView(mic_combobox_);

    layout->StartRowWithPadding(1.0f, 2, 0.0f, GetLengthByDPIScale(10));
    layout->AddView(label3);
    layout->AddView(mic_mute_btn_);
    layout->AddView(mic_volume_slider_);
    layout->AddView(mic_volume_label_);

    layout->StartRowWithPadding(1.0f, 0, 0.0f, GetLengthByDPIScale(30));
    layout->AddView(label4);

    layout->StartRowWithPadding(1.0f, 1, 0.0f, GetLengthByDPIScale(10));
    layout->AddView(label5);
    layout->AddView(speaker_combobox_);

    layout->StartRowWithPadding(1.0f, 2, 0.0f, GetLengthByDPIScale(10));
    layout->AddView(label6);
    layout->AddView(speaker_mute_btn_);
    layout->AddView(speaker_volume_slider_);
    layout->AddView(speaker_volume_label_);

    layout->StartRowWithPadding(1.0f, 0, 0.0f, GetLengthByDPIScale(30));
    layout->AddView(mic_adv_label_);

    layout->StartRowWithPadding(1.0f, 1, 0.0f, GetLengthByDPIScale(10));
    layout->AddView(mic_denoise_label_);
    layout->AddView(denoise_slider_);

    layout->StartRowWithPadding(1.0f, 1, 0.0f, GetLengthByDPIScale(10));
    layout->AddView(bit_rate_sub_view);
    layout->AddView(bitrate_combobox_);

    layout->StartRowWithPadding(1.0f, 1, 0.0f, GetLengthByDPIScale(10));
    layout->AddView(volume_sub_view);
    layout->AddView(track_combobox_);

    layout->StartRowWithPadding(1.0f, 0, 0.0f, GetLengthByDPIScale(16));
    layout->AddView(reverb_view_);

    ShowMicWidget(false);
}

void BililiveSoundEffectView::InitData()
{
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();

    if (presenter_) 
    {
        presenter_->GetIsReverbEnabled();
    }

    reverb_view_->SetPresenter(presenter_.get());

    //麦克风设备combbox
    auto mic_list = SoundEffectPropertyPresenterImpl::GetDeviceList(obs_proxy::kDefaultInputAudio);
    for (auto& mic : mic_list)
    {
        if (mic.dev_name == L"Default") 
        {
            int index = mic_combobox_->AddItem(
                rb.GetLocalizedString(IDS_CONFIG_AUDIO_DEFAULTDEVICE),
                mic.dev_id);
            mic_combobox_->SetSelectedIndex(index);
        }
        else 
        {
            mic_combobox_->AddItem(mic.dev_name, mic.dev_id);
        }
    }

    int index = mic_combobox_->FindItemData(SoundEffectPropertyPresenterImpl::GetCurMicId());
    if (-1 != index)
    {
        mic_combobox_->SetSelectedIndex(index);
    }

    //扬声器设备combbox
    auto speaker_list = SoundEffectPropertyPresenterImpl::GetDeviceList(obs_proxy::kDefaultOutputAudio);
    for (auto& speaker : speaker_list)
    {
        if (speaker.dev_name == L"Default")
        {
            int index = speaker_combobox_->AddItem(
                rb.GetLocalizedString(IDS_CONFIG_AUDIO_DEFAULTDEVICE),
                speaker.dev_id);
            speaker_combobox_->SetSelectedIndex(index);
        }
        else
        {
            speaker_combobox_->AddItem(speaker.dev_name, speaker.dev_id);
        }
    }

    index = speaker_combobox_->FindItemData(SoundEffectPropertyPresenterImpl::GetCurSpeakerId());
    if (-1 != index)
    {
        speaker_combobox_->SetSelectedIndex(index);
    }

    //音频码率
    auto media_prefs = SoundEffectPropertyPresenterImpl::GetMediaPrefsInfo();
    index = 0;
    for (auto& iter : media_prefs.audio_settings.audio_bitrate_list)
    {
        int i = bitrate_combobox_->AddItem(base::UTF8ToUTF16(iter.second), iter.first);
        if (iter.first == media_prefs.audio_settings.audio_bitrate_default)
        {
            index = i;
        }
    }
    bitrate_combobox_->SetSelectedIndex(index);

    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
    int nIndex = bitrate_combobox_->FindItemData(pref->GetInteger(prefs::kAudioBitRate));
    bitrate_combobox_->SetSelectedIndex(nIndex != -1 ? nIndex : 0);
    if (presenter_)
    {
        //麦克风降噪
        denoise_slider_->set_stay_time(static_cast<int>(presenter_->GetDenoiseLevel()));

        //声道
        int audio_btn_index = presenter_->GetMonoStatus() - kAudioButtonStartId;
        DCHECK(audio_btn_index >= 0 && audio_btn_index < 4);
        track_combobox_->SetSelectedIndex(audio_btn_index);

        //混响
        bool is_reverb_enabled = presenter_->GetIsReverbEnabled();
        reverb_view_->GetEnableCheckBox()->SetChecked(is_reverb_enabled);
        reverb_view_->OnEnableCheckBoxChanged();

        presenter_->Snapshot();
    }
}

void BililiveSoundEffectView::InitVolumeControllerData(bool is_mic)
{
    contracts::BililiveVolumeControllersContract* controller = nullptr;
    base::string16 device_name;

    if (is_mic) 
    {
        device_name = base::UTF8ToUTF16(obs_proxy::kDefaultInputAudio);
        mic_controllers_->SelectController(device_name);
        controller = mic_controllers_.get();

        if (controller->ControllerIsValid())
        {
            if (!presenter_) 
            {
                presenter_.reset(new SoundEffectPropertyPresenterImpl());
            }

            mic_deflection_ = controller->GetDeflection();
            mic_db_ = controller->GetDB();

            reverb_view_->InitMicVolumeController(controller);

            ShowMicWidget(true);

            audio_inited_ |= MIC_INIT;
        }
    }
    else 
    {
        device_name = base::UTF8ToUTF16(obs_proxy::kDefaultOutputAudio);
        speaker_controllers_->SelectController(device_name);
        controller = speaker_controllers_.get();

        if (controller->ControllerIsValid()) 
        {
            speaker_deflection_ = controller->GetDeflection();
            speaker_db_ = controller->GetDB();

            audio_inited_ |= SPEAKER_INIT;
        }
    }

    if (!controller->ControllerIsValid()) 
    {
        return;
    }

    SetVolumeBtnEnable(is_mic, true);

    controller->RegisterVolumeChangedHandler(
        std::bind(&BililiveSoundEffectView::OnVolumeChanged, this, std::placeholders::_1, std::placeholders::_2));

    controller->RegisterVolumeMuteChangedHandler(
        std::bind(&BililiveSoundEffectView::OnVolumeMuteChanged, this, std::placeholders::_1, std::placeholders::_2));

    bool is_muted = controller->GetMuted();
    SetVolumeBtnState(is_mic, is_muted);
    SetVolumeSliderValue(is_mic);
}

void BililiveSoundEffectView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    if (sender == reverb_view_->GetEnableCheckBox()) 
    {
        reverb_view_->OnEnableCheckBoxChanged();
        auto rect = Container()->GetVisibleRect();
        Container()->ScrollToPosition(true, rect.height(), Api);
    }
    else if (sender == mic_mute_btn_)
    {
        //mic_controllers_->GetMuted();
       bool muted = mic_mute_btn_->id();
       muted = !muted;
       mic_controllers_->SetMuted(muted);
       SetVolumeBtnState(true, muted);
    }
    else if (sender == speaker_mute_btn_)
    {
        bool muted = speaker_mute_btn_->id();
        muted = !muted;
        speaker_controllers_->SetMuted(muted);
        SetVolumeBtnState(false, muted);
    }
}

void BililiveSoundEffectView::SliderValueChanged(views::Slider* sender, float value, float old_value, views::SliderChangeReason reason)
{
    if (sender == mic_volume_slider_) 
    {
        if (audio_inited_ & MIC_INIT && reason != views::VALUE_CHANGED_BY_API)
        {
            OnVolumeSliderValueChanged(mic_controllers_.get(), mic_volume_slider_, mic_volume_label_, value, old_value);
        }
    }
    else if (sender == speaker_volume_slider_)
    {
        if (audio_inited_ & SPEAKER_INIT && reason != views::VALUE_CHANGED_BY_API)
        {
            OnVolumeSliderValueChanged(speaker_controllers_.get(), speaker_volume_slider_, speaker_volume_label_, value, old_value);
        }
    }
    else if (sender == denoise_slider_)
    {
        if (presenter_) 
        {
            presenter_->SetDenoiseLevel(denoise_slider_->stay_time());
        }
    }
}

void BililiveSoundEffectView::OnAudioSourceActivate(obs_proxy::VolumeController* audio_source)
{
    std::string device_name = audio_source->GetBoundSourceName();

    if (device_name == obs_proxy::kDefaultInputAudio) 
    {
        InitVolumeControllerData(true);
    }
    else 
    {
        InitVolumeControllerData(false);
    }
}

void BililiveSoundEffectView::Observe(int type, const base::NotificationSource& source, const base::NotificationDetails& details)
{
    if (type == bililive::NOTIFICATION_LIVEHIME_VOLUME_SYSTEM_DISABLE)
    {
        SetEnableSystemVoice(false);
    }
    else if (type == bililive::NOTIFICATION_LIVEHIME_VOLUME_SYSTEM_ENABLE)
    {
        SetEnableSystemVoice(true);
    }
}

void BililiveSoundEffectView::OnSelectedIndexChanged(BililiveComboboxEx* combobox)
{
    if (!presenter_)
    {
        return;
    }

    std::pair<std::string, std::string> mono_string;
    GetMonoSettings(mono_string);
    presenter_->SetMonoStatus(mono_string.second);
}

bool BililiveSoundEffectView::CheckSettingsValid()
{
    return true;
}

bool BililiveSoundEffectView::SaveNormalSettingsChange()
{
    return true;
}

void BililiveSoundEffectView::OnInitComboData(const std::string& device, const std::wstring& devname, const std::string& devid)
{

}

void BililiveSoundEffectView::SaveOrCheckStreamingSettingsChange(bool check, ChangeType& result)
{
    SaveOrCheckAudioDeviceSetting(check, result);

    auto scene_collection = SoundEffectPropertyPresenterImpl::ApplyStreamingSettings();
    if (scene_collection) 
    {
        scene_collection->Save();
    }

    SaveReverbSetting();
}

void BililiveSoundEffectView::SetInsets(const gfx::Insets& insets)
{
    insets_ = insets;
}

void BililiveSoundEffectView::SaveReverbSetting()
{
    if (!presenter_)
    {
        return;
    }

    std::pair<std::string, std::string> mono_string;
    std::pair<std::string, int> denoise_val;
    denoise_val = std::make_pair(prefs::kAudioDenoise, denoise_slider_->stay_time());
    GetMonoSettings(mono_string);
    presenter_->AudioSettingSaveOrCancel(&denoise_val, &mono_string);

    reverb_view_->OnSaveChanged();
}

void BililiveSoundEffectView::OnCancel()
{
    if (!presenter_) 
    {
        return;
    }

    if (mic_controllers_ && mic_controllers_->ControllerIsValid())
    {
        mic_controllers_->SetDB(mic_db_);
        mic_controllers_->SetDeflection(mic_deflection_);
    }

    if (speaker_controllers_ && speaker_controllers_->ControllerIsValid()) 
    {
        speaker_controllers_->SetDB(speaker_db_);
        speaker_controllers_->SetDeflection(speaker_deflection_);
    }

    presenter_->EndPreview();
    presenter_->Restore();
    presenter_->AudioSettingSaveOrCancel(NULL, NULL);
}

void BililiveSoundEffectView::SetEnableSystemVoice(bool enabled)
{
    SetVolumeBtnEnable(false, enabled);

    // 连快捷键也一并禁用掉
    bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(),
        IDC_LIVEHIME_HOTKEY_SYSVOL_ENABLE,
        CommandParams<bool>(&enabled));

    // UI和快捷键都禁用了，当前程序中没有其他地方可以控制音量静音与否了,
    // 就不通过Commmand的方式去执行Controller的SetMuted，在这里控制比较方便
    if (speaker_controllers_->ControllerIsValid())
    {
        static bool prev_system_muted_status = false;
        if (!enabled)
        {
            prev_system_muted_status = speaker_controllers_->GetMuted();
            speaker_controllers_->SetMuted(true);
        }
        else
        {
            speaker_controllers_->SetMuted(prev_system_muted_status);
        }
    }
}

void BililiveSoundEffectView::GetMonoSettings(std::pair<std::string, std::string>& device_string)
{
    int index = track_combobox_->selected_index();
    DCHECK(index != -1);
    int mix_id = track_combobox_->GetItemData<int>(index);
    switch (mix_id)
    {
        case kAudioNoMixRadioButtonId:
            device_string = std::make_pair(prefs::kAudioMonoChannelSetup, prefs::kAudioMonoNoMix);
        break;

        case kAudioLToDualRadioButtonId:
            device_string = std::make_pair(prefs::kAudioMonoChannelSetup, prefs::kAudioMonoLToDouble);
        break;

        case kAudioRToDualRadioButtonId:
            device_string = std::make_pair(prefs::kAudioMonoChannelSetup, prefs::kAudioMonoRToDouble);
        break;

        case kAudioMixRadioButtonId:
            device_string = std::make_pair(prefs::kAudioMonoChannelSetup, prefs::kAudioMonoMix);
        break;

        default:
            DCHECK(0);
    }
}

//创建一个由标签加按钮的组成子view
views::View* BililiveSoundEffectView::CreateSubView(views::View* label, views::View* button, int padding)
{
    views::View* sub_view = new views::View();
    bililive::LinearLayout* layout = new bililive::LinearLayout(bililive::LinearLayout::kHoirzontal, sub_view);
    sub_view->SetLayoutManager(layout);
    layout->AddView(label, bililive::LinearLayout::kLeading, bililive::LinearLayout::kCenter);
    layout->AddView(button, bililive::LinearLayout::kLeading, bililive::LinearLayout::kCenter, padding);

    return sub_view;
}

//麦克风，扬声器，码率设置保持
void BililiveSoundEffectView::SaveOrCheckAudioDeviceSetting(bool check, BaseSettingsView::ChangeType& result)
{
    std::map<std::string, std::string> device_string;
    std::map<std::string, int> device_integer;

    // 麦克风设备
    int index = mic_combobox_->selected_index();
    if (-1 != index) {
        std::string str = mic_combobox_->GetItemData<std::string>(index);
        if (!str.empty()) {
            device_string.insert(std::make_pair(prefs::kAudioMicDeviceInUse, str));
        }
    }

    // 扬声器设备
    index = speaker_combobox_->selected_index();
    if (-1 != index) {
        std::string str = speaker_combobox_->GetItemData<std::string>(index);
        if (!str.empty()) {
            device_string.insert(std::make_pair(prefs::kAudioSpeakerDeviceInUse, str));
        }
    }

    // 码率
    device_integer.insert(std::make_pair(prefs::kAudioBitRate,
        bitrate_combobox_->GetItemData<int>(bitrate_combobox_->selected_index())));

    // 要修改这里时，注意一下 device_string 和 device_integer 对应的 ChangeType 值
    audio_setting_presenter_->AudioSaveChange(device_string, device_integer, check, result);
    
}

void BililiveSoundEffectView::OnVolumeSliderValueChanged(
    contracts::BililiveVolumeControllersContract* controller,
    views::Slider* sender, 
    BililiveLabel* label, 
    float value, 
    float old_value)
{
    label->SetText(VolumeSliderValueToString(value));

    //如果过了刻度线，就改变键盘操作刻度
    if (UpdateVolumeSliderKeyboardIncrement(sender, old_value))
    {
        //阻止尝试跨越100这个边界的行为
        value = 0.5f;
        sender->SetValue(0.5f);
    }

    if (old_value != value)
    {
        float deflection, gainDB;
        SplitVolumeSliderValueToDeflectionAndGain(value, &deflection, &gainDB);

        // 声音一旦变动视为解除静音
        bool is_mic = controller == mic_controllers_.get();
        SetVolumeBtnState(is_mic, false);

        if (controller->ControllerIsValid())
        {
            is_self_operation_ = true;

            controller->SetMuted(false);
            controller->SetDeflection(deflection);
            controller->SetDB(gainDB);

            is_self_operation_ = false;
        }
    }
}

void BililiveSoundEffectView::OnVolumeChanged(const std::string& source_name, float db)
{
    if (is_self_operation_) 
    {
        return;
    }

    if (source_name == obs_proxy::kDefaultInputAudio)
    {
        base::MessageLoop::current()->PostTask(FROM_HERE,
            base::Bind(&BililiveSoundEffectView::OnVolumeChangedInternal, weakptr_factory_.GetWeakPtr(), true));
    }
    else if(source_name == obs_proxy::kDefaultOutputAudio)
    {
        base::MessageLoop::current()->PostTask(FROM_HERE,
            base::Bind(&BililiveSoundEffectView::OnVolumeChangedInternal, weakptr_factory_.GetWeakPtr(), false));
    }
}

void BililiveSoundEffectView::OnVolumeMuteChanged(const std::string& source_name, bool muted)
{
    if (source_name == obs_proxy::kDefaultInputAudio)
    {
        SetVolumeBtnState(true, muted);
    }
    else if(source_name == obs_proxy::kDefaultOutputAudio)
    {
        SetVolumeBtnState(false, muted);
    }
}

void BililiveSoundEffectView::OnVolumeChangedInternal(bool is_mic)
{
    if (is_mic) 
    {
        mic_volume_slider_->SetValue(MergeDeflectionAndGainToVolume(mic_controllers_->GetDeflection(), mic_controllers_->GetDB()));
    }
    else 
    {
        speaker_volume_slider_->SetValue(MergeDeflectionAndGainToVolume(speaker_controllers_->GetDeflection(), speaker_controllers_->GetDB()));
    }
}

void BililiveSoundEffectView::SetVolumeSliderValue(bool is_mic)
{
    if (is_mic) 
    {
        float v = MergeDeflectionAndGainToVolume(mic_controllers_->GetDeflection(), mic_controllers_->GetDB());
        mic_volume_slider_->SetValue(v);
        mic_volume_label_->SetText(VolumeSliderValueToString(v));
    }
    else 
    {
        float v = MergeDeflectionAndGainToVolume(speaker_controllers_->GetDeflection(), speaker_controllers_->GetDB());
        speaker_volume_slider_->SetValue(v);
        speaker_volume_label_->SetText(VolumeSliderValueToString(v));
    }
}

void BililiveSoundEffectView::ShowMicWidget(bool show)
{
    mic_adv_label_->SetVisible(show);
    mic_denoise_label_->SetVisible(show);
    audio_rate_label_->SetVisible(show);
    sound_track_label_->SetVisible(show);
    volume_tip_->SetVisible(show);
    denoise_slider_->SetVisible(show);
    bit_rate_tip_->SetVisible(show);
    bitrate_combobox_->SetVisible(show);
    track_combobox_->SetVisible(show);
    reverb_view_->SetVisible(show);
}

float BililiveSoundEffectView::MergeDeflectionAndGainToVolume(float deflection, float gain)
{
    return deflection * 0.5f + gain / obs_proxy::kAudioGain * 0.5;
}

bool BililiveSoundEffectView::UpdateVolumeSliderKeyboardIncrement(views::Slider* volSlider, float oldValue)
{
    volSlider->SetKeyboardIncrement(0.005f);

    int iVal = VolumeSliderValueToInteger(volSlider->value());
    int iOldVal = VolumeSliderValueToInteger(oldValue);

    if ((iOldVal - 100) * (iVal - 100) < 0)
        return true;
    else
        return false;
}

int BililiveSoundEffectView::VolumeSliderValueToInteger(float value)
{
    return (int)std::round(value * 200.0f);
}

void BililiveSoundEffectView::SplitVolumeSliderValueToDeflectionAndGain(float volume, float* deflection, float* gain)
{
    //对齐到整数刻度
    int iVal = VolumeSliderValueToInteger(volume);
    if (iVal <= 100)
    {
        *deflection = iVal / 100.0f;
        *gain = 0.0f;
    }
    else
    {
        *deflection = 1.0f;
        *gain = (iVal - 100) / 100.0f * obs_proxy::kAudioGain;
    }
}

std::wstring BililiveSoundEffectView::VolumeSliderValueToString(float value)
{
    int iVal = VolumeSliderValueToInteger(value);

    float deflection, gain;
    SplitVolumeSliderValueToDeflectionAndGain(value, &deflection, &gain);
    if (gain == 0.0f)
    {
        return std::to_wstring(iVal) + L"%";
    }
    else
    {
        wchar_t buf[64];
        swprintf_s<sizeof(buf) / sizeof(*buf)>(buf, L"%s%.1f%s", L"+", gain, L"dB");
        return buf;
    }
}

void BililiveSoundEffectView::SetVolumeBtnState(bool is_mic, bool mute)
{
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();
    int normal_id = 0;
    int hover_id = 0;
    if (is_mic) 
    {
        normal_id = mute ? IDR_PRESET_MATERIAL_MUTE_MIC : IDR_PRESET_MATERIAL_MIC;
        hover_id = mute ? IDR_PRESET_MATERIAL_MUTE_MIC_HOVER : IDR_PRESET_MATERIAL_MIC_HOVER;
        mic_mute_btn_->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(normal_id));
        mic_mute_btn_->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(hover_id));
        mic_mute_btn_->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(hover_id));
        mic_mute_btn_->set_id(mute);
    }
    else 
    {
        normal_id = mute ? IDR_PRESET_MATERIAL_MUTE_SPEAKER : IDR_PRESET_MATERIAL_SPEAKER;
        hover_id = mute ? IDR_PRESET_MATERIAL_MUTE_SPEAKER_HOVER : IDR_PRESET_MATERIAL_SPEAKER_HOVER;
        speaker_mute_btn_->SetImage(views::Button::STATE_NORMAL, rb.GetImageSkiaNamed(normal_id));
        speaker_mute_btn_->SetImage(views::Button::STATE_HOVERED, rb.GetImageSkiaNamed(hover_id));
        speaker_mute_btn_->SetImage(views::Button::STATE_PRESSED, rb.GetImageSkiaNamed(hover_id));
        speaker_mute_btn_->set_id(mute);
    }
}

void BililiveSoundEffectView::SetVolumeBtnEnable(bool is_mic, bool enable)
{
    if (is_mic)
    {
        mic_mute_btn_->SetEnabled(enable);
        mic_volume_slider_->SetEnabled(enable);
    }
    else 
    {
        speaker_mute_btn_->SetEnabled(enable);
        speaker_volume_slider_->SetEnabled(enable);
    }
}
