#include "bililive/bililive/ui/views/livehime/lottery/lottery_main_web_view.h"

#include <shellapi.h>

#include "base/bind.h"
#include "base/notification/notification_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/livehime/lottery/lottery_common_data.h"
#include "bililive/bililive/ui/views/controls/bililive_native_widget.h"
#include "bililive/bililive/ui/views/controls/browser/livehime_web_browser_view.h"
#include "bililive/bililive/ui/views/livehime/controls/livehime_theme_common.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"
#include "cef/bililive_browser/public/bililive_browser_ipc_messages.h"


namespace
{
    LotteryMainWebView* g_single_instance = nullptr;
}

void LotteryMainWebView::ShowWindow(views::Widget *parent, const std::string& web_url)
{
    hybrid_ui::LivehimeWebUrlDetails url_ex = hybrid_ui::GetUrlExInfo(web_url);

    if (!g_single_instance)
    {
        views::Widget *widget_ = new views::Widget();
        views::Widget::InitParams params;
        params.opacity = views::Widget::InitParams::OPAQUE_WINDOW;
        params.native_widget = new BililiveNativeWidgetWin(widget_);
        params.parent = parent->GetNativeView();

        g_single_instance = new LotteryMainWebView(url_ex);
        DoModalWidget(g_single_instance, widget_, params);
    }
    else
    {
        if (g_single_instance->GetWidget())
        {
            g_single_instance->GetWidget()->Activate();
        }
    }
}

LotteryMainWebView::LotteryMainWebView(const hybrid_ui::LivehimeWebUrlDetails& url_ex)
    : LivehimeHybridWebBrowserView(url_ex, false, WebViewPopupType::System, cef_proxy::client_handler_type::lottery)
    , weakptr_factory_(this)
{
    LotteryPresenter::GetInstance()->AddObserver(this);
    SetDefaultPrefSize(gfx::Size(GetLengthByDPIScale(530), GetLengthByDPIScale(635)));
}

LotteryMainWebView::~LotteryMainWebView() {
    LotteryPresenter::GetInstance()->RemoveObserver(this);
    g_single_instance = nullptr;
}

bool LotteryMainWebView::OnWebBrowserMsgReceived(const std::string& msg_name, const cef_proxy::calldata* data)
{
    bool handled = false;
    if (msg_name == ipc_messages::kLotteryClosed)
    {
        handled = true;
        GetWidget()->Close();
    }
    else if (msg_name == ipc_messages::kLotteryStatusChangedR)
    {
        handled = true;
        LotteryInfo info;
        info.status = data->at("status").numeric_union.int_;
        info.time = data->at("time").numeric_union.int_;

        LOG(INFO) << "[Lottery] " << "Receive" <<
            "status" << info.status <<
            "time" << info.time;

        base::NotificationService::current()->Notify(bililive::NOTIFICATION_LIVEHIME_LOTTERY_STATUS_CHANGED_R,
            base::NotificationService::AllSources(),
            base::Details<LotteryInfo>(&info));
    }

    return handled;
}

void LotteryMainWebView::OnStatusChanged(const std::string change_info)
{
    if (web_browser_view() && web_browser_view()->browser_bind_data())
    {
        cef_proxy::calldata data;
        cef_proxy::calldata_filed data_filed;
        data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;

        data_filed.str_ = change_info;
        data["data"] = data_filed;

        LOG(INFO) << "[Lottery] " << "Send" <<
            "data" << change_info;

        CefProxyWrapper::GetInstance()->ExecuteJSFunctionWithKV(
            *web_browser_view()->browser_bind_data(),
            ipc_messages::kLotteryStatusChangedS,
            &data);
    }
    else
    {
        LOG(WARNING) << "[Lottery] " << "web core not ready yet!";
    }
}

