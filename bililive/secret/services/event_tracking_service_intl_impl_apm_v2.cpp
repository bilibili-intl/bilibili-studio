#include "bililive/secret/services/event_tracking_service_intl_impl.h"

#include <tuple>

#include "base/ext/callable_callback.h"
#include "base/file_util.h"
#include "base/guid.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "base/values.h"

#include "net/http/http_request_headers.h"

#include "url/gurl.h"

#include "bililive/bililive/utils/net_util.h"
#include "bililive/common/bililive_context.h"
#include "bililive/secret/core/bililive_secret_core_impl.h"
#include "bililive/secret/services/service_utils.h"


namespace
{
    using secret::ResponseInfo;

    const char kURLLancerDataCenter[] = "";

    // 埋点常规处理
    using EventTrackingResult = std::tuple<bool>;
    using EventTrackingParser = std::function<EventTrackingResult(ResponseInfo, const std::string&)>;

    EventTrackingResult ParseApmEventTrackingResponse(ResponseInfo info, const std::string& data)
    {
        if (info.response_code != 200)
        {
            LOG(WARNING) << "Parse event_tracking response failure: invalid status!\n"
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
    RequestProxy EventTrackingServiceIntlImpl::ReportApmEventTracking(const std::string& content)
    {
        return RequestProxy(nullptr);

        //RequestHeaders header;
        //header["Content-Encoding"] = "gzip";
        //header[net::HttpRequestHeaders::kContentLength] = std::to_string(content.length());

        //RequestUpload upload_body("livehime-apm", content);
        //upload_body.mime_type = "application/octet-stream";
        ////upload_body.extra_params[kArgBFSUploadBucket] = kBFSUploadBucket;

        //RequestParams params;
        //AppendCSRF(params);
        //GURL gurl = AppendQueryStringToGURL(GURL(kURLLancerDataCenter),
        //    params.ToRequestContent().second);

        //RequestProxy proxy =
        //    conn_manager_->NewRequestConnection<EventTrackingParser, DefaultHandler>(
        //        gurl,
        //        RequestType::POST,
        //        header,
        //        upload_body,
        //        ParseApmEventTrackingResponse,
        //        HandleReportResponse);

        //return proxy;
    }

    RequestProxy EventTrackingServiceIntlImpl::ReportLivehimeApmEventV2(
        const std::string& event_data)
    {
        return RequestProxy(nullptr);

        ////return ReportApmEventTracking(event_data);

        //RequestRaw params(event_data);
        //return ReportEventTracking(params);
    }

}   // namespace secret
