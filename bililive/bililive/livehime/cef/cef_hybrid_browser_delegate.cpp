#include "cef_hybrid_browser_delegate.h"

#include "base/memory/singleton.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/prefs/pref_service.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/pref_names.h"


CefHybridBrowserDelegate::CefHybridBrowserDelegate()
{
}

CefHybridBrowserDelegate::~CefHybridBrowserDelegate()
{
}

CefHybridBrowserDelegate* CefHybridBrowserDelegate::GetInstance()
{
    return Singleton<CefHybridBrowserDelegate>::get();
}

void CefHybridBrowserDelegate::DelayDispatchJsEvent(const cef_proxy::browser_bind_data& bind_data,
    const std::string& ipc_msg_name, const cef_proxy::calldata arguments)
{
    bind_data_ = bind_data;
    ipc_msg_name_ = ipc_msg_name;
    arguments_ = arguments;
}

void CefHybridBrowserDelegate::OnWebBrowserWidgetDestroyed(views::Widget* widget)
{
    CefProxyWrapper::GetInstance()->DispatchJsEvent(bind_data_,
        ipc_msg_name_, &arguments_);
}