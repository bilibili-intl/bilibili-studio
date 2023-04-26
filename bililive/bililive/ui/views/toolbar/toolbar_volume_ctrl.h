#ifndef BILILIVE_BILILIVE_UI_VIEWS_TOOLBAR_TOOLBAR_VOLUME_CTRL_H
#define BILILIVE_BILILIVE_UI_VIEWS_TOOLBAR_TOOLBAR_VOLUME_CTRL_H

#include "base/notification/notification_observer.h"
#include "base/notification/notification_registrar.h"

#include "ui/views/controls/slider.h"
#include "ui/views/controls/button/button.h"

#include "bililive/bililive/livehime/volume/bililive_volume_controllers_devices_contract.h"

#include "obs/obs_proxy/public/proxy/obs_proxy.h"


namespace
{
    class VolumeSlider;
}

class BililiveImageButton;

enum class VolumeCtrlType
{
    System,
    Mic,

    Count,
};

// 音量控制控件，静音按钮+滑动条+音量值label
class ToolBarVolumeCtrl
    : public views::View
    , views::SliderListener
    , views::ButtonListener
    , base::NotificationObserver
    , obs_proxy_ui::OBSUIProxyObserver
{
    enum ImageButtonId
    {
        ID_SYSTEM_VOICE,
        ID_MIC_VOICE
    };

public:
    explicit ToolBarVolumeCtrl(VolumeCtrlType type);
	virtual ~ToolBarVolumeCtrl();

    VolumeCtrlType volume_type() const;

    gfx::Rect GetVolumeRect() const;

    int GetRenderError() const;

    bool IsMute() const;

    bool IsDeviceValid() const;

protected:
	// views::View
    void ViewHierarchyChanged(const views::View::ViewHierarchyChangedDetails &details) override;
    gfx::Size GetPreferredSize() override;
    void SetEnabled(bool enabled) override;

	// SliderListener
	void SliderValueChanged(views::Slider* sender,
		float value,
		float old_value,
        views::SliderChangeReason reason) override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // NotificationObserver
    void Observe(int type, const base::NotificationSource& source, const base::NotificationDetails& details) override;

    // OBSUIProxyObserver
    void OnAudioSourceActivate(obs_proxy::VolumeController* audio_source) override;

private:
    void InitViews();
    void InitData();
    void OnVolumeChangedInternal();
    void OnSlideVolumeChanged(const std::string& source_name, float db);
    void SetMutedButtonState(bool is_muted);
    void SetEnableSystemVoice(bool enabled);

    // contracts::BililiveVolumeControllersContract invoke
    void OnVolumeChanged(const std::string& source_name, float db);
    void OnVolumeLevelUpdate(const std::string& source_name,
                             const std::vector<float>& magnitude,
                             const std::vector<float>& peak,
                             const std::vector<float>& inputPeak);
    void OnVolumeMuteChanged(const std::string& source_name, bool muted);

    //volume process
    static void UpdateVolumeSliderKeyboardIncrement(views::Slider* volSlider);
    static bool UpdateVolumeSliderKeyboardIncrement(views::Slider* volSlider, float oldValue); //return: isCrossBoundary

    static void SplitVolumeSliderValueToDeflectionAndGain(float volume, float* deflection, float* gain);
    static float MergeDeflectionAndGainToVolume(float deflection, float gain);

    static int VolumeSliderValueToInteger(float value);
    static std::wstring VolumeSliderValueToString(float value);

private:
    VolumeCtrlType type_;
    std::string device_name_;
    BililiveImageButton* muted_button_;
    VolumeSlider* slider_;

    base::NotificationRegistrar notifation_register_;

    bool self_operating_ = false;
    std::unique_ptr<contracts::BililiveVolumeControllersContract> volume_controllers_;

    base::WeakPtrFactory<ToolBarVolumeCtrl> weakptr_factory_;

	DISALLOW_COPY_AND_ASSIGN(ToolBarVolumeCtrl);
};


#endif