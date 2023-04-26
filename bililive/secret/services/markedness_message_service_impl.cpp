#include "bililive/secret/services/live_streaming_service_impl.h"

#include "bililive/secret/services/live_streaming_common_data.h"
#include "bililive/secret/services/json_parse.h"
#include "bililive/secret/services/service_utils.h"

namespace {

    using secret::ResponseInfo;
    using secret::RequestParams;


    RequestParams MMRequestParams() {
        using namespace secret;

        return RequestParams{
            { kArgAppVersion, BililiveContext::Current()->GetExecutableVersionAsASCII() },
            { kArgAppKey, kAppKey },
            { kArgPlatform, kPlatform },
            { "build", std::to_string(BililiveContext::Current()->GetExecutableBuildNumber()) },
            { "access_key", QueryAccessToken() },
        };
    }

    using MarkednessMessageReportResult = std::tuple<bool, int, std::string>;
    using MarkednessMessageReportParser = std::function<MarkednessMessageReportResult(ResponseInfo, const std::string&)>;
    MarkednessMessageReportResult ParseMarkednessMessageReportResponse(ResponseInfo info, const std::string& data) {
        int code = 0;

        std::unique_ptr<base::DictionaryValue> content(CommonParse(info, data, "markedness-message-report", &code));
        if (!content) {
            return {};
        }

        std::string error_msg;

        if (code != 0) {
            content->GetString("msg", &error_msg);
        }

        return MarkednessMessageReportResult(true, code, error_msg);
    }

    using MarkednessMessageDeleteResult = std::tuple<bool, int, std::string>;
    using MarkednessMessageDeleteParser = std::function<MarkednessMessageDeleteResult(ResponseInfo, const std::string&)>;
    MarkednessMessageDeleteResult ParseMarkednessMessageDeleteResponse(ResponseInfo info, const std::string& data) {
        int code = 0;

        std::unique_ptr<base::DictionaryValue> content(CommonParse(info, data, "markedness-message-remove", &code));
        if (!content) {
            return {};
        }

        std::string error_msg;

        if (code != 0) {
            content->GetString("msg", &error_msg);
        }

        return MarkednessMessageDeleteResult(true, code, error_msg);
    }

}

namespace secret {



}