#ifndef BILILIVE_SECRET_SERVICES_DANMAKU_HIME_SERVICE_IMPL_H_
#define BILILIVE_SECRET_SERVICES_DANMAKU_HIME_SERVICE_IMPL_H_

#include "base/basictypes.h"

#include "bililive/secret/net/request_connection_manager.h"
#include "bililive/secret/public/danmaku_hime_service.h"


namespace secret {

class DanmakuHimeServiceImpl : public DanmakuHimeService {
public:
    explicit DanmakuHimeServiceImpl(RequestConnectionManager* manager);

    RequestProxy DownloadImage(
        const std::string& url, DownloadImageHandler handler) override;

    //º£Íâ°æ·¢ËÍµ¯Ä»
    RequestProxy SendViddupDanmaku(
        int64_t room_id, const std::string& msg,
        int64_t rnd, int font_size, int mode, SendDanmakuViddupHandler handler) override;
private:
    RequestConnectionManager* conn_manager_;

    DISALLOW_COPY_AND_ASSIGN(DanmakuHimeServiceImpl);
};

}  // namespace secret

#endif  // BILILIVE_SECRET_SERVICES_DANMAKU_HIME_SERVICE_IMPL_H_