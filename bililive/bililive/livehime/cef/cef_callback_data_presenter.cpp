#include "cef_callback_data_presenter.h"

#include "base\memory\singleton.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/prefs/pref_service.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/livehime/common_pref/common_pref_names.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/pref_names.h"


CefCalbackDataPresenter::CefCalbackDataPresenter()
{
}

CefCalbackDataPresenter::~CefCalbackDataPresenter()
{
}

CefCalbackDataPresenter* CefCalbackDataPresenter::GetInstance()
{
    return Singleton<CefCalbackDataPresenter>::get();
}

bool CefCalbackDataPresenter::ReadJsbridgeAlert(const cef_proxy::calldata* data, CefAlert& cef_alert)
{
    if (!data) {
        LOG(WARNING) << "[cef_data] receive cef json_data is empty ";
        return false;
    }

    cef_alert.confirmButton = data->at("confirmButton").wstr_;
    if (JurdgeExistByKey(data, "cancelButton")) {
        cef_alert.cancelButton = data->at("cancelButton").wstr_;
    }

    cef_alert.noRemindKey = data->at("noRemindKey").str_;
    cef_alert.type = base::UTF8ToUTF16(data->at("type").str_);

    cef_alert.title = data->at("title").wstr_;
    cef_alert.msg = data->at("message").wstr_;
    cef_alert.callbackId = data->at("callbackId").numeric_union.int_;
    cef_alert.successCallbackId = data->at("successCallbackId").numeric_union.int_;

    return true;
}

bool CefCalbackDataPresenter::JurdgeExistByKey(const cef_proxy::calldata* data, const std::string& key)
{
    for (auto it = data->begin(); it != data->end(); it++) {
        if (it->first == key) {
            return true;
        }
    }
    return false;
}

bool CefCalbackDataPresenter::ReadJsbridgeTitleBar(const cef_proxy::calldata* data, CefTitleBar& cef_title_bar)
{
    if (!data) {
        return false;
    }

    cef_title_bar.callbackId = data->at("callbackId").numeric_union.int_;
    cef_title_bar.successCallbackId = data->at("successCallbackId").numeric_union.int_;

    if (JurdgeExistByKey(data, "backgroundColor")) {
        std::string backgroundColor = data->at("backgroundColor").str_;
        std::vector<std::string> split_color;
        base::SplitString(backgroundColor, ',', &split_color);
        if (split_color.size() > 1) {
            cef_title_bar.backgroundColor = split_color.at(0);
            cef_title_bar.background_alpha = split_color.at(1);
        }
        else if (split_color.size() == 1) {
            cef_title_bar.backgroundColor = split_color.at(0);
        }
    }

    if (JurdgeExistByKey(data, "title")) {
        cef_title_bar.title = data->at("title").wstr_;
        if (cef_title_bar.title.empty()) {
            cef_title_bar.clear_title = true;
        }
    }

    if (JurdgeExistByKey(data, "icon")) {
        cef_title_bar.icon = data->at("icon").str_;
        if (cef_title_bar.icon.empty()) {
            cef_title_bar.clear_icon = true;
        }
    }

    if (JurdgeExistByKey(data, "titleColor")) {
        cef_title_bar.title_color = data->at("titleColor").str_;
    }

    if (JurdgeExistByKey(data, "right")) {
        cef_title_bar.re_default_btn = true;
        std::list<cef_proxy::calldata> right_list = data->at("right").list_;
        for (auto& it : right_list) {
            TitleBtn title_btn;
            if (JurdgeExistByKey(&it, "tagname")) {
                title_btn.tagname = it.at("tagname").str_;
            }
            if (JurdgeExistByKey(&it, "text")) {
                title_btn.text = it.at("text").wstr_;
            }

            if (JurdgeExistByKey(&it, "url")) {
                title_btn.url = it.at("url").wstr_;
            }

            if (!title_btn.tagname.empty()) {
                cef_title_bar.title_btns.push_back(std::move(title_btn));
            }
        }
    }
    return true;
}