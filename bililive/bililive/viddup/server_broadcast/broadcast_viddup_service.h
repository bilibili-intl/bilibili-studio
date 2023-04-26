#ifndef BILILIVE_BILILIVE_VIDDUP_SERVER_BROADCAST_BROADCAST_VIDDUP_SERVICE_H_
#define BILILIVE_BILILIVE_VIDDUP_SERVER_BROADCAST_BROADCAST_VIDDUP_SERVICE_H_

#include "build/build_config.h"

#include "SkColor.h"
#include "base/compiler_specific.h"
#include "base/file_util.h"
#include "base/json/json_file_value_serializer.h"
#include "base/memory/ref_counted.h"
#include "base/notification/notification_observer.h"
#include "base/notification/notification_registrar.h"
#include "base/values.h"
#include "base/observer_list.h"

#include "bililive/bililive/viddup/server_broadcast/danmaku_broadcast_viddup_presenter.h"
#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"
#include "bililive/bililive/livehime/server_broadcast/danmaku_statistics.h"
#include "bililive/secret/public/live_streaming_service.h"


struct DanmakuViddupInfo
{
    enum ChatType
    {
        System = 0,         //System Danmaku
        User,               //User send
        Gift,               //Gift Danmaku
        LiveKeyChange,      //Session change
        MuteMsg,            //Gagging broadcast
        WarnMs,             //Warning broadcast
        PopupMsg,           //Danmaku broadcast
        BlockMsg,           //Forbidden broadcast
        LikesMsg,           //Like broadcast
        UltimatePrizeMsg,   //Award broadcast
        RechargeBarMsg      //Charge bar broadcast
    };

    enum UserIdentity
    {
        Normal = 0,
        Rank1 = 1,
        Rank2 = 2,
        Rank3 = 3,
    };

    ChatType type = System;
    UserIdentity id = Normal;
    std::string content;
    std::string userName;
    bool isAdmin = false;
    std::string gifFileUrl;  //url
    std::string giftName;
    std::string comboString;
    int treasureID = 0;
    std::string notice;
    int64 end_time;

    std::string popupTitle;
    std::string popupContent;
    int duration = 0;

};

class BililiveBroadcastViddupObserver
{
public:
    virtual ~BililiveBroadcastViddupObserver() = default;

    virtual void NewDanmaku(const std::vector<DanmakuViddupInfo>& danmaku) {}
};

class BililiveBroadcastViddupService : public base::NotificationObserver
{
public:
    BililiveBroadcastViddupService();
    virtual ~BililiveBroadcastViddupService();

    void StartListening(int64_t uid, int64_t room_id);
    void StopListening();

    static int64_t audience() { return audience_; }
    static int64_t attention() { return attention_; }
    static int64_t gift();

    void AddObserver(BililiveBroadcastViddupObserver*observer) { observer_list_.AddObserver(observer); }
    void RemoveObserver(BililiveBroadcastViddupObserver*observer) { observer_list_.RemoveObserver(observer); }

protected:
    // NotificationObserver
    void Observe(int type,
        const base::NotificationSource& source,
        const base::NotificationDetails& details) OVERRIDE;

private:
    bool ParseDanmuMsg(scoped_ptr<base::DictionaryValue> &local_danmaku);

private:
    base::NotificationRegistrar registrar_;
    static int64_t audience_;
    static int64_t attention_;

    bool invoke_stop_listening_;

    livehime::DanmakuStatistics statistics_;
    std::unique_ptr<DanmakuBroadcastViddupPresenter> presenter_;

    ObserverList<BililiveBroadcastViddupObserver> observer_list_;

    DISALLOW_COPY_AND_ASSIGN(BililiveBroadcastViddupService);
};

#endif  // BILILIVE_BILILIVE_VIDDUP_SERVER_BROADCAST_BROADCAST_VIDDUP_SERVICE_H_