#pragma once

#include "bililive/bililive/livehime/hybrid_ui/hybrid_ui.h"
#include "bililive/bililive/ui/views/livehime/web_browser/livehime_web_browser_widget.h"

class LotteryMainWebView :
    public LivehimeHybridWebBrowserView
{
public:
    static void ShowWindow(views::Widget *parent, const std::string& web_url);

protected:
    // LivehimeWebBrowserDelegate
    bool OnWebBrowserMsgReceived(const std::string& msg_name, const cef_proxy::calldata* data) override;

private:
    explicit LotteryMainWebView(const hybrid_ui::LivehimeWebUrlDetails& url_ex);
    ~LotteryMainWebView();

private:
    base::WeakPtrFactory<LotteryMainWebView> weakptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(LotteryMainWebView);
};