#include "bililive/bililive/ui/bililive_command_handler_livehime.h"

#include <Windows.h>
#include <shellapi.h>
#include <shlwapi.h>

#include "base/bind.h"
#include "base/json/json_writer.h"
#include "base/path_service.h"
#include "base/prefs/pref_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/metro.h"

#include "ui/base/clipboard/scoped_clipboard_writer.h"

#include "obs/obs_proxy/core_proxy/core_proxy_impl.h"
#include "obs/obs_proxy/public/common/pref_constants.h"
#include "obs/obs_proxy/public/common/pref_names.h"

#include "bilibase/scope_guard.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/danmaku_hime/danmakuhime_data_handler.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"
#include "bililive/bililive/livehime/main_view/livehime_main_close_pref_constants.h"
#include "bililive/bililive/livehime/obs/obs_proxy_service.h"
#include "bililive/bililive/livehime/obs/output_controller.h"
#include "bililive/bililive/livehime/obs/source_creator.h"
#include "bililive/bililive/livehime/sources_properties/source_browser_property_presenter_impl.h"
#include "bililive/bililive/livehime/user_info/user_info_service.h"
#include "bililive/bililive/livehime/pluggable/pluggable_controller.h"
#include "bililive/bililive/profiles/profile.h"
#include "bililive/bililive/ui/bililive_command_macros.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/controls/bililive_bubble.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_hotkey_notify_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/feedback/livehime_feedback_web_view.h"
#include "bililive/bililive/ui/views/livehime/live_notice/live_notice_dialog.h"
#include "bililive/bililive/ui/views/livehime/lottery/lottery_main_web_view.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_close_widget.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/bililive/ui/views/livehime/volume/bililive_sound_effect_widget.h"
#include "bililive/bililive/ui/views/livehime/settings/settings_frame_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/source_camera_property_view.h"
#include "bililive/bililive/ui/views/livehime/sources_properties/browser_source/browser_source_interaction_view.h"
#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_widget.h"
#include "bililive/bililive/ui/views/livehime/web_browser/livehime_unite_browser_widget.h"
#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_datatype.h"
#include "bililive/bililive/ui/views/livehime/tips_dialog/tips_dialog.h"
#include "bililive/bililive/ui/views/menu/sceneitem_menu_cmd.h"
#include "bililive/bililive/ui/views/tabarea/tabarea_view.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/bililive_paths.h"
#include "bililive/public/common/except_common.h"
#include "bililive/public/common/pref_names.h"
#include "bililive/public/secret/bililive_secret.h"

#include "grit/generated_resources.h"
#include "base/ext/callable_callback.h"


namespace
{
struct CmdHandlerParam
{
    explicit CmdHandlerParam(int cmd_code)
        : cmd(cmd_code)
    {
    }

    int cmd = 0;

    SceneItemCmd scene_item_cmd;
};

// 源添加/编辑窗口打开时不允许热键切换场景，当前底层很多地方是根据scene_collection的当前选中场景current_scene来
// 操作item的，操作过程中切换场景可能导致操作结束后访问到错误的scene上
bool g_scene_hotkey_enabled = true;

// 连麦需要禁用系统音捕获，连麦窗口打开时将此值置false以禁用系统音静音快捷键
bool g_sysvol_hotkey_enabled = true;

const char* kGainFilterPropertyDB = "db";

using bililive::CreatingSourceParams;
using bililive::ExecuteCommandWithParamsLivehime;

std::wstring LocalStr(int res_id)
{
    return ResourceBundle::GetSharedInstance().GetLocalizedString(res_id);
}

bool StartLive(StartLiveType live_type, bool hot_key_invoke)
{
    StartLiveFlowResult ret = GetBililiveProcess()->bililive_obs()->live_room_controller()->
        OpenLiveRoom(live_type, hot_key_invoke);
    return ((ret == StartLiveFlowResult::Succeed) || (ret == StartLiveFlowResult::ChooseArea));
}

void StopLive()
{
    GetBililiveProcess()->bililive_obs()->live_room_controller()->
        CloseLiveRoom();
}

void OnReportFilterBeauty()
{
    bool has_beauty = []() {
        auto pProxy = OBSProxyService::GetInstance().GetOBSCoreProxy();
        if (pProxy && pProxy->GetCurrentSceneCollection())
        {
            if (pProxy->GetCurrentSceneCollection()->GetScenes().empty())
            {
                return false;
            }

            for (auto scene : pProxy->GetCurrentSceneCollection()->GetScenes())
            {
                if (scene->GetItems().empty())
                {
                    return false;
                }

                for (auto item : scene->GetItems())
                {
                    if (item->type() == obs_proxy::SceneItemType::VideoCaptureDevice)
                    {
                        if (item->GetFilters().empty())
                        {
                            return false;
                        }

                        for (auto flt : item->GetFilters())
                        {
                            if (flt->type() == obs_proxy::FilterType::Beauty)
                                return true;
                        }
                    }
                }
            }
        }

        return false;
    }();

    auto mid = GetBililiveProcess()->secret_core()->account_info().mid();
    std::string event_msg = base::StringPrintf("beauty:%d", has_beauty?1:0);
    GetBililiveProcess()->secret_core()->event_tracking_service()->ReportLivehimeBehaviorEvent(
        secret::LivehimeBehaviorEvent::LivehimeFilterNameBeauty, mid, event_msg
    ).Call();
}

void OnReportExcept()
{
    // get except msg by read except file
    base::FilePath except_path;
    PathService::Get(bililive::DIR_CRASH_DUMPS, &except_path);
    except_path.Append(L"except_file");

    std::string except_msg;

    {
        base::ThreadRestrictions::ScopedAllowIO allow;
        if (base::PathExists(except_path))
        {
            std::lock_guard<std::mutex> guard(bililive::g_except_report_files_sync);

            file_util::ReadFileToString(except_path, &except_msg);

            //delete except files
            base::DeleteFile(except_path, false);
        }
    }

    //report except msg
    if (!except_msg.empty()) {
        auto mid = GetBililiveProcess()->secret_core()->account_info().mid();
        std::string event_msg = base::StringPrintf("except_func:%s", except_msg.c_str());
        GetBililiveProcess()->secret_core()->event_tracking_service()->ReportLivehimeBehaviorEvent(
            secret::LivehimeBehaviorEvent::LivehimeExceptNames, mid, event_msg
        ).Call();
    }
}

void DoAppClose()
{
    LOG(INFO) << "do app close.";

    if (bililive::OutputController::GetInstance()->IsRecording())
    {
        LOG(INFO) << "still recording when doing app close, stop recording.";
        bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_STOP_RECORDING);
    }

    if (!LivehimeLiveRoomController::GetInstance()->IsLiveReady())
    {
        LOG(INFO) << "still streaming when doing app close, stop streaming.";
        bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_STOP_LIVE_STREAMING);
    }

    // 如果开着模态框那么直接exit关掉进程，防止窗口从底层到顶层一层层关闭后导致执行到最顶层窗口关闭的时候
    // 该窗口引用到了其已关闭并析构了的owner窗口的指针导致崩溃
    // 直接生硬的退掉，还能防止可能出现的程序退出过程中卡死导致不能结束进程致使更新安装包无法正常安装的问题
    // fixbug：在退出确认框异步执行OnMainCloseViewEndDialog的时候主界面有极小可能已经关掉了，obs_view()=nullptr，
    //         比如用户确认退出到回调执行期间用户极速右下角直接退出
    DCHECK(GetBililiveProcess()->bililive_obs()->obs_view());
    if (GetBililiveProcess()->bililive_obs()->obs_view() &&
        GetBililiveProcess()->bililive_obs()->obs_view()->IsInModalNow())
    {
        // 保存全局
        PrefService* global_prefs = GetBililiveProcess()->global_profile()->GetPrefs();
        global_prefs->CommitPendingWrite();
        // 保存用户
        PrefService* user_prefs = GetBililiveProcess()->profile()->GetPrefs();
        user_prefs->CommitPendingWrite();
        // 保存OBS
        OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection()->Save();
        LOG(INFO) << "--- Process Exit ---";
        ::ExitProcess(0);
        return;
    }

    // 上报美颜的埋点
    OnReportFilterBeauty();

    // 上报opengl try_catch的埋点
    OnReportExcept();

    views::Widget::CloseAllSecondaryWidgets();
}

void OnMainCloseViewEndDialog(int result, void* data)
{
    if (result == IDOK)
    {
        PrefService* global_prefs = GetBililiveProcess()->global_profile()->GetPrefs();
        int exit_style = global_prefs->GetInteger(prefs::kApplicationExitMode);
        if (exit_style == prefs::kMinimizeToTray)
        {
            GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->Hide();
        }
        else if (exit_style == prefs::kExitImmediately)
        {
            if (bililive::OutputController::GetInstance()->IsCloseApp()) {
                LOG(INFO) << "user close app on update.";
                return;
            }
            LOG(INFO) << "user close app.";
            DoAppClose();
        }
    }
}

void OnCloseApp(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    livehime->obs_view()->GetWidget()->Activate();

    // 三个退出点：右下角菜单/主界面右上角关闭按钮、新版本应用、登出，只有右下角菜单退出
    // 不受主界面modal态的影响而进行操作
    if (livehime->obs_view()->IsInModalNow())
    {
        BililiveWidgetDelegate::FlashModalWidget(livehime->obs_view()->GetWidget()->GetNativeView());
        return;
    }

    if (!params.map_key())
    {
        PrefService* global_prefs = GetBililiveProcess()->global_profile()->GetPrefs();

        bool remembered = global_prefs->GetBoolean(prefs::kApplicationExitRememberChoice);
        if (!remembered)
        {
            EndDialogSignalHandler handler;
            handler.closure = base::Bind(&OnMainCloseViewEndDialog);
            LivehimeMainCloseView::DoModal(livehime->obs_view()->GetWidget()->GetNativeView(),
                                           &handler);
            return;
        }
        else
        {
            int exit_style = global_prefs->GetInteger(prefs::kApplicationExitMode);
            if (exit_style == prefs::kMinimizeToTray)
            {
                livehime->obs_view()->GetWidget()->Hide();
                return;
            }
        }
    }
    if (bililive::OutputController::GetInstance()->IsCloseApp()) {
        LOG(INFO) << "user close app on update";
        return;
    }

    LOG(INFO) << "user close app";
    DoAppClose();
}

void OnCloseAppSilently(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    //livehime->obs_view()->GetWidget()->Activate();
    DoAppClose();
}

void OnRequsetLogoutViddup(bool success)
{
    GetBililiveProcess()->SetLogout();

    DoAppClose();
}

void OnLogoutMsgboxEndDialog(const base::string16& result, void* data)
{
    if (result == LocalStr(IDS_CANCEL) || result.empty())
    {
        return;
    }

    GetBililiveProcess()->secret_core()->user_account_service()->RequsetLogoutViddup(
        base::MakeCallable(base::Bind(&OnRequsetLogoutViddup)), GetBililiveProcess()->secret_core()->get_csrf_token()).Call();
}

void OnLogout(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    livehime->obs_view()->GetWidget()->Activate();

    livehime::MessageBoxEndDialogSignalHandler handler;
    handler.closure = base::Bind(&OnLogoutMsgboxEndDialog);
    gfx::NativeView main_widget = livehime->obs_view()->GetWidget()->GetNativeView();
    livehime::ShowMessageBox(
        main_widget,
        LocalStr(IDS_TIP_DLG_TIP),
        LocalStr(IDS_MESSAGEBOX_CONTENTS_CONFIRMLOGOUT),
        LocalStr(IDS_SURE) + L"," + LocalStr(IDS_CANCEL),
        &handler);
}

void OnTokenInvalidMsgboxEndDialog(const base::string16& result, void* data)
{
    GetBililiveProcess()->SetTokenInvalid();
	GetBililiveProcess()->SetLogout();
	DoAppClose();
}

void OnTokenInvalid(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    livehime->obs_view()->GetWidget()->Activate();

    gfx::NativeView main_widget = livehime->obs_view()->GetWidget()->GetNativeView();
    livehime::MessageBoxEndDialogSignalHandler handler;
    handler.closure = base::Bind(&OnTokenInvalidMsgboxEndDialog);
    livehime::ShowMessageBox(
        main_widget,
        LocalStr(IDS_TIP_DLG_TIP),
        LocalStr(IDS_TOKEN_INVALID),
        LocalStr(IDS_SURE),
        &handler);
}

void OnMinimizeWindow(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    livehime->obs_view()->GetWidget()->Minimize();
}

void OnMaximizeWindow(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    livehime->obs_view()->GetWidget()->Maximize();
}

void OnRestoreWindow(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    livehime->obs_view()->GetWidget()->Restore();
}

void OnActiveMainWindow(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    //livehime->obs_view()->GetWidget()->Show();
    // 不使用框架封装的views::Widget::Show，其内部使用SW_SHOWNORMAL会把最大化的窗口还原到普通尺寸
    if(livehime->obs_view())
    {
        ::ShowWindow(livehime->obs_view()->GetWidget()->GetNativeWindow(), SW_SHOW);
        livehime->obs_view()->GetWidget()->Activate();
    }
}

void OnShowSoundEffect(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    BililiveSoundEffectWidget::ShowForm(livehime->obs_view()->GetWidget());
}

void OnShowNoviceGuide(BililiveOBS* livehime, const CommandParamsDetails& params)
{

}

void OnCheckForUpdate(BililiveOBS* livehime, const CommandParamsDetails& params)
{
	int update_type = 0;
	int* ptr = CommandParams<int>(params).ptr();
	if (ptr) {
        update_type = *ptr;
	}
}

void OnSettings(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    // 主界面有未关闭的模态框时不要弹出设置框（从弹幕姬弹）
    if (GetBililiveProcess()->bililive_obs()->obs_view()->IsInModalNow())
    {
        BililiveWidgetDelegate::FlashModalWidget(
            GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeView());
        return;
    }

    int index = 0;
    int* ptr = CommandParams<int>(params).ptr();
    if (ptr) {
        index = *ptr;
    }

    SettingsFrameView::ShowForm(livehime->obs_view()->GetWidget(), index);
}

void OnShare(BililiveOBS* livehime, const CommandParamsDetails& params)
{

}

void OnShowGiftbox(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    
}

void OnShowAnchorNotice(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    LiveNoticeDialog::ShowWindow(GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget());
}

void OnShowUserCard(BililiveOBS* livehime, const CommandParamsDetails& params)
{

}
void OnSchemeCmd(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    PluggableInfo* ptr = CommandParams<PluggableInfo>(params).ptr();
    if (ptr&& livehime->obs_view()) {
        livehime->obs_view()->AddPluggableInfo(*ptr);
    }
}

void OnChatShowUserCard(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    int64_t uid = 0;
    if (params.map_key())
    {
        uid = *CommandParams<int64_t>(params).ptr();
    }

    if (uid > 0) {
        
    }
}

void OnShowLogFile(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    base::FilePath log_dir;
    PathService::Get(bililive::DIR_LOGS, &log_dir);
    if (!log_dir.empty()) {
        auto log_path = log_dir.Append(bililive::kBililiveLogFileName);
        std::wstring shell_cmd(L"/select,");
        shell_cmd += log_path.value();
        ShellExecuteW(nullptr, L"open", L"explorer", shell_cmd.c_str(), nullptr, SW_SHOWDEFAULT);
    }
}

void OnStartRecording(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    DCHECK(bililive::OutputController::GetInstance()->IsRecordingReady());

    bool hotkey = false;
    if (params.map_key())
    {
        hotkey = *CommandParams<bool>(params).ptr();
    }

    if (hotkey)
    {
        HotkeyNotifyView::ShowForm(HotkeyNotifyView::HN_RecordStart,
            livehime->obs_view()->get_preview_rect());
    }

    bililive::OutputController::GetInstance()->StartRecording();
}

void OnStopRecording(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    DCHECK(bililive::OutputController::GetInstance()->IsRecording());

    bililive::OutputController::GetInstance()->StopRecording();
}

void OnCheckAndAddSceneMsgboxEndDialog(const base::string16& result, void* data)
{
    // 用户决定场景为空也要播，则开播
    if (result == LocalStr(IDS_SURE)){
        // 以常规模式开播
        StartLive(StartLiveType::Normal, !!data);
    }
}

void OnStartLiveStreaming(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    DCHECK(LivehimeLiveRoomController::GetInstance()->IsLiveReady());

    const auto& hotkey = *CommandParams<bool>(params).ptr();

    // 检查场景是否为空
    auto* proxy = OBSProxyService::GetInstance().GetOBSCoreProxy();
    obs_proxy::Scene* scene = proxy->GetCurrentSceneCollection()->current_scene();
    // 空就弹窗提醒，否则就选分区
    if (scene->GetItems().empty()){
        livehime::MessageBoxEndDialogSignalHandler handler;
        handler.closure = base::Bind(&OnCheckAndAddSceneMsgboxEndDialog);
        handler.data = hotkey ? &base::NotificationService::DummyDetails() : nullptr;
        livehime::ShowMessageBox(
            livehime->obs_view()->GetWidget()->GetNativeView(),
            LocalStr(IDS_PROMPT_EMPTY_SCENE_TITLE),
            LocalStr(IDS_PROMPT_EMPTY_SCENE_MSG),
            LocalStr(IDS_SURE) + L"," + LocalStr(IDS_CANCEL),
            &handler);
    }
    else{
        // 以常规模式开播
        StartLive(StartLiveType::Normal, hotkey);
    }
}

void OnStopLiveStreaming(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    DCHECK(!LivehimeLiveRoomController::GetInstance()->IsLiveReady());

	bool invoke_by_click = false;
	if (params.map_key())
	{
		invoke_by_click = *CommandParams<bool>(params).ptr();
	}
    if (invoke_by_click)
    {
        //关播拦截只处理点击停止直播按钮或热键停止
		GetBililiveProcess()->bililive_obs()->live_room_controller()->
			CloseLiveRoomPre();
    }
    else
    {
        StopLive();
    }
}

void OnStopStudioLiveStreaming(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    DCHECK(!LivehimeLiveRoomController::GetInstance()->IsLiveReady());

    StopLive();
}

void OnStopLiveStreamingEndDialog(const base::string16&, void* data)
{
    static_cast<CommandUpdaterDelegate*>(data)->ExecuteCommandWithParams(
        IDC_LIVEHIME_STOP_LIVE_STREAMING, EmptyCommandParams());
}

void OnLiveRoomLocked(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    ui::ResourceBundle& resource = ui::ResourceBundle::GetSharedInstance();

    gfx::NativeView main_widget = livehime->obs_view()->GetWidget()->GetNativeView();

    livehime::MessageBoxEndDialogSignalHandler handler;
    handler.closure = base::Bind(OnStopLiveStreamingEndDialog);
    handler.data = livehime->command_controller();

    SetFocus(main_widget);
    livehime::ShowMessageBox(
        main_widget,
        resource.GetLocalizedString(IDS_TIP_DLG_TIP),
        resource.GetLocalizedString(IDS_TIP_LIVEROOM_LOCK),
        resource.GetLocalizedString(IDS_SURE),
        &handler);
}

void OnEnterIntoThirdPartyStreamMode(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    // 正在开播的不应该进入第三方推流模式，正在录制的也不要进
    if (!LivehimeLiveRoomController::GetInstance()->IsLiveReady() ||
        !bililive::OutputController::GetInstance()->IsRecordingReady())
    {
        livehime::ShowMessageBox(livehime->obs_view()->GetWidget()->GetNativeWindow(),
            L"提示",
            L"进入第三方推流模式需要先停止当前的直播和录制。",
            L"我知道了");
        return;
    }

    // 第三方推流模式下场景功能一律禁用
    bililive::EnableSceneHotkey(false);
    LivehimeLiveRoomController::GetInstance()->EnterIntoThirdPartyStreamingMode();
    livehime::PolarisEventReport(secret::LivehimePolarisBehaviorEvent::SourceSettingShow,
        "material_type:" + std::to_string((size_t)secret::BehaviorEventMaterialType::ThirdPartyStreaming));
}

void OnLeaveThirdPartyStreamMode(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    LivehimeLiveRoomController::GetInstance()->LeaveThirdPartyStreamingMode();
    // 第三方推流模式下场景功能一律禁用
    bililive::EnableSceneHotkey(true);
}

void OnHotkeySwitchRecord(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    // 第三方推流模式下不能用直播姬的录制功能
    if (LivehimeLiveRoomController::IsInTheThirdPartyStreamingMode())
    {
        return;
    }

    if (bililive::OutputController::GetInstance()->IsRecording())
    {
        bililive::ExecuteCommand(livehime, IDC_LIVEHIME_STOP_RECORDING);

        HotkeyNotifyView::ShowForm(HotkeyNotifyView::HN_RecordStop,
            livehime->obs_view()->get_preview_rect());
    }
    else if (bililive::OutputController::GetInstance()->IsRecordingReady())
    {
        bool hotkey = true;
        bililive::ExecuteCommandWithParams(livehime, IDC_LIVEHIME_START_RECORDING,
            CommandParams<bool>(&hotkey));
    }
}

void OnHotkeySwitchLive(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    // 第三方推流模式下不能直接开播，要依赖第三方流的状态来自动决定开播状态，
    // 但可以断播
    if (LivehimeLiveRoomController::IsInTheThirdPartyStreamingMode())
    {
        return;
    }

    if (LivehimeLiveRoomController::GetInstance()->IsLiveReady())
    {
        LOG(INFO) << "hotkey start live.";
        bool hotkey = true;
        bililive::ExecuteCommandWithParams(livehime, IDC_LIVEHIME_START_LIVE_STREAMING,
            CommandParams<bool>(&hotkey));
    }
    else
    {
        LOG(INFO) << "hotkey stop live.";
        bililive::ExecuteCommand(livehime, IDC_LIVEHIME_STOP_LIVE_STREAMING);

        HotkeyNotifyView::ShowForm(HotkeyNotifyView::HN_LiveStop,
            livehime->obs_view()->get_preview_rect());
    }
}

void OnHotkeyClearDanmakuTTSQueue(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    HotkeyNotifyView::ShowForm(
        HotkeyNotifyView::HN_ClearDanmakuTTSQueue,
        livehime->obs_view()->get_preview_rect());
}
void OnHotkeyFastForward(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    std::string web_url = "";
    if (!web_url.empty())
    {
        LivehimeHybridWebBrowserView::ShowWindow(
            livehime->obs_view()->GetWidget(),
            web_url,
            true,
            L"",
            WebViewPopupType::System,
            cef_proxy::client_handler_type::bilibili,
            "");
    }
}
void OnVolumeMuteChanging(BililiveOBS* livehime, bool is_system_voice, bool invoke_by_click)
{
    obs_proxy::SceneCollectionImpl *scene_collection_impl
        = static_cast<obs_proxy::SceneCollectionImpl *>(obs_proxy::GetCoreProxy()->GetCurrentSceneCollection());
    if (scene_collection_impl)
    {
        std::vector<obs_proxy::VolumeController*> vc_v = scene_collection_impl->GetVolumeControllers();

        std::string source_name = is_system_voice ? obs_proxy::kDefaultOutputAudio : obs_proxy::kDefaultInputAudio;
        auto volume = std::find_if(vc_v.begin(), vc_v.end(),
                                   [source_name](obs_proxy::VolumeController* dev)->bool {
            return (dev->GetBoundSourceName() == source_name);
        });
        if (volume != vc_v.end())
        {
            (*volume)->SetMuted(!(*volume)->IsMuted());

            if (!invoke_by_click)
            {
                HotkeyNotifyView::HotkeyNotify mute_hn = is_system_voice ?
                    HotkeyNotifyView::HN_SystemVolumeMute : HotkeyNotifyView::HN_MicMute;
                HotkeyNotifyView::HotkeyNotify restore_hn = is_system_voice ?
                    HotkeyNotifyView::HN_SystemVolumeRestore : HotkeyNotifyView::HN_MicRestore;
                HotkeyNotifyView::ShowForm(
                    (*volume)->IsMuted() ? mute_hn : restore_hn,
                    livehime->obs_view()->get_preview_rect());
            }

            // 声音、麦克风点击&音量变更埋点
            livehime::BehaviorEventReport(secret::LivehimeBehaviorEventV2::ToolbarItemClick,
                base::StringPrintf("button_type:%d;result:%s",
                    is_system_voice ? 1 : 2,
                    (*volume)->IsMuted() ? "close" : "open"));
        }
    }
}

void OnHotkeyMicSilenceResume(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    bool invoke_by_click = false;
    if(params.map_key())
    {
        invoke_by_click = *CommandParams<bool>(params).ptr();
    }
    OnVolumeMuteChanging(livehime, false, invoke_by_click);
}

void OnHotkeySysVolSilenceResume(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    if (!g_sysvol_hotkey_enabled)
    {
        return;
    }

    bool invoke_by_click = false;
    if (params.map_key())
    {
        invoke_by_click = *CommandParams<bool>(params).ptr();
    }
    OnVolumeMuteChanging(livehime, true, invoke_by_click);
}

void OnHotkeySysVolEnabled(BililiveOBS* livehime, const CommandParamsDetails& params)
{
    g_sysvol_hotkey_enabled = *CommandParams<bool>(params).ptr();
}

void OnTipsDialogDomodalClose(int result, void* data)
{
    DCHECK(data);
    scoped_ptr<std::string> scene_item_name((std::string*)data);

    if (data && GetBililiveProcess()->bililive_obs()->obs_view())
    {
        GetBililiveProcess()->bililive_obs()->obs_view()->tabarea_view()->ShowOperateView(*scene_item_name, false);
    }

    PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
    if (pref)
    {
        pref->SetBoolean(prefs::kBililiveObsCameraTipsShow, false);
    }
}

void OnAddSourceEndDialog(int result, void* data)
{
    DCHECK(data);

    scoped_ptr<bililive::SceneItemCmdHandlerParam> modal_param((bililive::SceneItemCmdHandlerParam*)data);
    bool shouldRevertAddSourceAction = (result != livehime::ResultCode::RC_OK);

    std::string scene_name = modal_param->scene_name;

    //解决崩溃,modal_param->scene_item 有可能释放掉，导致崩溃，这里根据名字来查找
    std::string scene_item_name = modal_param->item_name;
    obs_proxy::SceneItemType scene_item_type = static_cast<obs_proxy::SceneItemType>(modal_param->item_type);
    if (scene_item_name.empty() || modal_param->item_type == -1)//为空时走原来的直接获取modal_param->scene_item逻辑
    {
		obs_proxy::SceneItem* add_scene_item = (obs_proxy::SceneItem*)modal_param->scene_item;
		scene_item_name = add_scene_item->name();
        scene_item_type = add_scene_item->type();
    }

    auto scene_collection = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    obs_proxy::Scene* scene = scene_collection->GetScene(scene_name);

    if (!shouldRevertAddSourceAction)
    {
        if (!scene)
        {
            LOG(INFO) << "OnAddSourceEndDialog:: GetScene return nullptr";
            return;
        }

        SceneItemCmd cmd(new SCENEITEMCMD_(UTF8ToUTF16(scene_name), UTF8ToUTF16(scene_item_name)));
        OBSProxyService::GetInstance().obs_ui_proxy()->
            ExecuteCommandWithParams(IDC_LIVEHIME_SCENE_ITEM_USER_ADDED, CommandParams<SceneItemCmd>(&cmd));
        OBSProxyService::GetInstance().obs_ui_proxy()->
            ExecuteCommandWithParams(IDC_LIVEHIME_SCENE_ITEM_SELECT, CommandParams<SceneItemCmd>(&cmd));

        obs_proxy::SceneItem *scene_item = scene->GetItem(scene_item_name);
        if (scene_item)
        {
            scene_item->SetVisible(true);
        }

        // 摄像头美颜功能引导 [
        if (scene_item && scene_item->type() == obs_proxy::SceneItemType::VideoCaptureDevice)
        {
            //livehime::face::FaceEffectPresenter::GetInstance()->OnCameraSourceAdding();

			PrefService* pref = GetBililiveProcess()->profile()->GetPrefs();
            //从预设素材添加得camera，不需要显示tips，如果显示了，当tips关闭得时候会把主界面得引导页也关闭了
            bool need_show_tips_dialog = pref->GetBoolean(prefs::kBililiveObsCameraTipsShow) && !modal_param->is_from_preset_material;
            if (need_show_tips_dialog)
            {
                TabAreaView* tabarea_view = GetBililiveProcess()->bililive_obs()->obs_view()->tabarea_view();
                // 海外这块可能是 nullptr
                if (tabarea_view != nullptr) 
                {
                    tabarea_view->ShowOperateView(scene_item_name, true);
                    if (tabarea_view->GetWidget() && tabarea_view->GetWidget()->IsVisible())//左侧工具栏会收起来
                    {
                        //gfx::Rect ref_rect = tabarea_view->GetCameraItemEditBtnBounds(scene_item_name);
                        //if (!ref_rect.IsEmpty())
                        //{
                        //    scoped_ptr<std::string> modal_param(new std::string(scene_item_name));
                        //    EndDialogSignalHandler handler;
                        //    handler.closure = base::Bind(&OnTipsDialogDomodalClose);
                        //    handler.data = modal_param.release();

                        //    gfx::Rect rect(0, 0, 240, 240);
                        //    TipsDialog::ShowWindow(GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget(),
                        //        rect, TipsDialog::TipsType::CameraTips, &ref_rect, &handler, 3000);
                        //}
                    }
                }
            }
        }
        // ]

        // 竖屏模式下添加的摄像头和投屏源初始默认状态为全屏幕适配
        if (scene_item && !LiveModelController::GetInstance()->IsLandscapeModel() &&
            (scene_item->type() == obs_proxy::SceneItemType::VideoCaptureDevice ||
                scene_item->type() == obs_proxy::SceneItemType::ReceiverSource))
        {
            scene_item->SetFitToScreen(true);
        }
    }
    else
    {
        if(scene)
        {
            scene->RemoveItem(scene_item_name);
        }
        else
        {
            LOG(INFO) << "OnAddSourceEndDialog:: GetScene return nullptr";
        }
    }

    bililive::EnableSceneHotkey(true);
}

void* OnAddSource(BililiveOBS* livehime, const CommandParamsDetails& params,bool preset_mode = false)//preset_mode是否从预设素材添加
{
    auto scene_collection = OBSProxyService::GetInstance().GetOBSCoreProxy()->GetCurrentSceneCollection();
    obs_proxy::Scene* scene;
    obs_proxy::SceneItem* source;
    int  projection_type = 0;
    const auto& creating_params = *CommandParams<CreatingSourceParams>(params).ptr();

    scene = scene_collection->current_scene();
    source = bililive::CreateSource(creating_params);

    auto extra_data = creating_params.extra_data;

    scoped_ptr<bililive::SceneItemCmdHandlerParam> modal_param(new bililive::SceneItemCmdHandlerParam());
    modal_param->cmd = IDC_LIVEHIME_ADD_SOURCE;
    modal_param->scene_name = scene->name();
    modal_param->scene_item = source;
    modal_param->item_type = static_cast<int>(source->type());
    modal_param->item_name = source->name();

	if (source->type() == obs_proxy::SceneItemType::ReceiverSource) {

        projection_type = static_cast<const bililive::ProjectionData*>(creating_params.extra_data)->projection_type;
	}


    if (extra_data && (extra_data->silence_mode))
    {
        // 通过DragDrop/capture的方式添加的源直接添加并选中，不用弹窗了
        OnAddSourceEndDialog(livehime::ResultCode::RC_OK, modal_param.release());
    }
    else
    {
        bililive::EnableSceneHotkey(false);

        if (source->type() == obs_proxy::SceneItemType::MediaSource ||
            source->type() == obs_proxy::SceneItemType::ReceiverSource ||
            source->type() == obs_proxy::SceneItemType::AudioInputCapture ||
            source->type() == obs_proxy::SceneItemType::AudioOutputCapture ||
            source->type() == obs_proxy::SceneItemType::BrowserSource ||
            source->type() == obs_proxy::SceneItemType::VideoCaptureDevice ||
            source->type() == obs_proxy::SceneItemType::WindowCapture)
        {
            source->SetVisible(true);
            if (source->type() == obs_proxy::SceneItemType::ReceiverSource ||
                source->type() == obs_proxy::SceneItemType::MediaSource ||
                source->type() == obs_proxy::SceneItemType::WindowCapture)
            {
                source->SetFitToScreen(true);
            }
        }
        else if (source->type() == obs_proxy::SceneItemType::DmkhimeSource)
        {
            source->AddNewFilter(obs_proxy::FilterType::Unpremultiply, "unpremultiply_filter");
        }

        if (preset_mode)
        {
            return modal_param.release();
        }
        views::Widget* parent_widget = livehime->obs_view()->GetWidget();
        EndDialogSignalHandler handler;
        handler.closure = base::Bind(&OnAddSourceEndDialog);
        handler.data_type = EndDialogSignalHandler::DataType::DataType_SceneItemCmdHandlerParam;
        handler.data = modal_param.release();
        livehime::OpenSourcePropertyWidget(source->type(), parent_widget,
                                            source,
                                            livehime::ViewType::VT_CREATESOURCE,
                                            &handler,projection_type);
    }
    return nullptr;
}

void OnAddAudioInputFromMenu(BililiveOBS* livehime, const CommandParamsDetails& details)
{
    CreatingSourceParams params(bililive::SourceType::AudioInput);
    bililive::ExecuteCommandWithParams(
        livehime,
        IDC_LIVEHIME_ADD_SOURCE,
        CommandParams<CreatingSourceParams>(&params));
}

void OnAddAudioOutputFromMenu(BililiveOBS* livehime, const CommandParamsDetails& details)
{
    CreatingSourceParams params(bililive::SourceType::AudioOutput);
    bililive::ExecuteCommandWithParams(
        livehime,
        IDC_LIVEHIME_ADD_SOURCE,
        CommandParams<CreatingSourceParams>(&params));
}

void OnAddAlbumSourceFromMenu(BililiveOBS* livehime, const CommandParamsDetails& details)
{
    CreatingSourceParams params(bililive::SourceType::Album);
    bililive::ExecuteCommandWithParams(
        livehime,
        IDC_LIVEHIME_ADD_SOURCE,
        CommandParams<CreatingSourceParams>(&params));
}

void OnAddImageSourceFromMenu(BililiveOBS* livehime, const CommandParamsDetails& details)
{
    CreatingSourceParams params(bililive::SourceType::Image);
    bililive::ExecuteCommandWithParams(
        livehime,
        IDC_LIVEHIME_ADD_SOURCE,
        CommandParams<CreatingSourceParams>(&params));
}

void OnAddColorSourceFromMenu(BililiveOBS* livehime, const CommandParamsDetails& details)
{
    CreatingSourceParams params(bililive::SourceType::Color);
    bililive::ExecuteCommandWithParams(
        livehime,
        IDC_LIVEHIME_ADD_SOURCE,
        CommandParams<CreatingSourceParams>(&params));
}

void OnShowBilibiliWebWindow(BililiveOBS* livehime, const CommandParamsDetails& details)
{
    const WebBrowserPopupDetails& wbpd = *CommandParams<WebBrowserPopupDetails>(details).ptr();
    if (!wbpd.web_url.empty())
    {
        auto parent_widget = wbpd.parent ? wbpd.parent : livehime->obs_view()->GetWidget();
        BililiveUniteBrowserView::ShowWindow(
            parent_widget,
            wbpd.web_url,
            wbpd.title,
            wbpd.allow_popup,
            cef_proxy::client_handler_type::bilibili,
            wbpd.post_data);
    }
}

void OnShowFeedbackWindow(BililiveOBS* livehime, const CommandParamsDetails& details)
{
    const WebBrowserPopupDetails& wbpd = *CommandParams<WebBrowserPopupDetails>(details).ptr();
    if (!wbpd.web_url.empty())
    {
        LivehimeFeedbackWebView::ShowWindow(livehime->obs_view()->GetWidget(),
            wbpd.web_url);
    }
}

void OnShowSysBalloon(BililiveOBS* livehime, const CommandParamsDetails& details)
{
    auto info = reinterpret_cast<std::tuple<std::wstring, std::wstring>*>(details.map_key());
    if (info)
    {
        livehime->obs_view()->DisplaySysBalloon(std::get<0>(*info), std::get<1>(*info));
    }
}

void OnSceneItemPropertyEndDialog(int result, void* data)
{
    bililive::EnableSceneHotkey(true);
}

// 星云浏览器插件打开插件设置窗口
void OnBrowserPluginSceneItemSet(BililiveOBS* livehime, const CommandParamsDetails& details)
{
    SceneItemCmd& cmd_params = *((SceneItemCmd*)details.map_key());
    std::string scene_name = UTF16ToUTF8(cmd_params.get()->scene_name());
    std::string item_name = UTF16ToUTF8(cmd_params.get()->item_name());

    auto collection = obs_proxy::GetCoreProxy()->GetCurrentSceneCollection();
    obs_proxy::Scene* scene = collection->GetScene(scene_name);
    DCHECK(scene);
    if (scene)
    {
        obs_proxy::SceneItem* scene_item = scene->GetItem(item_name);
        DCHECK(scene_item);
        if (scene_item)
        {
            // 任务活动进度页源不展示属性页和交互
            if (scene_item->type() == obs_proxy::SceneItemType::BrowserSource)
            {
                auto scene_item_helper_ = std::make_unique<BrowserItemHelper>(scene_item);
                if (scene_item_helper_->GetIsPlugin())
                {
                    if (!scene_item_helper_->GetPluginSetUrl().empty()) {
                        
                    }
                    return;
                }
            }
        }
    }
}

void OnSceneItemProperty(BililiveOBS* livehime, const CommandParamsDetails& details)
{
    SceneItemCmd &cmd_params = *((SceneItemCmd *)details.map_key());
    std::string scene_name = UTF16ToUTF8(cmd_params.get()->scene_name());
    std::string item_name = UTF16ToUTF8(cmd_params.get()->item_name());
    std::string extend_parameter = UTF16ToUTF8(cmd_params.get()->extend_parameter_);

    auto collection = obs_proxy::GetCoreProxy()->GetCurrentSceneCollection();
    obs_proxy::Scene *scene = collection->GetScene(scene_name);
    DCHECK(scene);
    if (scene)
    {
        obs_proxy::SceneItem *scene_item = scene->GetItem(item_name);
        DCHECK(scene_item);
        if (scene_item)
        {
            // 任务活动进度页源不展示属性页和交互
            if (scene_item->type() == obs_proxy::SceneItemType::BrowserSource)
            {
                auto scene_item_helper_ = std::make_unique<BrowserItemHelper>(scene_item);
                if (scene_item_helper_->activity_type() == BrowserItemHelper::ActivityType::MissionActivity)
                {
                    return;
                }
            }

            bililive::EnableSceneHotkey(false);

            EndDialogSignalHandler handler;
            handler.closure = base::Bind(&OnSceneItemPropertyEndDialog);

            if (scene_item->type() == obs_proxy::SceneItemType::TextureRendererSource) {
                TextureRendererItemHelper helper(scene_item);
            }

            livehime::OpenSourcePropertyWidget(scene_item->type(), livehime->obs_view()->GetWidget(),
                                               scene_item, livehime::ViewType::VT_SETPROPERTY,
                                               &handler);
        }
    }
}

void OnSceneItemInteraction(BililiveOBS* livehime, const CommandParamsDetails& details)
{
    SceneItemCmd &cmd_params = *((SceneItemCmd *)details.map_key());
    std::string scene_name = UTF16ToUTF8(cmd_params.get()->scene_name());
    std::string item_name = UTF16ToUTF8(cmd_params.get()->item_name());

    obs_proxy::Scene *scene = obs_proxy::GetCoreProxy()->GetCurrentSceneCollection()->GetScene(scene_name);
    DCHECK(scene);
    if (scene)
    {
        obs_proxy::SceneItem *scene_item = scene->GetItem(item_name);
        DCHECK(scene_item);
        if (scene_item)
        {
            // 是否需要禁用热键切换场景？
            //////////////////////////////////////////////////////////////////////////

            LivehimeBrowserSourceInteractionView::ShowForm(
                livehime->obs_view()->GetWidget()->GetNativeView(), scene_item);
        }
    }
}

void OnSceneItemDeleteMsgboxEndDialog(const base::string16& result, void* data)
{
    DCHECK(data);
    scoped_ptr<CmdHandlerParam> modal_data((CmdHandlerParam*)data);
    if (result == LocalStr(IDS_SURE))
    {
        std::string scene_name = UTF16ToUTF8(modal_data->scene_item_cmd->scene_name());
        std::string item_name = UTF16ToUTF8(modal_data->scene_item_cmd->item_name());
        obs_proxy::Scene *scene = obs_proxy::GetCoreProxy()->GetCurrentSceneCollection()->GetScene(scene_name);
        bool is_veido_scene = false;
        DCHECK(scene);
        if (scene)
        {
            obs_proxy::SceneItem *scene_item = scene->GetItem(item_name);
            DCHECK(scene_item);
            if (scene_item)
            {
                LOG(INFO) << "user delete scene item: " << scene_name << "->" << item_name;

                // 浏览器源在删除时要指定其属性为不可见时删除掉，否则CEF底层的browser-client不会被关闭，
                // CEF相关对象仍然存在，如果WEB上有声音的话还是能听到的
                if (scene_item->type() == obs_proxy::SceneItemType::BrowserSource)
                {
                    std::unique_ptr<contracts::SourceBrowserPropertyPresenter> presenter(
                        std::make_unique<SourceBrowserPropertyPresenterImpl>(scene_item));
                    presenter->Shutdown();
                    presenter->Update();
                }

                if (scene_item->type() == obs_proxy::SceneItemType::VideoCaptureDevice)
                {
                    is_veido_scene = true;
                }
            }
        }

        OBSProxyService::GetInstance().obs_ui_proxy()->ExecuteCommandWithParams(
            modal_data->cmd, CommandParams<SceneItemCmd>(&modal_data->scene_item_cmd));

        if (is_veido_scene) {
            
        }
    }

    bililive::EnableSceneHotkey(true);
}

void OnSceneItemVisible(const CommandParamsDetails& params)
{
    SceneItemCmd& cmd_params = *CommandParams<SceneItemCmd>(params).ptr();

    OBSProxyService::GetInstance().obs_ui_proxy()->ExecuteCommandWithParams(IDC_LIVEHIME_SCENE_ITEM_VISIBLE, params);
}

void OnSceneItemDelete(BililiveOBS* livehime, const CommandParamsDetails& details)
{
    SceneItemCmd &cmd_params = *CommandParams<SceneItemCmd>(details).ptr();

    bililive::EnableSceneHotkey(false);

    scoped_ptr<CmdHandlerParam> modal_data(new CmdHandlerParam(IDC_LIVEHIME_SCENE_ITEM_DELETE));
    modal_data->scene_item_cmd.reset(new SCENEITEMCMD_(*cmd_params.get()));
    if (cmd_params->no_popup)
    {
        OnSceneItemDeleteMsgboxEndDialog(LocalStr(IDS_SURE), modal_data.release());
    }
    else
    {
        livehime::MessageBoxEndDialogSignalHandler handler;
        handler.closure = base::Bind(&OnSceneItemDeleteMsgboxEndDialog);
        handler.data = modal_data.release();
        livehime::ShowMessageBox(livehime->obs_view()->GetWidget()->GetNativeWindow(),
            LocalStr(IDS_TIP_DLG_TIP),
            LocalStr(IDS_TIP_DLG_REMOVE_ITEM),
            LocalStr(IDS_SURE).append(L",").append(LocalStr(IDS_CANCEL)),
            &handler);
    }
}

void OnSceneAllItemDeleteMsgboxEndDialog(const base::string16& result, void* data)
{
    DCHECK(data);
    scoped_ptr<CmdHandlerParam> modal_data((CmdHandlerParam*)data);
    if (result == LocalStr(IDS_SURE))
    {
        std::string scene_name = UTF16ToUTF8(modal_data->scene_item_cmd->scene_name());
        std::string item_name = UTF16ToUTF8(modal_data->scene_item_cmd->item_name());
        obs_proxy::Scene *scene = obs_proxy::GetCoreProxy()->GetCurrentSceneCollection()->GetScene(scene_name);
        DCHECK(scene);
        if (scene)
        {
            std::vector<obs_proxy::SceneItem*> scene_list = scene->GetItems();
            for (auto it = scene_list.begin(); it != scene_list.end(); it++)
            {
                obs_proxy::SceneItem* scene_item = (obs_proxy::SceneItem*)(*it);
                DCHECK(scene_item);
                if (scene_item)
                {
                    LOG(INFO) << "user delete scene item: " << scene_name << "->" << item_name;

                    // 浏览器源在删除时要指定其属性为不可见时删除掉，否则CEF底层的browser-client不会被关闭，
                    // CEF相关对象仍然存在，如果WEB上有声音的话还是能听到的
                    if (scene_item->type() == obs_proxy::SceneItemType::BrowserSource)
                    {
                        std::unique_ptr<contracts::SourceBrowserPropertyPresenter> presenter(
                            std::make_unique<SourceBrowserPropertyPresenterImpl>(scene_item));
                        presenter->Shutdown();
                        presenter->Update();
                    }
                }
                SceneItemCmd cmd_params_del;
                cmd_params_del.reset(new SCENEITEMCMD_(modal_data->scene_item_cmd->scene_name(), UTF8ToUTF16(scene_item->name())));
                OBSProxyService::GetInstance().obs_ui_proxy()->ExecuteCommandWithParams(
                    modal_data->cmd, CommandParams<SceneItemCmd>(&cmd_params_del));
            }

        }
        else
        {
            OBSProxyService::GetInstance().obs_ui_proxy()->ExecuteCommandWithParams(
                modal_data->cmd, CommandParams<SceneItemCmd>(&modal_data->scene_item_cmd));
        }
    }

    bililive::EnableSceneHotkey(true);
}

void OnSceneItemAllDelete(BililiveOBS* livehime, const CommandParamsDetails& details)
{
    bililive::EnableSceneHotkey(false);

    SceneItemCmd &cmd_params = *CommandParams<SceneItemCmd>(details).ptr();

    scoped_ptr<CmdHandlerParam> modal_data(new CmdHandlerParam(IDC_LIVEHIME_SCENE_ITEM_DELETE));
    modal_data->scene_item_cmd.reset(new SCENEITEMCMD_(*cmd_params.get()));
    livehime::MessageBoxEndDialogSignalHandler handler;
    handler.closure = base::Bind(&OnSceneAllItemDeleteMsgboxEndDialog);
    handler.data = modal_data.release();
    livehime::ShowMessageBox(livehime->obs_view()->GetWidget()->GetNativeWindow(),
        LocalStr(IDS_TIP_DLG_TIP),
        LocalStr(IDS_CLEAR_ALL_ADDED_SOURCE),// L"确认清空该场景下的所有素材吗？",
        LocalStr(IDS_SURE).append(L",").append(LocalStr(IDS_CANCEL)),
        &handler);
}

void OnRemovePluginById(BililiveOBS* livehime, const CommandParamsDetails& details)
{
    bililive::EnableSceneHotkey(false);

    int64_t plugin_id = *CommandParams<int64_t>(details).ptr();
    if (plugin_id > 0) {
        auto proxy = OBSProxyService::GetInstance().GetOBSCoreProxy();
        if (!proxy) {
            return;
        }
        auto collection = proxy->GetCurrentSceneCollection();
        obs_proxy::Scene* scene = collection->current_scene();

        std::vector<obs_proxy::SceneItem*> scene_list = scene->GetItems();
        for (auto it = scene_list.begin(); it != scene_list.end(); it++)
        {
            obs_proxy::SceneItem* scene_item = (obs_proxy::SceneItem*)(*it);
            DCHECK(scene_item);
            if (scene_item)
            {
                // 浏览器源在删除时要指定其属性为不可见时删除掉，否则CEF底层的browser-client不会被关闭，
                // CEF相关对象仍然存在，如果WEB上有声音的话还是能听到的
                if (scene_item->type() == obs_proxy::SceneItemType::BrowserSource)
                {
                    auto scene_item_helper_ = std::make_unique<BrowserItemHelper>(scene_item);
                    if (scene_item_helper_->GetIsPlugin() && plugin_id == scene_item_helper_->GetPluginId())
                    {
                        std::unique_ptr<contracts::SourceBrowserPropertyPresenter> presenter(
                            std::make_unique<SourceBrowserPropertyPresenterImpl>(scene_item));
                        presenter->Shutdown();
                        presenter->Update();

                        SceneItemCmd cmd_params;
                        cmd_params.reset(new SCENEITEMCMD_(UTF8ToUTF16(scene->name()), UTF8ToUTF16(scene_item->name())));
                        OBSProxyService::GetInstance().obs_ui_proxy()->ExecuteCommandWithParams(
                            IDC_LIVEHIME_SCENE_ITEM_DELETE, CommandParams<SceneItemCmd>(&cmd_params));
                    }
                }
            }
        }
    }

    bililive::EnableSceneHotkey(true);
}

void OnLanguageSettingMessageBoxEndDialog(const base::string16& result, void* data) {
    if (result != LocalStr(IDS_SURE))
        return;
    
    GetBililiveProcess()->SetRestart();

    DoAppClose();
}

void OnLanguageSettingChanged(BililiveOBS* livehime, const CommandParamsDetails& details)
{
    livehime::MessageBoxEndDialogSignalHandler handler;
    handler.closure = base::Bind(&OnLanguageSettingMessageBoxEndDialog);
    livehime::ShowMessageBox(livehime->obs_view()->GetWidget()->GetNativeWindow(),
        LocalStr(IDS_LANGUAGE_CHANGED_RESTART_TIP_TITLE),
        LocalStr(IDS_LANGUAGE_CHANGED_RESTART_TIP_CONTENT),
        LocalStr(IDS_SURE).append(L",").append(LocalStr(IDS_CANCEL)),
        &handler);
}

}// namesapce

namespace bililive {

void EnableSceneHotkey(bool enable)
{
    g_scene_hotkey_enabled = enable;
}

bool IsSceneHotkeyEnabled()
{
    return g_scene_hotkey_enabled;
}

void ExecuteCommandWithParamsLivehime(BililiveOBS* receiver, int id, const CommandParamsDetails& params)
{
    bool command_handled = true;

    switch (id)
    {
    // 热键处理
    case IDC_LIVEHIME_HOTKEY_SENCE1:
    case IDC_LIVEHIME_HOTKEY_SENCE2:
    case IDC_LIVEHIME_HOTKEY_SENCE3:
    case IDC_LIVEHIME_HOTKEY_VERTICAL_SENCE1:
    case IDC_LIVEHIME_HOTKEY_VERTICAL_SENCE2:
    case IDC_LIVEHIME_HOTKEY_VERTICAL_SENCE3:
    {
        if (!IsSceneHotkeyEnabled())
        {
            return;
        }

        static std::map<int, std::string> scene_map = {
            { IDC_LIVEHIME_HOTKEY_SENCE1, prefs::kFirstDefaultSceneName },
            { IDC_LIVEHIME_HOTKEY_SENCE2, prefs::kSecondDefaultSceneName },
            { IDC_LIVEHIME_HOTKEY_SENCE3, prefs::kThirdDefaultSceneName },
            { IDC_LIVEHIME_HOTKEY_VERTICAL_SENCE1, prefs::kVerticalFirstDefaultSceneName },
            { IDC_LIVEHIME_HOTKEY_VERTICAL_SENCE2, prefs::kVerticalSecondDefaultSceneName },
            { IDC_LIVEHIME_HOTKEY_VERTICAL_SENCE3, prefs::kVerticalThirdDefaultSceneName },
        };

        if (!LiveModelController::GetInstance()->ChangeCurrentScene(scene_map[id]))
        {
            break;
        }

        static std::map<int, HotkeyNotifyView::HotkeyNotify> type_map = {
            { IDC_LIVEHIME_HOTKEY_SENCE1, HotkeyNotifyView::HN_SceneSwitch1 },
            { IDC_LIVEHIME_HOTKEY_SENCE2, HotkeyNotifyView::HN_SceneSwitch2 },
            { IDC_LIVEHIME_HOTKEY_SENCE3, HotkeyNotifyView::HN_SceneSwitch3 },
            { IDC_LIVEHIME_HOTKEY_VERTICAL_SENCE1, HotkeyNotifyView::HN_SceneSwitch4 },
            { IDC_LIVEHIME_HOTKEY_VERTICAL_SENCE2, HotkeyNotifyView::HN_SceneSwitch5 },
            { IDC_LIVEHIME_HOTKEY_VERTICAL_SENCE3, HotkeyNotifyView::HN_SceneSwitch6 },
        };
        HotkeyNotifyView::ShowForm(type_map[id], receiver->obs_view()->get_preview_rect());
    }
        break;
    case IDC_LIVEHIME_SCENE_ITEM_DELETE_FROM_PREVIEW:
    {
        // 在主界面预览区直接按DELETE键，预览区无法直接获知当前的选中项信息，
        // 需要将此命令传至右侧scene table，由其提取选中项信息后再执行常规item delete命令IDC_LIVEHIME_ITEM_DELCUR
        //receiver->obs_view()->ExecuteCmdInSceneTabPane(IDC_LIVEHIME_SCENE_ITEM_DELETE_FROM_PREVIEW);
        // 发通知，菜单选择了一条与sceneitem操作相关的命令
        base::NotificationService::current()->Notify(bililive::NOTIFICATION_LIVEHIME_SCENE_ITEM_CMD_EXECUTED,
                                                     base::NotificationService::AllSources(),
                                                     base::Details<int>(&id));
        break;
    }

    // scene item pop menu [
    case IDC_LIVEHIME_SCENE_ITEM_ACTUAL_SIZE:
    case IDC_LIVEHIME_SCENE_ITEM_FIT_TO_SCREEN:
    case IDC_LIVEHIME_SCENE_ITEM_TOTOP:
    case IDC_LIVEHIME_SCENE_ITEM_TOBOTTOM:
        // scene table operate [
    case IDC_LIVEHIME_SCENE_ITEM_MOVEUP:
    case IDC_LIVEHIME_SCENE_ITEM_MOVEDOWN:
    case IDC_LIVEHIME_SCENE_ITEM_SELECT:
    case IDC_LIVEHIME_SCENE_ITEM_DESELECT:
    //case IDC_LIVEHIME_SCENE_ITEM_VISIBLE:
    case IDC_LIVEHIME_SCENE_ITEM_LOCK:
    case IDC_LIVEHIME_SCENE_ITEM_REORDER:
        // ] scene table operate
    {
        OBSProxyService::GetInstance().obs_ui_proxy()->ExecuteCommandWithParams(id, params);
    }
    break;

    case IDC_LIVEHIME_SCENE_ITEM_VISIBLE:
    {
        OnSceneItemVisible(params);
    }
    break;

    case IDC_LIVEHIME_SCENE_ITEM_INTERACTION:
        OnSceneItemInteraction(receiver, params);
        break;
    case IDC_LIVEHIME_SCENE_ITEM_PROPERTY:
        OnSceneItemProperty(receiver, params);
        break;
    case IDC_LIVEHIME_SCENE_ITEM_DELETE:
    {
        OnSceneItemDelete(receiver, params);
    }
        break;
    case IDC_LIVEHIME_ALL_SCENE_ITEM_DELETE:
    {
        OnSceneItemAllDelete(receiver, params);
    }
    break;
    case IDC_LIVEHIME_BROWSER_SCENE_SET:
    {
        OnBrowserPluginSceneItemSet(receiver, params);
        break;
    }
    case IDC_LIVEHIME_SCENE_ITEM_SCALE_ZOOM:
    case IDC_LIVEHIME_SCENE_ITEM_SCALE_STRETCH:
    case IDC_LIVEHIME_SCENE_ITEM_SCALE_CUT:
    {
        static std::map<int, std::string> mode_map = {
            { IDC_LIVEHIME_SCENE_ITEM_SCALE_ZOOM, prefs::kSourceDragModeZoom },
            { IDC_LIVEHIME_SCENE_ITEM_SCALE_STRETCH, prefs::kSourceDragModeStretch },
            { IDC_LIVEHIME_SCENE_ITEM_SCALE_CUT, prefs::kSourceDragModeCut }
        };
        PrefService *pref = GetBililiveProcess()->profile()->GetPrefs();
        pref->SetString(prefs::kSourceDragMode, mode_map[id]);
    }
        break;
    // ] scene item pop menu

    default:
        command_handled = false;
        break;
    }

    // Quite a lot commands using the fall-through trick to share one handler routine, which burdens
    // our refactoring work. We need to clean the mess before proceeding and we also need more time.
    // Just leave them here right now.
    if (command_handled) {
        return;
    }

    BEGIN_BILILIVE_COMMAND_MAP(receiver, id, params)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_CLOSE_APP, OnCloseApp)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_CLOSE_APP_SILENTLY, OnCloseAppSilently)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_LOGOUT, OnLogout)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_TOKEN_INVALID, OnTokenInvalid)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_MINIMIZE_WINDOW, OnMinimizeWindow)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_MAXIMIZE_WINDOW, OnMaximizeWindow)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_RESTORE_WINDOW, OnRestoreWindow)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_ACTIVE_MAIN_WINDOW, OnActiveMainWindow)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_SHOW_SOUND_EFFECT, OnShowSoundEffect)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_SHOW_NOVICE_GUIDE, OnShowNoviceGuide)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_CHECK_FOR_UPDATE, OnCheckForUpdate)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_SETUP, OnSettings)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_SHARE, OnShare)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_SHOW_GIFTBOX, OnShowGiftbox)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_SHOW_LOG_FILE, OnShowLogFile)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_START_RECORDING, OnStartRecording)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_STOP_RECORDING, OnStopRecording)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_START_LIVE_STREAMING, OnStartLiveStreaming)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_STOP_LIVE_STREAMING, OnStopLiveStreaming)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_STOP_STUDIO_LIVE_STREAMING, OnStopStudioLiveStreaming)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_LIVE_ROOM_LOCKED, OnLiveRoomLocked)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_ENTER_INTO_THIRD_PARTY_STREAM_MODE, OnEnterIntoThirdPartyStreamMode)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_LEAVE_THIRD_PARTY_STREAM_MODE, OnLeaveThirdPartyStreamMode)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_HOTKEY_SYSVOL_ENABLE, OnHotkeySysVolEnabled)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_HOTKEY_MIC_SILENCE_RESUME, OnHotkeyMicSilenceResume)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_HOTKEY_SYSVOL_SILENCE_RESUME, OnHotkeySysVolSilenceResume)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_HOTKEY_SWITCH_RECORD, OnHotkeySwitchRecord)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_HOTKEY_SWITCH_LIVE, OnHotkeySwitchLive)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_HOTKEY_CLEAR_DANMAKU_TTS_QUEUE, OnHotkeyClearDanmakuTTSQueue)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_HOTKEY_FAST_FORWARD, OnHotkeyFastForward)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_ADD_SOURCE, OnAddSource)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_ADD_ALBUM_SOURCE_FROM_MENU, OnAddAlbumSourceFromMenu)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_ADD_IMAGE_SOURCE_FROM_MENU, OnAddImageSourceFromMenu)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_ADD_COLOR_SOURCE_FROM_MENU, OnAddColorSourceFromMenu)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_ADD_AUDIO_INPUT_FROM_MENU, OnAddAudioInputFromMenu)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_ADD_AUDIO_OUTPUT_FROM_MENU, OnAddAudioOutputFromMenu)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_SHOW_BILIBILI_COMMON_WEB_WINDOW, OnShowBilibiliWebWindow)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_SHOW_FEEDBACK_WEB_WINDOW, OnShowFeedbackWindow)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_SHOW_ANCHOR_NOTICE, OnShowAnchorNotice)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_SHOW_SYS_BALLOON, OnShowSysBalloon)
		ON_BILILIVE_COMMAND(IDC_LIVEHIME_PLUGIN_ITEM_DELETE, OnRemovePluginById)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_OPEN_USER_CARD, OnShowUserCard)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_SCHEME_CMD, OnSchemeCmd)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_CREATE_CHAT_USERCARD, OnChatShowUserCard)
        ON_BILILIVE_COMMAND(IDC_LIVEHIME_LANGUAGE_SETTING_CHANGED, OnLanguageSettingChanged)
        ON_BILILIVE_COMMAND_UNHANDLED_ERROR()
    END_BILILIVE_COMMAND_MAP()
}

void* PresetMaterialAddSource(const CommandParamsDetails& params)
{
    return OnAddSource(nullptr, params, true);
}

void PresetMaterialAddSourceEnd(int result, void* data)
{
    bililive::SceneItemCmdHandlerParam* param = static_cast<bililive::SceneItemCmdHandlerParam*>(data);
    if (param)
    {
        param->is_from_preset_material = true;
    }
    OnAddSourceEndDialog(result, data);
}

}   // namespace bililive
