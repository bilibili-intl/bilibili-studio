#include "bililive/bililive/viddup/danmaku_hime/danmakuhime_data_viddup_handler.h"

#include <array>

#include "base/command_line.h"
#include "base/ext/callable_callback.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/notification/notification_service.h"
#include "base/prefs/pref_service.h"
#include "bililive/bililive/livehime/danmaku_hime/danmaku_hime_pref_service.h"
#include "bililive/bililive/text_to_speech/tts_helper.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_obs.h"

#include "bililive/bililive/ui/views/hotkey_notify/bililive_hotkey_notify_view.h"
#include "bililive/bililive/ui/views/livehime/danmaku_hime/danmaku_web_view.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/livehime/settings/settings_widget.h"
#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_datatype.h"
#include "bililive/bililive/ui/views/viddup/danmaku_hime/danmaku_merged_main_viddup_view.h"

#include "bililive/bililive/livehime/gift_image/image_fetcher.h"
#include "bililive/bililive/utils/convert_util.h"
#include "bililive/bililive/utils/time_span.h"
#include "bililive/common/bililive_features.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/bililive/livehime/audio_player/audio_player_presenter_imp.h"

#include "ui/base/resource/resource_bundle.h"

#include "bililive/bililive/livehime/danmaku_hime/MessageLoopView.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"


DanmakuhimeDataViddupHandler DanmakuhimeDataViddupHandler::danmaku_data_handler_instance_;

//const int64_t DanmakuhimeDataHandler::COMBO_FILTER_VALUE[4] = { 0, 100, 1000, 10000 };
//const int64_t DanmakuhimeDataHandler::GIFT_FILTER_VALUE[4] = { 0, 500, 2000, 10000 };
//const int64_t DanmakuhimeDataHandler::NAVY_FILTER_VALUE[3] = { 3, 2, 1 };

const std::map<int, int> kFpsTable{
    { 0, 24 }, { 1, 30 }, { 2, 45 }, { 3, 60 }
};

DanmakuhimeDataHandler* DanmakuhimeDataViddupHandler::GetInstance()
{
    return &danmaku_data_handler_instance_;
}

bool DanmakuhimeDataViddupHandler::FilterPaintMessage(HWND hwnd)
{
    if (DanmakuInteractionViddupView::GetDanmakuWidget() &&
        hwnd == DanmakuInteractionViddupView::GetDanmakuWidget()->GetNativeView() &&
        DanmakuInteractionViddupView::GetView()->GetShowType() == DanmakuShowType::kSplited &&
        DanmakuInteractionViddupView::GetView()->IsAnimating())
    {
        return true;
    }

    return false;
}

DanmakuhimeDataViddupHandler::DanmakuhimeDataViddupHandler() :
    weak_ptr_factory_(this)
{
}

DanmakuhimeDataViddupHandler::~DanmakuhimeDataViddupHandler()
{
}

void DanmakuhimeDataViddupHandler::Observe(int type, const base::NotificationSource& source, const base::NotificationDetails& details)
{
    if (!interaction_view_)
    {
        return;
    }

    switch (type)
    {
    case bililive::NOTIFICATION_LIVEHIME_DANMAKU_RELOADPREF:
    {
        LoadPref();

        interaction_view_->SchedulePaint();
    }
    break;
    case bililive::NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_NEW:
    {        

        return;
    }
    case bililive::NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_UPDATE:
    case bililive::NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_INIT:

        break;
    case bililive::NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_DELETE:
    {

        break;
    }
    case bililive::NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_CLOSE:
    {
    }
    break;
    case bililive::NOTIFICATION_LIVEHIME_DANMAKU_UI_PANINT: {
        interaction_view_->UpdateView();
        break;
    }
    case bililive::NOTIFICATION_LIVEHIME_LOAD_ROOMINFO_SUCCESS: {
        break;
    }
    default:
        break;
    }
}

void DanmakuhimeDataViddupHandler::OnOpenLiveRoomSuccessed(const secret::LiveStreamingService::StartLiveInfo& start_live_info)
{
}

void DanmakuhimeDataViddupHandler::OnPreLiveRoomStartStreaming(bool is_restreaming)
{
}

void DanmakuhimeDataViddupHandler::OnLiveRoomStreamingStopped()
{
}

void DanmakuhimeDataViddupHandler::OnLiveRoomStreamingError(
    obs_proxy::StreamingErrorCode streaming_errno, const std::wstring& error_message, int error_code)
{

}

void DanmakuhimeDataViddupHandler::OnWidgetDestroying(views::Widget* widget)
{
    if (interaction_view_)
    {
        SaveDanmakuStatePref();

        if (interaction_view_ && interaction_view_->GetDanmakuWidget())
        {
            interaction_view_->GetDanmakuWidget()->RemoveObserver(this);
        }
    }

    if (main_view_ && main_view_->GetWidget() == widget)
    {
        main_view_->GetWidget()->RemoveObserver(this);
        main_view_ = nullptr;
    }

    if (main_view_ && main_view_->GetWidget() == widget)
    {
        main_view_->GetWidget()->RemoveObserver(this);
        main_view_ = nullptr;
    }

    interaction_view_ = nullptr;
}

void DanmakuhimeDataViddupHandler::OnWidgetVisibilityChanged(views::Widget* widget, bool visible)
{
}

void DanmakuhimeDataViddupHandler::OnWidgetBoundsChanged(views::Widget* widget, const gfx::Rect& new_bounds)
{
    if (interaction_view_)
    {
        interaction_view_->RelocationMessageLoopView();
    }
}

DanmakuMergedMainViddupView* DanmakuhimeDataViddupHandler::GetDanmakuMergedMainViddupView()
{
    return danmaku_merged_main_view_;
}

DanmakuInteractionViddupView* DanmakuhimeDataViddupHandler::GetDanmakuInteractionViddupView()
{
    return interaction_view_;
}

void DanmakuhimeDataViddupHandler::SetMainView(BililiveOBSView* main_view)
{
    main_view->AttachMergedDanmakuMainView(GetDanmakuMergedMainViddupView());
}

void DanmakuhimeDataViddupHandler::Init(BililiveOBSView* main_view)
{
    notifation_register_ = std::make_unique<base::NotificationRegistrar>();
    danmaku_merged_main_view_ = new DanmakuMergedMainViddupView(this);

    interaction_view_ = DanmakuInteractionViddupView::GetView();

    gfx::Size mini_size = { dmkhime::kDanmakuGiftViewPopDefWidth, dmkhime::kDanmakuGiftViewPopMinHeight };
    //gift_view_->SetMiniSize(mini_size);
    mini_size = { dmkhime::kDanmakuViewPopDefWidth, dmkhime::kDanmakuInteractionViewPopMinHeight };
    interaction_view_->SetMiniSize(mini_size);
    mini_size = { dmkhime::kDanmakuViewPopDefWidth, dmkhime::kActivityTaskViewPopMinHeight };
    //activity_task_view_->SetMiniSize(mini_size);

    main_view_ = main_view;
    main_view_->GetWidget()->AddObserver(this);

    notifation_register_->Add(this, bililive::NOTIFICATION_LIVEHIME_DANMAKU_RELOADPREF,
        base::NotificationService::AllSources());

    notifation_register_->Add(this, bililive::NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_INIT,
        base::NotificationService::AllSources());

    notifation_register_->Add(this, bililive::NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_UPDATE,
        base::NotificationService::AllSources());

    notifation_register_->Add(this, bililive::NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_NEW,
        base::NotificationService::AllSources());

    notifation_register_->Add(this, bililive::NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_DELETE,
        base::NotificationService::AllSources());

    notifation_register_->Add(this, bililive::NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_CLOSE,
        base::NotificationService::AllSources());

    notifation_register_->Add(this, bililive::NOTIFICATION_LIVEHIME_MARKEDNESS_MESSAGE_TIMEREPEAT,
        base::NotificationService::AllSources());

    notifation_register_->Add(this, bililive::NOTIFICATION_LIVEHIME_DANMAKU_UI_PANINT,
        base::NotificationService::AllSources());

    LivehimeLiveRoomController::GetInstance()->AddObserver(this);

    GetBililiveProcess()->bililive_obs()->broadcast_viddup_service()->AddObserver(this);

    danmaku_merged_main_view_->AttachInteractionView(interaction_view_);

    main_view_->ShowDanmakuMergedView(true);

    LoadPref();
    LoadDanmakuStatePref();
    LoadEffectFilterSetting();
    SwitchToSplitMainViewShowModify();
}

void DanmakuhimeDataViddupHandler::Uninit()
{
    notifation_register_->RemoveAll();

    GetBililiveProcess()->bililive_obs()->broadcast_viddup_service()->RemoveObserver(this);
    LivehimeLiveRoomController::GetInstance()->RemoveObserver(this);

    room_id_ = 0;
}

void DanmakuhimeDataViddupHandler::NewDanmaku(const DanmakuInfo& danmaku)
{

}

void DanmakuhimeDataViddupHandler::NewNoticeDanmaku(const NoticeDanmakuInfo& danmaku)
{
    if (danmaku.msg_type == NMT_MELEE_LEVEL_MSG) {
        auto msg = UTF8ToUTF16(danmaku.msg_self);
        ReplaceSubstringsAfterOffset(&msg, 0, L"<%", L" ");
        ReplaceSubstringsAfterOffset(&msg, 0, L"%>", L" ");

        DanmakuData data;
        data.plain_text = msg;
        data.type = DT_SYS_MSG;
        data.sys_msg_type = DmkSysMsgType::MELEE;
    }
}

void DanmakuhimeDataViddupHandler::NewCommonNoticeDanmaku(const CommonNoticeDanmakuInfo& danmakuInfo)
{
    if (!interaction_view_)
    {
        return;
    }
}

void DanmakuhimeDataViddupHandler::AddLocalLiveAssistantDanmaku(const string16& msg)
{
    if (!interaction_view_ || !LivehimeLiveRoomController::GetInstance()->IsStreaming())
    {
        return;
    }

    DanmakuData data;
    data.type = DT_ANCHOR_BROADCAST;
    data.asistant_data.asit_title = L"Live-streaming assistant";
	  data.asistant_data.asit_msg = msg;

	  interaction_view_->AddDanmaku(data);
}

void DanmakuhimeDataViddupHandler::NewDanmaku(const std::vector<DanmakuViddupInfo>& danmaku) {
    std::string content;
    std::string userName;
    bool isAdmin = false;
    std::string gifFileUrl;  //url
    std::string giftName;
    std::string comboString;
    int treasureID = 0;

    DanmakuData data;

    for each (auto item in danmaku)
    {
        data.user_id = item.id;
        data.user_name = base::UTF8ToUTF16(item.userName);
        data.plain_text = base::UTF8ToUTF16(item.content);
        data.gift_id = item.treasureID;
        data.gift_name = base::UTF8ToUTF16(item.giftName);
        data.gift_num = 1;

        switch (item.type)
        {
        case DanmakuViddupInfo::User:
            data.type = DT_DANMU_MSG;
            break;
        case DanmakuViddupInfo::WarnMs:
            data.type = DT_ANCHOR_BROADCAST;
            data.warning_msg = true;
            data.asistant_data.asit_msg = base::UTF8ToUTF16(item.content);
            break;
        case  DanmakuViddupInfo::System:
        case DanmakuViddupInfo::PopupMsg:
        case DanmakuViddupInfo::MuteMsg:
        case DanmakuViddupInfo::BlockMsg:
            data.type = DT_SYS_MSG;
            if (item.type == DanmakuViddupInfo::PopupMsg)
            {
                data.popup_msg = true;
                auto& rb = ResourceBundle::GetSharedInstance();
                gfx::ImageSkia* image_skia_title = GetImageSkiaNamed(IDR_VIDDUP_DANMAKU_WARNING);

                livehime::ShowMessageBox(GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeView(),
                    base::UTF8ToUTF16(item.popupTitle), base::UTF8ToUTF16(item.popupContent), rb.GetLocalizedString(IDS_SURE), image_skia_title, nullptr, nullptr, gfx::ALIGN_LEFT);
                return;
            }
            else if (item.type == DanmakuViddupInfo::BlockMsg)
            {
                data.block_msg = true;
                //data.plain_text = base::UTF8ToUTF16(item.notice);
                data.file_duration = item.end_time;
            }
            break;
        case DanmakuViddupInfo::Gift:
            data.type = DT_SEND_GIFT;
            data.action = base::UTF8ToUTF16("sent");
            data.gift_combo_num = 0;
            data.gift_combo_string = base::UTF8ToUTF16(item.comboString);
            data.face_url = item.gifFileUrl;
            data.has_combo_send = true;
            break;
        }

        data.user_identity_tag_admin_ = item.isAdmin;
        data.user_identity_tag_top_ = item.id;
        data.limit_max_display = true;

        AddDanmakuToRenderViddupViews(data);
    }
}

void DanmakuhimeDataViddupHandler::AddDanmakuToRenderViddupViews(DanmakuData data)
{
    if (!interaction_view_)
    {
        return;
    }

    interaction_view_->AddDanmaku(data);
}

void DanmakuhimeDataViddupHandler::DanmakuStatus(int status)
{

}

void DanmakuhimeDataViddupHandler::LoadPref()
{
    DanmakuHimePreferenceInfo pref_info = DanmakuHimePrefService::LoadPref();

    first_pop_ = pref_info.first_pop;
    SetGiftIconEnabled(true);
    SetDanmakuFontSize(DanmakuWindowsType::Windows_Type_Gift,pref_info.font_size_gift);
    SetDanmakuFontSize(DanmakuWindowsType::Windows_Type_Interaction, pref_info.font_size_interaction);
    SetDanmakuFontSize(DanmakuWindowsType::Windows_Type_activity_task, pref_info.font_size_activity_task);

    SetScrollingFluencyIndex(pref_info.fluency);
    SetDanmakuHimeOpacity(DanmakuWindowsType::Windows_Type_Gift, pref_info.trans_gift / 100.f * 255);
    SetDanmakuHimeOpacity(DanmakuWindowsType::Windows_Type_Interaction, pref_info.trans_interaction / 100.f * 255);
    SetDanmakuHimeOpacity(DanmakuWindowsType::Windows_Type_activity_task, pref_info.trasn_activity_task / 100.f * 255);
    SetSuspendingScrollingEnabled(pref_info.enable_suspending);
    ShowFreeGifts(!pref_info.enable_gift_filter);
    ShowChargedGifts(pref_info.show_charged_gifts);
    SetChargedGiftFilterEnabled(pref_info.enable_charged_gift_thresold);
    SetChargedGiftCostFilter(pref_info.charged_gift_thresold);
    SetHardwareAccelerationEnabled(pref_info.enable_HWA);
    SetGiftEffectsEnabled(pref_info.enable_gift_effects);
    SwitchTheme(DanmakuWindowsType::Windows_Type_Gift, pref_info.theme_gift, true);
    SwitchTheme(DanmakuWindowsType::Windows_Type_Interaction, pref_info.theme_interaction, true);
    SwitchTheme(DanmakuWindowsType::Windows_Type_activity_task, pref_info.theme_activity_task, true);

    ShowUserEnterEffect(pref_info.show_user_enter_effect);
    SetUserEnterFilter(pref_info.enter_filter);
    SetUserEnterMinMedalLevel(pref_info.min_enter_medal_level);
    SetBlockLotteryEnabled(pref_info.block_lottery);
}

void DanmakuhimeDataViddupHandler::SaveDanmakuStatePref()
{
    if (!interaction_view_){
        return;
    }

    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
    float dpi = ui::GetDPIScale();

    pref->SetInteger(prefs::kDanmakuHimeInteractionViewShowMode, static_cast<int>(interaction_view_->GetShowType()));
    if (interaction_view_->GetDanmakuWidget()) {
        auto rect = interaction_view_->GetDanmakuWidget()->GetWindowBoundsInScreen();
        pref->SetInteger(prefs::kDanmakuHimeInteractionViewPosX, rect.x() / dpi);
        pref->SetInteger(prefs::kDanmakuHimeInteractionViewPosY, rect.y() / dpi);
        pref->SetInteger(prefs::kDanmakuHimeInteractionViewWidth, rect.width()/ dpi);
        pref->SetInteger(prefs::kDanmakuHimeInteractionViewHeight, rect.height()/ dpi);
    }
    else{
        pref->SetInteger(prefs::kDanmakuHimeInteractionViewPosX, -1);
        pref->SetInteger(prefs::kDanmakuHimeInteractionViewPosY, -1);
        pref->SetInteger(prefs::kDanmakuHimeInteractionViewWidth, -1);
        pref->SetInteger(prefs::kDanmakuHimeInteractionViewHeight, -1);
    }
    pref->SetBoolean(prefs::kDanmakuHimeInteractionViewPin, interaction_view_->IsPinned());
}

void DanmakuhimeDataViddupHandler::LoadDanmakuStatePref()
{
    if (!interaction_view_){
        return;
    }

    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();

    int x{}, y{}, width{}, height{};
    auto gift_show_type = static_cast<DanmakuShowType>(pref->GetInteger(prefs::kDanmakuHimeGiftViewShowMode));
    x = pref->GetInteger(prefs::kDanmakuHimeGiftViewPosX);
    y = pref->GetInteger(prefs::kDanmakuHimeGiftViewPosY);
    width = pref->GetInteger(prefs::kDanmakuHimeGiftViewWidth);
    height = pref->GetInteger(prefs::kDanmakuHimeGiftViewHeight);
    gift_pop_rect_ = GetDanmukaViewPrefRect(Windows_Type_Gift, x, y, width, height);

    auto interaction_show_type = static_cast<DanmakuShowType>(pref->GetInteger(prefs::kDanmakuHimeInteractionViewShowMode));
    x = pref->GetInteger(prefs::kDanmakuHimeInteractionViewPosX);
    y = pref->GetInteger(prefs::kDanmakuHimeInteractionViewPosY);
    width = pref->GetInteger(prefs::kDanmakuHimeInteractionViewWidth);
    height = pref->GetInteger(prefs::kDanmakuHimeInteractionViewHeight);
    interaction_pop_rect_ = GetDanmukaViewPrefRect(Windows_Type_Interaction, x, y, width, height);
    is_first_show_ = pref->GetBoolean(prefs::kDanmakuHimeInteractionViewFirstShow);
}

template<typename T>
T clamp(const T& v, const T& min_v, const T& max_v)
{
    return std::min(std::max(v, min_v), max_v);
}

void DanmakuhimeDataViddupHandler::LoadEffectFilterSetting()
{
    std::array<int64_t, 5> block_effect_value;

    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();

    block_effect_value[0] = pref->GetBoolean(prefs::kDanmakuHimeBlockAllEffect);
    is_all_effect_block_ = !block_effect_value[0];
    if (interaction_view_)
    {
        interaction_view_->GetTitleView()->EnableEffect(block_effect_value[0]);
    }

    bool b = pref->GetBoolean(prefs::kDanmakuHimeBlockAllCombo);
    int index = clamp(pref->GetInteger(prefs::kDanmakuHimeFilterCombo), 0, 4);
    if (b)
    {
        block_effect_value[1] = DanmakuhimeDataHandler::COMBO_FILTER_VALUE[index];
    }
    else
    {
        block_effect_value[1] = std::numeric_limits<int64_t>::max();
    }

    b = pref->GetBoolean(prefs::kDanmakuHimeBlockAllGift);
    index = clamp(pref->GetInteger(prefs::kDanmakuHimeFilterGift), 0, 4);
    if (b)
    {
        block_effect_value[2] = DanmakuhimeDataHandler::GIFT_FILTER_VALUE[index];
    }
    else
    {
        block_effect_value[2] = std::numeric_limits<int64_t>::max();
    }


    b = pref->GetBoolean(prefs::kDanmakuHimeBlockAllNavy);
    index = clamp(pref->GetInteger(prefs::kDanmakuHimeFilterNavy), 0, 3);
    if (b)
    {
        block_effect_value[3] = DanmakuhimeDataHandler::NAVY_FILTER_VALUE[index];
    }
    else
    {
        block_effect_value[3] = std::numeric_limits<int64_t>::max();
    }

    b = pref->GetBoolean(prefs::kDanmakuHimeShowFaceEffect);
    block_effect_value[4] = b;

    SetFilterEffect(block_effect_value);
}

void DanmakuhimeDataViddupHandler::Paint()
{
    if (interaction_view_)
    {
        interaction_view_->SchedulePaint();
    }
}

void DanmakuhimeDataViddupHandler::SetSuspendingScrollingEnabled(bool enabled)
{
    if (interaction_view_)
    {
        interaction_view_->SetSuspendingScrollingEnabled(enabled);
    }
}

void DanmakuhimeDataViddupHandler::SetHardwareAccelerationEnabled(bool enabled)
{
    if (enabled != rendering_params_interaction_.hardware_acceleration_)
    {
        rendering_params_interaction_.hardware_acceleration_ = enabled;
        rendering_params_interaction_.flags = DanmakuRenderingParams::Flag_HW;
        rendering_params_gift_.hardware_acceleration_ = enabled;
        rendering_params_gift_.flags = DanmakuRenderingParams::Flag_HW;
        RedrawRenderViewsBefore(Windows_Type_All);
    }
}

void DanmakuhimeDataViddupHandler::ShowFreeGifts(bool show)
{
    if (show != is_show_free_gift_)
    {
        is_show_free_gift_ = show;
        if (!show)
        {
            while (!free_queue_.empty())
            {
                free_queue_.pop();
            }
        }
    }
}

void DanmakuhimeDataViddupHandler::SetGiftEffectsEnabled(bool enabled)
{
    is_gift_effects_enabled_ = enabled;

    if (interaction_view_)
    {
        interaction_view_->SetGiftEffectsEnabled(enabled);
    }
}

void DanmakuhimeDataViddupHandler::SetGiftIconEnabled(bool enabled)
{
    if (enabled != rendering_params_interaction_.show_icon)
    {
        rendering_params_interaction_.show_icon = enabled;
        rendering_params_interaction_.flags = DanmakuRenderingParams::Flag_ShowIcon;
        rendering_params_gift_.show_icon = enabled;
        rendering_params_gift_.flags = DanmakuRenderingParams::Flag_ShowIcon;
        RedrawRenderViewsBefore(Windows_Type_All);
    }
}

void DanmakuhimeDataViddupHandler::SetDanmakuHimeOpacity(DanmakuWindowsType view_type, int opacity)
{
    if (view_type == Windows_Type_Interaction)
    {
        rendering_params_interaction_.opacity = opacity;
        rendering_params_interaction_.flags = DanmakuRenderingParams::Flag_Opacity;
        if (interaction_view_->GetShowType() == DanmakuShowType::kMerged)
        {
            return;
        }

        interaction_view_->RedrawRenderViews(rendering_params_interaction_);
        interaction_view_->ApplyOpacityToUI(opacity);
    }
}

void DanmakuhimeDataViddupHandler::SetDanmakuFontSize(DanmakuWindowsType view_type, int size)
{
    if (size < dmkhime::kMinFontSize || size > dmkhime::kMaxFontSize)
    {
        size = std::max(size, dmkhime::kMinFontSize);
        size = std::min(size, dmkhime::kMaxFontSize);
    }

    if (view_type == Windows_Type_Gift)
    {
        rendering_params_gift_.base_font_size = size;
        rendering_params_gift_.flags = DanmakuRenderingParams::Flag_BaseFontSize;
    }
    else if (view_type == Windows_Type_Interaction)
    {
        rendering_params_interaction_.base_font_size = size;
        rendering_params_interaction_.flags = DanmakuRenderingParams::Flag_BaseFontSize;
    }
    else if (view_type == Windows_Type_activity_task)
    {
        rendering_params_activity_task_.base_font_size = size;
        rendering_params_activity_task_.flags = DanmakuRenderingParams::Flag_BaseFontSize;
    }

    RedrawRenderViewsBefore(view_type);
}

void DanmakuhimeDataViddupHandler::SetScrollingFluencyIndex(int index)
{
    auto it = kFpsTable.find(index);
    if (it != kFpsTable.end())
    {
        if (interaction_view_)
        {
            interaction_view_->SetScrollingFps(it->second);
        }
    }
}

void DanmakuhimeDataViddupHandler::SwitchTheme(DanmakuWindowsType view_type, int theme, bool force)
{
    switch (theme)
    {
    case 0:
        SwitchTheme(view_type,dmkhime::Theme::SORCERESS, force);
        break;
    case 1:
        SwitchTheme(view_type,dmkhime::Theme::SNOW_FRUIT, force);
        break;
    case 2:
        SwitchTheme(view_type,dmkhime::Theme::SNOW_FRUIT, force);
        break;
    default:
        break;
    }
}

void DanmakuhimeDataViddupHandler::SwitchTheme(DanmakuWindowsType view_type, dmkhime::Theme theme, bool force)
{
    split_view_theme_ = theme;
    if (view_type == Windows_Type_Interaction)
    {
        if (force || theme != rendering_params_interaction_.theme)
        {
            rendering_params_interaction_.theme = theme;
            rendering_params_interaction_.flags = DanmakuRenderingParams::Flag_Theme;

            if (interaction_view_->GetShowType() == DanmakuShowType::kMerged)
            {
                return;
            }

            interaction_view_->RedrawRenderViews(rendering_params_interaction_);
            interaction_view_->SwitchTheme(theme == dmkhime::Theme::SNOW_FRUIT ? DanmakuThemeType::kWhite : DanmakuThemeType::kDark);
        }
    }
}

void DanmakuhimeDataViddupHandler::ShowUserEnterEffect(bool show)
{
    is_show_user_enter_effect_ = show;
}

void DanmakuhimeDataViddupHandler::ShowFaceEffect(bool show)
{
    is_show_face_effect_ = show;
}

bool DanmakuhimeDataViddupHandler::ShowFaceUISwitch()
{
    return AppFunctionController::GetInstance()->get_face_switch();
}

bool DanmakuhimeDataViddupHandler::JurdgeFaceEffectSwitch()
{
    return ShowFaceUISwitch() & is_show_face_effect_;
}

void DanmakuhimeDataViddupHandler::SetUserEnterFilter(int filter)
{
    user_enter_filter_ = filter;
}

void DanmakuhimeDataViddupHandler::SetUserEnterMinMedalLevel(int medal_level)
{
    min_enter_medal_level_ = medal_level;
}

void DanmakuhimeDataViddupHandler::SetBlockLotteryEnabled(bool enabled)
{
    is_lottery_blocking_enabled_ = enabled;
}

void DanmakuhimeDataViddupHandler::ShowChargedGifts(bool show)
{
    is_show_charged_gift_ = show;
}

void DanmakuhimeDataViddupHandler::SetChargedGiftFilterEnabled(bool enabled)
{
    is_charged_gift_filter_enabled_ = enabled;
}

void DanmakuhimeDataViddupHandler::SetChargedGiftCostFilter(int gold)
{
    charged_gift_min_gold_ = gold;
}

void DanmakuhimeDataViddupHandler::SetDanmakuTTSEnabled(bool enabled)
{
}

void DanmakuhimeDataViddupHandler::SetDanmakuTTSSource(const string16& name)
{
}

void DanmakuhimeDataViddupHandler::SetDanmakuTTSSpeed(int speed)
{
}

void DanmakuhimeDataViddupHandler::SetDanmakuTTSVolume(int volume)
{
}

void DanmakuhimeDataViddupHandler::ClearDanmakuTTSQueue()
{
}

void DanmakuhimeDataViddupHandler::SetFilterEffect(const std::array<int64_t, 5>& filter_value)
{
    if (!interaction_view_)
    {
        return;
    }

    bool enable_all_block = filter_value[0];
    int64_t combo_filter_value = filter_value[1];
    int64_t gift_filter_value = filter_value[2];
    int64_t navy_filter_value = filter_value[3];
    bool block_face_effect = filter_value[4];

    is_all_effect_block_ = !enable_all_block;

    if (interaction_view_)
    {
        interaction_view_->GetTitleView()->EnableEffect(enable_all_block);
    }
}

void DanmakuhimeDataViddupHandler::AddGenericDanmaku(
    const std::string& img_url, const std::string& img_data, const string16& plain_text,
    bool use_default_color, SkColor color,bool use_backgroundcolor)
{
}

void DanmakuhimeDataViddupHandler::AddTestDanmaku(DanmakuData& data)
{
}

void DanmakuhimeDataViddupHandler::OnGetUserInfo()
{
}

void DanmakuhimeDataViddupHandler::CommonNoticeDanmakuOpenWeb(const std::string& url)
{
    if (!DanmakuWebView::IsWebCreated())
    {
        DanmakuWebView::ShowWindow(nullptr, url, false, L"", WebViewPopupType::Native);
    }
}

bool DanmakuhimeDataViddupHandler::ProcessComboRecords(std::map<std::string, ComboRecord>& records, const std::string& combo_id, int64_t combo_num)
{
    if (combo_id.empty()) {
        return true;
    }

    int64_t ts = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    if (combo_ts_ == 0) {
        combo_ts_ = ts;
    }
    else if (ts - combo_ts_ > 10 * 60) {
        for (auto it = records.begin(); it != records.end();) {
            if (ts - it->second.ts > 5 * 60) {
                it = records.erase(it);
            }
            else {
                ++it;
            }
        }
        combo_ts_ = ts;
    }

    auto it = records.find(combo_id);
    if (it != records.end()) {
        if (it->second.combo_num >= combo_num) {
            return false;
        }

        it->second.ts = ts;
        it->second.combo_num = combo_num;
    }
    else {
        records[combo_id] = { ts, combo_num };
    }

    return true;
}

void DanmakuhimeDataViddupHandler::RedrawRenderViewsBefore(DanmakuWindowsType view_type)
{
    if(view_type == Windows_Type_Interaction || view_type == Windows_Type_All)
    {
        if (interaction_view_->GetShowType() == DanmakuShowType::kSplited)
        {
            interaction_view_->RedrawRenderViews(rendering_params_interaction_);
        }
        else
        {
            auto render_param = rendering_params_interaction_;
            render_param.flags |= DanmakuRenderingParams::Flag_Theme | DanmakuRenderingParams::Flag_Opacity;
            render_param.theme = dmkhime::Theme::SNOW_FRUIT;
            render_param.opacity = 255;
            interaction_view_->RedrawRenderViews(render_param);
        }
    }
}

gfx::Rect DanmakuhimeDataViddupHandler::GetDanmukaViewPrefRect(DanmakuWindowsType view_type, int x, int y, int width, int height)
{
    bool is_default_rect = x == -1 || y == -1 || width == -1 || height == -1;

    if (is_default_rect) {
        return GetDefaultPopRect(view_type);
    }
    else {
        int pop_width{}, pop_height{};
        if (Windows_Type_Gift == view_type)
        {
            pop_width = std::max(GetLengthByDPIScale(width), dmkhime::kDanmakuGiftViewPopDefWidth);
            pop_height = std::max(GetLengthByDPIScale(height), dmkhime::kDanmakuGiftViewPopMinHeight);
        }
        else if (Windows_Type_Interaction == view_type)
        {
            pop_width = std::max(GetLengthByDPIScale(width), dmkhime::kDanmakuViewPopDefWidth);
            pop_height = std::max(GetLengthByDPIScale(height), dmkhime::kDanmakuInteractionViewPopMinHeight);
        }
        else {
            pop_width = GetLengthByDPIScale(width);
            pop_height = GetLengthByDPIScale(height);
        }
        return gfx::Rect(GetLengthByDPIScale(x), GetLengthByDPIScale(y), pop_width, pop_height);
    }
}

gfx::Rect DanmakuhimeDataViddupHandler::GetDefaultPopRect(DanmakuWindowsType view_type)
{
    gfx::Rect rect;
    if (main_view_->GetWidget()) {
        gfx::Rect  main_rect = main_view_->GetWidget()->GetWindowBoundsInScreen();
        if (Windows_Type_activity_task == view_type) {
            rect.set_x(main_rect.x() + main_rect.width() - dmkhime::kDanmakuViewPopDefWidth - GetLengthByDPIScale(10));
            rect.set_y(main_rect.y() + GetLengthByDPIScale(60));
            rect.set_width(dmkhime::kDanmakuViewPopDefWidth);
            rect.set_height(dmkhime::kActivityTaskViewPopPopDefHeight);
        }
        else if (Windows_Type_Gift == view_type){
            rect.set_x(main_rect.x() + main_rect.width() - dmkhime::kDanmakuViewPopDefWidth - GetLengthByDPIScale(10));
            rect.set_y(main_rect.y() + dmkhime::kActivityTaskViewPopPopDefHeight+ GetLengthByDPIScale(60));
            rect.set_width(dmkhime::kDanmakuGiftViewPopDefWidth);
            rect.set_height(dmkhime::kDanmakuGiftViewPopDefHeight);
        }
        else if(Windows_Type_Interaction == view_type){
            rect.set_x(main_rect.x() + main_rect.width() - dmkhime::kDanmakuViewPopDefWidth - GetLengthByDPIScale(10));
            rect.set_y(main_rect.y() + main_rect.height() - dmkhime::kDanmakuInteractionViewPopDefHeight - GetLengthByDPIScale(10));
            rect.set_width(dmkhime::kDanmakuViewPopDefWidth);
            rect.set_height(dmkhime::kDanmakuInteractionViewPopDefHeight);
        }
    }
    else {
        if (Windows_Type_activity_task == view_type) {
         rect.set_width(dmkhime::kDanmakuViewPopDefWidth);
         rect.set_height(dmkhime::kActivityTaskViewPopPopDefHeight);
        }
        else if (Windows_Type_Gift == view_type) {
            rect.set_width(dmkhime::kDanmakuViewPopDefWidth);
            rect.set_height(dmkhime::kDanmakuGiftViewPopDefHeight);
        }
        else if (Windows_Type_Interaction == view_type) {
            rect.set_width(dmkhime::kDanmakuViewPopDefWidth);
            rect.set_height(dmkhime::kDanmakuInteractionViewPopDefHeight);
        }

    }
    return rect;
}


void DanmakuhimeDataViddupHandler::EnableAllEffect(bool enable)
{
    if (!interaction_view_)
    {
        return;
    }

    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();

    if (enable)
    {
        pref->SetBoolean(prefs::kDanmakuHimeBlockAllEffect, true);
    }
    else
    {
        pref->SetBoolean(prefs::kDanmakuHimeBlockAllEffect, false);
    }

    LoadEffectFilterSetting();
}

void DanmakuhimeDataViddupHandler::SwitchToMergedGiftTheme()
{
}

void DanmakuhimeDataViddupHandler::SwitchToMergedInteractionTheme()
{
    auto render_param = rendering_params_interaction_;
    render_param.flags |= DanmakuRenderingParams::Flag_Theme | DanmakuRenderingParams::Flag_Opacity;
    render_param.theme = dmkhime::Theme::SNOW_FRUIT;
    render_param.opacity = 255;

    if (interaction_view_)
    {
        interaction_view_->RedrawRenderViews(render_param);
        interaction_view_->ApplyOpacityToUI(render_param.opacity);
        interaction_view_->SwitchTheme(DanmakuThemeType::kWhite);
    }
}

void DanmakuhimeDataViddupHandler::SwitchToMergedActivityTaskTheme()
{
}

void DanmakuhimeDataViddupHandler::SwitchToSplitGiftTheme()
{
}

void DanmakuhimeDataViddupHandler::SwitchToSplitInteractionTheme()
{
    rendering_params_interaction_.flags = DanmakuRenderingParams::Flag_Theme | DanmakuRenderingParams::Flag_Opacity;
    if (interaction_view_)
    {
        interaction_view_->RedrawRenderViews(rendering_params_interaction_);
        interaction_view_->ApplyOpacityToUI(rendering_params_interaction_.opacity);
        interaction_view_->SwitchTheme(rendering_params_interaction_.theme == dmkhime::Theme::SNOW_FRUIT ? DanmakuThemeType::kWhite : DanmakuThemeType::kDark);
    }
}

void DanmakuhimeDataViddupHandler::SwitchToSplitActivityTaskTheme()
{
}

void DanmakuhimeDataViddupHandler::SwitchToSplitMainViewShowModify()
{
    if (main_view_ &&
        danmaku_merged_main_view_&&
        danmaku_merged_main_view_->child_count() == 0)
    {
        main_view_->ShowDanmakuMergedView(false);
    }
    else if (main_view_ &&
        danmaku_merged_main_view_&&
        danmaku_merged_main_view_->child_count() > 0)
    {
        main_view_->ShowDanmakuMergedView(true);
    }
}

int DanmakuhimeDataViddupHandler::GetRenderViewCurHeight(DanmakuWindowsType view_type)
{
    int height = 0;
    if (view_type == Windows_Type_Interaction)
    {
        int view_height = dmkhime::kDanmakuInteractionViewPopMinHeight;
        if (interaction_view_)
        {
            gfx::Rect rect = interaction_view_->GetBoundsInScreen();
            if (rect.height() > 0)
            {
                view_height = std::min(rect.height(), view_height);
            }
        }
        height = view_height - GetLengthByDPIScale(38) * 2;
    }

    return height;
}