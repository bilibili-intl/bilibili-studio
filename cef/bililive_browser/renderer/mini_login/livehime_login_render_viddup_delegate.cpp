#include "stdafx.h"
#include "livehime_login_render_viddup_delegate.h"

#include <fstream>

#include "bilibase/basic_types.h"

#include "bililive_browser/public/bililive_browser_js_values.h"
#include "bililive_browser/public/bililive_browser_ipc_messages.h"

#include "cef_proxy_dll/public/livehime_cef_proxy_calldata.h"

#include "public/bililive_cef_headers.h"
#include "util/bililive_browser_util.h"

namespace
{
    const char kModuleObject[] = "SendMsgToHost";

    // 注册几个我们在乎的事件钩子
    enum LoginEventType
    {
        //LoginSuccess,   // (string:login_type)
        //Cancel,         // (void)
        //SwitchLogin,    // (int width, int height)
        //SecondaryValidationResult,  // (void)
        //Login,          // (string:login_type)
        //ChangeSize,     // (int width, int height)
    };

    const std::map<LoginEventType, std::string> kSupportJsFunctionNames{
        //{ LoginSuccess, "LoginSuccess"},
        //{ Cancel, "Cancel"},
        //{ SwitchLogin, "SwitchLogin"},
        //{ SecondaryValidationResult, "SecondaryValidationResult"},
        //{ Login, "login"},
        //{ ChangeSize, "changeSize"},
    };

    CefString GetSuccessInfoString(CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Value> value)
    {
        CefString msg_str;
        if (value->IsObject())
        {
            //CefRefPtr<CefV8Value> root_type = value->GetValue("type");

            CefRefPtr<CefV8Value> by;
            CefRefPtr<CefV8Value> type;
            CefRefPtr<CefV8Value> root_type = value->GetValue("type");
            if (root_type)
            {
                by = root_type->GetValue("by");
                type = root_type->GetValue("type");
            }

            CefRefPtr<CefValue> root_node = CefValue::Create();

            CefRefPtr<CefDictionaryValue> dict = CefDictionaryValue::Create();
            dict->SetString("by", by->GetStringValue());
            dict->SetString("type", type->GetStringValue());

            root_node->SetDictionary(dict);

            msg_str = CefWriteJSON(root_node, JSON_WRITER_DEFAULT);
        }
        else if (value->IsString())
        {
            msg_str = value->GetStringValue();
        }
        return msg_str;
    }
}

LivehimeLoginRenderViddupDelegate::LivehimeLoginRenderViddupDelegate()
{
}

LivehimeLoginRenderViddupDelegate::~LivehimeLoginRenderViddupDelegate()
{
}

void LivehimeLoginRenderViddupDelegate::OnContextCreated(CefRefPtr<LivehimeCefAppRender> app,
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context)
{
    bool result = true;
    //CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
    //CefRefPtr<CefV8Value> inject = CefV8Value::CreateObject(nullptr, nullptr);
    //{
    //    for (auto& iter : kSupportJsFunctionNames)
    //    {
    //        CefRefPtr<CefV8Value> js_func = CefV8Value::CreateFunction(iter.second, this);
    //        result &= inject->SetValue(iter.second, js_func, V8_PROPERTY_ATTRIBUTE_NONE);
    //        LOG(INFO) << "[mini-login-viddup] " << "OnContextCreated: js_func=" << iter.second;
    //    }

    //    CefRefPtr<CefV8Value> user_data = CefV8Value::CreateInt(bilibase::enum_cast(cef_proxy::client_handler_type::mini_login));
    //    result &= inject->SetValue(js_values::kLivehimeUserData, user_data, V8_PROPERTY_ATTRIBUTE_NONE);
    //}

    //result &= globalObj->SetValue(kModuleObject, inject, V8_PROPERTY_ATTRIBUTE_NONE);
    auto object = context->GetGlobal();
    auto function = CefV8Value::CreateFunction(kModuleObject, this);
    result &= object->SetValue(kModuleObject, function, V8_PROPERTY_ATTRIBUTE_NONE);
    DCHECK(result);
}

bool LivehimeLoginRenderViddupDelegate::OnProcessMessageReceived(CefRefPtr<LivehimeCefAppRender> app,
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{
    //CefString msg_name = message->GetName();

    return false;
}

bool LivehimeLoginRenderViddupDelegate::Execute(const CefString& name,
    CefRefPtr<CefV8Value> object,
    const CefV8ValueList& arguments,
    CefRefPtr<CefV8Value>& retval,
    CefString& exception)
{
    LOG(INFO) << "[mini-login-viddup] " << "Execute: name=" << name.ToString();
    if (name == "SendMsgToHost") {
        CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
        CefRefPtr<CefBrowser> browser = context->GetBrowser();

        if (arguments.size() == 1) {
            auto msg = arguments.at(0)->GetStringValue();
            LOG(INFO) << "[mini-login-viddup] " << "Execute: arguments.size() == 1, msg=" << msg.ToString();

            //m_browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, CefProcessMessage::Create(msg));

            //CefString login_type = GetSuccessInfoString(context, arguments[0]);
            //CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kMiniLoginSuccess);
            //CefRefPtr<CefListValue> args = msg->GetArgumentList();
            //args->SetString(0, login_type);
            //SendBrowserProcessMessage(browser, PID_BROWSER, msg);
        } else if (arguments.size() == 2) {
            auto dataType = arguments.at(0)->GetStringValue();
            auto data = arguments.at(1)->GetStringValue();
            LOG(INFO) << "[mini-login-viddup] " << "Execute: arguments.size() == 2, dataType=" << dataType.ToString() << ", data=" << data.ToString();

            //msg format: dataType-data
            auto msg = dataType.ToString() + "-" + data.ToString();
            LOG(INFO) << "[mini-login-viddup] " << "Execute: arguments.size() == 2, msg=" << msg;
            //m_browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, CefProcessMessage::Create(msg));

            if (dataType == "login") {
                CefRefPtr<CefProcessMessage> ipc_msg = CefProcessMessage::Create(ipc_messages::kMiniLoginSuccess);
                CefRefPtr<CefListValue> args = ipc_msg->GetArgumentList();
                args->SetString(0, data);
                SendBrowserProcessMessage(browser, PID_BROWSER, ipc_msg);
                LOG(INFO) << "[mini-login-viddup] " << "Execute: SendBrowserProcessMessage kMiniLoginSuccess";
            } else if (dataType == "cancelLogin") {
                CefRefPtr<CefProcessMessage> ipc_msg = CefProcessMessage::Create(ipc_messages::kMiniLoginCancel);
                CefRefPtr<CefListValue> args = ipc_msg->GetArgumentList();
                args->SetString(0, data);
                SendBrowserProcessMessage(browser, PID_BROWSER, ipc_msg);
                LOG(INFO) << "[mini-login-viddup] " << "Execute: SendBrowserProcessMessage kMiniLoginCancel";
            } else if (dataType == "changeSize") {
                CefRefPtr<CefProcessMessage> ipc_msg = CefProcessMessage::Create(ipc_messages::kMiniLoginChangeLoginMode);
                CefRefPtr<CefListValue> args = ipc_msg->GetArgumentList();
                int w = 0, h = 0;  
                args->SetInt(0, w);
                args->SetInt(1, h);            
                SendBrowserProcessMessage(browser, PID_BROWSER, ipc_msg);
                LOG(INFO) << "[mini-login-viddup] " << "Execute: SendBrowserProcessMessage kMiniLoginChangeLoginMode";
            }
        }

        return true;
    }

    //if (js_bridge::IsInvokeOnModuleObj(object, cef_proxy::client_handler_type::mini_login))
    //{
    //    bool handled = false;

    //    auto iter = std::find_if(kSupportJsFunctionNames.begin(), kSupportJsFunctionNames.end(), 
    //        [&](const std::map<LoginEventType, std::string>::value_type& method_pair)->bool {
    //            return (name.ToString() == method_pair.second);
    //        });
    //    if (iter != kSupportJsFunctionNames.end())
    //    {
    //        /* https://info.bilibili.co/pages/viewpage.action?pageId=102989468
    //        *  ----------------------- 数据返回示例 ------------------------
    //        *  LoginSuccess
    //        * {"by":"password","type":"login"}
    //        *  
    //        *  Cancel
    //        *  void
    //        *  
    //        */
    //        CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();

    //        CefRefPtr<CefBrowser> browser = context->GetBrowser();

            //switch (iter->first)
            //{
            //case LoginSuccess:
            //    {
            //        CefString login_type = GetSuccessInfoString(context, arguments[0]);
            //        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kMiniLoginSuccess);
            //        CefRefPtr<CefListValue> args = msg->GetArgumentList();
            //        args->SetString(0, login_type);
            //        SendBrowserProcessMessage(browser, PID_BROWSER, msg);
            //    }
            //    break;
            //case Cancel:
            //    {
            //        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kMiniLoginCancel);
            //        SendBrowserProcessMessage(browser, PID_BROWSER, msg);
            //    }
            //    break;
            //case SwitchLogin:
            //    {
            //        int w = 0, h = 0;
            //        if (arguments.size() >= 2)
            //        {
            //            if (arguments[0]->IsInt())
            //            {
            //                w = arguments[0]->GetIntValue();
            //            }
            //            if (arguments[1]->IsInt())
            //            {
            //                h = arguments[1]->GetIntValue();
            //            }
            //        }
            //        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kMiniLoginChangeLoginMode);
            //        CefRefPtr<CefListValue> args = msg->GetArgumentList();
            //        args->SetInt(0, w);
            //        args->SetInt(1, h);
            //        SendBrowserProcessMessage(browser, PID_BROWSER, msg);
            //    }
            //    break;
            //case SecondaryValidationResult:
            //    {
            //        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create(ipc_messages::kMiniLoginSecVldResult);
            //        SendBrowserProcessMessage(browser, PID_BROWSER, msg);
            //    }
            //    break;
            //default:
            //    break;
            //}

    //        LOG(INFO) << "[mini-login-viddup] " << kModuleObject << " support js func: " << name.ToString();
    //    }
    //    else
    //    {
    //        LOG(WARNING) << "[mini-login] " << kModuleObject << " not support js func: " << name.ToString();
    //    }

    //    return true;
    //}

    return false;
}
