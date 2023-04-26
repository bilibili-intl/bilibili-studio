#ifndef BILILIVE_BILILIVE_LIVEHIME_FIFTER_BROADCAST_SUB_FIFTER_BROADCAST_H_
#define BILILIVE_BILILIVE_LIVEHIME_FIFTER_BROADCAST_SUB_FIFTER_BROADCAST_H_

#include <mutex>

#include "base/files/file_path.h"
#include "base/observer_list.h"
#include "base/strings/string16.h"
#include "base/notification/notification_observer.h"
#include "base/notification/notification_registrar.h"

#include "cef_proxy_dll/public/livehime_cef_proxy_calldata.h"

#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"
#include "bililive/public/bililive/bililive_notification_types.h"


namespace sub{
    typedef std::vector<std::string> StringList;
    struct broadCastInfo {
        std::string cmd;
        std::string json;
    };

    struct subInfo {
        std::string type;
        StringList cmd_list;
        cef_proxy::browser_bind_data bind_data;
        cef_proxy::calldata data;
    };

    enum class ManageSubMsg {
        SUB_ADD,
        SUB_DEL,
        SUB_SEND
    };
}

class SubFifterBroadcastController
    : base::NotificationObserver
{
public:
    SubFifterBroadcastController();
    ~SubFifterBroadcastController();

    static SubFifterBroadcastController* GetInstance();

    void SubBroadCastCmd(const cef_proxy::browser_bind_data& bind_data, const cef_proxy::calldata* data,const std::string& type,const std::string& cmd_list);
    void UnSubBroadCastCmd(const std::string& type);

protected:

    // NotificationObserver
    void Observe(int type,
        const base::NotificationSource& source,
        const base::NotificationDetails& details) OVERRIDE;

private:
    void ManageSubList(sub::ManageSubMsg sub_type, const sub::subInfo& info, const std::string& json_content);

    void SendBroadCastByCallback(const sub::subInfo& info,const std::string& json_content);

    sub::StringList SplitstrCmdList(const std::string& str, const std::string& pattern);
private:
    base::WeakPtrFactory<SubFifterBroadcastController> weakptr_factory_;
    std::mutex mutex_;

    std::list<sub::subInfo>  sub_list_;

    base::NotificationRegistrar registrar_;

    DISALLOW_COPY_AND_ASSIGN(SubFifterBroadcastController);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_FIFTER_BROADCAST_CEF_FIFTER_BROADCAST_H_