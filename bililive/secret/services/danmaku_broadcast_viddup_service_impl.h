#ifndef BILILIVE_SECRET_SERVICES_DANMAKU_BROADCAST_VIDDUP_SERVICE_IMPL_H_
#define BILILIVE_SECRET_SERVICES_DANMAKU_BROADCAST_VIDDUP_SERVICE_IMPL_H_

#include "bililive/secret/public/danmaku_broadcast_viddup_service.h"


namespace base {
    class MessageLoopProxy;
}

namespace internal {
    class DanmakuViddupServiceClientContext;
}

namespace secret {

class DanmakuBroadcastViddupServiceImpl : public DanmakuBroadcastViddupService {
public:
    explicit DanmakuBroadcastViddupServiceImpl(base::MessageLoopProxy *ui_proxy);
    ~DanmakuBroadcastViddupServiceImpl();

    void StartListening(
        int64_t uid, int64_t room_id, const ServerList& servers, const std::string& token,
        DanmakuBroadcastViddupCallback* c) override;
    void StopListening() override;

private:
    void StopContext();

    void OnDanmakuMsgFromContext(RefDictionary json,const std::string& json_content);
    void OnAudienceNumFromContext(int num);
    void OnTokenExpiredFromContext();
    void OnDanmakuACKFromContext(const std::set<uint32_t>& seqs);

    secret::DanmakuBroadcastViddupCallback* callback_;
    scoped_refptr<internal::DanmakuViddupServiceClientContext> context_;

    bool is_running_;
    base::MessageLoopProxy* ui_proxy_;
};

}

#endif  // BILILIVE_SECRET_SERVICES_DANMAKU_BROADCAST_VIDDUP_SERVICE_IMPL_H_