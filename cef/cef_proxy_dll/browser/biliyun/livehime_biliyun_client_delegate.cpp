#include "stdafx.h"
#include "livehime_biliyun_client_delegate.h"

#include "bililive_browser/public/bililive_browser_ipc_messages.h"
#include "bililive_browser/public/bililive_browser_js_values.h"

#include "event_dispatcher/livehime_cef_proxy_events_dispatcher.h"

#include "public/livehime_cef_proxy_constants.h"
#include "util/cef_proxy_util.h"
#include "bililive_browser/public/bililive_cef_headers.h"


LivehimeBiliyunClientDelegate::LivehimeBiliyunClientDelegate(const cef_proxy::browser_bind_data& bind_data)
    : LivehimeCefClientBrowser::Delegate(bind_data)
{
}

LivehimeBiliyunClientDelegate::~LivehimeBiliyunClientDelegate()
{
}

void LivehimeBiliyunClientDelegate::OnBrowserCreated(CefRefPtr<CefBrowser> browser)
{
    DCHECK(!browser_);
    browser_ = browser;
}

void LivehimeBiliyunClientDelegate::OnBrowserClosed(CefRefPtr<CefBrowser> browser)
{
    DCHECK(browser_);
    if (browser->IsSame(browser_))
    {
        browser_ = nullptr;
    }
}

void LivehimeBiliyunClientDelegate::OnBeforeContextMenu(CefRefPtr<CefFrame> frame, 
                                                          CefRefPtr<CefContextMenuParams> params,
                                                          CefRefPtr<CefMenuModel> model)
{
    model->Clear();
}

void LivehimeBiliyunClientDelegate::OnLoadStart(CefRefPtr<CefFrame> frame, CefLoadHandler::TransitionType transition_type)
{
    if (frame->IsMain())
    {
        std::string version = GetExecutableVersion();

        CefString eval_browser = "window.browser = {" \
                "version: {" \
                    "android: false, BiliApp: true, mobile: false," \
                    "ios: false, iPhone: false, iPad: false, MicroMessenger: false," \
                    "webApp: false, pc_link: true" \
                "}," \
                "language: 'zh-CN'," \
                "pc_link_scene: 'popup',"
                "app_version: '" + version + "',"\
                "rollout: 'support-new-browser'"
            "}";

        CefString url = frame->GetURL();
        frame->ExecuteJavaScript(eval_browser, url, 0);
    }
}

void LivehimeBiliyunClientDelegate::OnLoadEnd(CefRefPtr<CefFrame> frame, int httpStatusCode)
{
}

void LivehimeBiliyunClientDelegate::OnLoadError(CefRefPtr<CefFrame> frame, 
                                                  CefLoadHandler::ErrorCode errorCode, 
                                                  const CefString& errorText, 
                                                  const CefString& failedUrl)
{
}

bool LivehimeBiliyunClientDelegate::OnCertificateError(CefRefPtr<CefBrowser> browser, 
                                                         cef_errorcode_t cert_error, 
                                                         const CefString& request_url, 
                                                         CefRefPtr<CefSSLInfo> ssl_info, 
                                                         CefRefPtr<CefRequestCallback> callback)
{
    callback->Continue();
    return true;
}

void LivehimeBiliyunClientDelegate::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, CefRequestHandler::TerminationStatus status)
{
    // rendered进程异常退出了，要向上层通报一下，上层应该把上传任务停掉
    cef_proxy::calldata data;
    cef_proxy::calldata_filed data_filed;
    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

    // 构造和biliyun err一样的数据格式，方便上层统一处理
    switch (status)
    {
    case CefRequestHandler::TerminationStatus::TS_ABNORMAL_TERMINATION:
        data_filed.str_ = R"({ "detail" : "Non-zero exit status.", "type" : "TS_ABNORMAL_TERMINATION" })";
        break;
    case CefRequestHandler::TerminationStatus::TS_PROCESS_WAS_KILLED:
        data_filed.str_ = R"({ "detail" : "SIGKILL or task manager kill.", "type" : "TS_PROCESS_WAS_KILLED" })";
        break;
    case CefRequestHandler::TerminationStatus::TS_PROCESS_CRASHED:
        data_filed.str_ = R"({ "detail" : "Segmentation fault.", "type" : "TS_PROCESS_CRASHED" })";
        break;
    case CefRequestHandler::TerminationStatus::TS_PROCESS_OOM:
        data_filed.str_ = R"({ "detail" : "Out of memory. Some platforms may use TS_PROCESS_CRASHED instead.", "type" : "TS_PROCESS_OOM" })";
        break;
    default:
        data_filed.str_ = R"({ "detail" : "Unknown error status.", "type" : ")" + std::to_string((int)status) + R"(" })";
        break;
    }
    data["error_info"] = data_filed;


    LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), ipc_messages::kBiliyunRenderedTerminated, &data);
}

bool LivehimeBiliyunClientDelegate::OnProcessMessageReceived(CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
    if (ScheduleCommonRendererIPCMessage(bind_data(), message))
    {
        return true;
    }
    else
    {
        std::string message_name = message->GetName();
        if (message_name == ipc_messages::kBiliyunUploadError)
        {
            CefRefPtr<CefListValue> args = message->GetArgumentList();

            cef_proxy::calldata data;
            cef_proxy::calldata_filed data_filed;
            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

            data_filed.str_ = args->GetString(0);
            data["error_info"] = data_filed;

            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
            data_filed.numeric_union.int64_ = args->GetInt(1);
            data["task_id"] = data_filed;

            LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), message_name, &data);
            return true;
        }
        else if (message_name == ipc_messages::kBiliyunBeforePreUpload ||
            message_name == ipc_messages::kBiliyunBeforeUpload ||
            message_name == ipc_messages::kBiliyunUploadProgress ||
            message_name == ipc_messages::kBiliyunBeforeFileUploaded ||
            message_name == ipc_messages::kBiliyunFileUploaded)
        {
            CefRefPtr<CefListValue> args = message->GetArgumentList();

            cef_proxy::calldata data;
            cef_proxy::calldata_filed data_filed;
            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

            data_filed.str_ = args->GetString(0);
            data["box_info"] = data_filed;

            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
            data_filed.numeric_union.int64_ = args->GetInt(1);
            data["task_id"] = data_filed;

            LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data(), message_name, &data);
            return true;
        }
    }
    return false;
}

bool LivehimeBiliyunClientDelegate::DispatchJsEvent(const std::string& ipc_msg_name, const cef_proxy::calldata& arguments)
{
    if (ipc_msg_name.compare(ipc_messages::kBiliyunUploaderLocalFile) == 0)
    {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBiliyunUploaderLocalFile);
        CefRefPtr<CefListValue> args = msg->GetArgumentList();
        args->SetString(0, arguments.at("file_path").str_);
        args->SetString(1, arguments.at("file_name").str_);
        args->SetString(2, arguments.at("file_type").str_);
        args->SetInt(3, arguments.at("task_id").numeric_union.int64_);
        SendBrowserProcessMessage(browser_, PID_RENDERER, msg);
        return true;
    }
    else if (ipc_msg_name.compare(ipc_messages::kBiliyunUploaderDelete) == 0)
    {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBiliyunUploaderLocalFile);
        CefRefPtr<CefListValue> args = msg->GetArgumentList();
        args->SetInt(0, arguments.at("task_id").numeric_union.int64_);
        args->SetString(1, arguments.at("box_id").str_);
        SendBrowserProcessMessage(browser_, PID_RENDERER, msg);
        return true;
    }
    return false;
}
