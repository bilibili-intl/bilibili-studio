#include "toolbar_volume_ctrl.h"

#include "bilibase/basic_types.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/help_center/help_center.h"
#include "bililive/bililive/livehime/volume/bililive_volume_controllers_devices_present_impl.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_slider.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/utils/bililive_canvas_drawer.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"

#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/painter.h"

#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"

#include "obs/obs_proxy/common/obs_proxy_constants.h"
#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"
#include "obs/obs_proxy/core_proxy/scene_collection/obs_scene_collection_impl.h"
#include "obs/obs_proxy/public/proxy/obs_proxy.h"


namespace
{
    using namespace std::placeholders;

    const int kVolSilderSize = GetLengthByDPIScale(70);
    const int kVolThickness = GetLengthByDPIScale(3);

    // 音量slider
    class VolumeSlider : public LivehimeSlider
    {
    public:
        explicit VolumeSlider(views::SliderListener* listener)
            : LivehimeSlider(listener, views::Slider::HORIZONTAL, BililiveSlider::SBA_CENTER)
            , is_muted_(false)
        {
            for (int channelNr = 0; channelNr < MAX_AUDIO_CHANNELS; channelNr++) {
                currentPeak_[channelNr] = -obs_proxy::kPeekRange;
            }

            if(orientation() == views::Slider::HORIZONTAL)
            {
                SetBarDetails(kVolThickness, SK_ColorTRANSPARENT, SK_ColorTRANSPARENT, false);
            }
            SetThumbImage(GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_SLIDER_HV),
                          GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_SLIDER_HV),
                          GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_SLIDER_DIS));
        }

        virtual ~VolumeSlider(){}

        void SetMuted(bool muted)
        {

            if (muted == is_muted_)
            {
                return;
            }

            is_muted_ = muted;
            SetThumbImage(GetImageSkiaNamed(is_muted_ ? IDR_LIVEHIME_V3_TOOLBAR_SLIDER : IDR_LIVEHIME_V3_TOOLBAR_SLIDER_HV),
                            GetImageSkiaNamed(is_muted_ ? IDR_LIVEHIME_V3_TOOLBAR_SLIDER : IDR_LIVEHIME_V3_TOOLBAR_SLIDER_HV),
                            GetImageSkiaNamed(IDR_LIVEHIME_V3_TOOLBAR_SLIDER_DIS));

            SchedulePaint();
        }

        void VolumeLevelUpdate(const std::vector<float>& magnitude,
                               const std::vector<float>& peak,
                               const std::vector<float>& inputPeak)
        {
            for (int channelNr = 0; channelNr < MAX_AUDIO_CHANNELS; channelNr++) {
                currentPeak_[channelNr] = peak[channelNr];
            }

            SchedulePaint();
        }

        // views::View overrides:
        gfx::Size GetPreferredSize() override
        {
            gfx::Size size;
            if (orientation() == views::Slider::VERTICAL)
            {
                static int cx = std::max(ftPrimary.GetStringWidth(L"100%"), ftPrimary.GetStringWidth(L"+00.0dB"));
                size.SetSize(cx, kVolSilderSize);
            }
            else
            {
                size = __super::GetPreferredSize();
                size.set_width(kVolSilderSize);
            }
            return size;
        }

    protected:
        // views::View overrides:
        void OnPaintBackground(gfx::Canvas* canvas) override
        {
            float value = __super::value();
            gfx::Rect content = GetContentsBoundsImp();

            if (orientation() == views::Slider::HORIZONTAL)
            {
                int vol_y = content.y() + std::round((content.height() - kVolThickness) * 1.0f / 2);

                static int kBarInsets = 4;// 3.0的滑块图边缘透明的区域较大，数值条左右多缩进一点

                SkPath ret_path;
                bool is_hovered = !is_muted_;
                SkPaint paint;
                paint.setAntiAlias(true);
                if (enabled())
                {
                    paint.setColor(is_hovered ? clrVolumeHoverGradientEnd : clrVolumeNormalGradientEnd);
                    bililive::DrawCircleRect(canvas, content.x() + kBarInsets, vol_y, content.width() - kBarInsets * 2, kVolThickness, paint,
                                             true, &ret_path);
                }
                else
                {
                    paint.setColor(clrVolumeDisable);
                    bililive::DrawCircleRect(canvas, content.x() + kBarInsets, vol_y, content.width() - kBarInsets * 2, kVolThickness, paint,
                                             true, &ret_path);
                }

                if (value != 0.0f)
                {
                    float peak = 0.0f;

                    float max_peek = obs_proxy::GetMaxPeek(currentPeak_);
                    peak = (obs_proxy::kPeekRange + max_peek) / obs_proxy::kPeekRange;

                    if (peak > 0.0)
                    {
                        int cx = peak * (content.width() - kBarInsets * 2);

                        scoped_ptr<views::Painter> painter(views::Painter::CreateHorizontalGradient(
                            is_hovered ? clrVolumeHoverGradientBegin : clrVolumeNormalGradientBegin,
                            is_hovered ? clrVolumeHoverGradientEnd : clrVolumeNormalGradientEnd));
                        canvas->Save();
                        canvas->sk_canvas()->clipPath(ret_path, SkRegion::kIntersect_Op, true);
                        views::Painter::PaintPainterAt(canvas, painter.get(), gfx::Rect(content.x() + kBarInsets, vol_y, cx, kVolThickness));
                        canvas->Restore();
                    }
                }
            }
        }

        void OnEnabledChanged() override
        {
            // 音量控件用的不是统一风格，直接越过统一风格的LivehimeSlider::OnEnabledChanged
            BililiveSlider::OnEnabledChanged();
        }

    private:
        bool is_muted_;
        float currentPeak_[MAX_AUDIO_CHANNELS];

        DISALLOW_COPY_AND_ASSIGN(VolumeSlider);
    };

}   // namespace


// ToolBarVolumeCtrl
ToolBarVolumeCtrl::ToolBarVolumeCtrl(VolumeCtrlType type)
    : type_(type)
    , muted_button_(nullptr)
    , slider_(nullptr)
    , volume_controllers_(std::make_unique<BililiveVolumeControllersPresenterImpl>())
    , weakptr_factory_(this)
{
    DCHECK(type < VolumeCtrlType::Count);

    device_name_ = ((type_ == VolumeCtrlType::System) ? obs_proxy::kDefaultOutputAudio : obs_proxy::kDefaultInputAudio);
}

ToolBarVolumeCtrl::~ToolBarVolumeCtrl()
{
}

VolumeCtrlType ToolBarVolumeCtrl::volume_type() const
{
    return type_;
}

gfx::Rect ToolBarVolumeCtrl::GetVolumeRect() const
{
    return slider_->bounds();
}

int ToolBarVolumeCtrl::GetRenderError() const
{
    return volume_controllers_->GetRenderError();
}

bool ToolBarVolumeCtrl::IsMute() const
{
    return volume_controllers_->GetMuted();
}

bool ToolBarVolumeCtrl::IsDeviceValid() const
{
    return volume_controllers_->ControllerIsValid();
}

void ToolBarVolumeCtrl::ViewHierarchyChanged(const View::ViewHierarchyChangedDetails &details)
{
    if (details.child == this)
    {
        if(details.is_add)
        {
            InitViews();

            SetMutedButtonState(false);

            // 初始一律禁用，等obs底层成功添加了audio/video的时候通过回调决定是否启用
            SetEnabled(false);

            // 界面创建时scenecolllection还没创建，不能拿到音量数据，此时InitData是失败的，
            // 若程序已正常运行了则成功，就不需要等到OnAudioSourceActivateHandler去进行init了
            if (OBSProxyService::GetInstance().obs_ui_proxy()->is_initialized())
            {
                InitData();
            }

            OBSProxyService::GetInstance().obs_ui_proxy()->AddObserver(this);

            if(type_ == VolumeCtrlType::System)
            {
                notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_VOLUME_SYSTEM_DISABLE,
                                         base::NotificationService::AllSources());
                notifation_register_.Add(this, bililive::NOTIFICATION_LIVEHIME_VOLUME_SYSTEM_ENABLE,
                                         base::NotificationService::AllSources());
            }
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

gfx::Size ToolBarVolumeCtrl::GetPreferredSize()
{
    gfx::Size pref_size = __super::GetPreferredSize();
    pref_size.Enlarge(slider_->GetPreferredSize().width(), 0);
    return pref_size;
}

void ToolBarVolumeCtrl::SetEnabled(bool enabled)
{
    __super::SetEnabled(enabled);

    muted_button_->SetEnabled(enabled);
    slider_->SetEnabled(enabled);
}

void ToolBarVolumeCtrl::InitViews()
{
    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    views::ColumnSet *column_set = layout->AddColumnSet(0);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 1.0f, views::GridLayout::FIXED, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
    column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL, 0, views::GridLayout::USE_PREF, 0, 0);

    muted_button_ = new BililiveImageButton(this);

    BililiveLabel *label = new LivehimeContentLabel(L"0%");
    static int label_cx = std::max(ftFourteen.GetStringWidth(L"100%"), ftFourteen.GetStringWidth(L"+10.0dB"));
    label->SetPreferredSize(gfx::Size(label_cx, ftTwelve.GetHeight()));
    label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    label->SetTextColor(clrTextALL);
    label->SetFont(ftTwelve);

    slider_ = new VolumeSlider(this);
    slider_->SetAssociationLabel(label);
    slider_->SetAssociationName(base::UTF8ToUTF16(device_name_));
    UpdateVolumeSliderKeyboardIncrement(slider_);

    layout->AddPaddingRow(1.0f, 0);
    layout->StartRow(0, 0);
    layout->AddView(muted_button_);
    layout->AddView(slider_);
    layout->AddView(label);
    layout->AddPaddingRow(1.0f, 0);
}

void ToolBarVolumeCtrl::InitData()
{
    if (volume_controllers_->ControllerIsValid())
    {
        return;
    }

    volume_controllers_->SelectController(base::UTF8ToUTF16(device_name_));
    if (volume_controllers_->ControllerIsValid())
    {
        SetEnabled(true);

        volume_controllers_->RegisterVolumeLevelUpdatedHandler(
            std::bind(&ToolBarVolumeCtrl::OnVolumeLevelUpdate, this, _1, _2, _3, _4));

        volume_controllers_->RegisterVolumeMuteChangedHandler(
            std::bind(&ToolBarVolumeCtrl::OnVolumeMuteChanged, this, _1, _2));

        volume_controllers_->RegisterVolumeChangedHandler(
            std::bind(&ToolBarVolumeCtrl::OnVolumeChanged, this, _1, _2));

        bool is_muted = volume_controllers_->GetMuted();
        SetMutedButtonState(is_muted);

        slider_->SetValue(MergeDeflectionAndGainToVolume(volume_controllers_->GetDeflection(), volume_controllers_->GetDB()));
    }
}

void ToolBarVolumeCtrl::OnVolumeChangedInternal()
{
    slider_->SetValue(MergeDeflectionAndGainToVolume(volume_controllers_->GetDeflection(), volume_controllers_->GetDB()));
}

void ToolBarVolumeCtrl::OnSlideVolumeChanged(const std::string& source_name,float db)
{
    if ((device_name_ == source_name) && volume_controllers_->ControllerIsValid())
    {
        slider_->SetValue(MergeDeflectionAndGainToVolume(volume_controllers_->GetDeflection(), volume_controllers_->GetDB()));
    }

    bool muted = (db < 0.0001f);
}

void ToolBarVolumeCtrl::OnVolumeChanged(const std::string& source_name, float db)
{
    if (!self_operating_)
    {
        // obs底层是先signal再设置的，所以在此不直接获取音量设置，抛任务获取即可
        base::MessageLoop::current()->PostTask(FROM_HERE, base::Bind(&ToolBarVolumeCtrl::OnSlideVolumeChanged,
            weakptr_factory_.GetWeakPtr(), source_name, db));
    }
}

void ToolBarVolumeCtrl::OnVolumeLevelUpdate(const std::string& source_name,
                                            const std::vector<float>& magnitude,
                                            const std::vector<float>& peak,
                                            const std::vector<float>& inputPeak)
{
    if (device_name_ == source_name)
    {
        slider_->VolumeLevelUpdate(magnitude, peak, inputPeak);
    }
}

void ToolBarVolumeCtrl::OnVolumeMuteChanged(const std::string& source_name, bool muted)
{
    if (device_name_ == source_name)
    {
        SetMutedButtonState(muted);
    }
}

void ToolBarVolumeCtrl::SetMutedButtonState(bool is_muted)
{
    int normal_id = 0;
    int hover_id = 0;
    int dis_id = 0;
    int tipstr_id = 0;
    if (type_ == VolumeCtrlType::System)
    {
        normal_id = is_muted ? IDR_LIVEHIME_V3_TOOLBAR_VOL_MUTE : IDR_LIVEHIME_V3_TOOLBAR_VOL;
        hover_id = is_muted ? IDR_LIVEHIME_V3_TOOLBAR_VOL_MUTE_HV : IDR_LIVEHIME_V3_TOOLBAR_VOL_HV;
        tipstr_id = is_muted ? IDS_TOOLBAR_VOLUME_SYSTEM_SOUND_OPEN_TIPS : IDS_TOOLBAR_VOLUME_SYSTEM_SOUND_CLOSE_TIPS;
        dis_id = IDR_LIVEHIME_V3_TOOLBAR_VOL_DIS;
    }
    else
    {
        normal_id = is_muted ? IDR_LIVEHIME_V3_TOOLBAR_MIC_MUTE : IDR_LIVEHIME_V3_TOOLBAR_MIC;
        hover_id = is_muted ? IDR_LIVEHIME_V3_TOOLBAR_MIC_MUTE_HV : IDR_LIVEHIME_V3_TOOLBAR_MIC_HV;
        tipstr_id = is_muted ? IDS_TOOLBAR_VOLUME_MIC_OPEN_TIPS : IDS_TOOLBAR_VOLUME_MIC_CLOSE_TIPS;
        dis_id = IDR_LIVEHIME_V3_TOOLBAR_MIC_DIS;
    }
    slider_->SetMuted(is_muted);
    muted_button_->SetImage(views::Button::STATE_NORMAL, GetImageSkiaNamed(normal_id));
    muted_button_->SetImage(views::Button::STATE_HOVERED, GetImageSkiaNamed(hover_id));
    muted_button_->SetImage(views::Button::STATE_PRESSED, GetImageSkiaNamed(hover_id));
    muted_button_->SetImage(views::Button::STATE_DISABLED, GetImageSkiaNamed(dis_id));
    muted_button_->SetTooltipText(GetLocalizedString(tipstr_id));
    muted_button_->SchedulePaint();
}

void ToolBarVolumeCtrl::SliderValueChanged(views::Slider* sender, float value, float old_value, views::SliderChangeReason reason)
{
    VolumeSlider *slider = (VolumeSlider*)sender;
    if (!slider)
    {
        return;
    }

    views::Label *label = NULL;
    label = slider->GetAssociationLabel();
    if (label)
    {
        label->SetText(VolumeSliderValueToString(value));
    }

    if (views::SliderChangeReason::VALUE_CHANGED_BY_API != reason)
    {
        //如果过了刻度线，就改变键盘操作刻度
        if (UpdateVolumeSliderKeyboardIncrement(slider, old_value))
        {
            //阻止尝试跨越100这个边界的行为
            value = 0.5f;
            slider->SetValue(0.5f);
        }

        if (old_value != value)
        {
            float deflection, gainDB;
            SplitVolumeSliderValueToDeflectionAndGain(value, &deflection, &gainDB);

            // 声音一旦变动视为解除静音
            SetMutedButtonState(false);

            if (volume_controllers_->ControllerIsValid())
            {
                self_operating_ = true;

                volume_controllers_->SetMuted(false);
                volume_controllers_->SetDeflection(deflection);
                volume_controllers_->SetDB(gainDB);

                self_operating_ = false;
            }
        }
    }
}

void ToolBarVolumeCtrl::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    if (volume_controllers_->ControllerIsValid())
    {
        static bool invoke_by_click = true;
        bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(),
                                           type_ == VolumeCtrlType::System ?
                                           IDC_LIVEHIME_HOTKEY_SYSVOL_SILENCE_RESUME :
                                           IDC_LIVEHIME_HOTKEY_MIC_SILENCE_RESUME,
                                           CommandParams<bool>(&invoke_by_click));
    }
}

void ToolBarVolumeCtrl::Observe(int type, const base::NotificationSource& source, const base::NotificationDetails& details)
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

void ToolBarVolumeCtrl::SetEnableSystemVoice(bool enabled)
{
    if (type_ == VolumeCtrlType::System)
    {
        SetEnabled(enabled);

        // 连快捷键也一并禁用掉
        bililive::ExecuteCommandWithParams(GetBililiveProcess()->bililive_obs(),
                                           IDC_LIVEHIME_HOTKEY_SYSVOL_ENABLE,
                                           CommandParams<bool>(&enabled));

        // UI和快捷键都禁用了，当前程序中没有其他地方可以控制音量静音与否了,
        // 就不通过Commmand的方式去执行Controller的SetMuted，在这里控制比较方便
        if (volume_controllers_->ControllerIsValid())
        {
            static bool prev_system_muted_status = false;
            if (!enabled)
            {
                prev_system_muted_status = volume_controllers_->GetMuted();
                volume_controllers_->SetMuted(true);
            }
            else
            {
                volume_controllers_->SetMuted(prev_system_muted_status);
            }
        }
    }
}

// static
void ToolBarVolumeCtrl::UpdateVolumeSliderKeyboardIncrement(views::Slider* volSlider)
{
    volSlider->SetKeyboardIncrement(0.005f);
}

bool ToolBarVolumeCtrl::UpdateVolumeSliderKeyboardIncrement(views::Slider* volSlider, float oldValue)
{
    volSlider->SetKeyboardIncrement(0.005f);

    int iVal = VolumeSliderValueToInteger(volSlider->value());
    int iOldVal = VolumeSliderValueToInteger(oldValue);

    if ((iOldVal - 100) * (iVal - 100) < 0)
        return true;
    else
        return false;
}

int ToolBarVolumeCtrl::VolumeSliderValueToInteger(float value)
{
    return (int)std::round(value * 200.0f);
}

void ToolBarVolumeCtrl::SplitVolumeSliderValueToDeflectionAndGain(float volume, float* deflection, float* gain)
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

float ToolBarVolumeCtrl::MergeDeflectionAndGainToVolume(float deflection, float gain)
{
    return deflection * 0.5f + gain / obs_proxy::kAudioGain * 0.5;
}

std::wstring ToolBarVolumeCtrl::VolumeSliderValueToString(float value)
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

// CoreProxyImpl callback
void ToolBarVolumeCtrl::OnAudioSourceActivate(obs_proxy::VolumeController * audio_source)
{
    std::string device_name = audio_source->GetBoundSourceName();
    if (device_name == device_name_)
    {
        InitData();
    }
}
