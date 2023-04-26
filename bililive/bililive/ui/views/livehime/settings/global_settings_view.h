#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_GLOBAL_SETTINGS_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_GLOBAL_SETTINGS_VIEW_H_

#include "bililive/bililive/livehime/settings/settings_contract.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/settings/base_settings_view.h"

class LivehimeCheckbox;
class LivehimeCombobox;

class GlobalSettingsView
    : public BaseSettingsView,
    public contracts::GlobalSettingsView,
    public views::ButtonListener
{
public:
    GlobalSettingsView();
    void ShowLiveReplaySetting(bool publish, bool archive) override;
    // views::ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    void SetNewRoomViewVisible(bool visible);
protected:
    // BaseSettingsView
    void InitViews() override;
    void InitData() override;
    bool CheckSettingsValid() override { return true; }
    bool SaveNormalSettingsChange() override;
    void UpdateNewRoomCheckbox(bool check);
    void UpdateTemporaryConf(int join_slide);

protected:
    void InitCmbModel();
private:
    void UpdateLiveReplayConfig(bool is_auto);
    static void ConfirmDialog(int code, void* data);

    void UpdateLiveArchiveConfig(bool is_auto);
private:
    LivehimeCheckbox* performance_mode_checkbox_;
    LivehimeCheckbox* exclude_from_capture_checkbox_;
    LivehimeCheckbox* drag_full_windows_checkbox_;

    LivehimeRadioButton* min_radio_button_;
    LivehimeRadioButton* exit_radio_botton_;
    LivehimeCheckbox* remind_choice_;
    LivehimeRadioButton* auto_replay_radio_button_ = nullptr;
    LivehimeRadioButton* ask_replay_radio_button_ = nullptr;
    LivehimeRadioButton* auto_archive_radio_button_ = nullptr;
    LivehimeRadioButton* no_archive_radio_button_ = nullptr;

    bool auto_replay_ = false;
    bool auto_archive_ = false;

    LivehimeCheckbox* new_room_ = nullptr;
    views::View* live_replay_view_ = nullptr;
    views::View* new_room_view_ = nullptr;

    std::unique_ptr<contracts::SettingsPresenter> presenter_;

    DISALLOW_COPY_AND_ASSIGN(GlobalSettingsView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_GLOBAL_SETTINGS_VIEW_H_