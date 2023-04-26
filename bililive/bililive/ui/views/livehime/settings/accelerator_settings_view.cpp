#include "accelerator_settings_view.h"

#include "base/prefs/pref_service.h"

#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/common_pref/common_pref_service.h"
#include "bililive/bililive/livehime/settings/settings_presenter_impl.h"

#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/common/bililive_features.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_process.h"

#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"


namespace {
    enum {
        kMicSilenceResumeButtonId = 1001,
        kSysvolSilenceResumeButtonId,
        kScene1ButtonId,
        kScene2ButtonId,
        kScene3ButtonId,
        kSwitchLiveButtonId,
        kSwitchRecordButtonId,
        kClearDanmakuTTSQueueButtonId,
    };
}

AcceleratorSettingsView::AcceleratorSettingsView()
    : BaseSettingsView(),
    mic_silence_resume_textfield_(nullptr),
    sysvol_silence_resume_textfield_(nullptr),
    scene1_textfield_(nullptr),
    scene2_textfield_(nullptr),
    scene3_textfield_(nullptr),
    switch_live_textfield_(nullptr),
    switch_record_textfield_(nullptr),
    clear_tts_queue_textfield_(nullptr) {
    presenter_ = std::make_unique<SettingsPresenterImpl>(this);
}

bool AcceleratorSettingsView::CheckSettingsValid() {
    static const wchar_t indent[] = L"    ";
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();
    bool bValid = true;
    base::string16 errmsg(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_CONFLICT));
    AcceleratorsTextfield* textfield = nullptr;
    UINT nModify, nVk;

    textfield = mic_silence_resume_textfield_->accelerators_text_field();
    textfield->GetHotkey(nModify, nVk);
    if (!CommonPrefService::RegisterBililiveHotkey(IDC_LIVEHIME_HOTKEY_MIC_SILENCE_RESUME, nModify, nVk))
    {
        errmsg.append(L"\r\n");
        errmsg.append(indent);
        errmsg.append(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_AUDIO_MIC));
        bValid = false;
    }

    textfield = sysvol_silence_resume_textfield_->accelerators_text_field();
    textfield->GetHotkey(nModify, nVk);
    if (!CommonPrefService::RegisterBililiveHotkey(IDC_LIVEHIME_HOTKEY_SYSVOL_SILENCE_RESUME, nModify, nVk))
    {
        errmsg.append(L"\r\n");
        errmsg.append(indent);
        errmsg.append(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_AUDIO_SYSVOL));
        bValid = false;
    }

    textfield = scene1_textfield_->accelerators_text_field();
    textfield->GetHotkey(nModify, nVk);
    if (!CommonPrefService::RegisterBililiveHotkey(IDC_LIVEHIME_HOTKEY_SENCE1, nModify, nVk))
    {
        errmsg.append(L"\r\n");
        errmsg.append(indent);
        errmsg.append(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_SENCE_SWAP) + std::to_wstring(1));
        bValid = false;
    }

    textfield = scene2_textfield_->accelerators_text_field();
    textfield->GetHotkey(nModify, nVk);
    if (!CommonPrefService::RegisterBililiveHotkey(IDC_LIVEHIME_HOTKEY_SENCE2, nModify, nVk))
    {
        errmsg.append(L"\r\n");
        errmsg.append(indent);
        errmsg.append(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_SENCE_SWAP) + std::to_wstring(2));
        bValid = false;
    }

    textfield = scene3_textfield_->accelerators_text_field();
    textfield->GetHotkey(nModify, nVk);
    if (!CommonPrefService::RegisterBililiveHotkey(IDC_LIVEHIME_HOTKEY_SENCE3, nModify, nVk))
    {
        errmsg.append(L"\r\n");
        errmsg.append(indent);
        errmsg.append(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_SENCE_SWAP) + std::to_wstring(3));
        bValid = false;
    }

    textfield = switch_live_textfield_->accelerators_text_field();
    textfield->GetHotkey(nModify, nVk);
    if (!CommonPrefService::RegisterBililiveHotkey(IDC_LIVEHIME_HOTKEY_SWITCH_LIVE, nModify, nVk))
    {
        errmsg.append(L"\r\n");
        errmsg.append(indent);
        errmsg.append(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_LIVE_LIVE));
        bValid = false;
    }

    textfield = switch_record_textfield_->accelerators_text_field();
    textfield->GetHotkey(nModify, nVk);
    if (!CommonPrefService::RegisterBililiveHotkey(IDC_LIVEHIME_HOTKEY_SWITCH_RECORD, nModify, nVk))
    {
        errmsg.append(L"\r\n");
        errmsg.append(indent);
        errmsg.append(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_LIVE_RECORD));
        bValid = false;
    }

    if (BililiveFeatures::current()->Enabled(BililiveFeatures::TTSDanmaku)) {
        textfield = clear_tts_queue_textfield_->accelerators_text_field();
        textfield->GetHotkey(nModify, nVk);
        if (!CommonPrefService::RegisterBililiveHotkey(
            IDC_LIVEHIME_HOTKEY_CLEAR_DANMAKU_TTS_QUEUE, nModify, nVk)) {
            errmsg.append(L"\r\n");
            errmsg.append(indent);
            errmsg.append(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_OTHERS_CLEAR_DANMAKU_TTS_QUEUE));
            bValid = false;
        }
    }

    // 一切还原到原配置
    CommonPrefService::RegisterBililiveHotkey();
    if (!bValid)
    {
        errmsg.append(L"\r\n\r\n");
        errmsg.append(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_CONFLICT_DEAL));

        auto sure = rb.GetLocalizedString(IDS_SURE);

        auto ret = livehime::ShowModalMessageBox(
            GetWidget()->GetNativeWindow(),
            rb.GetLocalizedString(IDS_TIP_DLG_TIP),
            errmsg,
            sure + L"," + rb.GetLocalizedString(IDS_CANCEL));
        if (ret == sure) {
            bValid = true;
        }
    }

    return bValid;
}

bool AcceleratorSettingsView::SaveNormalSettingsChange() {
    AcceleratorsTextfield* textfield = nullptr;
    UINT nModify, nVk;

    std::map<std::string, int> hotkey;

    textfield = mic_silence_resume_textfield_->accelerators_text_field();
    textfield->GetHotkey(nModify, nVk);
    hotkey.insert(std::make_pair(prefs::kHotkeyMicSwitch, MAKELONG(nVk, nModify)));

    textfield = sysvol_silence_resume_textfield_->accelerators_text_field();
    textfield->GetHotkey(nModify, nVk);
    hotkey.insert(std::make_pair(prefs::kHotkeySysVolSwitch, MAKELONG(nVk, nModify)));

    textfield = scene1_textfield_->accelerators_text_field();
    textfield->GetHotkey(nModify, nVk);
    hotkey.insert(std::make_pair(prefs::kHotkeyScene1, MAKELONG(nVk, nModify)));

    textfield = scene2_textfield_->accelerators_text_field();
    textfield->GetHotkey(nModify, nVk);
    hotkey.insert(std::make_pair(prefs::kHotkeyScene2, MAKELONG(nVk, nModify)));

    textfield = scene3_textfield_->accelerators_text_field();
    textfield->GetHotkey(nModify, nVk);
    hotkey.insert(std::make_pair(prefs::kHotkeyScene3, MAKELONG(nVk, nModify)));

    textfield = switch_live_textfield_->accelerators_text_field();
    textfield->GetHotkey(nModify, nVk);
    hotkey.insert(std::make_pair(prefs::kHotkeyLiveSwitch, MAKELONG(nVk, nModify)));

    textfield = switch_record_textfield_->accelerators_text_field();
    textfield->GetHotkey(nModify, nVk);
    hotkey.insert(std::make_pair(prefs::kHotkeyRecordSwitch, MAKELONG(nVk, nModify)));

    if (BililiveFeatures::current()->Enabled(BililiveFeatures::TTSDanmaku)) {
        textfield = clear_tts_queue_textfield_->accelerators_text_field();
        textfield->GetHotkey(nModify, nVk);
        hotkey.insert(std::make_pair(prefs::kHotkeyClearDanmakuTTSQueue, MAKELONG(nVk, nModify)));
    }

    presenter_->AcceleratorSaveChange(hotkey);

    return true;
}

void AcceleratorSettingsView::InitViews() {
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();
    base::string16 text = rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_INPUT);
    base::string16 del = rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_DEL);

    views::GridLayout *layout = new views::GridLayout(this);
    SetLayoutManager(layout);

    auto column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);
    column_set->AddColumn(
        views::GridLayout::LEADING, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);

    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);
    column_set->AddColumn(
        views::GridLayout::LEADING, views::GridLayout::CENTER,
        0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(
        views::GridLayout::FILL, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForGroupCtrls);
    column_set->AddColumn(
        views::GridLayout::FILL, views::GridLayout::FILL,
        0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);

    // 音频快捷键
    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightEnds);
    BililiveLabel *label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_AUDIO));
    layout->AddView(label);

    // -麦克风静音/恢复
    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    label = new LivehimeContentLabel(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_AUDIO_MIC));
    layout->AddView(label);
    mic_silence_resume_textfield_ = new LivehimeAcceleratorTextfield();
    mic_silence_resume_textfield_->set_placeholder_text(text);
    mic_silence_resume_textfield_->SetAcceleratorsTextfieldDelegate(this);
    layout->AddView(mic_silence_resume_textfield_);
    BililiveLabelButton *labelbtn = new LivehimeFunctionLabelButton(this, del);
    labelbtn->set_id(kMicSilenceResumeButtonId);
    layout->AddView(labelbtn);

    // -系统静音/恢复
    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    label = new LivehimeContentLabel(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_AUDIO_SYSVOL));
    layout->AddView(label);
    sysvol_silence_resume_textfield_ = new LivehimeAcceleratorTextfield();
    sysvol_silence_resume_textfield_->set_placeholder_text(text);
    sysvol_silence_resume_textfield_->SetAcceleratorsTextfieldDelegate(this);
    layout->AddView(sysvol_silence_resume_textfield_);
    labelbtn = new LivehimeFunctionLabelButton(this, del);
    labelbtn->set_id(kSysvolSilenceResumeButtonId);
    layout->AddView(labelbtn);

    // 场景快捷键
    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForDiffGroups);
    label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_SENCE));
    layout->AddView(label);

    // -切换至场景1
    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    label = new LivehimeContentLabel(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_SENCE_SWAP) + std::to_wstring(1));
    layout->AddView(label);
    scene1_textfield_ = new LivehimeAcceleratorTextfield();
    scene1_textfield_->set_placeholder_text(text);
    scene1_textfield_->SetAcceleratorsTextfieldDelegate(this);
    layout->AddView(scene1_textfield_);
    labelbtn = new LivehimeFunctionLabelButton(this, del);
    labelbtn->set_id(kScene1ButtonId);
    layout->AddView(labelbtn);

    // -切换至场景2
    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    label = new LivehimeContentLabel(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_SENCE_SWAP) + std::to_wstring(2));
    layout->AddView(label);
    scene2_textfield_ = new LivehimeAcceleratorTextfield();
    scene2_textfield_->set_placeholder_text(text);
    scene2_textfield_->SetAcceleratorsTextfieldDelegate(this);
    layout->AddView(scene2_textfield_);
    labelbtn = new LivehimeFunctionLabelButton(this, del);
    labelbtn->set_id(kScene2ButtonId);
    layout->AddView(labelbtn);

    // -切换至场景3
    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    label = new LivehimeContentLabel(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_SENCE_SWAP) + std::to_wstring(3));
    layout->AddView(label);
    scene3_textfield_ = new LivehimeAcceleratorTextfield();
    scene3_textfield_->set_placeholder_text(text);
    scene3_textfield_->SetAcceleratorsTextfieldDelegate(this);
    layout->AddView(scene3_textfield_);
    labelbtn = new LivehimeFunctionLabelButton(this, del);
    labelbtn->set_id(kScene3ButtonId);
    layout->AddView(labelbtn);

    // 直播和录制快捷键
    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForDiffGroups);
    label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_LIVE));
    layout->AddView(label);

    // -开始/停止直播
    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    label = new LivehimeContentLabel(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_LIVE_LIVE));
    layout->AddView(label);
    switch_live_textfield_ = new LivehimeAcceleratorTextfield();
    switch_live_textfield_->set_placeholder_text(text);
    switch_live_textfield_->SetAcceleratorsTextfieldDelegate(this);
    layout->AddView(switch_live_textfield_);
    labelbtn = new LivehimeFunctionLabelButton(this, del);
    labelbtn->set_id(kSwitchLiveButtonId);
    layout->AddView(labelbtn);

    // -开始/停止录制
    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    label = new LivehimeContentLabel(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_LIVE_RECORD));
    layout->AddView(label);
    switch_record_textfield_ = new LivehimeAcceleratorTextfield();
    switch_record_textfield_->set_placeholder_text(text);
    switch_record_textfield_->SetAcceleratorsTextfieldDelegate(this);
    layout->AddView(switch_record_textfield_);
    labelbtn = new LivehimeFunctionLabelButton(this, del);
    labelbtn->set_id(kSwitchRecordButtonId);
    layout->AddView(labelbtn);

    if (BililiveFeatures::current()->Enabled(BililiveFeatures::TTSDanmaku)) {
        // 其它
        layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForDiffGroups);
        label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_OTHERS));
        layout->AddView(label);

        // -清空语音弹幕
        layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
        label = new LivehimeContentLabel(
            rb.GetLocalizedString(IDS_CONFIG_ACCELERATORS_OTHERS_CLEAR_DANMAKU_TTS_QUEUE));
        layout->AddView(label);
        clear_tts_queue_textfield_ = new LivehimeAcceleratorTextfield();
        clear_tts_queue_textfield_->set_placeholder_text(text);
        clear_tts_queue_textfield_->SetAcceleratorsTextfieldDelegate(this);
        layout->AddView(clear_tts_queue_textfield_);
        labelbtn = new LivehimeFunctionLabelButton(this, del);
        labelbtn->set_id(kClearDanmakuTTSQueueButtonId);
        layout->AddView(labelbtn);
    }

    vector_.push_back(mic_silence_resume_textfield_);
    vector_.push_back(sysvol_silence_resume_textfield_);
    vector_.push_back(scene1_textfield_);
    vector_.push_back(scene2_textfield_);
    vector_.push_back(scene3_textfield_);
    vector_.push_back(switch_live_textfield_);
    vector_.push_back(switch_record_textfield_);

    if (BililiveFeatures::current()->Enabled(BililiveFeatures::TTSDanmaku)) {
        vector_.push_back(clear_tts_queue_textfield_);
    }
}

void AcceleratorSettingsView::InitData() {
    presenter_->AcceleratorInitData();
}

void AcceleratorSettingsView::OnAcceleratorInitData(const std::vector<int>& hotkey) {
    for (unsigned i = 0; i < hotkey.size(); i++) {
        if (i >= vector_.size()) return;

        int cfg = hotkey[i];
        vector_[i]->accelerators_text_field()->SetHotkey(HIWORD(cfg), LOWORD(cfg));
    }
}

void AcceleratorSettingsView::ButtonPressed(views::Button* sender, const ui::Event& event) {
    int id = sender->id();
    switch (id)
    {
    case kMicSilenceResumeButtonId:
        ((AcceleratorsTextfield*)mic_silence_resume_textfield_->accelerators_text_field())->DelHotKey();
        presenter_->UnregisterHotKey(IDC_LIVEHIME_HOTKEY_MIC_SILENCE_RESUME);
        break;
    case kSysvolSilenceResumeButtonId:
        ((AcceleratorsTextfield*)sysvol_silence_resume_textfield_->accelerators_text_field())->DelHotKey();
        presenter_->UnregisterHotKey(IDC_LIVEHIME_HOTKEY_SYSVOL_SILENCE_RESUME);
        break;
    case kScene1ButtonId:
        ((AcceleratorsTextfield*)scene1_textfield_->accelerators_text_field())->DelHotKey();
        presenter_->UnregisterHotKey(IDC_LIVEHIME_HOTKEY_SENCE1);
        break;
    case kScene2ButtonId:
        ((AcceleratorsTextfield*)scene2_textfield_->accelerators_text_field())->DelHotKey();
        presenter_->UnregisterHotKey(IDC_LIVEHIME_HOTKEY_SENCE2);
        break;
    case kScene3ButtonId:
        ((AcceleratorsTextfield*)scene3_textfield_->accelerators_text_field())->DelHotKey();
        presenter_->UnregisterHotKey(IDC_LIVEHIME_HOTKEY_SENCE3);
        break;
    case kSwitchLiveButtonId:
        ((AcceleratorsTextfield*)switch_live_textfield_->accelerators_text_field())->DelHotKey();
        presenter_->UnregisterHotKey(IDC_LIVEHIME_HOTKEY_SWITCH_LIVE);
        break;
    case kSwitchRecordButtonId:
        ((AcceleratorsTextfield*)switch_record_textfield_->accelerators_text_field())->DelHotKey();
        presenter_->UnregisterHotKey(IDC_LIVEHIME_HOTKEY_SWITCH_RECORD);
        break;
    case kClearDanmakuTTSQueueButtonId:
        ((AcceleratorsTextfield*)clear_tts_queue_textfield_->accelerators_text_field())->DelHotKey();
        presenter_->UnregisterHotKey(IDC_LIVEHIME_HOTKEY_CLEAR_DANMAKU_TTS_QUEUE);
        break;
    default:
        break;
    }
}

void AcceleratorSettingsView::OnHotkeyChange(AcceleratorsTextfield* sender, UINT &nModify, UINT &nVk) {
    UINT Modify, Vk;
    for (unsigned int i = 0; i < vector_.size(); i++)
    {
        if (vector_[i]->accelerators_text_field() != sender)
        {
            vector_[i]->accelerators_text_field()->GetHotkey(Modify, Vk);
            // 新设的热键和其他项完全重复的新设项的优先级最高，把其他项清空
            if ((Modify == nModify) && (Vk == nVk))
            {
                vector_[i]->accelerators_text_field()->SetHotkey(0, 0);
            }
        }
    }
}
