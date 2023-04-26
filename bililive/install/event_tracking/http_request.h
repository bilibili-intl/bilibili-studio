/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_INSTALL_EVENT_TRACKING_HTTP_REQUEST_H_
#define BILILIVE_INSTALL_EVENT_TRACKING_HTTP_REQUEST_H_

#include <chrono>
#include <string>

namespace bililive {

class HttpRequest {
public:
    enum Method {
        Get,
        Post
    };

    HttpRequest() = delete;

    HttpRequest(const HttpRequest&) = delete;

    ~HttpRequest() = delete;

    HttpRequest& operator=(const HttpRequest&) = delete;

    // Each header line in `headers` must be terminated with CRLF.
    // Escape query-string in `params` if required before calling this fucntion.
    // If `timeout` is zero, it is ignored.
    // `response_body` or `response_code` can be NULL, if you have no interest on any of them.
    static bool Send(const std::string& url,
                     Method method,
                     const std::string& headers,
                     const std::string& params,
                     std::chrono::seconds timeout,
                     std::string* response_body,
                     int* response_code);
};

}   // namespace bililive

#endif  // BILILIVE_INSTALL_EVENT_TRACKING_HTTP_REQUEST_H_
