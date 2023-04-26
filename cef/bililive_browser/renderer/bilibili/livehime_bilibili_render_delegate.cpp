#include "stdafx.h"
#include "livehime_bilibili_render_delegate.h"

#include "bilibase/basic_types.h"

#include "bililive_browser/public/bililive_browser_js_values.h"
#include "bililive_browser/public/bililive_browser_ipc_messages.h"

#include "cef_proxy_dll/public/livehime_cef_proxy_calldata.h"

#include "public/bililive_cef_headers.h"
#include "util/bililive_browser_util.h"


namespace
{
    const char kModuleObject[] = "livehime_bilibili";
}

LivehimeBilibiliRenderDelegate::LivehimeBilibiliRenderDelegate()
{
}

LivehimeBilibiliRenderDelegate::~LivehimeBilibiliRenderDelegate()
{
}

void LivehimeBilibiliRenderDelegate::OnContextCreated(CefRefPtr<LivehimeCefAppRender> app,
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context)
{
    /*
    *   window.biliInject.postMessage(string)
    *   {
    *       window.livehime_bilibili.postMessage(string)
    *   }
    */

    bool result = js_bridge::InjectModuleObjToV8Context(context, this,
        cef_proxy::client_handler_type::bilibili, kModuleObject);
    DCHECK(result);

    kSupportBridgeMethodNames_ = js_bridge::kSupportBridgeMethodNamesDefalut;
}

bool LivehimeBilibiliRenderDelegate::OnProcessMessageReceived(CefRefPtr<LivehimeCefAppRender> app,
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{
    if (ScheduleCommonBrowserIPCMessage(browser, message))
    {
        return true;
    }
    return false;
}

bool LivehimeBilibiliRenderDelegate::Execute(const CefString& name,
    CefRefPtr<CefV8Value> object,
    const CefV8ValueList& arguments,
    CefRefPtr<CefV8Value>& retval,
    CefString& exception)
{
    if (js_bridge::IsInvokeOnModuleObj(object, cef_proxy::client_handler_type::bilibili))
    {
        bool handled = false;
        if (!js_bridge::ScheduleCommonJsFunciton(this, kSupportBridgeMethodNames_, kModuleObject, name, arguments, handled))
        {
            LOG(WARNING) << kModuleObject << " not support js func: " << name.ToString();
        }

        return true;
    }

    return false;
}
