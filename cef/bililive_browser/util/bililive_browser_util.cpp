// cef_proxy_dll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "bililive_browser_util.h"

#include "include/wrapper/cef_helpers.h"

#include "public/bililive_browser_ipc_messages.h"

/*
 *   当前B站的js的postMessage方法是判断native是安卓就将method&&data以常规的string&&map形式的json字串传进来，
 *   并且其data字段是map；
 *   {"method":"global.getAllSupport","data":{"callbackId":1}}
 *   否则就以object的方式将method&&data传入，并且其data字段是string，坑爹，所以对于传入的是object的情况要对其
 *   data字段做一下处理,
 *   {"method":"global.getAllSupport","data":"{\"callbackId\":1}"}
 *
 *   if (window.selfBrowser.version.android)
 *   {
 *       postMessage(JSON.stringify({
 *         method: option.method,
 *         data : Object.assign(option.data, {
 *           callbackId: window.BiliJsBridge.selfCallbackId++
 *         })
 *                                  }));
 *   }
 *   else
 *   {
 *       postMessage({
 *         method: option.method,
 *         data : JSON.stringify(Object.assign(option.data, {
 *           callbackId: window.BiliJsBridge.selfCallbackId++
 *         }))
 *                   });
 *   }
 */
CefString GetBilibiliPostMessageJsonString(CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Value> value,
    CefRefPtr<CefV8Value>* out_callback/* = nullptr*/)
{
    CEF_REQUIRE_RENDERER_THREAD();

    CefString msg_str;
    if (value->IsObject())
    {
        // JavaScript的JSON.stringify()方法能将js object转成json字串，但当前这个蛋疼的data格式（字串而不是map），
        // 导致即使用stringify得到json字串还是要再对data做一下处理，就干脆从头开始就我们自己处理算了
        /*CefRefPtr<CefV8Value> window_obj = context->GetGlobal();
        CefRefPtr<CefV8Value> JSON_obj = window_obj->GetValue("JSON");
        if (JSON_obj && JSON_obj->IsObject())
        {
            CefRefPtr<CefV8Value> stringify = JSON_obj->GetValue("stringify");
            if (stringify && stringify->IsFunction())
            {
                CefV8ValueList stringify_args;
                stringify_args.push_back(value);
                CefRefPtr<CefV8Value> retval = stringify->ExecuteFunction(NULL, stringify_args);
                if (retval && retval->IsValid() && retval->IsString())
                {
                    msg_str = retval->GetStringValue();
                }
            }
        }*/

        CefRefPtr<CefV8Value> method = value->GetValue("method");
        CefString method_str = method->GetStringValue();

        // test for watch jsbridge object details [
        /*std::vector<CefString> keys;
        value->GetKeys(keys);
        DLOG(INFO) << "method: " << method_str;
        for (auto& iter : keys)
        {
            DLOG(INFO) << "\t" << iter;
        }*/
        // ]

        CefRefPtr<CefDictionaryValue> data_dict;
        CefRefPtr<CefV8Value> data = value->GetValue("data");
        CefString data_str = data->GetStringValue();
        if (!data_str.empty())
        {
            CefRefPtr<CefValue> data_value = CefParseJSON(data_str, JSON_PARSER_RFC);
            if (data_value)
            {
                data_dict = data_value->GetDictionary();
            }
        }
        if (!data_dict)
        {
            data_dict = CefDictionaryValue::Create();
        }

        CefRefPtr<CefValue> root_node = CefValue::Create();
        CefRefPtr<CefDictionaryValue> dict_node = CefDictionaryValue::Create();
        dict_node->SetString("method", method_str);
        dict_node->SetDictionary("data", data_dict);
        root_node->SetDictionary(dict_node);
        msg_str = CefWriteJSON(root_node, JSON_WRITER_DEFAULT);

        CefRefPtr<CefV8Value> callback = value->GetValue("callback");
        if (callback && callback->IsValid() && callback->IsFunction())
        {
            if (out_callback)
            {
                *out_callback = callback;
            }
        }
    }
    else if (value->IsString())
    {
        msg_str = value->GetStringValue();
    }
    return msg_str;
}

CefRefPtr<CefV8Value> GetBiliJsBridgeCallback(CefRefPtr<CefV8Context> context, int callbackId)
{
    CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
    if (globalObj->HasValue("BiliJsBridge"))
    {
        CefRefPtr<CefV8Value> BiliJsBridge = globalObj->GetValue("BiliJsBridge");
        if (BiliJsBridge->HasValue("callbacks"))
        {
            CefRefPtr<CefV8Value> callbacks = BiliJsBridge->GetValue("callbacks");
            if (callbacks->IsArray())
            {
                int arylen = callbacks->GetArrayLength();
                if ((callbackId > 0) && (callbackId <= arylen))
                {
                    CefRefPtr<CefV8Value> callback_item = callbacks->GetValue(callbackId - 1);
                    if (callback_item->HasValue("callbackId") && callback_item->HasValue("callback"))
                    {
                        int cbid = callback_item->GetValue("callbackId")->GetIntValue();
                        if (cbid == callbackId)
                        {
                            CefRefPtr<CefV8Value> callback = callback_item->GetValue("callback");
                            if (callback->IsFunction())
                            {
                                return callback;
                            }
                        }
                    }
                }
            }
        }
    }

    return nullptr;
}

void InvokeJsCallbackSync(CefRefPtr<CefV8Context> context, CefRefPtr<CefV8Value> callback, CefRefPtr<CefV8Value> param)
{
    if (callback && callback->IsFunction()) {
        context->Enter();
        CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
        if (globalObj->HasValue("BiliJsBridge"))
        {
            if (param) {
                callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { param });
            } else {
                callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { });
            }
        }

        context->Exit();
    }
}

bool ScheduleCommonBrowserIPCMessage(CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> message)
{
    bool handled = false;

    CefString msg_name = message->GetName();
    if (msg_name == ipc_messages::kBilibiliPageGetInfo)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        auto dict = args->GetDictionary(0);

        CefRefPtr<CefV8Context> context = browser->GetMainFrame()->GetV8Context();

        context->Enter();

        int callbackId = dict->GetInt("callbackId");
        CefRefPtr<CefV8Value> callback = GetBiliJsBridgeCallback(context, callbackId);
        if (callback)
        {
            CefRefPtr<CefV8Value> param = CefV8Value::CreateObject(nullptr, nullptr);
            param->SetValue("type", CefV8Value::CreateString(dict->GetString("type")), V8_PROPERTY_ATTRIBUTE_NONE);
            param->SetValue("params", CefV8Value::CreateString(dict->GetString("params")), V8_PROPERTY_ATTRIBUTE_NONE);
            param->SetValue("info", CefV8Value::CreateString(dict->GetString("info")), V8_PROPERTY_ATTRIBUTE_NONE);
            CefRefPtr<CefV8Value> info = CefV8Value::CreateString(dict->GetString("info"));
            // test [
            //param->SetValue("bili_jct", CefV8Value::CreateString("livehime_test"), V8_PROPERTY_ATTRIBUTE_NONE);
            // ]

            CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
            if (globalObj->HasValue("BiliJsBridge"))
            {
                if (dict->GetString("type") == "check_talk_subject")
                {
                    callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { info });
                }
                else
                {
                    callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { param });
                }
            }
        }

        context->Exit();

        handled = true;
    }
    else if (msg_name == ipc_messages::kBilibiliPageSubBroadcast)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        auto dict = args->GetDictionary(0);

        CefRefPtr<CefV8Context> context = browser->GetMainFrame()->GetV8Context();

        context->Enter();

        int callbackId = dict->GetInt("successCallbackId");
        CefRefPtr<CefV8Value> callback = GetBiliJsBridgeCallback(context, callbackId);
        if (callback)
        {
            CefRefPtr<CefV8Value> param = CefV8Value::CreateObject(nullptr, nullptr);
            param->SetValue("type", CefV8Value::CreateString(dict->GetString("type")), V8_PROPERTY_ATTRIBUTE_NONE);
            param->SetValue("cmd", CefV8Value::CreateString(dict->GetString("cmd")), V8_PROPERTY_ATTRIBUTE_NONE);

            param->SetValue("info", CefV8Value::CreateString(dict->GetString("info")), V8_PROPERTY_ATTRIBUTE_NONE);
            CefRefPtr<CefV8Value> info = CefV8Value::CreateString(dict->GetString("info"));

            CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
            if (globalObj->HasValue("BiliJsBridge"))
            {
                callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { info });
            }
        }

        context->Exit();

        handled = true;
    }
    else if (msg_name == ipc_messages::kBilibiliPageAlert)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        auto dict = args->GetDictionary(0);

        CefRefPtr<CefV8Context> context = browser->GetMainFrame()->GetV8Context();
        context->Enter();

        cef_proxy::callback_type callback_type = (cef_proxy::callback_type)dict->GetInt(cef_proxy::kCallbackTypeField);
        switch (callback_type) {
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK:
        {
            int callbackId = dict->GetInt("callbackId");
            CefRefPtr<CefV8Value> callback = GetBiliJsBridgeCallback(context, callbackId);
            if (callback)
            {
                CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
                if (globalObj->HasValue("BiliJsBridge"))
                {
                    callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { });
                }
            }

            break;
        }
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK_DATA:
        {
            int callbackId = dict->GetInt("callbackId");
            CefRefPtr<CefV8Value> callback = GetBiliJsBridgeCallback(context, callbackId);
            if (callback)
            {
                CefRefPtr<CefV8Value> param = CefV8Value::CreateObject(nullptr, nullptr);
                param->SetValue("hidden", CefV8Value::CreateInt(dict->GetInt("hidden")), V8_PROPERTY_ATTRIBUTE_NONE);

                CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
                if (globalObj->HasValue("BiliJsBridge"))
                {
                    callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { param });
                }
            }

            break;
        }
        case cef_proxy::callback_type::CALL_BACK_TYPE_SUCCESS_DATA:
        {
            int callbackId = dict->GetInt("successCallbackId");
            CefRefPtr<CefV8Value> callback = GetBiliJsBridgeCallback(context, callbackId);
            if (callback)
            {
                CefRefPtr<CefV8Value> param = CefV8Value::CreateObject(nullptr, nullptr);
                param->SetValue("type", CefV8Value::CreateString(dict->GetString("type")), V8_PROPERTY_ATTRIBUTE_NONE);

                CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
                if (globalObj->HasValue("BiliJsBridge"))
                {
                    callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { param });
                }
            }

            break;
        }
        default:
            //DCHECK(FALSE);
            break;
        }

        context->Exit();
        handled = true;
    }
    else if (msg_name == ipc_messages::kBilibiliPageToast)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        auto dict = args->GetDictionary(0);

        CefRefPtr<CefV8Context> context = browser->GetMainFrame()->GetV8Context();
        context->Enter();

        cef_proxy::callback_type callback_type = (cef_proxy::callback_type)dict->GetInt(cef_proxy::kCallbackTypeField);
        switch (callback_type) {
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK:
        {
            int callbackId = dict->GetInt("callbackId");
            CefRefPtr<CefV8Value> callback = GetBiliJsBridgeCallback(context, callbackId);
            if (callback)
            {
                CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
                if (globalObj->HasValue("BiliJsBridge"))
                {
                    callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { });
                }
            }

            break;
        }
        default:
            //DCHECK(FALSE);
            break;
        }

        context->Exit();
        handled = true;
    }
    else if (msg_name == ipc_messages::kBilibiliPageSetTitle)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        auto dict = args->GetDictionary(0);

        CefRefPtr<CefV8Context> context = browser->GetMainFrame()->GetV8Context();
        context->Enter();

        cef_proxy::callback_type callback_type = (cef_proxy::callback_type)dict->GetInt(cef_proxy::kCallbackTypeField);
        switch (callback_type) {
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK:
        {
            int callbackId = dict->GetInt("callbackId");
            CefRefPtr<CefV8Value> callback = GetBiliJsBridgeCallback(context, callbackId);
            if (callback)
            {
                CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
                if (globalObj->HasValue("BiliJsBridge"))
                {
                    callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { });
                }
            }

            break;
        }
        default:
            //DCHECK(FALSE);
            break;
        }

        context->Exit();
        handled = true;
    }
    else if (msg_name == ipc_messages::kBilibiliPageSetNavMenu)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        auto dict = args->GetDictionary(0);

        CefRefPtr<CefV8Context> context = browser->GetMainFrame()->GetV8Context();
        context->Enter();

        cef_proxy::callback_type callback_type = (cef_proxy::callback_type)dict->GetInt(cef_proxy::kCallbackTypeField);
        switch (callback_type) {
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK:
        {
            int callbackId = dict->GetInt("callbackId");
            CefRefPtr<CefV8Value> callback = GetBiliJsBridgeCallback(context, callbackId);
            if (callback)
            {
                CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
                if (globalObj->HasValue("BiliJsBridge"))
                {
                    callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { });
                }
            }

            break;
        }
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK_DATA:
        {
            int callbackId = dict->GetInt("callbackId");
            CefRefPtr<CefV8Value> callback = GetBiliJsBridgeCallback(context, callbackId);
            if (callback)
            {
                CefRefPtr<CefV8Value> param = CefV8Value::CreateObject(nullptr, nullptr);
                param->SetValue("tagname", CefV8Value::CreateString(dict->GetString("tagname")), V8_PROPERTY_ATTRIBUTE_NONE);

                CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
                if (globalObj->HasValue("BiliJsBridge"))
                {
                    callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { param });
                }
            }

            break;
        }
        default:
            //DCHECK(FALSE);
            break;
        }

        context->Exit();
        handled = true;
    }
    else if (msg_name == ipc_messages::kBilibiliPageTitleBar)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        auto dict = args->GetDictionary(0);

        CefRefPtr<CefV8Context> context = browser->GetMainFrame()->GetV8Context();
        context->Enter();

        cef_proxy::callback_type callback_type = (cef_proxy::callback_type)dict->GetInt(cef_proxy::kCallbackTypeField);
        switch (callback_type) {
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK:
        {
            int callbackId = dict->GetInt("callbackId");
            CefRefPtr<CefV8Value> callback = GetBiliJsBridgeCallback(context, callbackId);
            if (callback)
            {
                CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
                if (globalObj->HasValue("BiliJsBridge"))
                {
                    callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { });
                }
            }

            break;
        }
        case cef_proxy::callback_type::CALL_BACK_TYPE_SUCCESS_DATA:
        {
            int callbackId = dict->GetInt("successCallbackId");
            CefRefPtr<CefV8Value> callback = GetBiliJsBridgeCallback(context, callbackId);
            if (callback)
            {
                CefRefPtr<CefV8Value> param = CefV8Value::CreateObject(nullptr, nullptr);
                param->SetValue("tagname", CefV8Value::CreateString(dict->GetString("tagname")), V8_PROPERTY_ATTRIBUTE_NONE);

                CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
                if (globalObj->HasValue("BiliJsBridge"))
                {
                    callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { param });
                }
            }

            break;
        }
        default:
            //DCHECK(FALSE);
            break;
        }

        context->Exit();
        handled = true;
    }
    else if (msg_name == ipc_messages::kBilibiliPageGetContainerInfo)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        auto dict = args->GetDictionary(0);

        CefRefPtr<CefV8Context> context = browser->GetMainFrame()->GetV8Context();
        context->Enter();

        cef_proxy::callback_type callback_type = (cef_proxy::callback_type)dict->GetInt(cef_proxy::kCallbackTypeField);
        switch (callback_type) {
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK_DATA:
        {
            int callbackId = dict->GetInt("callbackId");
            CefRefPtr<CefV8Value> callback = GetBiliJsBridgeCallback(context, callbackId);
            if (callback)
            {
                CefRefPtr<CefV8Value> param = CefV8Value::CreateObject(nullptr, nullptr);
                param->SetValue("platform", CefV8Value::CreateString(dict->GetString("platform")), V8_PROPERTY_ATTRIBUTE_NONE);
                param->SetValue("appVersion", CefV8Value::CreateString(dict->GetString("appVersion")), V8_PROPERTY_ATTRIBUTE_NONE);
                param->SetValue("appBuild", CefV8Value::CreateString(dict->GetString("appBuild")), V8_PROPERTY_ATTRIBUTE_NONE);
                param->SetValue("osVersion", CefV8Value::CreateString(dict->GetString("osVersion")), V8_PROPERTY_ATTRIBUTE_NONE);
                param->SetValue("osArch", CefV8Value::CreateString(dict->GetString("osArch")), V8_PROPERTY_ATTRIBUTE_NONE);
                param->SetValue("cpu", CefV8Value::CreateString(dict->GetString("cpu")), V8_PROPERTY_ATTRIBUTE_NONE);
                param->SetValue("memory", CefV8Value::CreateString(dict->GetString("memory")), V8_PROPERTY_ATTRIBUTE_NONE);
                param->SetValue("navBarHeight", CefV8Value::CreateInt(dict->GetInt("navBarHeight")), V8_PROPERTY_ATTRIBUTE_NONE);
                param->SetValue("time", CefV8Value::CreateString(dict->GetString("time")), V8_PROPERTY_ATTRIBUTE_NONE);

                CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
                if (globalObj->HasValue("BiliJsBridge"))
                {
                    callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { param });
                }
            }

            break;
        }
        default:
            //DCHECK(FALSE);
            break;
        }

        context->Exit();
        handled = true;
    }
    else if (msg_name == ipc_messages::kBilibiliPageLocalCache)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        auto dict = args->GetDictionary(0);

        CefRefPtr<CefV8Context> context = browser->GetMainFrame()->GetV8Context();
        context->Enter();

        cef_proxy::callback_type callback_type = (cef_proxy::callback_type)dict->GetInt(cef_proxy::kCallbackTypeField);
        switch (callback_type) {
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK:
        {
            int callbackId = dict->GetInt("callbackId");
            CefRefPtr<CefV8Value> callback = GetBiliJsBridgeCallback(context, callbackId);
            if (callback)
            {
                CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
                if (globalObj->HasValue("BiliJsBridge"))
                {
                    callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { });
                }
            }

            break;
        }
        case cef_proxy::callback_type::CALL_BACK_TYPE_SUCCESS_DATA:
        {
            int callbackId = dict->GetInt("successCallbackId");
            CefRefPtr<CefV8Value> callback = GetBiliJsBridgeCallback(context, callbackId);
            if (callback)
            {
                CefRefPtr<CefV8Value> param = CefV8Value::CreateObject(nullptr, nullptr);
                param->SetValue("value", CefV8Value::CreateString(dict->GetString("value")), V8_PROPERTY_ATTRIBUTE_NONE);

                CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
                if (globalObj->HasValue("BiliJsBridge"))
                {
                    callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { param });
                }
            }

            break;
        }
        default:
            //DCHECK(FALSE);
            break;
        }

        context->Exit();
        handled = true;
    }
    else if (msg_name == ipc_messages::kBilibiliPageForward)
    {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        auto dict = args->GetDictionary(0);

        CefRefPtr<CefV8Context> context = browser->GetMainFrame()->GetV8Context();
        context->Enter();

        cef_proxy::callback_type callback_type = (cef_proxy::callback_type)dict->GetInt(cef_proxy::kCallbackTypeField);
        switch (callback_type) {
        case cef_proxy::callback_type::CALL_BACK_TYPE_CALLBACK:
        {
            int callbackId = dict->GetInt("callbackId");
            CefRefPtr<CefV8Value> callback = GetBiliJsBridgeCallback(context, callbackId);
            if (callback)
            {
                CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
                if (globalObj->HasValue("BiliJsBridge"))
                {
                    callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { });
                }
            }

            break;
        }
        case cef_proxy::callback_type::CALL_BACK_TYPE_SUCCESS_DATA:
        {
            int callbackId = dict->GetInt("successCallbackId");
            CefRefPtr<CefV8Value> callback = GetBiliJsBridgeCallback(context, callbackId);
            if (callback)
            {
                CefRefPtr<CefV8Value> globalObj = context->GetGlobal();
                if (globalObj->HasValue("BiliJsBridge"))
                {
                    callback->ExecuteFunction(globalObj->GetValue("BiliJsBridge"), { });
                }
            }

            break;
        }
        default:
            //DCHECK(FALSE);
            break;
        }

        context->Exit();
        handled = true;
    }

    return handled;
}
