#include "bililive/secret/services/event_tracking_service_impl.h"

#include <tuple>

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/guid.h"
#include "base/logging.h"
#include "base/values.h"
#include "base/ext/callable_callback.h"
#include "base/json/json_writer.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"

#include "net/http/http_request_headers.h"

#include "url/gurl.h"

#include "bilibase/basic_types.h"
#include "bililive/bililive/utils/net_util.h"
#include "bililive/common/bililive_context.h"
#include "bililive/secret/core/bililive_secret_core_impl.h"
#include "bililive/secret/services/live_streaming_common_data.h"
#include "bililive/secret/services/service_utils.h"


namespace
{
    using secret::ResponseInfo;

    // @http://bapi.bilibili.co/project/2611/interface/api/226246
    const char kURLServerReport[] = "";

    const char kURLServerReportNew[] = "";
    // 埋点常规处理
    using EventTrackingResult = std::tuple<bool>;
    using EventTrackingParser = std::function<EventTrackingResult(ResponseInfo, const std::string&)>;

    EventTrackingResult ParseEventTrackingResponse(ResponseInfo info, const std::string& data)
    {
        if (info.response_code != 200)
        {
            LOG(WARNING) << "Parse ervs_event_tracking response failure: invalid status!\n"
                << "response info: " << info;
            return EventTrackingResult(false);
        }

        return EventTrackingResult(true);
    }
    EventTrackingResult ParseEventTrackingResponseNew(ResponseInfo info, const std::string& data)
    {
        if (info.response_code != 200)
        {
            LOG(WARNING) << "Parse ervs_event_tracking response failure: invalid status!\n"
                << "response info: " << info;
            return EventTrackingResult(false);
        }

        return EventTrackingResult(true);
    }

    // We now do nothing even if the report fails.
    using DefaultHandler = std::function<void(bool)>;
    void HandleReportResponse(bool)
    {
    }

}   // namespace

namespace secret
{
    RequestProxy EventTrackingServiceImpl::ReportLivehimeBehaviorEventViaSrv(
        LivehimeViaServerBehaviorEvent event_id,
        const base::StringPairs& event_params)
    {
        return RequestProxy(nullptr);
    }

    RequestProxy EventTrackingServiceImpl::ReportLivehimeBehaviorEventViaSrvNew(
        LivehimeViaServerBehaviorEventNew event_id,
        const base::StringPairs& event_params)
    {
        return RequestProxy(nullptr);
    }

}   // namespace secret
