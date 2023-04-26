#ifndef BILILIVE_BILILIVE_LIVEHIME_SETTINGS_SETTINGS_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_SETTINGS_SETTINGS_CONTRACT_H_

#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/ui/views/livehime/settings/base_settings_view.h"
#include "bililive/secret/public/live_streaming_service.h"


namespace contracts {

struct IntegerPrefsInfo {
    std::string path;
    int value;
};

struct BooleanPrefsInfo {
    std::string path;
    bool value;
};

struct StringPrefsInfo {
    std::string path;
    std::string value;
};

// 音视频配置参数
using MediaPrefsInfo = MediaSettings;

class SettingsPresenter {
public:
    virtual ~SettingsPresenter() = default;

    virtual void DiscardSettings() = 0;

    virtual void RequestLivehimeEvent() = 0;

    virtual void SettingChanged() = 0;

    virtual void ApplyAudioDeviceSettings() = 0;

    // AcceleratorView
    virtual void AcceleratorInitData() = 0;

    virtual void AcceleratorSaveChange(const std::map<std::string, int>& hotkey) = 0;

    virtual void UnregisterHotKey(int id) = 0;

    // AudioSettingView
    virtual void AudioSaveChange(
        const std::map<std::string, std::string>& device_string,
        const std::map<std::string, int>& device_integer,
        bool check, BaseSettingsView::ChangeType &result) = 0;

    virtual void InitComboData(const std::string& device) = 0;

    virtual std::string GetCurMicId() const = 0;
    virtual std::string GetCurSpeakerId() const = 0;

    // VideoSettingView
    virtual void VideoSaveChange(
        const std::map<std::string, std::string>& device_string,
        const std::map<std::string, int>& device_integer,
        const std::map<std::string, bool>& device_boolean,
        bool check, BaseSettingsView::ChangeType &result) = 0;

    virtual const MediaPrefsInfo& GetMediaPrefsInfo() const = 0;

    // RecordSettingView
    virtual void RecordSaveChange(
        const std::map<std::string, std::string>& device_string,
        const std::map<std::string, int>& device_integer,
        const std::map<std::string, bool>& device_boolean,
        bool check, BaseSettingsView::ChangeType &result) = 0;

    // DanmakuHimeSettingsView
    virtual void DanmakuSaveChange(const std::vector<IntegerPrefsInfo>& integer_prefs,
        const std::vector<BooleanPrefsInfo>& boolean_prefs,
        const std::vector<StringPrefsInfo>& string_prefs) = 0;

    virtual void RequestGetAnchorSwitchStatus() = 0;

    virtual void RequestUpdateAnchorSwitch(bool switch_val) = 0;

    virtual void RequestGetNewRoomSwitch() = 0;

    // GlobalSettingsView
    virtual void GlobalSaveChange(
        bool performance_check,
        bool exclude_from_capture,
        bool drag_full_window_check,
        bool min_radio_check,
        bool remind_choice_check) = 0;

    virtual void SettingsFrameViewDeleted() = 0;
};

class SettingsFrameView {
public:
    virtual ~SettingsFrameView() = default;

    virtual void SaveOrCheckStreamingSettingsChange(
        bool check, BaseSettingsView::ChangeType &result) = 0;

    virtual void SaveNormalSettingsChange() = 0;

    virtual bool ShowRestreamingDialog() = 0;

    virtual bool CheckExclusive() = 0;

    virtual void ShowLiveReplaySetting(bool publish,bool archive) = 0;

};

class AcceleratorSettingsView {
public:
    virtual ~AcceleratorSettingsView() = default;

    virtual void OnAcceleratorInitData(const std::vector<int>& hotkey) = 0;
};

class AudioSettingsView {
public:
    virtual ~AudioSettingsView() = default;

    virtual void OnInitComboData(
       const std::string& device,
       const std::wstring& devname,
       const std::string& devid) = 0;
};

class VideoSettingsView {
public:
    virtual ~VideoSettingsView() = default;
};

class RecordSettingsView {
public:
    virtual ~RecordSettingsView() = default;
};

class StreamingSettingsView {
public:
    virtual ~StreamingSettingsView() = default;
};

class GlobalSettingsView {
public:
    virtual ~GlobalSettingsView() = default;
    virtual void ShowLiveReplaySetting(bool publish, bool archive) = 0;
};

}  // namespace contracts

#endif  // BILILIVE_BILILIVE_LIVEHIME_SETTINGS_SETTINGS_CONTRACT_H_