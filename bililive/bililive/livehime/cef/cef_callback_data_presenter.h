#pragma once

#include <vector>

#include "base\strings\string16.h"
#include "bililive/bililive/livehime/cef/cef_proxy_wrapper.h"

#include "cef/cef_proxy_dll/public/livehime_cef_proxy_exports.h"

struct CefToast {
    std::string toast_msg;
    int show_time = 5;
};

struct CefSetTitle {
    std::string title_msg;
    std::string icon;
};

struct TitleBtn {
    std::string tagname;
    base::string16 text;
    base::string16 url;     //tagnameΪclose customʱ
    std::string btn_color;
    std::string btn_alpha;
    bool is_left = false;
};

struct CefTitleBar {
    base::string16 title;
    bool clear_title = false;
    bool clear_icon = false;
    std::string icon;
    std::string title_color;
    std::string title_alpha;
    std::string backgroundColor;
    std::string background_alpha;
    std::vector<TitleBtn> title_btns;
    bool re_default_btn = false;

    int callbackId = -1;
    int successCallbackId = -1;
};

struct CefAlert {
    base::string16 title;
    base::string16 msg;
    base::string16 confirmButton;
    base::string16 cancelButton;
    base::string16 type;       // alert|conform
    std::string noRemindKey;

    int callbackId = -1;
    int successCallbackId = -1;
};

class CefCalbackDataPresenter {
public:
    CefCalbackDataPresenter();
    ~CefCalbackDataPresenter();

    static CefCalbackDataPresenter* GetInstance();

    bool ReadJsbridgeTitleBar(const cef_proxy::calldata* data, CefTitleBar& cef_title_bar);
    bool ReadJsbridgeAlert(const cef_proxy::calldata* data, CefAlert& cef_alert);

private:
    bool JurdgeExistByKey(const cef_proxy::calldata* data,const std::string& key);

};