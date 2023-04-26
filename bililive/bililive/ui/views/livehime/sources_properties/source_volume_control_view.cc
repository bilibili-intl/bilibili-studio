#include "source_volume_control_view.h"

#include "base/strings/stringprintf.h"

#include "grit/theme_resources.h"
#include "grit/generated_resources.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_hover_tip_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"

#include "obs/obs_proxy/public/proxy/obs_volume_controller.h"
#include "obs/obs_proxy/public/proxy/obs_scene_collection.h"

namespace
{
    static obs_proxy::SceneCollection* CurrentSceneCollection()
    {
        return OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    }

    enum CtrlID
    {
        Button_Muted = 1,
    };
}


SouceVolumeControlView::SouceVolumeControlView(obs_proxy::SceneItem* sceneItem,
    SourceVolumeCtrlType type, bool for_camera)
    : muted_button_(nullptr)
    , volume_slider_(nullptr)
    , volume_value_label_(nullptr)
    , sceneItem_(sceneItem)
    , type_(type)
    , need_listening_(false)
    , for_camera_(for_camera)
{
}

SouceVolumeControlView::SouceVolumeControlView(SourceVolumeCtrlType type)
    : muted_button_(nullptr)
    , volume_slider_(nullptr)
    , volume_value_label_(nullptr)
    , sceneItem_(nullptr)
    , type_(type)
    , need_listening_(false)
{
}

SouceVolumeControlView::~SouceVolumeControlView()
{
}

void SouceVolumeControlView::SetMutedButtonState(bool is_muted)
{
    if (!muted_button_)
        return;

    int normal_id = 0;
    int hover_id = 0;
    int dis_id = 0;
    int tipstr_id = 0;
    if (type_ == SourceVolumeCtrlType::Media || type_ == SourceVolumeCtrlType::AudioOutputDevice ||
        type_ == SourceVolumeCtrlType::Mobile || type_ == SourceVolumeCtrlType::Browser)
    {
        normal_id = is_muted ? IDR_LIVEHIME_V3_TOOLBAR_VOL_MUTE : IDR_LIVEHIME_V3_TOOLBAR_VOL;
        hover_id = is_muted ? IDR_LIVEHIME_V3_TOOLBAR_VOL_MUTE_HV : IDR_LIVEHIME_V3_TOOLBAR_VOL_HV;
        tipstr_id = is_muted ? IDS_SRCPROP_COMMON_MUTED_CANCEL_TIP : IDS_SRCPROP_COMMON_MUTED_TIP;
        dis_id = IDR_LIVEHIME_V3_TOOLBAR_VOL_DIS;
    }
    else
    {
        normal_id = is_muted ? IDR_LIVEHIME_V3_TOOLBAR_MIC_MUTE : IDR_LIVEHIME_V3_TOOLBAR_MIC;
        hover_id = is_muted ? IDR_LIVEHIME_V3_TOOLBAR_MIC_MUTE_HV : IDR_LIVEHIME_V3_TOOLBAR_MIC_HV;
        tipstr_id = is_muted ? IDS_SRCPROP_COMMON_MUTED_CANCEL_TIP : IDS_SRCPROP_COMMON_MUTED_TIP;
        dis_id = IDR_LIVEHIME_V3_TOOLBAR_MIC_DIS;
    }

    muted_button_->SetImage(views::Button::STATE_NORMAL, GetImageSkiaNamed(normal_id));
    muted_button_->SetImage(views::Button::STATE_HOVERED, GetImageSkiaNamed(hover_id));
    muted_button_->SetImage(views::Button::STATE_PRESSED, GetImageSkiaNamed(hover_id));
    muted_button_->SetImage(views::Button::STATE_DISABLED, GetImageSkiaNamed(dis_id));
    muted_button_->SetTooltipText(GetLocalizedString(tipstr_id));
    muted_button_->SchedulePaint();
}

void SouceVolumeControlView::SetVolume(float val)
{
    if (volume_value_label_ && volume_slider_)
    {
        volume_value_label_->SetText(base::StringPrintf(L"%d", MapFloatToInt(val)));
        volume_slider_->SetValue(val);
    }
}

void SouceVolumeControlView::InitViews()
{
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();
    views::GridLayout* layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    static int kMinWidth = LivehimeContentLabel::GetFont().GetStringWidth(L"0000");

    BililiveLabel* label = nullptr;
    if (type_ == SourceVolumeCtrlType::Mobile)
    {
        label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_SRCPROP_MOBILE_VOLUME));
    }
    else
    {
        label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_TOOLBAR_VOLUME));
    }


    if (for_camera_) {
        label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
        label->SetTextColor(SkColorSetRGB(0x8D, 0x9A, 0xA4));
        label->SetFont(ftThirteen);
    }
    else {
        label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    }

    views::ColumnSet* column_set = layout->AddColumnSet(0);
    if (for_camera_) {
        column_set->AddColumn(views::GridLayout::LEADING, 
            views::GridLayout::CENTER, 0, views::GridLayout::FIXED, GetLengthByDPIScale(70), 0);
    }
    else {
        column_set->AddColumn(views::GridLayout::TRAILING, 
            views::GridLayout::CENTER, 0, views::GridLayout::FIXED, GetLengthByDPIScale(75), 0);
        column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    }
    //column_set->AddColumn(views::GridLayout::LEADING, views::GridLayout::CENTER, 0, views::GridLayout::FIXED, GetLengthByDPIScale(70), 0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 1.0f, views::GridLayout::FIXED, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    if (for_camera_) {
        column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::CENTER, 0, views::GridLayout::USE_PREF, 0, 0);
    }

    muted_button_ = new BililiveImageButton(this);
    muted_button_->set_id(Button_Muted);

    volume_slider_ = new LivehimeSlider(this, views::Slider::HORIZONTAL);
    volume_slider_->SetKeyboardIncrement(1.0f / 100.f);

    volume_value_label_ = new LivehimeContentLabel(L"0");
    volume_value_label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    volume_value_label_->SetPreferredSize(gfx::Size(kMinWidth, 1));

    layout->StartRow(0, 0);
    layout->AddView(label);
    layout->AddView(muted_button_);
    layout->AddView(volume_slider_);
    layout->AddView(volume_value_label_);
    if (for_camera_) {
        auto volume_tip = new LivehimeHoverTipButton(GetLocalizedString(IDS_SRCPROP_COMMON_TIPS_TITLE),
            GetLocalizedString(IDS_SRCPROP_CAMERA_AUDIO_TIP));
        layout->AddView(volume_tip);
    }

    label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_SRCPROP_AUDIO_DEVICE_MONITER));
    if (for_camera_) {
        label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
        label->SetTextColor(SkColorSetRGB(0x8D, 0x9A, 0xA4));
        label->SetFont(ftThirteen);
    }
    else {
        label->SetHorizontalAlignment(gfx::ALIGN_RIGHT);
    }

    monitor_combobox_ = new LivehimeSrcPropCombobox();
    monitor_combobox_->set_listener(this);

    monitor_combobox_->AddItem(rb.GetLocalizedString(IDS_SRCPROP_AUDIO_DEVICE_MONITER_NONE));
    monitor_combobox_->AddItem(rb.GetLocalizedString(IDS_SRCPROP_AUDIO_DEVICE_MONITER_ONLY));
    monitor_combobox_->AddItem(rb.GetLocalizedString(IDS_SRCPROP_AUDIO_DEVICE_MONITER_AND_OUTPUT));
    monitor_combobox_->SetSelectedIndex(0);
    monitor_combobox_->SetFont(ftTwelve);

    auto help_view = new livehime::HelpSupportView(monitor_combobox_,
        livehime::HelpType::SourceMediaCaptureOption, kPaddingColWidthForGroupCtrls);

    layout->AddPaddingRow(0, kPaddingRowHeightForGroupCtrls);
    layout->StartRow(0, 0);
    layout->AddView(label);
    int col_span = for_camera_ ? 6 : 5;
    layout->AddView(help_view, col_span, 1);

    SetVolume(1.0f);
    SetMutedButtonState(false);

    if (sceneItem_)
    {
        auto vol_ctrl = CurrentSceneCollection()->GetVolumeControllerForAudioSource(sceneItem_);
        if (vol_ctrl)
        {
            SetVolume(MapFloatFromInt(std::round(vol_ctrl->GetDeflection()*100.0f)));
            SetMutedButtonState(vol_ctrl->IsMuted());
            monitor_combobox_->SetSelectedIndex(vol_ctrl->GetMonitoringType());
        }
    }
}

void SouceVolumeControlView::SliderValueChanged(views::Slider* sender, float value,
    float old_value, views::SliderChangeReason reason)
{
    if (volume_slider_ == sender)
    {
        if (need_listening_)
        {
            if (sceneItem_)
            {
                auto vol_ctrl = CurrentSceneCollection()->GetVolumeControllerForAudioSource(sceneItem_);
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
                    SetVolume(MapFloatFromInt(std::round(vol_ctrl->GetDeflection()*100.0f)));
                    SetMutedButtonState(vol_ctrl->IsMuted());
                }
            }
        }
        else
        {
            need_listening_ = true;
        }

    }
}

void SouceVolumeControlView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    switch (sender->id())
    {
    case Button_Muted:
        {
            if (sceneItem_)
            {
                auto vol_ctrl = CurrentSceneCollection()->GetVolumeControllerForAudioSource(sceneItem_);
                if (vol_ctrl)
                {
                    if (vol_ctrl->IsMuted())
                    {
                        vol_ctrl->SetMuted(false);
                    }
                    else
                    {
                        vol_ctrl->SetMuted(true);
                    }
                    SetMutedButtonState(vol_ctrl->IsMuted());
                }
            }
        }
        break;
    default:
        break;
    }
}

void SouceVolumeControlView::SetSceneItem(obs_proxy::SceneItem* sceneItem)
{
    sceneItem_ = sceneItem;
    if (sceneItem_)
    {
        auto vol_ctrl = CurrentSceneCollection()->GetVolumeControllerForAudioSource(sceneItem_);
        if (vol_ctrl)
        {
            SetVolume(MapFloatFromInt(std::round(vol_ctrl->GetDeflection()*100.0f)));
            SetMutedButtonState(vol_ctrl->IsMuted());
            monitor_combobox_->SetSelectedIndex(vol_ctrl->GetMonitoringType());
        }
    }
}

void SouceVolumeControlView::ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details)
{
    if (details.child == this)
    {
        if (details.is_add)
        {
            InitViews();

            if (OBSProxyService::GetInstance().obs_ui_proxy())
            {
                OBSProxyService::GetInstance().obs_ui_proxy()->AddObserver(this);
            }
        }
        else
        {
            if (OBSProxyService::GetInstance().obs_ui_proxy())
            {
                OBSProxyService::GetInstance().obs_ui_proxy()->RemoveObserver(this);
            }
        }
    }
}

void SouceVolumeControlView::OnAudioSourceActivate(obs_proxy::VolumeController* audio_source)
{
    if (sceneItem_)
    {
        auto vol_ctrl = CurrentSceneCollection()->GetVolumeControllerForAudioSource(sceneItem_);
        if (audio_source == vol_ctrl)
        {
            if (type_ == SourceVolumeCtrlType::Mobile)
            {
                vol_ctrl->SetMuted(false);
                vol_ctrl->SetDeflection(1.0f);
            }
            SetMutedButtonState(vol_ctrl->IsMuted());
            SetVolume(MapFloatFromInt(std::round(vol_ctrl->GetDeflection()*100.0f)));
            monitor_combobox_->SetSelectedIndex(vol_ctrl->GetMonitoringType());
        }
    }
}

void SouceVolumeControlView::OnSelectedIndexChanged(BililiveComboboxEx* combobox)
{
    if (combobox == monitor_combobox_)
    {
        if (sceneItem_)
        {
            auto vol_ctrl = CurrentSceneCollection()->GetVolumeControllerForAudioSource(sceneItem_);
            if (vol_ctrl)
            {
                vol_ctrl->SetMonitoringType(monitor_combobox_->selected_index());
            }
        }
    }
}