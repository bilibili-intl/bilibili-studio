#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_ACCELERATORS_SETTINGS_VIEW_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_ACCELERATORS_SETTINGS_VIEW_H_

#include "ui/views/controls/button/button.h"

#include "bililive/bililive/livehime/settings/settings_contract.h"

#include "bililive/bililive/ui/views/livehime/controls/livehime_textfield.h"
#include "bililive/bililive/ui/views/livehime/settings/base_settings_view.h"


// øÏΩ›º¸±‡º≠øÚ
class LivehimeAcceleratorTextfield : public LivehimeTextfield
{
public:
    LivehimeAcceleratorTextfield() {
        SetTextfield(new AcceleratorsTextfield());
        // here to set common color style
        SetCommonStyle();
    }

    AcceleratorsTextfield* accelerators_text_field() const {
        return (AcceleratorsTextfield*)text_field();
    }

    void SetAcceleratorsTextfieldDelegate(AcceleratorsTextfieldDelegate *delegate) {
        AcceleratorsTextfield *textfield = (AcceleratorsTextfield*)text_field();
        if (textfield) {
            textfield->SetAcceleratorsTextfieldDelegate(delegate);
        }
    }

private:
    DISALLOW_COPY_AND_ASSIGN(LivehimeAcceleratorTextfield);
};

// øÏΩ›º¸…Ë÷√“≥
class AcceleratorSettingsView
    : public BaseSettingsView,
    public views::ButtonListener,
    public AcceleratorsTextfieldDelegate,
    public contracts::AcceleratorSettingsView
{
public:
    AcceleratorSettingsView();

    void OnAcceleratorInitData(const std::vector<int>& hotkey) override;

protected:
    // BaseSettingsView
    void InitViews() override;
    void InitData() override;
    bool CheckSettingsValid() override;
    bool SaveNormalSettingsChange() override;

    // ButtonListener
    void ButtonPressed(views::Button* sender, const ui::Event& event) override;

    // AcceleratorsTextfieldDelegate
    void OnHotkeyChange(AcceleratorsTextfield* sender, UINT &nModify, UINT &nVk) override;

private:
    LivehimeAcceleratorTextfield *mic_silence_resume_textfield_;
    LivehimeAcceleratorTextfield *sysvol_silence_resume_textfield_;
    LivehimeAcceleratorTextfield *scene1_textfield_;
    LivehimeAcceleratorTextfield *scene2_textfield_;
    LivehimeAcceleratorTextfield *scene3_textfield_;
    LivehimeAcceleratorTextfield *switch_live_textfield_;
    LivehimeAcceleratorTextfield *switch_record_textfield_;
    LivehimeAcceleratorTextfield *clear_tts_queue_textfield_;
    std::vector<LivehimeAcceleratorTextfield*> vector_;

    std::unique_ptr<contracts::SettingsPresenter> presenter_;

    DISALLOW_COPY_AND_ASSIGN(AcceleratorSettingsView);
};

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_SETTINGS_ACCELERATORS_SETTINGS_VIEW_H_