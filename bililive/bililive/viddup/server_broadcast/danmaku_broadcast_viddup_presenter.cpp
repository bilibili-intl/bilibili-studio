#include "danmaku_broadcast_viddup_presenter.h"

#include "base/command_line.h"
#include "base/ext/callable_callback.h"
#include "base/notification/notification_service.h"
#include "base/strings/string_util.h"

#include "bililive/bililive/livehime/sub_broadcast/sub_fifter_broadcast.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/secret/bililive_secret.h"


DanmakuBroadcastViddupPresenter::DanmakuBroadcastViddupPresenter()
    : uid_(-1),
      room_id_(-1),
      is_starting_(false),
      weak_ptr_factory_(this) {
}

void DanmakuBroadcastViddupPresenter::StartListening(int64_t uid, int64_t room_id) {
    CommandLine* cmd = CommandLine::ForCurrentProcess();
    std::string str = cmd->GetSwitchValueASCII(bililive::kSwitchDanmakuRoomId);
    if (!str.empty()) {
        room_id = std::atoi(str.c_str());
    }

    if (uid == -1 || room_id == -1) {
        return;
    }

    uid_ = uid;
    room_id_ = room_id;

    DCHECK(!is_starting_);

    GetBililiveProcess()->secret_core()->danmaku_broadcast_viddup_service()->
        StartListening(uid_, room_id_, servers_, token_, this);
}

void DanmakuBroadcastViddupPresenter::StopListening() {
    if (is_starting_) {
        is_starting_ = false;
        return;
    }

    GetBililiveProcess()->secret_core()->danmaku_broadcast_viddup_service()->
        StopListening();

    int status = 0;
    base::NotificationService::current()->Notify(
        bililive::NOTIFICATION_BILILIVE_DANMAKU_STATUS,
        base::NotificationService::AllSources(),
        base::Details<int>(&status));
}

void DanmakuBroadcastViddupPresenter::OnDanmakuMessage(const RefDictionary& dict, const std::string& json_content) {
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    scoped_ptr<base::DictionaryValue> danmaku_msg;
    danmaku_msg.reset(RefCountedDictionary::RecursiveDeepCopy(dict->value()));

    std::string cmd;
    if(!danmaku_msg->GetString("cmd", &cmd)) {
        LOG(WARNING) << "Parsing danmaku broadcast failed! Cannot get 'cmd' field!";
        return;
    }

    bool is_msg = false;
    bililive::NotificationType notification_type;

    {
        is_msg = true;
        notification_type = bililive::NOTIFICATION_BILILIVE_DANMAKU_MSG;
    }

    if (!is_msg) {
        base::NotificationService::current()->Notify(
            notification_type,
            base::NotificationService::AllSources(),
            base::Details<void>(nullptr));
    } else {
        base::NotificationService::current()->Notify(
            bililive::NOTIFICATION_BILILIVE_DANMAKU_MSG,
            base::NotificationService::AllSources(),
            base::Details<scoped_ptr<base::DictionaryValue>>(&danmaku_msg));

        sub::broadCastInfo cast_info;
        cast_info.cmd = cmd;
        cast_info.json = json_content;
        base::NotificationService::current()->Notify(
            bililive::NOTIFICATION_LIVEHIME_FIFTER_BROADCAST, base::NotificationService::AllSources(),
            base::Details<sub::broadCastInfo>(&cast_info));
    }
}

void DanmakuBroadcastViddupPresenter::OnAudienceNumber(int64_t num) {
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    base::NotificationService::current()->Notify(
        bililive::NOTIFICATION_BILILIVE_AUDIENCES_COUNT,
        base::NotificationService::AllSources(),
        base::Details<int64_t>(&num));
}

void DanmakuBroadcastViddupPresenter::OnTokenExpired() {
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    GetBililiveProcess()->secret_core()->danmaku_broadcast_viddup_service()->
        StopListening();

    servers_.clear();
    token_.clear();

    StartListening(uid_, room_id_);
}

void DanmakuBroadcastViddupPresenter::OnDanmakuACK(const std::set<uint32_t>& seqs) {
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));

    if (seqs.empty()) {
        return;
    }

    auto callback = base::MakeCallable(base::Bind(
        &DanmakuBroadcastViddupPresenter::OnDanmakuACKResponse,
        weak_ptr_factory_.GetWeakPtr()));
}

void DanmakuBroadcastViddupPresenter::OnDanmakuACKResponse(
    bool valid, int code, const std::string& msg)
{
}