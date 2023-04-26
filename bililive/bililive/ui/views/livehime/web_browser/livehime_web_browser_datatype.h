#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_WEB_BROWSER_DATATYPE_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_WEB_BROWSER_DATATYPE_H_

#include <string>

#include "bililive/bililive/ui/views/controls/browser/livehime_web_browser_view.h"

namespace views {
    class Widget;
}

struct WebBrowserPopupDetails
{
    std::string web_url;
    std::string post_data;
    views::Widget* parent = nullptr;
    WebViewPopupType allow_popup = WebViewPopupType::NotAllow;

    bool show_native_titlebar = false;
    string16 title;
};

namespace livehime_test
{
    void ShowPopupLivehimeBrowser(views::Widget* parent);
}

#endif