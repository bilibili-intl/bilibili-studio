/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_SERVICES_LIVE_STREAMING_SERVICE_IMPL_H_
#define BILILIVE_SECRET_SERVICES_LIVE_STREAMING_SERVICE_IMPL_H_

#include "base/basictypes.h"

#include "bililive/secret/net/request_connection_manager.h"
#include "bililive/secret/public/live_streaming_service.h"


namespace secret {

class LiveStreamingServiceImpl : public LiveStreamingService {

public:
    explicit LiveStreamingServiceImpl(RequestConnectionManager* manager);

    ~LiveStreamingServiceImpl();

    RequestProxy GetIntlRoomInfo(int64_t mid, GetIntlRoomInfoHandler handler) override;

    // 海外开播 - 新增开播接口
    RequestProxy StartViddupLive(OpenLiveRoomHandler handler) override;

private:
    DISALLOW_COPY_AND_ASSIGN(LiveStreamingServiceImpl);

private:
    RequestConnectionManager* conn_manager_;
};

}   // namespace secret

#endif  // BILILIVE_SECRET_SERVICES_LIVE_STREAMING_SERVICE_IMPL_H_
