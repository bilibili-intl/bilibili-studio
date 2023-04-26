#include "cef_proxy_ipc_handler.h"
#include "cef_hybrid_cache.h"

#include <shellapi.h>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/message_loop/message_loop.h"
#include "base/prefs/pref_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/string_number_conversions.h"
#include "base/sys_info.h"

#include "ui/base/win/dpi.h"
#include "ui/base/win/hwnd_util.h"
#include "ui/views/controls/native/native_view_host.h"
#include "ui/views/widget/widget.h"
#include "ui/gfx/canvas.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/livehime/cef/cef_hybrid_cache.h"
#include "bililive/bililive/livehime/cef/cef_hybrid_browser_delegate.h"
#include "bililive/bililive/livehime/event_report/event_report_center.h"
#include "bililive/bililive/livehime/function_control/app_function_controller.h"
#include "bililive/bililive/livehime/sub_broadcast/sub_fifter_broadcast.h"
#include "bililive/bililive/ui/views/hotkey_notify/bililive_toast_notify_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_message_box.h"
#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_widget.h"
#include "bililive/bililive/ui/views/livehime/web_browser/livehime_unite_browser_widget.h"
#include "bililive/bililive/utils/net_util.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/public/event_tracking_service.h"
#include "bililive/secret/services/service_utils.h"
#include "bililive/common/bililive_context.h"
#include "bililive/bililive/livehime/live_room/live_controller.h"

#include "cef/bililive_browser/public/bililive_browser_ipc_messages.h"
#include "cef/bililive_browser/public/bililive_browser_switches.h"
#include "base/ext/callable_callback.h"
#include "bililive/bililive/utils/fast_forward_url_convert.h"

#define N  9  // 精度为小数点后1位

namespace
{
    void CefJsBridgePolarisEventReport(secret::LivehimePolarisBehaviorEvent type, const std::string& method, const std::string& page_url, const std::string& data)
    {
        LOG(INFO) << "[jsbridge] PolarisEventReport method=" << method << ", url=" << page_url << ", data=" << data;

        // 通过KV获取jsbridge采样率
        double jsbridge_report_sampling_rat =
            AppFunctionController::GetInstance()->get_jsbridge_report_sampling_rat();
        int seed = static_cast<int>(base::Time::Now().ToInternalValue());
        srand(seed);
        double random = rand() % (N + 1) / (double)(N + 1);
        bool need_report = random > (1.0 - jsbridge_report_sampling_rat);
        if (need_report) {
            std::string url = page_url;
            size_t suffix_pos = url.find('?');
            if (suffix_pos != std::string::npos) {
                url = url.substr(0, suffix_pos);
            }

            base::StringPairs event_ext;
            event_ext.push_back(std::pair<std::string, std::string>("method", method));
            event_ext.push_back(std::pair<std::string, std::string>("url", url));
            event_ext.push_back(std::pair<std::string, std::string>("data", data));
            livehime::PolarisEventReport(type, event_ext);
        }
    }

    std::string GetCallDataString(const cef_proxy::calldata* data, std::string key)
    {
        std::string retStr;
        if (data)
        {
            std::map<std::string, cef_proxy::calldata_filed>::const_iterator it = data->find(key);
            if (it != data->end())
            {
                retStr = it->second.str_;
            }
        }

        return retStr;
    }

    std::string GetUnixTimestampMilli()
    {
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        return std::to_string(timestamp);
    }
}


void CefProxyMsgHandle(LivehimeWebBrowserDelegate* delegate, CefProxyMsgHandleExtraDetails* details,
    const cef_proxy::browser_bind_data& bind_data,
    const std::string& msg_name, const std::string& page_url, const cef_proxy::calldata* data)
{
    // 在这里要判断一下这个特定业务模块的通知是不是此webview的
    //if (bind_data_ && (*bind_data_ == bind_data))
    {
        static std::string webview_load_time;
        std::string method_origin_name;
        std::string method_origin_data;
        bool should_dispatch_to_delegate = false;

        // 页面加载状态和JS回调通知分开
        if (msg_name == ipc_messages::kLivehimeCefOnLoadStart)
        {
            if (delegate)
            {
                delegate->OnWebBrowserLoadStart(data->at("url").str_);
                webview_load_time = GetUnixTimestampMilli();
            }
        }
        else if (msg_name == ipc_messages::kLivehimeCefOnLoadEnd)
        {
            if (delegate)
            {
                delegate->OnWebBrowserLoadEnd(data->at("url").str_,
                    data->at("http_status_code").numeric_union.int_);
            }
        }
        else if (msg_name == ipc_messages::kLivehimeCefOnLoadError)
        {
            if (delegate)
            {
                delegate->OnWebBrowserLoadError(data->at("url").str_,
                    data->at("error_code").numeric_union.int_,
                    data->at("error_text").str_);
            }
        }
        else if (msg_name == ipc_messages::kLivehimeCefOnWindowClose)
        {
            if (details && details->widget)
            {
                details->widget->Close();
            }
        }
        else if (msg_name == ipc_messages::kLivehimeCefOnPopup)
        {
            std::string target_url(data->at("target_url").str_);
            int target_disposition = data->at("target_disposition").numeric_union.int_;
            WebViewPopupType handle = WebViewPopupType::NotAllow;
            if (delegate)
            {
                handle = delegate->OnWebBrowserPopup(target_url, target_disposition);
            }

            switch (handle)
            {
            case WebViewPopupType::Native:
                // 目前公共H5窗口暂时最多只允许二级弹窗了，alllow_popup参数就不要递归传递了
                if (details && details->widget)
                {
                    if (target_url.find("pc_ui") == target_url.npos) {
                        LivehimeHybridWebBrowserView::ShowWindow(details->widget, target_url, true, L"", WebViewPopupType::NotAllow,
                            cef_proxy::client_handler_type::bilibili, "", nullptr, true);
                    }
                    else {
                        BililiveUniteBrowserView::ShowWindow(details->widget, target_url, L"", WebViewPopupType::NotAllow,
                        cef_proxy::client_handler_type::bilibili, "", nullptr, true);
                    }
                }
                break;
            case WebViewPopupType::System:
                ShellExecuteA(nullptr, "open", bililive::FastForwardChangeEnv(target_url).c_str(), nullptr, nullptr, SW_SHOW);
                break;
            default:
                break;
            }
        }
        else if (msg_name == ipc_messages::kLivehimeCefOnPreKeyEvent)
        {
            cef_proxy::calldata_filed data_filed = data->at("msg");
            if (data_filed.type == cef_proxy::calldata_type::CALL_DATA_TYPE_KEY_EVENT)
            {
                bool handle = false;
                if (delegate)
                {
                    handle = delegate->OnWebBrowserPreKeyEvent(data_filed.key_event_);
                }

                if (!handle)
                {
                    if (details && details->OnCefKeyEvent)
                    {
                        details->OnCefKeyEvent(true, data_filed.key_event_);
                    }
                }
            }
        }
        else if (msg_name == ipc_messages::kLivehimeCefOnKeyEvent)
        {
            cef_proxy::calldata_filed data_filed = data->at("msg");
            if (data_filed.type == cef_proxy::calldata_type::CALL_DATA_TYPE_KEY_EVENT)
            {
                bool handle = false;
                if (delegate)
                {
                    handle = delegate->OnWebBrowserKeyEvent(data_filed.key_event_);
                }

                if (!handle)
                {
                    if (details && details->OnCefKeyEvent)
                    {
                        details->OnCefKeyEvent(false, data_filed.key_event_);
                    }
                }
            }
        }
        else if (msg_name == ipc_messages::kLivehimeCefOnCertificateError)
        {
            // 判断一下，debug模式下或是带有h5 devtool命令行参数的才进行SSL错误提示
            bool need_notify = false;
#ifdef _DEBUG
            need_notify = true;
#endif

            CommandLine* cmd = CommandLine::ForCurrentProcess();
            if (cmd->HasSwitch(switches::kBililiveBrowserEnableDevTools))
            {
                need_notify = true;
            }

            if (need_notify)
            {
                if (details && details->widget)
                {
                    std::string url = data->at("url").str_;
                    string16 msg = L"检测到页面发生SSL错误，请检查一下本机是否安装了证书。\n";
                    msg.append(UTF8ToUTF16(url));

                    livehime::ShowMessageBox(details->widget->GetNativeWindow(),
                        GetLocalizedString(IDS_TIP_DLG_TIP),
                        msg,
                        GetLocalizedString(IDS_IKNOW));
                }
            }
        }
        else if (msg_name == ipc_messages::kLivehimeCefOnLoadTitle)
        {
            if (details && details->widget)
            {
                delegate->OnWebBrowserLoadTitle(data->at("title").wstr_);
            }
        }
        else if (msg_name == ipc_messages::kBilibiliPageFinished)
        {
            method_origin_name = GetCallDataString(data, "method_origin_name");

            if (details && details->NativeControlIgnoreViewVisibilityChanged)
            {
                details->NativeControlIgnoreViewVisibilityChanged(false, true);
            }
        }
        else if (msg_name == ipc_messages::kBilibiliPageReport)
        {
            std::string event_id = data->at("id").str_;
            std::string event_msg = data->at("msg").str_;
            method_origin_name = GetCallDataString(data, "method_origin_name");

            livehime::BehaviorEventReport(event_id, event_msg);
        }
        else if (msg_name == ipc_messages::kBilibiliPageTecReport)
        {
            std::string event_id = data->at("id").str_;
            std::string event_msg = data->at("msg").str_;
            method_origin_name = GetCallDataString(data, "method_origin_name");

            auto secret_core = GetBililiveProcess()->secret_core();
            secret_core->event_tracking_service()->ReportLivehimeBehaviorEvent(
                event_id, event_msg).Call();
        }
        else if (msg_name == ipc_messages::kBilibiliPagePolarisReport)
        {
            std::string event_id = data->at("id").str_;
            std::string event_msg = data->at("msg").str_;
            method_origin_name = GetCallDataString(data, "method_origin_name");

            livehime::PolarisEventReport(event_id, event_msg);
        }
        else if (msg_name == ipc_messages::kBilibiliPageArouse)
        {
            std::string params = data->at("params").str_;
            std::string moduleName = data->at("moduleName").str_;
            bool closeCurrent = data->at("closeCurrent").numeric_union.bool_;
            method_origin_name = GetCallDataString(data, "method_origin_name");

            should_dispatch_to_delegate = true;
            
        }
        else if (msg_name == ipc_messages::kBilibiliPageProjection)
        {
            std::string mission_id = data->at("id").str_;
            std::string hybrid_url = data->at("url").str_;
            method_origin_name = GetCallDataString(data, "method_origin_name");
        }
        else if (msg_name == ipc_messages::kBilibiliPageGetInfo)
        {
            method_origin_name = GetCallDataString(data, "method_origin_name");
            std::string type;
            auto itr = data->find("type");
            if (itr != data->end()) {
                type = itr->second.str_;
            }
            //std::string params = data->at("params").str_;

            if (type == "report_via_srv")   // 开播数据上报
            {
                std::vector<std::string> vct;
                base::StringPairs sp = livehime::CommonFieldsForERVS();
                for (auto& iter : sp)
                {
                    vct.push_back(iter.first + "=" + iter.second);
                }
                std::string info = JoinString(vct, "&");

                // 直接复用形参data，h5传来什么上层就回传什么
                cef_proxy::calldata_filed data_filed;
                data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
                data_filed.str_ = info;
                const_cast<cef_proxy::calldata*>(data)->insert({ "info", data_filed });

                CefProxyWrapper::GetInstance()->DispatchJsEvent(bind_data,
                    ipc_messages::kBilibiliPageGetInfo, data);
            }
            else if(type == "get_pc_system_info")
            {
                std::vector<std::string> vct;
                auto build_num = BililiveContext::Current()->GetExecutableBuildNumber();
                vct.push_back(base::StringPrintf("build=%d", build_num));
                auto is_streaming = LivehimeLiveRoomController::GetInstance()->IsStreaming();
                vct.push_back(base::StringPrintf("live_state=%d", is_streaming? 1:0));
                std::string info = JoinString(vct, "&");
                // 直接复用形参data，h5传来什么上层就回传什么
                cef_proxy::calldata_filed data_filed;
                data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
                data_filed.str_ = info;
                const_cast<cef_proxy::calldata*>(data)->insert({ "info", data_filed });

                CefProxyWrapper::GetInstance()->DispatchJsEvent(bind_data,
                    ipc_messages::kBilibiliPageGetInfo, data);

            }
            else if (type == "check_talk_subject")
            {
                std::string params = data->at("params").str_;

                if (delegate)
                {
                    delegate->OnWebJionTalkTopic(bind_data, params, data);
                }
            }
            else if (  //互动游戏中心 监听事件
                type == "get_game_list" ||
                type == "game_download" ||
                type == "game_delete" ||
                type == "game_open" ||
                type == "game_close" ||
                type == "weblink_open_game") {
                if (delegate) {
                    delegate->OnInteractivePlayCenterEvent(bind_data,data);
                }
            }
        }
        else if (msg_name == ipc_messages::kBilibiliPageSubBroadcast)
        {
            std::string type = data->at("type").str_;
            std::string cmd = data->at("cmd").str_;
            method_origin_name = GetCallDataString(data, "method_origin_name");

            SubFifterBroadcastController::GetInstance()->SubBroadCastCmd(bind_data, data,type, cmd);
        }
        else if (msg_name == ipc_messages::kBilibiliPageUnSubBroadcast)
        {
            std::string type = data->at("type").str_;
            bool closeCurrent = data->at("closeCurrent").numeric_union.bool_;
            method_origin_name = GetCallDataString(data, "method_origin_name");

            SubFifterBroadcastController::GetInstance()->UnSubBroadCastCmd(type);

            if (closeCurrent)
            {
                if (details && details->widget)
                {
                    details->widget->Close();
                }
            }
        }
        else if (msg_name == ipc_messages::kBilibiliPageAlert)
        {
            std::string noRemindKey = data->at("noRemindKey").str_;
            int callbackId = data->at("callbackId").numeric_union.int_;
            int successCallbackId = data->at("successCallbackId").numeric_union.int_;
            method_origin_name = GetCallDataString(data, "method_origin_name");

            cef_proxy::calldata calldata;
            cef_proxy::calldata_filed data_filed;
            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
            data_filed.numeric_union.int_ = (int)(cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK_DATA);
            const_cast<cef_proxy::calldata&>(calldata).insert({ cef_proxy::kCallbackTypeField, data_filed });

            std::wstring msg = L"";
            bool no_remind = false;
            if (!noRemindKey.empty()) {
                base::string16 value = CefHybridCache::GetInstance()->GetHybridCacheByKey(noRemindKey);
                if (value == L"1") {
                    no_remind = true;
                }
            }

            data_filed.reset();
            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
            data_filed.numeric_union.int_ = no_remind;
            const_cast<cef_proxy::calldata&>(calldata).insert({ "hidden", data_filed });

            data_filed.reset();
            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
            data_filed.numeric_union.int_ = callbackId;
            const_cast<cef_proxy::calldata&>(calldata).insert({ "callbackId", data_filed });

            CefProxyWrapper::GetInstance()->DispatchJsEvent(bind_data,
                ipc_messages::kBilibiliPageAlert, &calldata);

            if (!no_remind) {
                if (details && details->widget) {
                    delegate->OnWebBrowserGeneralEvent(bind_data, ipc_messages::kBilibiliPageAlert, data);
                }
            }
        }
        else if (msg_name == ipc_messages::kBilibiliPageToast)
        {
            std::wstring msg = data->at("msg").wstr_;
            std::string type = data->at("type").str_;
            int callbackId = data->at("callbackId").numeric_union.int_;
            method_origin_name = GetCallDataString(data, "method_origin_name");

            cef_proxy::calldata calldata;
            cef_proxy::calldata_filed data_filed;
            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
            data_filed.numeric_union.int_ = (int)(cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK);
            const_cast<cef_proxy::calldata&>(calldata).insert({ cef_proxy::kCallbackTypeField, data_filed });

            data_filed.reset();
            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;

            data_filed.numeric_union.int_ = callbackId;
            const_cast<cef_proxy::calldata&>(calldata).insert({ "callbackId", data_filed });

            // 固定应答回调，不需要带参数
            CefProxyWrapper::GetInstance()->DispatchJsEvent(bind_data,
                ipc_messages::kBilibiliPageToast, &calldata);

            if (details && details->widget) {
                gfx::Rect hnv_s = details->widget->GetWindowBoundsInScreen();

                ToastNotifyView::ShowForm(msg,
                    hnv_s, nullptr, &ftPrimary,type == "long"? 7:5);
            }
        }
        else if (msg_name == ipc_messages::kBilibiliPageTitleBar)
        {
            int callbackId = data->at("callbackId").numeric_union.int_;
            int successCallbackId = data->at("successCallbackId").numeric_union.int_;
            method_origin_name = GetCallDataString(data, "method_origin_name");

            if (delegate) {
                cef_proxy::calldata calldata;
                cef_proxy::calldata_filed data_filed;
                data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
                data_filed.numeric_union.int_ = (int)(cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK);
                const_cast<cef_proxy::calldata&>(calldata).insert({ cef_proxy::kCallbackTypeField, data_filed });

                data_filed.reset();
                data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
                data_filed.numeric_union.int_ = callbackId;
                const_cast<cef_proxy::calldata&>(calldata).insert({ "callbackId", data_filed });

                // 固定应答回调，需要带参数
                CefProxyWrapper::GetInstance()->DispatchJsEvent(bind_data,
                    ipc_messages::kBilibiliPageTitleBar, &calldata);

                delegate->OnWebBrowserGeneralEvent(bind_data, ipc_messages::kBilibiliPageTitleBar, data);
                return;
            }
        }
        else if (msg_name == ipc_messages::kBilibiliPageGetContainerInfo)
        {
            int callbackId = data->at("callbackId").numeric_union.int_;
            method_origin_name = GetCallDataString(data, "method_origin_name");

            auto app_version = base::WideToUTF8(BililiveContext::Current()->GetExecutableVersion());
            auto app_uild = std::to_string(BililiveContext::Current()->GetExecutableBuildNumber());
            auto os_version = base::SysInfo::OperatingSystemVersion();
            auto os_arch = base::SysInfo::OperatingSystemArchitecture();
            auto cpu = base::SysInfo::CPUModelName();
            auto memory = base::IntToString(base::SysInfo::AmountOfPhysicalMemoryMB());
            auto navbar_height = 30;  //h5的navBarHeight标题栏高度固定为30
            auto time = webview_load_time;

            cef_proxy::calldata calldata;
            cef_proxy::calldata_filed data_filed;
            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

            data_filed.str_ = "pc_link";
            const_cast<cef_proxy::calldata&>(calldata).insert({ "platform", data_filed });

            data_filed.str_ = app_version;
            const_cast<cef_proxy::calldata&>(calldata).insert({ "appVersion", data_filed });

            data_filed.str_ = app_uild;
            const_cast<cef_proxy::calldata&>(calldata).insert({ "appBuild", data_filed });

            data_filed.reset();
            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
            data_filed.str_ = os_version;
            const_cast<cef_proxy::calldata&>(calldata).insert({ "osVersion", data_filed });

            data_filed.str_ = os_arch;
            const_cast<cef_proxy::calldata&>(calldata).insert({ "osArch", data_filed });

            data_filed.str_ = cpu;
            const_cast<cef_proxy::calldata&>(calldata).insert({ "cpu", data_filed });

            data_filed.str_ = memory;
            const_cast<cef_proxy::calldata&>(calldata).insert({ "memory", data_filed });

            data_filed.reset();
            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
            data_filed.numeric_union.int_ = navbar_height;
            const_cast<cef_proxy::calldata&>(calldata).insert({ "navBarHeight", data_filed });

            data_filed.reset();
            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
            data_filed.str_ = time;
            const_cast<cef_proxy::calldata&>(calldata).insert({ "time", data_filed });

            data_filed.reset();
            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
            data_filed.numeric_union.int_ = callbackId;
            const_cast<cef_proxy::calldata&>(calldata).insert({ "callbackId", data_filed });

            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
            data_filed.numeric_union.int_ = (int)cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK_DATA;
            const_cast<cef_proxy::calldata&>(calldata).insert({ cef_proxy::kCallbackTypeField, data_filed });

            CefProxyWrapper::GetInstance()->DispatchJsEvent(bind_data,
                ipc_messages::kBilibiliPageGetContainerInfo, &calldata);
        }
        else if (msg_name == ipc_messages::kBilibiliPageImport) {
            method_origin_name = GetCallDataString(data, "method_origin_name");
            //method_origin_data = GetCallDataString(data, "namespace");
        }
        else if (msg_name == ipc_messages::kBilibiliPageForward)
        {
            std::string topage = data->at("topage").str_;
            std::string type = data->at("type").str_;
            int callbackId = data->at("callbackId").numeric_union.int_;
            int successCallbackId = data->at("successCallbackId").numeric_union.int_;
            method_origin_name = GetCallDataString(data, "method_origin_name");

            cef_proxy::calldata calldata;
            cef_proxy::calldata_filed data_filed;
            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;

            data_filed.numeric_union.int_ = callbackId;
            const_cast<cef_proxy::calldata&>(calldata).insert({ "callbackId", data_filed });

            data_filed.numeric_union.int_ = (int)cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK;
            const_cast<cef_proxy::calldata&>(calldata).insert({ cef_proxy::kCallbackTypeField, data_filed });

            CefProxyWrapper::GetInstance()->DispatchJsEvent(bind_data,
                ipc_messages::kBilibiliPageForward, &calldata);

            // 目前公共H5窗口暂时最多只允许二级弹窗了，alllow_popup参数就不要递归传递了
            if (type == "h5") {
                if (details && details->widget) {
                    data_filed.reset();
                    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
                    data_filed.numeric_union.int_ = successCallbackId;
                    const_cast<cef_proxy::calldata&>(calldata).insert({ "successCallbackId", data_filed });

                    data_filed.numeric_union.int_ = (int)cef_proxy::callback_type::CALL_BACK_TYPE_SUCCESS_DATA;
                    const_cast<cef_proxy::calldata&>(calldata)[cef_proxy::kCallbackTypeField] = data_filed;

                    CefHybridBrowserDelegate* web_delegate = CefHybridBrowserDelegate::GetInstance();
                    web_delegate->DelayDispatchJsEvent(bind_data, ipc_messages::kBilibiliPageForward, calldata);

                    BililiveUniteBrowserView::ShowWindow(details->widget, topage, L"", WebViewPopupType::NotAllow,
                        cef_proxy::client_handler_type::bilibili, "", CefHybridBrowserDelegate::GetInstance(), true);
                }
            } else if (type == "native") {
               
            } else if (type == "browser") {
                ShellExecuteA(nullptr, "open", bililive::FastForwardChangeEnv(topage).c_str(), nullptr, nullptr, SW_SHOW);
            }
        }
        else if (msg_name == ipc_messages::kBilibiliPageLocalCache)
        {
            std::string operation = data->at("operation").str_;
            std::string key = data->at("key").str_;
            base::string16 value = data->at("value").wstr_;
            int callbackId = data->at("callbackId").numeric_union.int_;
            int successCallbackId = data->at("successCallbackId").numeric_union.int_;
            method_origin_name = GetCallDataString(data, "method_origin_name");

            bool ret = true;
            cef_proxy::calldata calldata;
            cef_proxy::calldata_filed data_filed;
            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;

            data_filed.numeric_union.int_ = callbackId;
            const_cast<cef_proxy::calldata&>(calldata).insert({ "callbackId", data_filed });

            data_filed.numeric_union.int_ = (int)cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK;
            const_cast<cef_proxy::calldata&>(calldata).insert({ cef_proxy::kCallbackTypeField, data_filed });

            CefProxyWrapper::GetInstance()->DispatchJsEvent(bind_data,
                ipc_messages::kBilibiliPageLocalCache, &calldata);

            base::string16 ret_value;
            if (operation == "set") {
                ret = CefHybridCache::GetInstance()->AddHybridCacheByKey(key, value);
            } else if (operation == "get") {
                ret_value = CefHybridCache::GetInstance()->GetHybridCacheByKey(key);
            } else if (operation == "remove") {
                ret = CefHybridCache::GetInstance()->RemoveHybridCacheByKey(key);
            } /*else if (operation == "clear") {
                CefHybridCache::GetInstance()->ClearHybridCache();
            }*/

            if (operation == "set" || operation == "get" || operation == "remove") {
                data_filed.reset();
                data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_WSTRING;
                data_filed.wstr_ = ret_value;
                const_cast<cef_proxy::calldata&>(calldata).insert({ "value", data_filed });

                data_filed.reset();
                data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
                data_filed.numeric_union.int_ = successCallbackId;
                const_cast<cef_proxy::calldata&>(calldata).insert({ "successCallbackId", data_filed });

                data_filed.numeric_union.int_ = (int)cef_proxy::callback_type::CALL_BACK_TYPE_SUCCESS_DATA;
                const_cast<cef_proxy::calldata&>(calldata)[cef_proxy::kCallbackTypeField] = data_filed;

                CefProxyWrapper::GetInstance()->DispatchJsEvent(bind_data,
                    ipc_messages::kBilibiliPageLocalCache, &calldata);
            }
        }
        else if (msg_name == ipc_messages::kBilibiliPageClosed)
        {
            method_origin_name = GetCallDataString(data, "method_origin_name");
            if (details && details->widget)
            {
                details->widget->Close();
            }
        }
        else if (msg_name == ipc_messages::kLivehimeCefPopupDevTools)
        {
            if (delegate)
            {
                delegate->OnPopupWebviewDetails();
            }
            LOG(INFO) << "[webview] user pressed ctrl+shift+f11";
        }
        else if(msg_name == ipc_messages::kLivehimeCefClientDestoryed)
        {
            // WebView自身在解构时就把自己从CefWrapper的observerlist中移除了，应该不会收到这个通知的
            NOTREACHED();
        }
        else
        {
            should_dispatch_to_delegate = true;
        }

        // 没有进行专门的delegate回调的情况都以OnWebBrowserMsgReceived的形式向外通知
        if (should_dispatch_to_delegate)
        {
            bool handled = false;
            if (delegate)
            {
                handled = delegate->OnWebBrowserMsgReceived(msg_name, data);
            }

            if (!handled)
            {
                LOG(INFO) << "[webview] ipc msg '" << msg_name << "' not handled.";
            }
        }

        // bridge北极星埋点
        if (!method_origin_name.empty())
        {
            CefJsBridgePolarisEventReport(secret::LivehimePolarisBehaviorEvent::CefJsBridgeCallTrack,
                method_origin_name, page_url, method_origin_data);
        }
    }
}
