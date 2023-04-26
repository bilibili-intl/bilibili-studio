#include "sub_fifter_broadcast.h"

#include "base/memory/singleton.h"
#include "base/notification/notification_service.h"
#include "base/prefs/pref_service.h"

#include "bililive/bililive/livehime/cef/cef_proxy_wrapper.h"
#include "bililive/public/secret/bililive_secret.h"
#include "cef/bililive_browser/public/bililive_browser_ipc_messages.h"


SubFifterBroadcastController::SubFifterBroadcastController()
    : weakptr_factory_(this)
{
    registrar_.Add(this, bililive::NOTIFICATION_LIVEHIME_FIFTER_BROADCAST, base::NotificationService::AllSources());
}

SubFifterBroadcastController::~SubFifterBroadcastController()
{
    registrar_.Remove(this, bililive::NOTIFICATION_LIVEHIME_FIFTER_BROADCAST, base::NotificationService::AllSources());
}

SubFifterBroadcastController* SubFifterBroadcastController::GetInstance()
{
    return Singleton<SubFifterBroadcastController>::get();
}

void SubFifterBroadcastController::Observe(
    int type, const base::NotificationSource& source, const base::NotificationDetails& details)
{
    auto info = reinterpret_cast<sub::broadCastInfo*>(details.map_key());
    sub::subInfo sub_info;
    sub_info.type = info->cmd;
    ManageSubList(sub::ManageSubMsg::SUB_SEND, sub_info, info->json);
}

void SubFifterBroadcastController::SubBroadCastCmd(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::calldata* data, const std::string& type, const std::string& cmd_list)
{
    if (type.empty() || cmd_list.empty())
    {
        return;
    }
    sub::subInfo info;
    info.type = type;
    info.bind_data = bind_data;
    info.cmd_list = SplitstrCmdList(cmd_list, ",");
    info.data = *data;
    ManageSubList(sub::ManageSubMsg::SUB_ADD,info,"");
}

void SubFifterBroadcastController::UnSubBroadCastCmd(const std::string& type)
{
    if (type.empty())
    {
        return;
    }
    sub::subInfo info;
    info.type = type;
    ManageSubList(sub::ManageSubMsg::SUB_DEL,info,"");
}

void SubFifterBroadcastController::ManageSubList(sub::ManageSubMsg sub_type, const sub::subInfo& info,const std::string& json_content)
{
    std::unique_lock<std::mutex> lck(mutex_);
    if (sub_type == sub::ManageSubMsg::SUB_ADD)
    {
        sub_list_.push_back(info);
    }
    else if (sub_type == sub::ManageSubMsg::SUB_DEL)
    {
        std::list<sub::subInfo>::iterator it;
        for (it = sub_list_.begin(); it!= sub_list_.end(); it++)
        {
            if (it->type == info.type)
            {
                sub_list_.erase(it);
                break;
            }
        }
    }
    else
    {
        for (auto it : sub_list_)
        {
            for (auto item : it.cmd_list)
            {
                if (item == info.type)
                {
                    SendBroadCastByCallback(it, json_content);
                    break;
                }
            }
        }
    }
}

void SubFifterBroadcastController::SendBroadCastByCallback(const sub::subInfo& info, const std::string& json_content)
{
    cef_proxy::calldata_filed data_filed;
    data_filed.type = cef_proxy::calldata_type::CALL_DATA_TYPE_STRING;
    data_filed.str_ = json_content;
    const_cast<cef_proxy::calldata&>(info.data).insert({ "info", data_filed });

    CefProxyWrapper::GetInstance()->DispatchJsEvent(info.bind_data,
            ipc_messages::kBilibiliPageSubBroadcast, &info.data);
}

sub::StringList SubFifterBroadcastController::SplitstrCmdList(const std::string& cmd_str, const std::string& pattern)
{
    sub::StringList  cmd_list;
    std::string sub_str;
    std::string tPattern;
    size_t      patternLen = pattern.length();
    size_t      strLen = cmd_str.length();

    for (size_t i = 0; i < cmd_str.length(); i++)
    {
        if (pattern[0] == cmd_str[i] && ((strLen - i) >= patternLen))
        {
            if (memcmp(&pattern[0], &cmd_str[i], patternLen) == 0)
            {
                i += patternLen - 1;
                if (!sub_str.empty())
                {
                    cmd_list.push_back(sub_str);
                    sub_str.clear();
                }
            }
            else
            {
                sub_str.push_back(cmd_str[i]);
            }
        }
        else
        {
            sub_str.push_back(cmd_str[i]);
        }
    }
    if (!sub_str.empty())
    {
        cmd_list.push_back(sub_str);
    }

    return cmd_list;
}
#pragma endregion