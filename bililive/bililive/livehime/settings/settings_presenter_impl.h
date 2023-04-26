#ifndef BILILIVE_BILILIVE_LIVEHIME_SETTINGS_SETTINGS_PRESENTER_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_SETTINGS_SETTINGS_PRESENTER_IMPL_H_

#include "base/memory/weak_ptr.h"

#include "bililive/bililive/livehime/settings/settings_contract.h"

class SettingsPresenterImpl
    : public contracts::SettingsPresenter {
public:
    explicit SettingsPresenterImpl(contracts::SettingsFrameView* view);

    explicit SettingsPresenterImpl(contracts::AcceleratorSettingsView *view);

    explicit SettingsPresenterImpl(contracts::AudioSettingsView* view);

    explicit SettingsPresenterImpl(contracts::VideoSettingsView* view);

    explicit SettingsPresenterImpl(contracts::RecordSettingsView* view);

    explicit SettingsPresenterImpl(contracts::StreamingSettingsView* view);

    explicit SettingsPresenterImpl(contracts::GlobalSettingsView* view);

    void DiscardSettings() override;

    void RequestLivehimeEvent() override;

    void SettingChanged() override;

    void ApplyAudioDeviceSettings() override;

    //AcceleratorView;
    void AcceleratorInitData() override;

    void AcceleratorSaveChange(const std::map<std::string, int>& hotkey) override;

    void UnregisterHotKey(int id) override;

    //AudioSettingView
    void AudioSaveChange(
        const std::map<std::string, std::string>& device_string,
        const std::map<std::string, int>& device_integer,
        bool check, BaseSettingsView::ChangeType &result) override;

    void InitComboData(const std::string& device) override;

    std::string GetCurMicId() const override;
    std::string GetCurSpeakerId() const override;

    // VideoSettingView
    void VideoSaveChange(
        const std::map<std::string, std::string>& device_string,
        const std::map<std::string, int>& device_integer,
        const std::map<std::string, bool>& device_boolean,
        bool check, BaseSettingsView::ChangeType &result) override;

    // RecordSettingsView
    void RecordSaveChange(
        const std::map<std::string, std::string>& device_string,
        const std::map<std::string, int>& device_integer,
        const std::map<std::string, bool>& device_boolean,
        bool check, BaseSettingsView::ChangeType& result) override;

    const contracts::MediaPrefsInfo& GetMediaPrefsInfo() const override;

    // DanmakuHimeSettingsView
    void DanmakuSaveChange(const std::vector<contracts::IntegerPrefsInfo>& integer_prefs,
        const std::vector<contracts::BooleanPrefsInfo>& boolean_prefs,
        const std::vector<contracts::StringPrefsInfo>& string_prefs) override;

    void RequestGetAnchorSwitchStatus() override;

    void RequestUpdateAnchorSwitch(bool switch_val) override;

    void RequestGetNewRoomSwitch() override;

    // GlobalSettingsView
    void GlobalSaveChange(
        bool performance_check,
        bool exclude_from_capture,
        bool drag_full_window_check,
        bool min_radio_check,
        bool remind_choice_check) override;

    void SettingsFrameViewDeleted() override { settings_frame_view_ = nullptr; };

private:
    void SaveOrCheckInteger(const char *path, int value,
        BaseSettingsView::ChangeType flag, bool check, BaseSettingsView::ChangeType &result);

    void SaveOrCheckString(const char *path, const std::string &value,
        BaseSettingsView::ChangeType flag, bool check, BaseSettingsView::ChangeType &result);

    void SaveOrCheckBool(const char *path, bool value,
        BaseSettingsView::ChangeType flag, bool check, BaseSettingsView::ChangeType &result);

    void SaveOrCheckDictionary(const char *path, const base::DictionaryValue &value,
        BaseSettingsView::ChangeType flag, bool check, BaseSettingsView::ChangeType &result);

    void OnUpdateAnchorSwitch(bool valid_response, int code);

    void OnRequestGetNewRoomSwitchRes(bool valid_response, int code,bool swtich_on);

private:
    contracts::SettingsFrameView* settings_frame_view_ = nullptr;
    contracts::AcceleratorSettingsView* accelerator_settings_view_ = nullptr;
    contracts::AudioSettingsView* audio_settings_view_ = nullptr;
    contracts::VideoSettingsView* video_settings_view_ = nullptr;
    contracts::RecordSettingsView* record_settings_view_ = nullptr;
    contracts::StreamingSettingsView* streaming_settings_view_ = nullptr;
    contracts::GlobalSettingsView* global_settings_view_ = nullptr;

    base::WeakPtrFactory<SettingsPresenterImpl> weak_ptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(SettingsPresenterImpl);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_SETTINGS_SETTINGS_PRESENTER_IMPL_H_