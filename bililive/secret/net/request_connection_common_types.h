/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_NET_REQUEST_CONNECTION_COMMON_TYPES_H_
#define BILILIVE_SECRET_NET_REQUEST_CONNECTION_COMMON_TYPES_H_

#include <map>
#include <string>

#include "net/url_request/url_fetcher.h"

#include "url/gurl.h"

namespace secret {

using RequestType = net::URLFetcher::RequestType;

using RequestHeaders = std::map<std::string, std::string>;

// `query_string` should be already escaped.
GURL AppendQueryStringToGURL(const GURL& original, const std::string& query_string);

// (content-type, content)
using RequestContent = std::pair<std::string, std::string>;

struct RequestData {
    virtual ~RequestData() = default;

    virtual bool empty() const = 0;

    virtual RequestContent ToRequestContent() const = 0;

    virtual std::unique_ptr<RequestData> Clone() const = 0;
};

struct RequestRaw : RequestData {
    std::string raw_data;

    explicit RequestRaw(const std::string& raw);

    bool empty() const override;

    RequestContent ToRequestContent() const override;

    std::unique_ptr<RequestData> Clone() const override;
};

struct RequestParams : RequestData {
    using Params = std::map<std::string, std::string>;
    using key_compare = Params::key_compare;

    Params params;

    RequestParams() = default;

    explicit RequestParams(const Params& params);

    RequestParams(std::initializer_list<Params::value_type> init);

    Params::mapped_type& operator[](const Params::key_type& key);

    bool Get(const Params::key_type& key, Params::mapped_type& value) const;

    bool empty() const override;

    RequestContent ToRequestContent() const override;

    std::unique_ptr<RequestData> Clone() const override;
};

/**
 * 以 std::vector 作为容器，用于传递多个参数名一样的参数
 */
struct RequestParams2 : RequestData {
    using Params = std::vector<std::pair<std::string, std::string>>;

    Params params;

    RequestParams2() = default;

    explicit RequestParams2(const Params& params);

    RequestParams2(std::initializer_list<Params::value_type> init);

    std::string& operator[](const std::string& key);

    bool empty() const override;

    RequestContent ToRequestContent() const override;

    std::unique_ptr<RequestData> Clone() const override;
};

struct RequestJSON : RequestData {
    std::string json_data;

    explicit RequestJSON(const std::string& json);

    bool empty() const override;

    RequestContent ToRequestContent() const override;

    std::unique_ptr<RequestData> Clone() const override;
};

struct RequestUpload : RequestData {
    using Data = std::string;
    using ExtraParams = std::map<std::string, std::string>;

    std::string data_name;
    std::string filename;
    std::string mime_type;
    Data data;

    ExtraParams extra_params;

    RequestUpload::RequestUpload(const std::string& filename, const Data& data);

    RequestUpload::RequestUpload(const std::string& filename, const Data& data, const ExtraParams& params);

    bool empty() const override;

    RequestContent ToRequestContent() const override;

    std::unique_ptr<RequestData> Clone() const override;

private:
    std::string GenerateMultipartRequestBody(const std::string& boundary) const;
};

struct ResponseInfo {
    // Underlying implementation-defined error code.
    int error_code;

    // HTTP response code.
    int response_code;

    // HTTP response headers.
    const net::HttpResponseHeaders* response_headers;

    // Some request information for diagnosis.

    const GURL* request_url;

    const RequestData* request_data;

    ResponseInfo(int error_code, int response_code, net::HttpResponseHeaders* headers, const GURL* url,
                 const RequestData* request_data);
};

std::ostream& operator<<(std::ostream& os, const ResponseInfo& info);

}   // namespace secret

#endif  // BILILIVE_SECRET_NET_REQUEST_CONNECTION_COMMON_TYPES_H_
