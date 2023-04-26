#ifndef BILILIVE_BILILIVE_LIVEHIME_SERVER_BROADCAST_DANMAKU_BROADCAST_VIDDUP_PRESENTER_H_
#define BILILIVE_BILILIVE_LIVEHIME_SERVER_BROADCAST_DANMAKU_BROADCAST_VIDDUP_PRESENTER_H_

#include "base/memory/weak_ptr.h"

#include "bililive/secret/public/danmaku_broadcast_viddup_service.h"
#include "bililive/secret/public/danmaku_hime_service.h"


class DanmakuBroadcastViddupPresenter
    : public secret::DanmakuBroadcastViddupCallback {
public:
    DanmakuBroadcastViddupPresenter();

    void StartListening(int64_t uid, int64_t room_id);
    void StopListening();

private:
    using ServerList = secret::DanmakuBroadcastViddupService::ServerList;

    //void OnGetDanmakuConfig(
    //    bool valid_resp, int code,
    //    const secret::DanmakuHimeService::DanmakuConfig& info);

    // secret::DanmakuBroadcastCallback
    void OnDanmakuMessage(const RefDictionary& dict, const std::string& json_content) override;
    void OnAudienceNumber(int64_t num) override;
    void OnTokenExpired() override;
    void OnDanmakuACK(const std::set<uint32_t>& seqs) override;

    void OnDanmakuACKResponse(bool valid, int code, const std::string& msg);

    int64_t uid_;
    int64_t room_id_;
    bool is_starting_;
    std::string token_;
    ServerList servers_;

    base::WeakPtrFactory<DanmakuBroadcastViddupPresenter> weak_ptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(DanmakuBroadcastViddupPresenter);
};

#endif  // BILILIVE_BILILIVE_LIVEHIME_SERVER_BROADCAST_DANMAKU_BROADCAST_VIDDUP_PRESENTER_H_
