
#ifndef BILILIVE_SECRET_SERVICES_EVENT_TRACKING_SERVICE_IMPL_H_
#define BILILIVE_SECRET_SERVICES_EVENT_TRACKING_SERVICE_IMPL_H_

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"

#include "bililive/secret/net/request_connection_manager.h"
#include "bililive/secret/public/event_tracking_service.h"

namespace secret {

class EventTrackingServiceImpl : public EventTrackingService {
public:
    explicit EventTrackingServiceImpl(RequestConnectionManager* manager);

    ~EventTrackingServiceImpl();

    RequestProxy ReportLivehimeBehaviorEvent(
        LivehimeBehaviorEvent event_id,
        int64_t mid, const std::string& event_msg) override;

    RequestProxy ReportLivehimeBehaviorEvent(
        const std::string& event_id,
        const std::string& event_msg) override;

    // V2�汾����㣬V2��ɵĲ��棬�ɵ���ҵ�����ĵط�������ɾ����������㻹�Ǽ����þɱ�
    // �����V2ָ��ҵ������V2���������Э���V2���Ա�����ǰֱ����ֱ����ר�����������ҵ��������
    RequestProxy ReportLivehimeBehaviorEventV2(
        LivehimeBehaviorEventV2 event_id, 
        const base::StringPairs& params) override;

    RequestProxy ReportLivehimeBehaviorEventV2(
        const std::string& event_id,
        const base::StringPairs& event_msg) override;

    // ����ƽ̨V2�汾����Э�����㣬�õ�/log/web�ӿڣ�Ŀǰר����apm���ݵ��ϱ�
    RequestProxy ReportLivehimeApmEventV2(
        const std::string& event_data) override;

    // ͨ������˽������
    RequestProxy ReportLivehimeBehaviorEventViaSrv(
        LivehimeViaServerBehaviorEvent event_id,
        const base::StringPairs& params) override;

    RequestProxy ReportLivehimeBehaviorEventViaSrvNew(
        LivehimeViaServerBehaviorEventNew event_id,
        const base::StringPairs& params) override;

    // ��������㣨Ŀǰû����������߼�������ֻ�ǽ������������logid�����ѣ�
    RequestProxy ReportLivehimeBehaviorEventPolaris(
        LivehimePolarisBehaviorEvent event_id,
        const base::StringPairs& meta_params) override;
    RequestProxy ReportLivehimeBehaviorEventPolaris(
        const std::string& event_id,
        const base::StringPairs& params) override;

private:
    RequestConnectionManager* conn_manager_;
    std::string buvid_;
    std::string fts_;

    static EthernetAddressInfo ethernet_address_info_;
    std::string browser_resolution_;
    BehaviorEventNetwork network_ = BehaviorEventNetwork::Unknown;

    base::WeakPtrFactory<EventTrackingServiceImpl> weak_ptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(EventTrackingServiceImpl);
};

}   // namespace secret

#endif  // BILILIVE_SECRET_SERVICES_EVENT_TRACKING_SERVICE_IMPL_H_
