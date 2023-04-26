#ifndef BILILIVE_SECRET_PUBLIC_DANMAKU_BROADCAST_VIDDUP_SERVICE_H_
#define BILILIVE_SECRET_PUBLIC_DANMAKU_BROADCAST_VIDDUP_SERVICE_H_

#include "bililive/public/common/refcounted_dictionary.h"


namespace secret {

class DanmakuBroadcastViddupCallback {
public:
    virtual ~DanmakuBroadcastViddupCallback() = default;

    virtual void OnDanmakuMessage(const RefDictionary& dict,const std::string& json_content) = 0;
    virtual void OnAudienceNumber(int64_t num) = 0;
    virtual void OnTokenExpired() = 0;
    virtual void OnDanmakuACK(const std::set<uint32_t>& seqs) = 0;
};

class DanmakuBroadcastViddupService {
public:
    using ServerList = std::vector<std::pair<std::string, unsigned short>>;

    virtual ~DanmakuBroadcastViddupService() = default;

    // Service don't own callback
    virtual void StartListening(
        int64_t uid, int64_t room_id, const ServerList& servers, const std::string& token,
        DanmakuBroadcastViddupCallback* c) = 0;
    virtual void StopListening() = 0;
};

}

#endif  // BILILIVE_SECRET_PUBLIC_DANMAKU_BROADCAST_VIDDUP_SERVICE_H_