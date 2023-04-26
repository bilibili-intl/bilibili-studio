// cef_proxy_dll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "cef_proxy_util.h"

#include <crtdbg.h>
#include <algorithm>

#include "bililive_browser/public/bililive_browser_ipc_messages.h"
#include "event_dispatcher/livehime_cef_proxy_events_dispatcher.h"

std::string exe_version("9999");

std::string device_name_("");

std::string device_platform_("");

std::string buvid3_("");

std::wstring MakeAbsoluteFilePath(const std::wstring& input)
{
    std::wstring path = input;
    path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());
    wchar_t file_path[MAX_PATH] = { 0 };
    if (!_wfullpath(file_path, path.c_str(), MAX_PATH))
        return std::wstring();
    return std::wstring(file_path);
}

std::string GetExecutableVersion()
{
    return exe_version;
}

std::string GetDeviceName()
{
	return device_name_;
}

std::string GetDevicePlatform()
{
	return device_platform_;
}

std::string GetBuvid3_()
{
	return buvid3_;
}



// 这里处理的公共IPC消息是由render进程在“cef\bililive_browser\util\bililive_js_bridge_contract.cpp：ScheduleCommonMethod”抛送
bool ScheduleCommonRendererIPCMessage(const cef_proxy::browser_bind_data& bind_data,
    CefRefPtr<CefProcessMessage> message)
{
    std::string message_name = message->GetName();
    if (message_name == ipc_messages::kLivehimeCefOnWindowClose)
    {
        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPageClosed ||
        message_name == ipc_messages::kBilibiliPageFinished)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();

        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

        data_filed.str_ = args->GetString(0);
        data["method_origin_name"] = data_filed;

        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPageArouse)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();

        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

        data_filed.str_ = args->GetString(0);
        data["params"] = data_filed;

        data_filed.str_ = args->GetString(1);
        data["moduleName"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_BOOL;
        data_filed.numeric_union.bool_ = args->GetBool(2);
        data["closeCurrent"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
        data_filed.str_ = args->GetString(3);
        data["method_origin_name"] = data_filed;

        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPageReport)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();

        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

        data_filed.str_ = args->GetString(0);
        data["id"] = data_filed;

        data_filed.str_ = args->GetString(1);
        data["msg"] = data_filed;

        data_filed.str_ = args->GetString(2);
        data["method_origin_name"] = data_filed;

        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPageTecReport)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();

        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

        data_filed.str_ = args->GetString(0);
        data["id"] = data_filed;

        data_filed.str_ = args->GetString(1);
        data["msg"] = data_filed;

        data_filed.str_ = args->GetString(2);
        data["method_origin_name"] = data_filed;

        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPagePolarisReport)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();

        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

        data_filed.str_ = args->GetString(0);
        data["id"] = data_filed;

        data_filed.str_ = args->GetString(1);
        data["msg"] = data_filed;

        data_filed.str_ = args->GetString(2);
        data["method_origin_name"] = data_filed;

        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPageProjection)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();

        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

        data_filed.str_ = args->GetString(0);
        data["id"] = data_filed;

        data_filed.str_ = args->GetString(1);
        data["url"] = data_filed;

        data_filed.str_ = args->GetString(2);
        data["method_origin_name"] = data_filed;

        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPageGetInfo)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();

        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

        data_filed.str_ = args->GetString(0);
        data["type"] = data_filed;

        data_filed.str_ = args->GetString(1);
        data["params"] = data_filed;

        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
        data_filed.numeric_union.int_ = args->GetInt(2);
        data["callbackId"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
        data_filed.str_ = args->GetString(3);
        data["method_origin_name"] = data_filed;

        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPageSubBroadcast)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();

        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

        data_filed.str_ = args->GetString(0);
        data["type"] = data_filed;

        data_filed.str_ = args->GetString(1);
        data["cmd"] = data_filed;

        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
        data_filed.numeric_union.int_ = args->GetInt(2);
        data["successCallbackId"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
        data_filed.str_ = args->GetString(3);
        data["method_origin_name"] = data_filed;

        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPageUnSubBroadcast)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();

        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

        data_filed.str_ = args->GetString(0);
        data["type"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_BOOL;
        data_filed.numeric_union.bool_ = args->GetBool(1);
        data["closeCurrent"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
        data_filed.str_ = args->GetString(2);
        data["method_origin_name"] = data_filed;

        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPageAlert)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();

        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

        data_filed.wstr_ = args->GetString(0);
        data["confirmButton"] = data_filed;

        std::wstring cancelButton = args->GetString(1);
        if (cancelButton != L"null") {
            data_filed.wstr_ = cancelButton;
            data["cancelButton"] = data_filed;
        }

        data_filed.wstr_ = args->GetString(2);
        data["neutralButton"] = data_filed;

        data_filed.str_ = args->GetString(3);
        data["type"] = data_filed;

        data_filed.str_ = args->GetString(4);
        data["noRemindKey"] = data_filed;

        data_filed.wstr_ = args->GetString(5);
        data["title"] = data_filed;

        data_filed.wstr_ = args->GetString(6);
        data["message"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
        data_filed.numeric_union.int_ = args->GetInt(7);
        data["callbackId"] = data_filed;

        data_filed.numeric_union.int_ = args->GetInt(8);
        data["successCallbackId"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
        data_filed.str_ = args->GetString(9);
        data["method_origin_name"] = data_filed;

        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPageToast)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();

        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_WSTRING;

        data_filed.wstr_ = args->GetString(0);
        data["msg"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
        data_filed.str_ = args->GetString(1);
        data["type"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
        data_filed.numeric_union.int_ = args->GetInt(2);
        data["callbackId"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
        data_filed.str_ = args->GetString(3);
        data["method_origin_name"] = data_filed;

        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPageSetTitle)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();

        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_WSTRING;

        data_filed.wstr_ = args->GetString(0);
        data["title"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
        data_filed.str_ = args->GetString(1);
        data["icon"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
        data_filed.numeric_union.int_ = args->GetInt(2);
        data["callbackId"] = data_filed;

        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPageSetNavMenu)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        CefRefPtr<CefListValue> args_list = args->GetList(0);

        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        for (int i = 0; i < args_list->GetSize(); i++) {
            CefRefPtr<CefListValue> list = args_list->GetList(i);
            cef_proxy::calldata child_data;
            cef_proxy::calldata_filed child_data_filed;
            child_data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

            child_data_filed.str_ = list->GetString(0);
            child_data["tagname"] = child_data_filed;

            child_data_filed.str_ = list->GetString(1);
            child_data["color"] = child_data_filed;

            child_data_filed.reset();
            child_data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_WSTRING;
            child_data_filed.wstr_ = list->GetString(2);
            child_data["text"] = child_data_filed;

            child_data_filed.wstr_ = list->GetString(3);
            child_data["badge"] = child_data_filed;

            data_filed.list_.push_back(child_data);
        }

        data["list"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
        data_filed.numeric_union.int_ = args->GetInt(1);
        data["callbackId"] = data_filed;

        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPageTitleBar)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();

        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;

        //data_filed.numeric_union.int_ = args->GetInt(0);
        //data["display"] = data_filed;

        std::wstring title = args->GetString(0);
        if (title != L"null") {
            data_filed.reset();
            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_WSTRING;
            data_filed.wstr_ = title;
            data["title"] = data_filed;
        }

        std::string icon = args->GetString(1);
        if (icon != "null") {
            data_filed.reset();
            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
            data_filed.str_ = args->GetString(1);
            data["icon"] = data_filed;
        }

        std::string backgroundColor = args->GetString(2);
        if (backgroundColor != "null") {
            data_filed.reset();
            data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
            data_filed.str_ = args->GetString(2);
            data["backgroundColor"] = data_filed;
        }

        std::string titleColor = args->GetString(3);
        if (titleColor != "null") {
            data_filed.str_ = args->GetString(3);
            data["titleColor"] = data_filed;
        }

        //data_filed.reset();
        //CefRefPtr<CefListValue> args_left_list = args->GetList(4);
        //for (int i = 0; i < args_left_list->GetSize(); i++) {
        //    CefRefPtr<CefListValue> list = args_left_list->GetList(i);
        //    cef_proxy::calldata child_data;
        //    cef_proxy::calldata_filed child_data_filed;
        //    child_data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

        //    child_data_filed.str_ = list->GetString(0);
        //    child_data["tagname"] = child_data_filed;

        //    child_data_filed.str_ = list->GetString(1);
        //    child_data["color"] = child_data_filed;

        //    child_data_filed.reset();
        //    child_data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_WSTRING;
        //    child_data_filed.wstr_ = list->GetString(2);
        //    child_data["text"] = child_data_filed;

        //    child_data_filed.wstr_ = list->GetString(3);
        //    child_data["badge"] = child_data_filed;

        //    data_filed.list_.push_back(child_data);
        //}

        //data["left"] = data_filed;

        bool has_right = args->GetBool(4);
        if (has_right) {
            CefRefPtr<CefListValue> args_right_list = args->GetList(5);
            if (args_right_list) {
                data_filed.reset();
                for (int i = 0; i < args_right_list->GetSize(); i++) {
                    CefRefPtr<CefListValue> list = args_right_list->GetList(i);
                    cef_proxy::calldata child_data;
                    cef_proxy::calldata_filed child_data_filed;
                    child_data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

                    std::string tagname = list->GetString(0);
                    if (tagname != "null") {
                        child_data_filed.str_ = list->GetString(0);
                        child_data["tagname"] = child_data_filed;
                    }

                    //child_data_filed.str_ = list->GetString(1);
                    //child_data["color"] = child_data_filed;

                    std::wstring text = list->GetString(1);
                    if (text != L"null") {
                        child_data_filed.reset();
                        child_data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_WSTRING;
                        child_data_filed.wstr_ = list->GetString(1);
                        child_data["text"] = child_data_filed;
                    }

                    std::wstring url = list->GetString(2);
                    if (url != L"null") {
                        child_data_filed.reset();
                        child_data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_WSTRING;
                        child_data_filed.wstr_ = list->GetString(2);
                        child_data["url"] = child_data_filed;
                    }

                    //child_data_filed.wstr_ = list->GetString(3);
                    //child_data["badge"] = child_data_filed;

                    data_filed.list_.push_back(child_data);
                }

                data["right"] = data_filed;
            }
        }

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
        data_filed.numeric_union.int_ = args->GetInt(6);
        data["callbackId"] = data_filed;

        data_filed.numeric_union.int_ = args->GetInt(7);
        data["successCallbackId"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
        data_filed.str_ = args->GetString(8);
        data["method_origin_name"] = data_filed;

        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPageGetContainerInfo)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        
        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
        data_filed.numeric_union.int_ = args->GetInt(0);
        data["callbackId"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
        data_filed.str_ = args->GetString(1);
        data["method_origin_name"] = data_filed;
        
        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPageImport)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        
        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
        data_filed.str_ = args->GetString(0);
        data["method_origin_name"] = data_filed;

        data_filed.str_ = args->GetString(1);
        data["namespace"] = data_filed;
        
        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPageForward)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        
        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

        data_filed.str_ = args->GetString(0);
        data["topage"] = data_filed;

        data_filed.str_ = args->GetString(1);
        data["type"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
        data_filed.numeric_union.int_ = args->GetInt(2);
        data["callbackId"] = data_filed;

        data_filed.numeric_union.int_ = args->GetInt(3);
        data["successCallbackId"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
        data_filed.str_ = args->GetString(4);
        data["method_origin_name"] = data_filed;
        
        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }
    else if (message_name == ipc_messages::kBilibiliPageLocalCache)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        
        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

        data_filed.str_ = args->GetString(0);
        data["operation"] = data_filed;

        data_filed.str_ = args->GetString(1);
        data["key"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_WSTRING;
        data_filed.wstr_ = args->GetString(2);
        data["value"] = data_filed;

        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_INT;
        data_filed.numeric_union.int_ = args->GetInt(3);
        data["callbackId"] = data_filed;

        data_filed.numeric_union.int_ = args->GetInt(4);
        data["successCallbackId"] = data_filed;

        data_filed.reset();
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
        data_filed.str_ = args->GetString(5);
        data["method_origin_name"] = data_filed;

        LivehimeCefProxyEventsDispatcher::DispatchEvent(bind_data, message_name, &data);
        return true;
    }

    return false;
}

bool ScheduleCommonBrowserIPCMessage(CefRefPtr<CefBrowser> browser, const std::string& ipc_msg_name,
    const cef_proxy::calldata& arguments)
{
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_msg_name);
    CefRefPtr<CefListValue> args = msg->GetArgumentList();

    if (ipc_msg_name == ipc_messages::kBilibiliPageGetInfo)
    {
        CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
        dict->SetString("type", arguments.at("type").str_);
        dict->SetString("params", arguments.at("params").str_);
        dict->SetInt("callbackId", arguments.at("callbackId").numeric_union.int_);
        dict->SetString("info", arguments.at("info").str_);

        args->SetDictionary(0, dict);

        SendBrowserProcessMessage(browser, PID_RENDERER, msg);
        return true;
    }
    else if (ipc_msg_name == ipc_messages::kBilibiliPageSubBroadcast)
    {
        CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
        dict->SetString("type", arguments.at("type").str_);
        dict->SetString("cmd", arguments.at("cmd").str_);
        dict->SetInt("successCallbackId", arguments.at("successCallbackId").numeric_union.int_);
        dict->SetString("info", arguments.at("info").str_);

        args->SetDictionary(0, dict);

        SendBrowserProcessMessage(browser, PID_RENDERER, msg);
        return true;
    }
    else if (ipc_msg_name == ipc_messages::kBilibiliPageAlert)
    {
        CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
        cef_proxy::callback_type callback_type = cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK;
        callback_type = (cef_proxy::callback_type)arguments.at(cef_proxy::kCallbackTypeField).numeric_union.int_;
        switch (callback_type) {
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK:
            dict->SetInt(cef_proxy::kCallbackTypeField, (int)callback_type);
            dict->SetInt("callbackId", arguments.at("callbackId").numeric_union.int_);
            break;
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK_DATA:
            dict->SetInt(cef_proxy::kCallbackTypeField, (int)callback_type);
            dict->SetInt("hidden", arguments.at("hidden").numeric_union.int_);
            dict->SetInt("callbackId", arguments.at("callbackId").numeric_union.int_);
            break;
        case cef_proxy::callback_type::CALL_BACK_TYPE_SUCCESS_DATA:
            dict->SetInt(cef_proxy::kCallbackTypeField, (int)callback_type);
            dict->SetString("type", arguments.at("type").str_);
            dict->SetInt("successCallbackId", arguments.at("successCallbackId").numeric_union.int_);
            break;
        default:
            //DCHECK(FALSE);
            break;
        }

        args->SetDictionary(0, dict);

        SendBrowserProcessMessage(browser, PID_RENDERER, msg);
        return true;
    }
    else if (ipc_msg_name == ipc_messages::kBilibiliPageToast)
    {
        CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
        cef_proxy::callback_type callback_type = cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK;
        callback_type = (cef_proxy::callback_type)arguments.at(cef_proxy::kCallbackTypeField).numeric_union.int_;
        switch (callback_type) {
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK:
            dict->SetInt(cef_proxy::kCallbackTypeField, (int)callback_type);
            dict->SetInt("callbackId", arguments.at("callbackId").numeric_union.int_);
            break;
        default:
            //DCHECK(FALSE);
            break;
        }

        args->SetDictionary(0, dict);

        SendBrowserProcessMessage(browser, PID_RENDERER, msg);
        return true;
    }
    else if (ipc_msg_name == ipc_messages::kBilibiliPageSetTitle)
    {
        CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
        cef_proxy::callback_type callback_type = cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK;
        callback_type = (cef_proxy::callback_type)arguments.at(cef_proxy::kCallbackTypeField).numeric_union.int_;
        switch (callback_type) {
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK:
            dict->SetInt(cef_proxy::kCallbackTypeField, (int)callback_type);
            dict->SetInt("callbackId", arguments.at("callbackId").numeric_union.int_);
            break;
        default:
            //DCHECK(FALSE);
            break;
        }

        args->SetDictionary(0, dict);

        SendBrowserProcessMessage(browser, PID_RENDERER, msg);
        return true;
    }
    else if (ipc_msg_name == ipc_messages::kBilibiliPageSetNavMenu)
    {
        CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
        cef_proxy::callback_type callback_type = cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK;
        callback_type = (cef_proxy::callback_type)arguments.at(cef_proxy::kCallbackTypeField).numeric_union.int_;
        switch (callback_type) {
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK:
            dict->SetInt(cef_proxy::kCallbackTypeField, (int)callback_type);
            dict->SetInt("callbackId", arguments.at("callbackId").numeric_union.int_);
            break;
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK_DATA:
            dict->SetInt(cef_proxy::kCallbackTypeField, (int)callback_type);
            dict->SetString("tagname", arguments.at("tagname").wstr_);
            dict->SetInt("callbackId", arguments.at("callbackId").numeric_union.int_);
            break;
        default:
            //DCHECK(FALSE);
            break;
        }

        args->SetDictionary(0, dict);

        SendBrowserProcessMessage(browser, PID_RENDERER, msg);
        return true;
    }
    else if (ipc_msg_name == ipc_messages::kBilibiliPageTitleBar)
    {
        CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
        cef_proxy::callback_type callback_type = cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK;
        callback_type = (cef_proxy::callback_type)arguments.at(cef_proxy::kCallbackTypeField).numeric_union.int_;
        switch (callback_type) {
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK:
            dict->SetInt(cef_proxy::kCallbackTypeField, (int)callback_type);
            dict->SetInt("callbackId", arguments.at("callbackId").numeric_union.int_);
            break;
        case cef_proxy::callback_type::CALL_BACK_TYPE_SUCCESS_DATA:
            dict->SetInt(cef_proxy::kCallbackTypeField, (int)callback_type);
            dict->SetString("tagname", arguments.at("tagname").wstr_);
            dict->SetInt("successCallbackId", arguments.at("successCallbackId").numeric_union.int_);
            break;
        default:
            //DCHECK(FALSE);
            break;
        }

        args->SetDictionary(0, dict);

        SendBrowserProcessMessage(browser, PID_RENDERER, msg);
        return true;
    }
    else if (ipc_msg_name == ipc_messages::kBilibiliPageGetContainerInfo)
    {
        CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
        cef_proxy::callback_type callback_type = cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK;
        callback_type = (cef_proxy::callback_type)arguments.at(cef_proxy::kCallbackTypeField).numeric_union.int_;
        switch (callback_type) {
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK_DATA:
            dict->SetInt(cef_proxy::kCallbackTypeField, (int)callback_type);
            dict->SetString("platform", arguments.at("platform").str_);
            dict->SetString("appVersion", arguments.at("appVersion").str_);
            dict->SetString("appBuild", arguments.at("appBuild").str_);
            dict->SetString("osVersion", arguments.at("osVersion").str_);
            dict->SetString("osArch", arguments.at("osArch").str_);
            dict->SetString("cpu", arguments.at("cpu").str_);
            dict->SetString("memory", arguments.at("memory").str_);
            dict->SetInt("navBarHeight", arguments.at("navBarHeight").numeric_union.int_);
            dict->SetString("time", arguments.at("time").str_);
            dict->SetInt("callbackId", arguments.at("callbackId").numeric_union.int_);
            break;
        default:
            //DCHECK(FALSE);
            break;
        }

        args->SetDictionary(0, dict);

        SendBrowserProcessMessage(browser, PID_RENDERER, msg);
        return true;
    }
    else if (ipc_msg_name == ipc_messages::kBilibiliPageLocalCache)
    {
        CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
        cef_proxy::callback_type callback_type = cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK;
        callback_type = (cef_proxy::callback_type)arguments.at(cef_proxy::kCallbackTypeField).numeric_union.int_;
        switch (callback_type) {
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK:
            dict->SetInt(cef_proxy::kCallbackTypeField, (int)callback_type);
            dict->SetInt("callbackId", arguments.at("callbackId").numeric_union.int_);
            break;
        case cef_proxy::callback_type::CALL_BACK_TYPE_SUCCESS_DATA:
            dict->SetInt(cef_proxy::kCallbackTypeField, (int)callback_type);
            dict->SetString("value", arguments.at("value").wstr_);
            dict->SetInt("successCallbackId", arguments.at("successCallbackId").numeric_union.int_);
            break;
        default:
            //DCHECK(FALSE);
            break;
        }

        args->SetDictionary(0, dict);

        SendBrowserProcessMessage(browser, PID_RENDERER, msg);
        return true;
    }
    else if (ipc_msg_name == ipc_messages::kBilibiliPageForward)
    {
        CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
        cef_proxy::callback_type callback_type = cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK;
        callback_type = (cef_proxy::callback_type)arguments.at(cef_proxy::kCallbackTypeField).numeric_union.int_;
        switch (callback_type) {
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK:
            dict->SetInt(cef_proxy::kCallbackTypeField, (int)callback_type);
            dict->SetInt("callbackId", arguments.at("callbackId").numeric_union.int_);
            break;
        case cef_proxy::callback_type::CALL_BACK_TYPE_SUCCESS_DATA:
            dict->SetInt(cef_proxy::kCallbackTypeField, (int)callback_type);
            dict->SetInt("successCallbackId", arguments.at("successCallbackId").numeric_union.int_);
            break;
        default:
            //DCHECK(FALSE);
            break;
        }

        args->SetDictionary(0, dict);

        SendBrowserProcessMessage(browser, PID_RENDERER, msg);
        return true;
    }

    return false;
}
