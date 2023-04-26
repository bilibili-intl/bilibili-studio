#ifndef BILILIVE_BILILIVE_LIVEHIME_EVENT_REPORT_CENTER_H_
#define BILILIVE_BILILIVE_LIVEHIME_EVENT_REPORT_CENTER_H_

#include "base/strings/string_split.h"

namespace secret
{
    enum class LivehimeBehaviorEvent;
    enum class LivehimeBehaviorEventV2;
    enum class LivehimeViaServerBehaviorEvent;
    enum class LivehimePolarisBehaviorEvent;
    enum class LivehimeViaServerBehaviorEventNew;
    enum class LivehimeIntlBehaviorEvent;
}

// 新的埋点使用这里的接口
// 通用控件（主要是按钮）继承SupportsEventReport，在其基类的NotifyClick函数中判断是否需要点击埋点；
// 对于其他业务逻辑埋点还是直接调EventReport
namespace livehime
{
    class SupportsEventReportV2
    {
        struct ReportTask
        {
            enum EventType
            {
                V2,
                Polaris,
            };
            union EventID
            {
                secret::LivehimeBehaviorEventV2 v2_type;
                secret::LivehimePolarisBehaviorEvent polaris_type;
            } event_id;

            EventType event_type = V2; // 0, v2; 1, 北极星
            std::string event_msg;
        };

    public:
        // 直接以指定好的事件ID附上当前指定的数据进行埋点
        void ReportBehaviorEvent(const std::string* msg = nullptr);

        // 只修改埋点id、数据
        void SetReportEventDetails(secret::LivehimeBehaviorEventV2 event_type, const std::string& event_msg,
            bool enable = true);

        // 只修改北极星埋点id、数据
        void SetReportEventDetails(secret::LivehimePolarisBehaviorEvent event_type, const std::string& event_msg,
            bool enable = true);

        void EnableReport(bool enable);
        bool IsEnableReport() const;

    protected:
        SupportsEventReportV2();
        virtual ~SupportsEventReportV2() = default;

    private:
        bool enable_report_ = false;
        std::vector<ReportTask> report_tasks_;
    };

    // 技术埋点，“key:value;key:value;...”，logid=000100
    void TechnologyEventReport(secret::LivehimeBehaviorEvent type, const std::string& msg);

    // 日志上报
    void UploadLogFilesAndReport(bool user_report = false);

    // V2业务埋点，logid=005434
    void BehaviorEventReport(secret::LivehimeBehaviorEventV2 type, const std::string& msg);
    void BehaviorEventReport(const std::string& event_id, const std::string& msg);

    // Apm数据埋点，logid=006474
    void ApmEventReport(const std::string& msg);

    // 配合服务端埋点相关的数据搜集帮助函数,ERVS=EventReportViaServer
    base::StringPairs CommonFieldsForERVS();
    // 通过服务端进行埋点
    void BehaviorEventReportViaServer(secret::LivehimeViaServerBehaviorEvent type, const std::string& msg);

    // 新的服务器埋点,会逐步替换老的,暂时只有 [唱歌识别] 使用
    void BehaviorEventReportViaServerNew(secret::LivehimeViaServerBehaviorEventNew type, const std::string& msg);

    // 北极星埋点
    void PolarisEventReport(secret::LivehimePolarisBehaviorEvent type, const std::string& msg);
    void PolarisEventReport(secret::LivehimePolarisBehaviorEvent type, const base::StringPairs& msg);
    void PolarisEventReport(const std::string& event_id, const std::string& msg);
}

// 海外的埋点使用这里的接口
// 通过命令空间进行区分，仅供海外业务模块使用
namespace livehimeintl
{
    // 海外数平埋点（后续可根据具体参数等调整）
    void BehaviorEventReport(secret::LivehimeIntlBehaviorEvent type, const std::string& msg);
    void BehaviorEventReport(secret::LivehimeIntlBehaviorEvent type, const base::StringPairs& msg);
    void BehaviorEventReport(const std::string& event_id, const std::string& msg);
}

#endif