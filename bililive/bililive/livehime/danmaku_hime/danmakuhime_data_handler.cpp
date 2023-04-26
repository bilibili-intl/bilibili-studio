#include "danmakuhime_data_handler.h"

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

#include "bililive/bililive/viddup/danmaku_hime/danmakuhime_data_viddup_handler.h"


DanmakuhimeDataHandler DanmakuhimeDataHandler::danmaku_data_handler_instance_;

const int64_t DanmakuhimeDataHandler::COMBO_FILTER_VALUE[4] = { 0, 100, 1000, 10000 };
const int64_t DanmakuhimeDataHandler::GIFT_FILTER_VALUE[4] = { 0, 500, 2000, 10000 };
const int64_t DanmakuhimeDataHandler::NAVY_FILTER_VALUE[3] = { 3, 2, 1 };


using MarkednessMessageInfo = secret::LiveStreamingService::MarkednessMessageInfo;

const std::map<int, int> kFpsTable{
    { 0, 24 }, { 1, 30 }, { 2, 45 }, { 3, 60 }
};

DanmakuhimeDataHandler* DanmakuhimeDataHandler::GetInstance()
{
    return DanmakuhimeDataViddupHandler::GetInstance();
}

bool DanmakuhimeDataHandler::FilterPaintMessage(HWND hwnd)
{
    return false;
}

DanmakuhimeDataHandler::DanmakuhimeDataHandler() :
    weak_ptr_factory_(this)
{
}

DanmakuhimeDataHandler::~DanmakuhimeDataHandler()
{

}

void DanmakuhimeDataHandler::Observe(int type, const base::NotificationSource& source, const base::NotificationDetails& details)
{

}

void DanmakuhimeDataHandler::OnOpenLiveRoomSuccessed(const secret::LiveStreamingService::StartLiveInfo& start_live_info)
{
    total_gold_coin_ = 0;

}

void DanmakuhimeDataHandler::OnPreLiveRoomStartStreaming(bool is_restreaming)
{
   /* if (danmaku_split_main_view_)
    {
        danmaku_split_main_view_->OnPreLiveRoomStartStreaming(is_restreaming);
    }*/
}

void DanmakuhimeDataHandler::OnLiveRoomStreamingStopped()
{

}

void DanmakuhimeDataHandler::OnLiveRoomStreamingError(
    obs_proxy::StreamingErrorCode streaming_errno, const std::wstring& error_message, int error_code)
{

}

void DanmakuhimeDataHandler::OnWidgetDestroying(views::Widget* widget)
{
    
}

void DanmakuhimeDataHandler::OnWidgetVisibilityChanged(views::Widget* widget, bool visible)
{

}

void DanmakuhimeDataHandler::OnWidgetBoundsChanged(views::Widget* widget, const gfx::Rect& new_bounds)
{

}

DanmakuMergedMainView* DanmakuhimeDataHandler::GetDanmakuMergedMainView()
{
    return danmaku_merged_main_view_;
}

DanmakuInteractionView* DanmakuhimeDataHandler::GetDanmakuInteractionView()
{
    return interaction_view_;
}

void DanmakuhimeDataHandler::SetMainView(BililiveOBSView* main_view)
{
    //main_view->AttachMergedDanmakuMainView(GetDanmakuMergedMainView());
}

void DanmakuhimeDataHandler::Init(BililiveOBSView* main_view)
{
   
}

void DanmakuhimeDataHandler::Uninit()
{

}

void DanmakuhimeDataHandler::NewDanmaku(const DanmakuInfo & danmaku)
{
   
}

void DanmakuhimeDataHandler::NewNoticeDanmaku(const NoticeDanmakuInfo& danmaku)
{

}

void DanmakuhimeDataHandler::NewCommonNoticeDanmaku(const CommonNoticeDanmakuInfo& danmakuInfo)
{

}

void DanmakuhimeDataHandler::DanmakuStatus(int status)
{
    /*if (danmaku_split_main_view_) {
        danmaku_split_main_view_->DanmakuStatus(status);
    }*/
}

void DanmakuhimeDataHandler::AddGiftEffect(const DanmakuData& data)
{
   
}

void DanmakuhimeDataHandler::AddInteractionViewGiftEffect(const DanmakuData& data)
{
   
}

void DanmakuhimeDataHandler::AddDanmakuToRenderViews(const DanmakuData& data)
{
    
}

void DanmakuhimeDataHandler::SendToTTS(const DanmakuData& data)
{
   
}

void DanmakuhimeDataHandler::LoadPref()
{
   
}

void DanmakuhimeDataHandler::SaveDanmakuStatePref()
{
   
}

void DanmakuhimeDataHandler::LoadDanmakuStatePref()
{
    
}

template<typename T>
T clamp(const T& v, const T& min_v, const T& max_v)
{
    return std::min(std::max(v, min_v), max_v);
}

void DanmakuhimeDataHandler::LoadEffectFilterSetting()
{
    
}

void DanmakuhimeDataHandler::Paint()
{

}

void DanmakuhimeDataHandler::SetSuspendingScrollingEnabled(bool enabled)
{
   
}

void DanmakuhimeDataHandler::SetHardwareAccelerationEnabled(bool enabled)
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

void DanmakuhimeDataHandler::ShowFreeGifts(bool show)
{
    if (show != is_show_free_gift_)
    {
        is_show_free_gift_ = show;
        if (!show)
        {
            // 开启银瓜子过滤的话，就把等待队列全清掉
            while (!free_queue_.empty())
            {
                free_queue_.pop();
            }
        }
    }
}

void DanmakuhimeDataHandler::SetGiftEffectsEnabled(bool enabled)
{

}

void DanmakuhimeDataHandler::SetGiftIconEnabled(bool enabled)
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

void DanmakuhimeDataHandler::SetDanmakuHimeOpacity(DanmakuWindowsType view_type, int opacity)
{
}

void DanmakuhimeDataHandler::SetDanmakuFontSize(DanmakuWindowsType view_type, int size)
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

void DanmakuhimeDataHandler::SetScrollingFluencyIndex(int index)
{

}

void DanmakuhimeDataHandler::SwitchTheme(DanmakuWindowsType view_type, int theme, bool force)
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

void DanmakuhimeDataHandler::SwitchTheme(DanmakuWindowsType view_type, dmkhime::Theme theme, bool force)
{
    split_view_theme_ = theme;
    if (view_type == Windows_Type_Gift)
    {
        if (force || theme != rendering_params_gift_.theme)
        {
            rendering_params_gift_.theme = theme;
            rendering_params_gift_.flags = DanmakuRenderingParams::Flag_Theme;
        }
    }
    else if (view_type == Windows_Type_Interaction)
    {
        if (force || theme != rendering_params_interaction_.theme)
        {
            rendering_params_interaction_.theme = theme;
            rendering_params_interaction_.flags = DanmakuRenderingParams::Flag_Theme;           
        }
    }
    else if (view_type == Windows_Type_activity_task) {

    }
}

void DanmakuhimeDataHandler::ShowUserEnterEffect(bool show)
{
    is_show_user_enter_effect_ = show;
}

void DanmakuhimeDataHandler::ShowFaceEffect(bool show)
{
    is_show_face_effect_ = show;
}

bool DanmakuhimeDataHandler::ShowFaceUISwitch()
{
    return AppFunctionController::GetInstance()->get_face_switch();
}

bool DanmakuhimeDataHandler::JurdgeFaceEffectSwitch()
{
    return ShowFaceUISwitch() & is_show_face_effect_;
}

void DanmakuhimeDataHandler::SetUserEnterFilter(int filter)
{
    user_enter_filter_ = filter;
}

void DanmakuhimeDataHandler::SetUserEnterMinMedalLevel(int medal_level)
{
    min_enter_medal_level_ = medal_level;
}

void DanmakuhimeDataHandler::SetBlockLotteryEnabled(bool enabled)
{
    is_lottery_blocking_enabled_ = enabled;
}

void DanmakuhimeDataHandler::ShowChargedGifts(bool show)
{
    is_show_charged_gift_ = show;
}

void DanmakuhimeDataHandler::SetChargedGiftFilterEnabled(bool enabled)
{
    is_charged_gift_filter_enabled_ = enabled;
}

void DanmakuhimeDataHandler::SetChargedGiftCostFilter(int gold)
{
    charged_gift_min_gold_ = gold;
}

void DanmakuhimeDataHandler::AddTestDanmaku(DanmakuData& data)
{
    data.is_spread = false;
    AddDanmakuToRenderViews(data);
}


void DanmakuhimeDataHandler::CommonNoticeDanmakuOpenWeb(const std::string& url)
{
    if (!DanmakuWebView::IsWebCreated())
    {
        DanmakuWebView::ShowWindow(nullptr, url, false, L"", WebViewPopupType::Native);
    }
}

bool DanmakuhimeDataHandler::ProcessComboRecords(std::map<std::string, ComboRecord>& records, const std::string& combo_id, int64_t combo_num)
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
        // 每10分钟检查一次
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

void DanmakuhimeDataHandler::RedrawRenderViewsBefore(DanmakuWindowsType view_type)
{
   
}

gfx::Rect DanmakuhimeDataHandler::GetDanmukaViewPrefRect(DanmakuWindowsType view_type, int x, int y, int width, int height)
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

gfx::Rect DanmakuhimeDataHandler::GetDefaultPopRect(DanmakuWindowsType view_type)
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


void DanmakuhimeDataHandler::EnableAllEffect(bool enable)
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

void DanmakuhimeDataHandler::SwitchToMergedGiftTheme()
{
    auto render_param = rendering_params_gift_;
    render_param.flags |= DanmakuRenderingParams::Flag_Theme | DanmakuRenderingParams::Flag_Opacity;
    render_param.theme = dmkhime::Theme::SNOW_FRUIT;
    render_param.opacity = 255;
}

void DanmakuhimeDataHandler::SwitchToMergedInteractionTheme()
{

}

void DanmakuhimeDataHandler::SwitchToMergedActivityTaskTheme()
{
   
}

void DanmakuhimeDataHandler::SwitchToSplitGiftTheme()
{

}

void DanmakuhimeDataHandler::SwitchToSplitInteractionTheme()
{

}

void DanmakuhimeDataHandler::SwitchToSplitActivityTaskTheme()
{

}

void DanmakuhimeDataHandler::SwitchToSplitMainViewShowModify()
{
    //if (main_view_ &&
    //    danmaku_merged_main_view_&&
    //    danmaku_merged_main_view_->child_count() == 0)
    //{
    //    main_view_->ShowDanmakuMergedView(false);
    //}
    //else if (main_view_ &&
    //    danmaku_merged_main_view_&&
    //    danmaku_merged_main_view_->child_count() > 0)
    //{
    //    main_view_->ShowDanmakuMergedView(true);
    //}
}

void DanmakuhimeDataHandler::ReportWindowsStatusChangedEvent()
{

}

int DanmakuhimeDataHandler::GetRenderViewCurHeight(DanmakuWindowsType view_type)
{
    int height = 0;
    if (view_type == Windows_Type_Gift)
    {
        int view_height = dmkhime::kDanmakuGiftViewPopMinHeight;
        //if (gift_view_)
        //{
        //    gfx::Rect rect = gift_view_->GetBoundsInScreen();
        //    if (rect.height() > 0)
        //    {
        //        view_height = std::min(rect.height(), view_height);
        //    }
        //}
        height = view_height - GetLengthByDPIScale(38) * 2;//除去title和过滤条件行高
    }else if (view_type == Windows_Type_Interaction)
    {
        int view_height = dmkhime::kDanmakuInteractionViewPopMinHeight;
        //if (interaction_view_)
        //{
        //    gfx::Rect rect = interaction_view_->GetBoundsInScreen();
        //    if (rect.height() > 0)
        //    {
        //        view_height = std::min(rect.height(), view_height);
        //    }
        //}
        height = view_height - GetLengthByDPIScale(38) * 2;//除去title和发送栏高度
    }
    else if (view_type == Windows_Type_activity_task)
    {
        int view_height = dmkhime::kDanmakuInteractionViewPopMinHeight;

        height = view_height - GetLengthByDPIScale(38) * 2;//除去title和发送栏高度
    }
    return height;
}