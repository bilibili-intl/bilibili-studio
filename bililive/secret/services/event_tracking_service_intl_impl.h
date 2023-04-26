
#ifndef BILILIVE_SECRET_SERVICES_EVENT_TRACKING_SERVICE_INTL_IMPL_H_
#define BILILIVE_SECRET_SERVICES_EVENT_TRACKING_SERVICE_INTL_IMPL_H_

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"

#include "bililive/secret/net/request_connection_manager.h"
#include "bililive/secret/public/event_tracking_service.h"


namespace secret {

class EventTrackingServiceIntlImpl : public EventTrackingService {
public:
    explicit EventTrackingServiceIntlImpl(RequestConnectionManager* manager);

    ~EventTrackingServiceIntlImpl();

    RequestProxy ReportLivehimeBehaviorEvent(
        LivehimeBehaviorEvent event_id,
        int64_t mid, const std::string& event_msg) override;

    RequestProxy ReportLivehimeBehaviorEvent(
        const std::string& event_id,
        const std::string& event_msg) override;

    // V2版本的埋点，V2与旧的并存，旧的在业务埋点的地方可以逐步删掉，技术埋点还是继续用旧表
    // 这里的V2指的业务层面的V2，并非埋点协议的V2，以便与以前直接埋到直播姬专属技术埋点表的业务做区分
    RequestProxy ReportLivehimeBehaviorEventV2(
        LivehimeBehaviorEventV2 event_id, 
        const base::StringPairs& params) override;

    RequestProxy ReportLivehimeBehaviorEventV2(
        const std::string& event_id,
        const base::StringPairs& event_msg) override;

    // 数据平台V2版本数据协议的埋点，用的/log/web接口，目前专用于apm数据的上报
    RequestProxy ReportLivehimeApmEventV2(
        const std::string& event_data) override;

    // 通过服务端进行埋点
    RequestProxy ReportLivehimeBehaviorEventViaSrv(
        LivehimeViaServerBehaviorEvent event_id,
        const base::StringPairs& params) override;

    RequestProxy ReportLivehimeBehaviorEventViaSrvNew(
        LivehimeViaServerBehaviorEventNew event_id,
        const base::StringPairs& params) override;

    // 北极星埋点（目前没有埋点重试逻辑，仅仅只是将埋点往北极星logid报而已）
    RequestProxy ReportLivehimeBehaviorEventPolaris(
        LivehimePolarisBehaviorEvent event_id,
        const base::StringPairs& meta_params) override;
    RequestProxy ReportLivehimeBehaviorEventPolaris(
        const std::string& event_id,
        const base::StringPairs& params) override;

    // 海外数平埋点（具体实现方式待后面数平方确认）
    RequestProxy ReportLivehimeIntlBehaviorEvent(
        LivehimeIntlBehaviorEvent event_id,
        const base::StringPairs& params) override;
    RequestProxy ReportLivehimeIntlBehaviorEvent(
        const std::string& event_id,
        const base::StringPairs& params) override;
    
private:
    void SetupBuvidForLivehime();
    void SetupEthernetAddressInfo();
    static void HandleGetEthernetAddressResponse(bool valid, const EthernetAddressInfo& info);

    void GetBuvid();

    RequestProxy ReportEventTracking(RequestRaw params);
    RequestProxy ReportApmEventTracking(const std::string& data);

    // It is now an asynchronous callback, and there is a chance when this handler is called after
    // the whole secret has gone.
    void HandleBuvidGot(const std::string& buvid);
    
    void PostBuvidGot();

    bool CheckInCommonBehaviorEventTable(
        std::string event_name,
        std::string event_id,
        secret::LivehimeIntlBehaviorEvent& intl_event);

private:
    RequestConnectionManager* conn_manager_;
    std::string buvid_;
    std::string fts_;

    static EthernetAddressInfo ethernet_address_info_;
    std::string browser_resolution_;
    BehaviorEventNetwork network_ = BehaviorEventNetwork::Unknown;

    base::WeakPtrFactory<EventTrackingServiceIntlImpl> weak_ptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(EventTrackingServiceIntlImpl);
};

}   // namespace secret

#endif  // BILILIVE_SECRET_SERVICES_EVENT_TRACKING_SERVICE_INTL_IMPL_H_
