#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_STREAMING_SETTINGS_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_STREAMING_SETTINGS_VIEW_H_

#include "ui/views/controls/button/button.h"

#include "bililive/bililive/livehime/settings/settings_contract.h"
#include "bililive/bililive/ui/views/livehime/settings/base_settings_view.h"
#include "bililive/secret/public/live_streaming_service.h"


class HideAwareView;
class LivehimeCombobox;
class LivehimeContentLabel;
class LivehimeRadioButton;
class LivehimeTextfield;

// Õ∆¡˜…Ë÷√“≥
class StreamingSettingsView :
    public BaseSettingsView,
    public contracts::StreamingSettingsView
{

protected:
    // BaseSettingsView
    void InitViews() override;
    void InitData() override;
    bool CheckSettingsValid() override { return true; }
    bool SaveNormalSettingsChange() override { return true; }
    void SaveOrCheckStreamingSettingsChange(bool check, ChangeType &result) override;

private:
    void InitCdnViews();

private:
    HideAwareView *cdn_views_;
    LivehimeCombobox *cdn_combobox_;
    std::unique_ptr<contracts::SettingsPresenter> presenter_;

    DISALLOW_COPY_AND_ASSIGN(StreamingSettingsView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_STREAMING_SETTINGS_VIEW_H_