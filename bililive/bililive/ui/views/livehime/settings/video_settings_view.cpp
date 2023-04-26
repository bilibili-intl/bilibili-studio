#include "video_settings_view.h"

#include "base/file_util.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/utf_string_conversions.h"

#include "ui/views/layout/grid_layout.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/settings/settings_presenter_impl.h"
#include "bililive/bililive/ui/views/controls/bililive_imagebutton.h"
#include "bililive/bililive/ui/views/controls/util/bililive_util_views.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_button.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_combobox.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_label.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_textfield.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/bililive/utils/setting_util.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/log_ext/log_constants.h"

#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"


namespace {

    enum {
        RadioButton_Auto,
        RadioButton_Custom,
    };

}


VideoSettingsView::VideoSettingsView()
    : BaseSettingsView(),
    corerate_combobox_(nullptr),
    bitrate_control_combobox_(nullptr),
    framerate_combobox_(nullptr),
    streaming_res_combobox_(nullptr),
    encoder_combobox_(nullptr),
    streaming_quality_combobox_(nullptr),
    weak_ptr_factory_(this) {
    presenter_ = std::make_unique<SettingsPresenterImpl>(this);
}

void VideoSettingsView::SaveOrCheckStreamingSettingsChange(bool check, ChangeType& result) {
    std::map<std::string, std::string> device_string;
    std::map<std::string, int> device_integer;
    std::map<std::string, bool> device_boolean;

    // 如果选了自动，那么自定义的设置只保存到kCustomVideoXXXXX的设置项；
    // 如果没有选自动，那么自定义的设置还需额外再保存到kVideoXXXXX中；
    // 这里如果用户选了自动，那么在这里保存之后会进行测速，测速模块会在测速结束后直接依据测速结果
    // 将计算出来的视频参数直接写入到kVideoXXXXX中；
    // 因为obs推流层是直接读的kVideoXXXXX设置项，改了kVideoXXXXX之后要求底层重推流即可使用新参数。
    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
    bool bNewAuto = auto_radio_btn_->checked();
    bool bOldAuto = pref->GetBoolean(prefs::kAutoVideoSwitch);
    device_boolean.insert(std::make_pair(prefs::kAutoVideoSwitch, bNewAuto));

    // 这里要针对“自动”做一下处理：
    // 1、如果原先不是自动，这次变更为自动，那么
    if (check && bNewAuto)
    {
        if (bOldAuto != bNewAuto)
        {
            result = (ChangeType)(result | BaseSettingsView::TYPE_CHANGE_TO_AUTO);
        }
    }

    // 码率
    device_integer.insert(std::make_pair(prefs::kCustomVideoBitRate,
        corerate_combobox_->GetItemData<int>(corerate_combobox_->selected_index())));
    if (!bNewAuto)
    {
        device_integer.insert(std::make_pair(prefs::kVideoBitRate,
            corerate_combobox_->GetItemData<int>(corerate_combobox_->selected_index())));
    }

    // 码率控制
    device_string.insert(std::make_pair(prefs::kCustomVideoBitRateControl,
        bitrate_control_combobox_->GetItemData<std::string>(
            bitrate_control_combobox_->selected_index())));
    if (!bNewAuto)
    {
        device_string.insert(std::make_pair(prefs::kVideoBitRateControl,
            bitrate_control_combobox_->GetItemData<std::string>(
                bitrate_control_combobox_->selected_index())));
    }

    // 帧率
    int nCfg = framerate_combobox_->GetItemData<int>(framerate_combobox_->selected_index());
    device_string.insert(std::make_pair(prefs::kCustomVideoFPSCommon, base::IntToString(nCfg)));
    if (!bNewAuto)
    {
        device_string.insert(std::make_pair(prefs::kVideoFPSCommon, base::IntToString(nCfg)));
    }

    // 输出分辨率
    nCfg = streaming_res_combobox_->GetItemData<int>(streaming_res_combobox_->selected_index());
    int cx = LOWORD(nCfg), cy = HIWORD(nCfg);
    device_integer.insert(std::make_pair(prefs::kCustomVideoOutputCX, cx));
    device_integer.insert(std::make_pair(prefs::kCustomVideoOutputCY, cy));
    if (!bNewAuto)
    {
        device_integer.insert(std::make_pair(prefs::kVideoOutputCX, cx));
        device_integer.insert(std::make_pair(prefs::kVideoOutputCY, cy));
    }

    ResourceBundle& rb = ResourceBundle::GetSharedInstance();
    base::string16 szCfg = encoder_combobox_->GetItemText(encoder_combobox_->selected_index());

    // 编码器
    std::string szCodec;
    if (szCfg.compare(rb.GetLocalizedString(IDS_CONFIG_VIDEO_CMB_ENCODER_SOFTWARE).c_str()) == 0) {
        szCodec = prefs::kDefaultOutputStreamVideoEncoder;
    }
    else
    {
        if (szCfg.compare(rb.GetLocalizedString(IDS_CONFIG_VIDEO_CMB_ENCODER_NVIDIA).c_str()) == 0) {
            szCodec = prefs::kVideoStreamNVENC;
        }
        else if (szCfg.compare(rb.GetLocalizedString(IDS_CONFIG_VIDEO_CMB_ENCODER_INTEL).c_str()) == 0) {
            szCodec = prefs::kVideoStreamQSV;
        }
        else if (szCfg.compare(rb.GetLocalizedString(IDS_CONFIG_VIDEO_CMB_ENCODER_AMD).c_str()) == 0) {
            szCodec = prefs::kVideoStreamAMD;
        }
        else if (szCfg.compare(rb.GetLocalizedString(IDS_CONFIG_VIDEO_CMB_ENCODER_NVIDIA_TURING).c_str()) == 0) {
            szCodec = prefs::kVideoStreamNVTURINGENC;
        }
    }
    device_string.insert(std::make_pair(prefs::kCustomOutputStreamVideoEncoder, szCodec));
    if (!bNewAuto)
    {
        device_string.insert(std::make_pair(prefs::kOutputStreamVideoEncoder, szCodec));
    }

    // 输出质量
    nCfg = streaming_quality_combobox_->GetItemData<int>(streaming_quality_combobox_->selected_index());
    device_integer.insert(std::make_pair(prefs::kCustomOutputStreamVideoQuality, nCfg));
    if (!bNewAuto)
    {
        device_integer.insert(std::make_pair(prefs::kOutputStreamVideoQuality, nCfg));
    }

    if (!check)
    {
        if (bOldAuto != bNewAuto && bNewAuto)
        {
            device_boolean.insert(std::make_pair(prefs::kChangedCustomToAutoOnceBefore, true));
        }
    }

    presenter_->VideoSaveChange(device_string, device_integer, device_boolean, check, result);

    if (!check)
    {
        // 自定义切自动，立刻应用分辨率、帧率
        if (bOldAuto != bNewAuto && bNewAuto)
        {
            livehime::ApplyAutoMediaConfig();
        }

        std::string fps = pref->GetString(prefs::kVideoFPSCommon);
        std::string codec = pref->GetString(prefs::kOutputStreamVideoEncoder);
        int width = pref->GetInteger(prefs::kVideoOutputCX);
        int height = pref->GetInteger(prefs::kVideoOutputCY);
        int rate = pref->GetInteger(prefs::kCustomVideoBitRate);
        LOG(INFO) << app_log::kLogClarityChange <<
            base::StringPrintf("invoker:user; size:%dx%d; fps:%s; rate:%d; codec:%s",
                width, height, fps.c_str(), rate, codec.c_str());
    }
}

void VideoSettingsView::InitViews()
{
    InitCmbModel();
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();

    views::GridLayout* layout = new views::GridLayout(this);
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
        views::GridLayout::FILL, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);

    column_set = layout->AddColumnSet(2);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);
    column_set->AddColumn(
        views::GridLayout::LEADING, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddColumn(
        views::GridLayout::LEADING, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);

    column_set = layout->AddColumnSet(3);
    column_set->AddColumn(
        views::GridLayout::FILL, views::GridLayout::FILL,
        1, views::GridLayout::FIXED, 0, 0);

    // 画质设置
    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightEnds);
    BililiveLabel* label = new LivehimeTitleLabel(GetLocalizedString(IDS_CONFIG_VIDEO_SETTING_TITLE));
    layout->AddView(label);

    ++radio_button_groupid_;

    layout->StartRowWithPadding(0, 2, 0, kPaddingRowHeightForGroupCtrls);
    auto_radio_btn_ = new LivehimeRadioButton(GetLocalizedString(IDS_SRCPROP_WINDOW_CAPTURE_MODEL_AUTO), radio_button_groupid_, this);
    auto_radio_btn_->set_id(RadioButton_Auto);
    layout->AddView(auto_radio_btn_);

    custom_radio_btn_ = new LivehimeRadioButton(GetLocalizedString(IDS_VOICELINK_SETTING_CUSTOM), radio_button_groupid_, this);
    custom_radio_btn_->set_id(RadioButton_Custom);
    layout->AddView(custom_radio_btn_);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForCtrlTips);
    mode_tip_label_ = new LivehimeTipLabel();
    mode_tip_label_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    layout->AddView(mode_tip_label_);

    InitCustomViews();

    layout->StartRow(0, 3);
    layout->AddView(custom_view_);

    custom_view_->SetVisible(false);
}

void VideoSettingsView::InitCustomViews() {
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();

    custom_view_ = new BililiveHideAwareView();
    auto layout = new views::GridLayout(custom_view_);
    custom_view_->SetLayoutManager(layout);

    auto column_set = layout->AddColumnSet(0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);
    column_set->AddColumn(
        views::GridLayout::LEADING, views::GridLayout::FILL,
        0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, GetLengthByDPIScale(20));
    column_set->AddColumn(
        views::GridLayout::LEADING, views::GridLayout::FILL,
        0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);

    column_set = layout->AddColumnSet(1);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);
    column_set->AddColumn(
        views::GridLayout::FILL, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);

    column_set = layout->AddColumnSet(2);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);
    column_set->AddColumn(
        views::GridLayout::LEADING, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddColumn(
        views::GridLayout::LEADING, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);

    column_set = layout->AddColumnSet(3);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);
    column_set->AddColumn(
        views::GridLayout::LEADING, views::GridLayout::FILL,
        0, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthForCtrlTips);
    column_set->AddColumn(
        views::GridLayout::FILL, views::GridLayout::FILL,
        1, views::GridLayout::USE_PREF, 0, 0);
    column_set->AddPaddingColumn(0, kPaddingColWidthEnds);

    // --码率
    layout->StartRowWithPadding(0, 3, 0, kPaddingRowHeightForGroupCtrls);
    BililiveLabel* label = new LivehimeSmallContentLabel(rb.GetLocalizedString(IDS_CONFIG_VIDEO_CORERATE));
    label->SetFont(ftThirteen);
    layout->AddView(label);
    layout->AddView(corerate_combobox_);

    // --帧率
    layout->StartRowWithPadding(0, 3, 0, kPaddingRowHeightForGroupCtrls);
    label = new LivehimeSmallContentLabel(rb.GetLocalizedString(IDS_CONFIG_VIDEO_FRAMERATE));
    label->SetFont(ftThirteen);
    layout->AddView(label);
    layout->AddView(framerate_combobox_);

    // --分辨率
    layout->StartRowWithPadding(0, 3, 0, kPaddingRowHeightForGroupCtrls);
    label = new LivehimeSmallContentLabel(rb.GetLocalizedString(IDS_CONFIG_VIDEO_STREAMING_RES));
    label->SetFont(ftThirteen);
    layout->AddView(label);
    layout->AddView(streaming_res_combobox_);

    // 高级设置
    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightEnds);
    label = new LivehimeTitleLabel(rb.GetLocalizedString(IDS_CONFIG_VIDEO_SETTING_ADAVANCE));
    layout->AddView(label);

    // --码率控制
    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForGroupCtrls);
    label = new LivehimeSmallContentLabel(rb.GetLocalizedString(IDS_CONFIG_VIDEO_BITRATE_CONTROL));
    label->SetFont(ftThirteen);
    layout->AddView(label);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    layout->AddView(bitrate_control_combobox_);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForCtrlTips);
    label = new LivehimeTipLabel(rb.GetLocalizedString(IDS_CONFIG_VIDEO_BITRATE_CONTROL_TIPS));
    label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    layout->AddView(label);

    // --编码器
    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForDiffGroups);
    label = new LivehimeSmallContentLabel(rb.GetLocalizedString(IDS_CONFIG_VIDEO_ENCODER));
    label->SetFont(ftThirteen);
    auto help_view = new livehime::HelpSupportView(label,
        livehime::HelpType::SoftwareEncoderError);
    layout->AddView(help_view);

    code_tip_ = new LivehimeTipLabel(rb.GetLocalizedString(IDS_CONFIG_VIDEO_ENCODER_TIP_LABEL));
    code_tip_->SetMultiLine(true);
    code_tip_->SetAllowCharacterBreak(true);
    code_tip_->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    layout->AddView(code_tip_);
    code_tip_->SetVisible(false);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    layout->AddView(encoder_combobox_);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForCtrlTips);
    label = new LivehimeTipLabel(rb.GetLocalizedString(IDS_CONFIG_VIDEO_ENCODER_TIP));
    label->SetMultiLine(true);
    label->SetAllowCharacterBreak(true);
    label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    layout->AddView(label);

    // --性能平衡
    layout->StartRowWithPadding(0, 0, 0, kPaddingRowHeightForDiffGroups);
    label = new LivehimeSmallContentLabel(rb.GetLocalizedString(IDS_CONFIG_VIDEO_PERFORMANCE_BALANCE));
    label->SetFont(ftThirteen);
    layout->AddView(label);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForGroupCtrls);
    layout->AddView(streaming_quality_combobox_);

    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForCtrlTips);
    label = new LivehimeTipLabel(rb.GetLocalizedString(IDS_CONFIG_VIDEO_PERFORMANCE_BALANCE_TIP));
    label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    layout->AddView(label);

    // 最下方tips
    layout->StartRowWithPadding(0, 1, 0, kPaddingRowHeightForDiffGroups);
    label = new LivehimeTipLabel(rb.GetLocalizedString(IDS_CONFIG_VIDEO_TIP));
    label->SetHorizontalAlignment(gfx::HorizontalAlignment::ALIGN_LEFT);
    layout->AddView(label);
}

void VideoSettingsView::InitCmbModel()
{
    auto media_prefs = presenter_->GetMediaPrefsInfo();

    // 码率下拉框
    int index = 0;
    ResourceBundle& rb = ResourceBundle::GetSharedInstance();
    corerate_combobox_ = new LivehimeCombobox();
    corerate_combobox_->set_listener(this);
    for (auto& iter : media_prefs.video_settings.video_bitrate_list)
    {
        int i = corerate_combobox_->AddItem(base::UTF8ToUTF16(iter.second), iter.first);
        if (iter.first == media_prefs.video_settings.video_bitrate_default)
        {
            index = i;
        }
    }
    corerate_combobox_->SetSelectedIndex(index);

    // 码率控制下拉框
    index = 0;
    bitrate_control_combobox_ = new LivehimeCombobox();
    bitrate_control_combobox_->set_listener(this);
    for (auto& iter : media_prefs.video_settings.video_bitrate_ctrl_list)
    {
        int i = bitrate_control_combobox_->AddItem(base::UTF8ToUTF16(iter.second), iter.first);
        if (iter.first == media_prefs.video_settings.video_bitrate_ctrl_default)
        {
            index = i;
        }
    }
    bitrate_control_combobox_->SetSelectedIndex(index);

    // 帧率下拉框
    index = 0;
    framerate_combobox_ = new LivehimeCombobox(false);
    framerate_combobox_->SetEditInputType(BililiveComboboxEx::EDIT_INPUT_TYPE_NUMBER);
    framerate_combobox_->set_listener(this);
    for (auto& iter : media_prefs.video_settings.video_fps_list)
    {
        int i = framerate_combobox_->AddItem(base::UTF8ToUTF16(iter.second), iter.first);
        if (iter.first == media_prefs.video_settings.video_fps_default)
        {
            index = i;
        }
    }
    framerate_combobox_->SetSelectedIndex(index);

    // 分辨率下拉框
    index = 0;
    streaming_res_combobox_ = new LivehimeCombobox();
    for (auto& iter : media_prefs.video_settings.video_resolution_list)
    {
        std::vector<std::string> wh;
        base::SplitString(iter.first, 'x', &wh);
        DCHECK(wh.size() == 2);
        if (wh.size() == 2)
        {
            int w, h;
            if(base::StringToInt(wh[0], &w) && base::StringToInt(wh[1], &h))
            {
                int i = streaming_res_combobox_->AddItem(base::UTF8ToUTF16(iter.second), MAKELONG(w, h));
                if (iter.first == media_prefs.video_settings.video_resolution_default)
                {
                    index = i;
                }
            }
            else
            {
                NOTREACHED();
            }
        }
    }
    streaming_res_combobox_->SetSelectedIndex(index);

    encoder_combobox_ = new LivehimeCombobox();

    // 编码质量下拉框
    index = 0;
    streaming_quality_combobox_ = new LivehimeCombobox();
    for (auto& iter : media_prefs.video_settings.video_quality_list)
    {
        auto text = base::UTF8ToUTF16(iter.second);

        int i = streaming_quality_combobox_->AddItem(text, iter.first);
        if (iter.first == media_prefs.video_settings.video_quality_default)
        {
            index = i;
        }
    }
    streaming_quality_combobox_->SetSelectedIndex(index);
}

void VideoSettingsView::InitData()
{
    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
    auto media_prefs = presenter_->GetMediaPrefsInfo();
    // 自动/自定义
    bool bCfg = pref->GetBoolean(prefs::kAutoVideoSwitch);
    auto_radio_btn_->SetChecked(bCfg);
    custom_radio_btn_->SetChecked(!bCfg);
    custom_view_->SetVisible(!bCfg);
    mode_tip_label_->SetText(bCfg ? GetLocalizedString(IDS_CONFIG_VIDEO_SETTING_TIP_AUTO) :
        GetLocalizedString(IDS_CONFIG_VIDEO_SETTING_TIP_CUSTOM));

    // 码率
    int nCfg = pref->GetInteger(prefs::kCustomVideoBitRate);
    int nIndex = corerate_combobox_->FindItemData(nCfg);
    if (-1 == nIndex)
    {
        nCfg = livehime::AdjustBitRate(true, nCfg);
        nIndex = corerate_combobox_->FindItemData(nCfg);
    }
    corerate_combobox_->SetSelectedIndex(nIndex);

    // 码率控制
    std::string brc = pref->GetString(prefs::kCustomVideoBitRateControl);
    nIndex = bitrate_control_combobox_->FindItemData(brc);
    if (-1 != nIndex)
    {
        bitrate_control_combobox_->SetSelectedIndex(nIndex);
    }

    //优先使用KV的配置
    if (!pref->GetBoolean(prefs::kCustomVideoBitRateControlUseKv)) {
        nIndex = bitrate_control_combobox_->FindItemData(media_prefs.video_settings.video_bitrate_ctrl_default);
        if (-1 != nIndex){
            bitrate_control_combobox_->SetSelectedIndex(nIndex);
            pref->SetBoolean(prefs::kCustomVideoBitRateControlUseKv,true);
        }
    }

    // 帧率
    base::string16 szCfg = base::UTF8ToUTF16(pref->GetString(prefs::kCustomVideoFPSCommon));
    int framerate = 30;
    base::StringToInt(szCfg, &framerate);
    nIndex = framerate_combobox_->FindItemData(framerate);
    if (-1 != nIndex)
    {
        framerate_combobox_->SetSelectedIndex(nIndex);
    }

    // 输出分辨率
    int cx = pref->GetInteger(prefs::kCustomVideoOutputCX);
    int cy = pref->GetInteger(prefs::kCustomVideoOutputCY);
    nIndex = streaming_res_combobox_->FindItemData(MAKELONG(cx, cy));
    if (-1 == nIndex)
    {
        szCfg = base::IntToString16(cx) + L"x" + base::IntToString16(cy);
        nIndex = streaming_res_combobox_->AddItem(szCfg, MAKELONG(cx, cy));
    }
    streaming_res_combobox_->SetSelectedIndex(nIndex);

    // 编码器
    std::string szACfg = pref->GetString(prefs::kCustomOutputStreamVideoEncoder);
    std::vector<livehime::CodecInfo> codecs = livehime::GetSupportVideoEncoders();
    encoder_combobox_->ClearItems();
    nIndex = 0;
    for (auto& encoder : codecs)
    {
        int curIndex = encoder_combobox_->AddItem(encoder.ui_name);

        if (szACfg.compare(encoder.pref_name) == 0)
        {
            nIndex = curIndex;
        }

        //第一次使用KV配置编码器，而不是本地的
        if (!media_prefs.video_settings.codec_device_default.empty() && !pref->GetBoolean(prefs::kCustomStreamVideoEncoderUseKv)) {
            if (encoder.pref_name == media_prefs.video_settings.codec_device_default) {
                nIndex = curIndex;
                pref->SetBoolean(prefs::kCustomStreamVideoEncoderUseKv,true);
            }
        }
    }
    encoder_combobox_->SetSelectedIndex(nIndex);

    bool has_hardware{ false };
    std::for_each(codecs.begin(), codecs.end(),
        [&has_hardware](const livehime::CodecInfo& info)
        {
            if (info.is_hardware)
            {
                has_hardware = true;
            }
        });

    if (!has_hardware)
    {
        code_tip_->SetVisible(true);
    }

    // 输出质量
    nIndex = -1;
    nCfg = pref->GetInteger(prefs::kCustomOutputStreamVideoQuality);
    nIndex = streaming_quality_combobox_->FindItemData(nCfg);
    if (-1 != nIndex)
    {
        streaming_quality_combobox_->SetSelectedIndex(nIndex);
    }
}

void VideoSettingsView::ButtonPressed(views::Button* sender, const ui::Event& event)
{
    switch (sender->id()) {
    case RadioButton_Auto:
        if (custom_view_->visible()) {
            mode_tip_label_->SetText(GetLocalizedString(IDS_CONFIG_VIDEO_SETTING_TIP_AUTO));
            custom_view_->SetVisible(false);
            custom_view_->InvalidateLayout();
            View* v = Container();
            v->Layout();
        }
        break;

    case RadioButton_Custom:
        if (!custom_view_->visible()) {
            mode_tip_label_->SetText(GetLocalizedString(IDS_CONFIG_VIDEO_SETTING_TIP_CUSTOM));
            custom_view_->SetVisible(true);
            custom_view_->InvalidateLayout();
            View* v = Container();
            v->Layout();
        }
        break;

    default:
        break;
    }
}

template<class T1, class T2>
bool is_in(T1 val, const std::initializer_list<T2>& arg)
{
    for (auto x : arg)
        if (val == x)
            return true;
    return false;
}

void VideoSettingsView::OnBililiveComboboxExEditBlur(BililiveComboboxEx* combobox, const base::string16& text, int reason)
{
    //if (reason == BililiveComboboxEx::EDIT_BLUR_REASON_RETURN)
    if (is_in(combobox, {
        corerate_combobox_,      bitrate_control_combobox_, framerate_combobox_,
        streaming_res_combobox_, encoder_combobox_,         streaming_quality_combobox_
        }))
    {
        int64 data = 0;
        base::StringToInt64(text, &data);
        if (data > 0)
        {
            base::string16 data_str = base::Int64ToString16(data);
            int index = combobox->FindItemData(data);
            if (-1 == index)
            {
                index = combobox->AddItem(data_str, data);
            }
            combobox->SetSelectedIndex(index);
        }
    }
}

base::string16 VideoSettingsView::OnBililiveComboboxExEditShow(BililiveComboboxEx* combobox, const base::string16& label_text)
{
    if (is_in(combobox, { corerate_combobox_, framerate_combobox_ }))
    {
        int64 data = 0;
        base::StringToInt64(label_text, &data);
        return base::Int64ToString16(data);
    }
    return label_text;
}

void VideoSettingsView::OnSelectedIndexChanged(BililiveComboboxEx* combobox)
{
    auto media_prefs = presenter_->GetMediaPrefsInfo();
    if (combobox == corerate_combobox_) {
         int bitrate = corerate_combobox_->GetItemData<int>(
             corerate_combobox_->selected_index());

         for (auto& iter : media_prefs.bitrate_fps_resolution_settings) {
             if (bitrate == iter.bitrate) {
                 if (framerate_combobox_) {
                     int cur_index = framerate_combobox_->FindItemData(iter.fps_select);
                     if (cur_index >= 0) {
                         framerate_combobox_->SetSelectedIndex(cur_index);
                     }
                 }

                 if (streaming_res_combobox_) {
                     std::vector<std::string> wh;
                     base::SplitString(iter.video_resolution_select, 'x', &wh);
                     DCHECK(wh.size() == 2);
                     if (wh.size() == 2) {
                         int w, h;
                         if (base::StringToInt(wh[0], &w) && base::StringToInt(wh[1], &h)) {
                             int index = streaming_res_combobox_->FindItemData(MAKELONG(w, h));
                             if (index >= 0) {
                                 streaming_res_combobox_->SetSelectedIndex(index);
                             }
                         }
                     }
                 }
             }
         }
    }
}
