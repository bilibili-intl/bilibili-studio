
#include "stdafx.h"
#include "bililive_js_bridge_contract.h"

#include "bilibase/basic_types.h"

#include "public/bililive_browser_ipc_messages.h"
#include "public/bililive_browser_js_values.h"
#include "util/bililive_browser_util.h"


namespace js_bridge
{
    bool InjectModuleObjToV8Context(CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Handler> handler,
        cef_proxy::client_handler_type type, const std::string& obj_name)
    {
        bool result = true;
        CefRefPtr<CefV8Value> globalObj = context->GetGlobal();

        CefRefPtr<CefV8Value> inject = CefV8Value::CreateObject(nullptr, nullptr);
        {
            for (auto& iter : js_bridge::kSupportJsFunctionNames)
            {
                CefRefPtr<CefV8Value> js_func = CefV8Value::CreateFunction(iter.second, handler);
                result &= inject->SetValue(iter.second, js_func, V8_PROPERTY_ATTRIBUTE_NONE);
            }

            CefRefPtr<CefV8Value> user_data = CefV8Value::CreateInt(bilibase::enum_cast(type));
            result &= inject->SetValue(js_values::kLivehimeUserData, user_data, V8_PROPERTY_ATTRIBUTE_NONE);
        }
        result &= globalObj->SetValue(obj_name, inject, V8_PROPERTY_ATTRIBUTE_NONE);

        DCHECK(result);
        return result;
    }

    bool IsInvokeOnModuleObj(CefRefPtr<CefV8Value> object, cef_proxy::client_handler_type type)
    {
        CefRefPtr<CefV8Value> user_data = object->GetValue(js_values::kLivehimeUserData);
        if (user_data && user_data->IsValid() && user_data->IsInt())
        {
            if (user_data->GetIntValue() == bilibase::enum_cast(type))
            {
                return true;
            }
        }
        return false;
    }

    // 对支持的js函数进行处理，当前只处理biliInject.postMessage
    bool ScheduleCommonJsFunciton(CefRefPtr<CefV8Handler> handler, std::map<std::string, SupportBridgeMethodType>& kSupportBridgeMethodNames,
        const std::string& obj_name, const CefString& name, const CefV8ValueList& arguments, bool& handled_result)
    {
        handled_result = false;
        if (name == kSupportJsFunctionNames.at(SupportJsFunctionType::postMessage))
        {
            if (arguments.empty() || !arguments[0]->IsValid())
            {
                LOG(WARNING) << obj_name << "." << name.ToString() << " with invalid arguments!";
                return true;
            }

            CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();

            // 我们只处理json字符串，如果传过来的是js_json_object就调用js将object转成字符串
            CefRefPtr<CefV8Value> callback;
            CefString msg_str = GetBilibiliPostMessageJsonString(context, arguments[0], &callback);
            if (msg_str.empty())
            {
                LOG(WARNING) << obj_name << "." << name.ToString() << " with empty data!";
                return true;
            }

            // { "method":"live_ui_half.pageFinished"}
            // { "method":"live_location_half.closeWindow", "data" : {"callbackId":31} }
            // { "method":"live_location_full.closeWindow", "data":{"callbackId":6}}
            // {"data":{"callbackId":27,"id":"live.live-zbj.banner.to-extension.click","msg":""},"method":"live_location_event_report"}
            // {"data":{report_msg},"method":"live_location_half_report"}
            // {"data":{"callbackId":28,"closeCurrent":1,"moduleName":"live_cpm","params":"is_activity=1"},"method":"live_arouse_pc_link_module"}
            CefRefPtr<CefValue> msg_value = CefParseJSON(msg_str, JSON_PARSER_RFC);
            if (!msg_value || !msg_value->IsValid())
            {
                LOG(WARNING) << obj_name << "." << name.ToString() << " data not in json format!";
                return true;
            }

            CefRefPtr<CefDictionaryValue> msg_dct = msg_value->GetDictionary();
            if (!msg_dct || !msg_dct->IsValid())
            {
                LOG(WARNING) << obj_name << "." << name.ToString() << " data not in kv format!";
                return true;
            }

            CefString method = msg_dct->GetString("method");
            std::string method_name = method.ToString();

            // 没能直接从postMesssage的object参数中拿到回调函数的话
            // 就通过callbackId从全局BiliJsBridge.callbacks数组中拿到回调函数列表中的指定回调函数项
            CefRefPtr<CefDictionaryValue> data = msg_dct->GetDictionary("data");
            if (!callback)
            {
                int callbackId = data->GetInt("callbackId");
                callback = GetBiliJsBridgeCallback(context, callbackId);
            }

            // 先尝试进行常规method的处理，没有处理再针对专门的定制业务method进行解析处理
            handled_result = ScheduleCommonBiliJsBridgeMethod(handler, kSupportBridgeMethodNames, method_name, data, context, callback);
            if (handled_result)
            {
                LOG(INFO) << obj_name << "." << name.ToString() << " disposed method: " << msg_str.ToString();
            }
            else
            {
                LOG(INFO) << obj_name << "." << name.ToString() << " not support method: " << msg_str.ToString();
            }

            return true;
        }

        return false;
    }

    // 对biliInject.postMessage中的某些method进行处理
    // 这里抛送到browser进程的公共IPC消息的处理在“cef\cef_proxy_dll\util\cef_proxy_util.cpp：ScheduleCommonBrowserIPCMessage”处理
    bool ScheduleCommonBiliJsBridgeMethod(CefRefPtr<CefV8Handler> handler, std::map<std::string, SupportBridgeMethodType>& kSupportBridgeMethodNames, 
        const std::string& method_name, CefRefPtr<CefDictionaryValue> data, CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Value> callback)
    {
        // 不支持的bridge method不做处理
        auto found_iter = std::find_if(kSupportBridgeMethodNames.begin(), kSupportBridgeMethodNames.end(),
            [&](const std::map<std::string, SupportBridgeMethodType>::value_type& iter)->bool{
                return iter.first == method_name;
        });
        if (found_iter == kSupportBridgeMethodNames.end())
        {
            return false;
        }

        // 不需要解析参数（就是单纯的事件触发通知）可以直接向上层业务发ipc消息的method
        static const std::map<SupportBridgeMethodType, std::string> kCommonMethodMappingIPCMsgTable{
            { SupportBridgeMethodType::pageFinished, ipc_messages::kBilibiliPageFinished},
            { SupportBridgeMethodType::full_closeWindow, ipc_messages::kBilibiliPageClosed},
            { SupportBridgeMethodType::half_closeWindow, ipc_messages::kBilibiliPageClosed},
        };

        CefRefPtr<CefBrowser> browser = context->GetBrowser();

        auto found_common_iter = std::find_if(kCommonMethodMappingIPCMsgTable.begin(), kCommonMethodMappingIPCMsgTable.end(),
            [&](const std::map<SupportBridgeMethodType, std::string>::value_type& iter)->bool {
                return iter.first == found_iter->second;
            });
        if (found_common_iter != kCommonMethodMappingIPCMsgTable.end())
        {
            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(found_common_iter->second);
            CefRefPtr<CefListValue> args = msg->GetArgumentList();
            args->SetString(0, CefString(found_iter->first));
            SendBrowserProcessMessage(browser, PID_BROWSER, msg);

            InvokeJsCallbackSync(context, callback, nullptr);
            return true;
        }

        // 针对特定method进行专门的js参数解析和ipc消息参数重封包
        bool support = true;
        switch (found_iter->second)
        {
        case SupportBridgeMethodType::arouse:
            {
                CefString params = data->GetString("params");
                CefString moduleName = data->GetString("moduleName");
                bool closeCurrent = !!data->GetInt("closeCurrent");

                CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPageArouse);
                CefRefPtr<CefListValue> args = msg->GetArgumentList();
                args->SetString(0, params);
                args->SetString(1, moduleName);
                args->SetBool(2, closeCurrent);
                args->SetString(3, CefString(found_iter->first));

                SendBrowserProcessMessage(browser, PID_BROWSER, msg);
                InvokeJsCallbackSync(context, callback, nullptr);
            }
            break;
        case SupportBridgeMethodType::bsns_report:
            {
                CefString event_id = data->GetString("id");
                CefString event_msg = data->GetString("msg");

                CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPageReport);
                CefRefPtr<CefListValue> args = msg->GetArgumentList();
                args->SetString(0, event_id);
                args->SetString(1, event_msg);
                args->SetString(2, CefString(found_iter->first));

                SendBrowserProcessMessage(browser, PID_BROWSER, msg);
                InvokeJsCallbackSync(context, callback, nullptr);
            }
            break;
        case SupportBridgeMethodType::tec_report:
            {
                CefString event_id = data->GetString("id");
                CefString event_msg = data->GetString("msg");

                CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPageTecReport);
                CefRefPtr<CefListValue> args = msg->GetArgumentList();
                args->SetString(0, event_id);
                args->SetString(1, event_msg);
                args->SetString(2, CefString(found_iter->first));

                SendBrowserProcessMessage(browser, PID_BROWSER, msg);
                InvokeJsCallbackSync(context, callback, nullptr);
            }
            break;
        case SupportBridgeMethodType::polaris_report:
            {
                CefString event_id = data->GetString("id");
                CefString event_msg = data->GetString("msg");

                CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPagePolarisReport);
                CefRefPtr<CefListValue> args = msg->GetArgumentList();
                args->SetString(0, event_id);
                args->SetString(1, event_msg);
                args->SetString(2, CefString(found_iter->first));

                SendBrowserProcessMessage(browser, PID_BROWSER, msg);
                InvokeJsCallbackSync(context, callback, nullptr);
            }
            break;
        case SupportBridgeMethodType::web_projection:
            {
                CefString activity_id = data->GetString("id");
                CefString url = data->GetString("url");

                CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPageProjection);
                CefRefPtr<CefListValue> args = msg->GetArgumentList();
                args->SetString(0, activity_id);
                args->SetString(1, url);
                args->SetString(2, CefString(found_iter->first));

                SendBrowserProcessMessage(browser, PID_BROWSER, msg);
                InvokeJsCallbackSync(context, callback, nullptr);
            }
            break;
        case SupportBridgeMethodType::get_pc_link_info:
            {
                if (callback && callback->IsFunction())
                {
                    CefString type = data->GetString("type");
                    CefString params = data->GetString("params");
                    int callbackId = data->GetInt("callbackId");

                    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPageGetInfo);
                    CefRefPtr<CefListValue> args = msg->GetArgumentList();
                    args->SetString(0, type);
                    args->SetString(1, params);
                    // 把callbackId记下来，browser回传数据后renderer根据callbackId从bridge拿回调函数执行通知
                    args->SetInt(2, callbackId);
                    args->SetString(3, CefString(found_iter->first));

                    SendBrowserProcessMessage(browser, PID_BROWSER, msg);
                }
            }
            break;
        case SupportBridgeMethodType::subscribe_pc_broadcast:
        {
            if (callback && callback->IsFunction())
            {
                CefString type = data->GetString("type");
                CefString cmd = data->GetString("cmd");
                int callbackId = data->GetInt("successCallbackId");

                CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPageSubBroadcast);
                CefRefPtr<CefListValue> args = msg->GetArgumentList();
                args->SetString(0, type);
                args->SetString(1, cmd);
                // 把callbackId记下来，browser回传数据后renderer根据callbackId从bridge拿回调函数执行通知
                args->SetInt(2, callbackId);
                args->SetString(3, CefString(found_iter->first));

                SendBrowserProcessMessage(browser, PID_BROWSER, msg);
                InvokeJsCallbackSync(context, callback, nullptr);
            }
        }
        break;
        case SupportBridgeMethodType::unsubscribe_broadcast:
        {
            CefString type = data->GetString("type");
            bool closeCurrent = !!data->GetInt("closeCurrent");

            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPageUnSubBroadcast);
            CefRefPtr<CefListValue> args = msg->GetArgumentList();
            args->SetString(0, type);
            args->SetBool(1, closeCurrent);
            args->SetString(2, CefString(found_iter->first));

            SendBrowserProcessMessage(browser, PID_BROWSER, msg);
            InvokeJsCallbackSync(context, callback, nullptr);
        }
        break;

        case SupportBridgeMethodType::alert:
        {
            CefString confirmButton = data->GetString("confirmButton");
            bool has_cancelButton = data->HasKey("cancelButton");
            CefString cancelButton = data->GetString("cancelButton");
            CefString neutralButton = data->GetString("neutralButton");
            CefString type = data->GetString("type");
            CefString noRemindKey = data->GetString("noRemindKey");
            CefString title = data->GetString("title");
            CefString message = data->GetString("message");
            int callbackId = data->GetInt("callbackId");
            int successCallbackId = data->GetInt("successCallbackId");

            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPageAlert);
            CefRefPtr<CefListValue> args = msg->GetArgumentList();
            args->SetString(0, confirmButton);
            args->SetString(1, has_cancelButton ? cancelButton : L"null");
            args->SetString(2, neutralButton);
            args->SetString(3, type);
            args->SetString(4, noRemindKey);
            args->SetString(5, title);
            args->SetString(6, message);
            // 把callbackId、successCallbackId记下来，browser回传数据后renderer根据callbackId从bridge拿回调函数执行通知
            args->SetInt(7, callbackId);
            args->SetInt(8, successCallbackId);
            args->SetString(9, CefString(found_iter->first));
            
            SendBrowserProcessMessage(browser, PID_BROWSER, msg);
        }
        break;
        case SupportBridgeMethodType::toast:
        {
            CefString message = data->GetString("msg");
            CefString type = data->GetString("type");
            int callbackId = data->GetInt("callbackId");

            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPageToast);
            CefRefPtr<CefListValue> args = msg->GetArgumentList();
            args->SetString(0, message);
            args->SetString(1, type);
            // 把callbackId记下来，browser回传数据后renderer根据callbackId从bridge拿回调函数执行通知
            args->SetInt(2, callbackId);
            args->SetString(3, CefString(found_iter->first));

            SendBrowserProcessMessage(browser, PID_BROWSER, msg);
        }
        break;
        case SupportBridgeMethodType::set_title:
        {
            CefString title = data->GetString("title");
            CefString icon = data->GetString("icon");
            int callbackId = data->GetInt("callbackId");

            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPageSetTitle);
            CefRefPtr<CefListValue> args = msg->GetArgumentList();
            args->SetString(0, title);
            args->SetString(1, icon);
            // 把callbackId记下来，browser回传数据后renderer根据callbackId从bridge拿回调函数执行通知
            args->SetInt(2, callbackId);

            SendBrowserProcessMessage(browser, PID_BROWSER, msg);
        }
        break;
        case SupportBridgeMethodType::set_nav_menu:
        {
            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPageSetNavMenu);
            CefRefPtr<CefListValue> args = msg->GetArgumentList();

            CefRefPtr<CefListValue> args_list = CefListValue::Create();
            CefRefPtr<CefListValue> list = data->GetList("list");
            for (int i = 0; i < list->GetSize(); i++)
            {
                CefRefPtr<CefDictionaryValue> child_value = list->GetDictionary(i);
                CefString tagname = child_value->GetString("tagname");
                CefString color = child_value->GetString("color");
                CefString text = child_value->GetString("text");
                CefString badge = child_value->GetString("badge");

                CefRefPtr<CefListValue> child_list = CefListValue::Create();
                child_list->SetString(0, tagname);
                child_list->SetString(1, color);
                child_list->SetString(2, text);
                child_list->SetString(3, badge);
                args_list->SetList(i, child_list);
            }

            args->SetList(0, args_list);
            int callbackId = data->GetInt("callbackId");
            // 把callbackId记下来，browser回传数据后renderer根据callbackId从bridge拿回调函数执行通知
            args->SetInt(1, callbackId);

            SendBrowserProcessMessage(browser, PID_BROWSER, msg);
        }
        break;
        case SupportBridgeMethodType::title_bar:
        {
            //int display = data->GetInt("display");
            bool has_title = data->HasKey("title");
            CefString title = data->GetString("title");
            bool has_icon = data->HasKey("icon");
            CefString icon = data->GetString("icon");
            bool has_backgroundColor = data->HasKey("style") && data->GetDictionary("style")->HasKey("backgroundColor");
            CefString backgroundColor;
            if (has_backgroundColor) {
                backgroundColor = data->GetDictionary("style")->GetString("backgroundColor");
            }

            bool has_titleColor = data->HasKey("style") && data->GetDictionary("style")->HasKey("titleColor");
            CefString titleColor;
            if (has_titleColor) {
                titleColor = data->GetDictionary("style")->GetString("titleColor");
            }

            int callbackId = data->GetInt("callbackId");
            int successCallbackId = data->GetInt("successCallbackId");

            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPageTitleBar);
            CefRefPtr<CefListValue> args = msg->GetArgumentList();
            //args->SetInt(0, display);
            args->SetString(0, has_title ? title : L"null" );
            args->SetString(1, has_icon ? icon : L"null");
            args->SetString(2, has_backgroundColor ? backgroundColor : "null");
            args->SetString(3, has_titleColor ? titleColor : "null");

            //CefRefPtr<CefListValue> args_left_list = CefListValue::Create();
            //CefRefPtr<CefListValue> left_list = data->GetList("left");
            //for (int i = 0; i < left_list->GetSize(); i++)
            //{
            //    CefRefPtr<CefDictionaryValue> child_value = left_list->GetDictionary(i);
            //    CefString tagname = child_value->GetString("tagname");
            //    CefString color = child_value->GetString("color");
            //    CefString text = child_value->GetString("text");
            //    CefString badge = child_value->GetString("badge");

            //    CefRefPtr<CefListValue> child_list = CefListValue::Create();
            //    child_list->SetString(0, tagname);
            //    child_list->SetString(1, color);
            //    child_list->SetString(2, text);
            //    child_list->SetString(3, badge);
            //    args_left_list->SetList(i, child_list);
            //}

            CefRefPtr<CefListValue> args_right_list = CefListValue::Create();
            CefRefPtr<CefListValue> right_list = data->GetList("right");
            if (right_list) {
                for (int i = 0; i < right_list->GetSize(); i++)
                {
                    CefRefPtr<CefDictionaryValue> child_value = right_list->GetDictionary(i);
                    bool has_tagname = child_value->HasKey("tagname");
                    CefString tagname = child_value->GetString("tagname");
                    //CefString color = child_value->GetString("color");
                    bool has_text = child_value->HasKey("text");
                    CefString text = child_value->GetString("text");
                    //CefString badge = child_value->GetString("badge");
                    bool has_url = child_value->HasKey("url");
                    CefString url = child_value->GetString("url");

                    CefRefPtr<CefListValue> child_list = CefListValue::Create();
                    child_list->SetString(0, has_tagname ? tagname : "null");
                    //child_list->SetString(1, color);
                    child_list->SetString(1, has_text ? text : "null");
                    child_list->SetString(2, has_url ? url : "null");

                    //child_list->SetString(3, badge);
                    args_right_list->SetList(i, child_list);
                }
            }

            //args->SetList(4, args_left_list);
            args->SetBool(4, right_list ? true : false);
            args->SetList(5, args_right_list);
            // 把callbackId记下来，browser回传数据后renderer根据callbackId从bridge拿回调函数执行通知
            args->SetInt(6, callbackId);
            args->SetInt(7, successCallbackId);
            args->SetString(8, CefString(found_iter->first));

            SendBrowserProcessMessage(browser, PID_BROWSER, msg);
        }
        break;
        case SupportBridgeMethodType::get_all_support:
        {
            int index = 0;
            CefRefPtr<CefV8Value> list = CefV8Value::CreateArray(kSupportBridgeMethodNames.size());
            for (auto& it : kSupportBridgeMethodNames) {
                list->SetValue(index++, CefV8Value::CreateString(it.first));
            }

            CefRefPtr<CefV8Value> param = CefV8Value::CreateObject(nullptr, nullptr);
            param->SetValue("data", list, V8_PROPERTY_ATTRIBUTE_NONE);

            InvokeJsCallbackSync(context, callback, param);
        }
        break;
        case SupportBridgeMethodType::get_container_info:
        {
            int callbackId = data->GetInt("callbackId");

            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPageGetContainerInfo);
            CefRefPtr<CefListValue> args = msg->GetArgumentList();
            // 把callbackId记下来，browser回传数据后renderer根据callbackId从bridge拿回调函数执行通知
            args->SetInt(0, callbackId);
            args->SetString(1, CefString(found_iter->first));

            SendBrowserProcessMessage(browser, PID_BROWSER, msg);
        }
        break;
        case SupportBridgeMethodType::import:
        {
            CefString name_space = data->GetString("namespace");

            // 先获取当前namespace对应的bridge列表
            std::map<std::string, std::vector<bridge_method>>::const_iterator it =
                kSupportBridgeNamespaceMapping.find(name_space);
            if (it != kSupportBridgeNamespaceMapping.end()) {
                for (auto& index : it->second) {
                    kSupportBridgeMethodNames[index.name] = index.type;
                }

                CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPageImport);
                CefRefPtr<CefListValue> args = msg->GetArgumentList();
                args->SetString(0, CefString(found_iter->first));
                args->SetString(1, name_space);

                SendBrowserProcessMessage(browser, PID_BROWSER, msg);
            }

            InvokeJsCallbackSync(context, callback, nullptr);
        }
        break;
        case SupportBridgeMethodType::forward:
        {
            CefString topage = data->GetString("topage");
            CefString type = data->GetString("type");
            int callbackId = data->GetInt("callbackId");
            int successCallbackId = data->GetInt("successCallbackId");

            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPageForward);
            CefRefPtr<CefListValue> args = msg->GetArgumentList();
            args->SetString(0, topage);
            args->SetString(1, type);
            // 把callbackId记下来，browser回传数据后renderer根据callbackId从bridge拿回调函数执行通知
            args->SetInt(2, callbackId);
            args->SetInt(3, successCallbackId);
            args->SetString(4, CefString(found_iter->first));

            SendBrowserProcessMessage(browser, PID_BROWSER, msg);
        }
        break;
        case SupportBridgeMethodType::local_cache:
        {
            CefString operation = data->GetString("operation");
            CefString key = data->GetString("key");
            CefString value = data->GetString("value");
            int callbackId = data->GetInt("callbackId");
            int successCallbackId = data->GetInt("successCallbackId");

            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPageLocalCache);
            CefRefPtr<CefListValue> args = msg->GetArgumentList();
            args->SetString(0, operation);
            args->SetString(1, key);
            args->SetString(2, value);
            // 把callbackId记下来，browser回传数据后renderer根据callbackId从bridge拿回调函数执行通知
            args->SetInt(3, callbackId);
            args->SetInt(4, successCallbackId);
            args->SetString(5, CefString(found_iter->first));

            SendBrowserProcessMessage(browser, PID_BROWSER, msg);
        }
        break;
        case SupportBridgeMethodType::open_in_app:
        {
            CefString params = data->GetString("params");
            CefString moduleName = "open_in_app";  // moduleName固定为open_in_app，协议与arouse中的open_in_app一致
            bool closeCurrent = !!data->GetInt("closeCurrent");

            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kBilibiliPageArouse);
            CefRefPtr<CefListValue> args = msg->GetArgumentList();
            args->SetString(0, params);
            args->SetString(1, moduleName);
            args->SetBool(2, closeCurrent);
            args->SetString(3, CefString(found_iter->first));

            SendBrowserProcessMessage(browser, PID_BROWSER, msg);
            InvokeJsCallbackSync(context, callback, nullptr);
        }
        break;
        default:
            support = false;
            break;
        }

        return support;
    }

}