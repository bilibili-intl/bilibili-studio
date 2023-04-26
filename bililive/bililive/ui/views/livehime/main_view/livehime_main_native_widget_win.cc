#include "livehime_main_native_widget_win.h"

#include "base/notification/notification_service.h"
#include "base/prefs/pref_service.h"

#include "ui/base/resource/resource_bundle.h"
#include "ui/base/win/shell.h"
#include "ui/views/controls/messagebox/message_box.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
//#include "bililive/bililive/livehime/chat_room/live_chat_room_controller.h"
#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/public/common/pref_names.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_command_ids.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/bililive/ui/bililive_commands.h"
#include "bililive/bililive/ui/bililive_obs.h"

#include "bililive/public/log_ext/log_constants.h"


BilibiliLivehimeNativeWidgetWin::BilibiliLivehimeNativeWidgetWin(views::internal::NativeWidgetDelegate* delegate)
    : views::NativeWidgetWin(delegate)
    , weakptr_factory_(this)
    , delegate_(delegate)
{
}

BilibiliLivehimeNativeWidgetWin::~BilibiliLivehimeNativeWidgetWin()
{
}

void BilibiliLivehimeNativeWidgetWin::OnMiniMainWidget(const string16& btn, void* data)
{
    if (btn == L"最小化") {
        mini_dlg_sure_ = true;
        bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_MINIMIZE_WINDOW);
    }
}

void BilibiliLivehimeNativeWidgetWin::MiniMainWidget()
{
    //if (livehime::chat::LiveChatRoomController::GetInstance()->IsIdleStatus() ||
    //    !AppFunctionController::GetInstance()->get_chat_mini_switch()) {
    //}

    ui::ResourceBundle& rs = ui::ResourceBundle::GetSharedInstance();

    base::string16 btns = L"取消,最小化";
    base::string16 def_btn = L"最小化";
    livehime::MessageBoxEndDialogSignalHandler handler;
    handler.closure = base::Bind(&BilibiliLivehimeNativeWidgetWin::OnMiniMainWidget, weakptr_factory_.GetWeakPtr());
    livehime::ShowMessageBox(GetWidget()->GetNativeView(),
        rs.GetLocalizedString(IDS_TIP_DLG_TIP), L"语聊房期间,窗口最小化会影响画面渲染，不建议窗口最小化！",
        btns, &handler, livehime::MessageBoxType_NONEICON,
        def_btn, false, livehime::UIPropType::Unknown,
        GetLengthByDPIScale(8));
}

bool BilibiliLivehimeNativeWidgetWin::PreHandleMSG(UINT message,
    WPARAM w_param,
    LPARAM l_param,
    LRESULT *result)
{
    switch (message)
    {
    case WM_ACTIVATE:
    {
        PrefService *pref = GetBililiveProcess()->global_profile()->GetPrefs();
        if (pref)
        {
            BOOL bDragFullWindows = pref->GetBoolean(prefs::kDragFullWindows) ? TRUE : FALSE;

            static BOOL drag_full_windows = TRUE;
            if (w_param)
            {
                if (::SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, NULL, &drag_full_windows, NULL))
                {
                    if (drag_full_windows != bDragFullWindows)
                    {
                        SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, bDragFullWindows, NULL, 0);
                    }
                }
            }
            else
            {
                SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, drag_full_windows, NULL, 0);
            }
        }

        base::NotificationService *service = base::NotificationService::current();
        if (service)
        {
            MSG msg = { 0 };
            msg.hwnd = 0;
            msg.message = message;
            msg.wParam = w_param;
            msg.lParam = l_param;
            service->Notify(bililive::NOTIFICATION_BILILIVE_WIDGET_MESSAGE, base::NotificationService::AllSources(), base::Details<MSG>(&msg));
        }
    }
        return false;
    case WM_PRINT:
        return false;
    case WM_ENDSESSION:
        return true;
    case WM_INITMENUPOPUP:
        return true;

    case WM_MOUSEMOVE:
        return false;

    case WM_KEYDOWN:
    case WM_KEYUP:
        return false;

    case WM_SYSCOMMAND:
        {
            if (w_param == SC_CLOSE)
            {
                if (::IsWindowEnabled(GetWidget()->GetNativeView()))
                {
                    bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), IDC_LIVEHIME_CLOSE_APP);
                }
                return true;
            }

            static bool last_min = false;
            if (w_param == SC_MINIMIZE)
            {
                if (/*livehime::chat::LiveChatRoomController::GetInstance()->IsIdleStatus() ||*/
                    !AppFunctionController::GetInstance()->get_chat_mini_switch()) {
                    if (!last_min)
                    {
                        last_min = true;
                        LOG(INFO) << app_log::kLogEnterBackground << "main window minimize";
                    }
                    return false;
                }
                else {
                    if (mini_dlg_sure_) {
                        if (!last_min)
                        {
                            last_min = true;
                            LOG(INFO) << app_log::kLogEnterBackground << "main window minimize";
                        }
                        mini_dlg_sure_ = false;
                        return false;
                    }

                    MiniMainWidget();
                }
                return true;
            }
            else if (w_param == SC_RESTORE)
            {
                if (last_min)
                {
                    last_min = false;
                    LOG(INFO) << app_log::kLogEnterForeground << "main window restore";
                }
            }
        }
        return false;


    case WM_DISPLAYCHANGE:
        views::InsureWidgetVisible(GetWidget(), views::InsureShowCorner::ISC_CENTER);
        break;

    case WM_WINDOWPOSCHANGING:
        {
            static bool last_vis = false;
            WINDOWPOS* pos = (WINDOWPOS*)l_param;
            if (pos->flags & SWP_HIDEWINDOW)
            {
                if (last_vis)
                {
                    last_vis = false;
                    LOG(INFO) << app_log::kLogEnterBackground << "main window hide";
                }
            }
            else if (pos->flags & SWP_SHOWWINDOW)
            {
                if (!last_vis)
                {
                    last_vis = true;
                    LOG(INFO) << app_log::kLogEnterForeground << "main window show";
                }
            }
        }
        break;
    }
    return false;
}

void BilibiliLivehimeNativeWidgetWin::PostHandleMSG(UINT message,
    WPARAM w_param,
    LPARAM l_param)
{
    static BOOL fWinArrange;
    static BOOL fSnapSizing;
    switch (message)
    {
    case WM_CREATE:
        break;

    case WM_HOTKEY:
    {
        switch (w_param)
        {
        case IDC_LIVEHIME_HOTKEY_MIC_SILENCE_RESUME:
        case IDC_LIVEHIME_HOTKEY_SYSVOL_SILENCE_RESUME:
        case IDC_LIVEHIME_HOTKEY_SENCE1:
        case IDC_LIVEHIME_HOTKEY_SENCE2:
        case IDC_LIVEHIME_HOTKEY_SENCE3:
        case IDC_LIVEHIME_HOTKEY_SWITCH_LIVE:
        case IDC_LIVEHIME_HOTKEY_SWITCH_RECORD:
        case IDC_LIVEHIME_HOTKEY_CLEAR_DANMAKU_TTS_QUEUE:
        case IDC_LIVEHIME_HOTKEY_FAST_FORWARD:
            bililive::ExecuteCommand(GetBililiveProcess()->bililive_obs(), w_param);
            break;
        default:
            break;
        }
    }
    break;

    default:
        break;
    }
}

bool BilibiliLivehimeNativeWidgetWin::GetClientAreaInsets(gfx::Insets *insets) const
{
    // 这里指定非客户区大小。
    // 返回 false 表示由底层指定大小。如果返回 false，insets 必须指定全0，否则底层会 DCHECK。
    // POPUP 窗口需要返回 false 以防止一些问题（比如打开窗口时闪现一个预料之外的颜色）
    if (IsUsingCustomFrame()) {
        return false;
    } else {
        // -1 是防止窗口在调整大小时闪烁，客户区会陷进窗口底部 1 像素
        insets->Set(0, 0, -1, 0);
        return true;
    }
}

void BilibiliLivehimeNativeWidgetWin::HandleFrameChanged() {

}