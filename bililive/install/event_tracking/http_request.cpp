/*
 @ 0xCCCCCCCC
*/

#include "bililive/install/event_tracking/http_request.h"

#include <Windows.h>
#include <WinInet.h>

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"

#include "bilibase/scope_guard.h"

namespace {

using bililive::HttpRequest;

const wchar_t kUserAgent[] = L"BilibiliUploader/2.0 (Windows)";
const wchar_t kContentTypeURLEncoded[] = L"Content-Type: application/x-www-form-urlencoded\r\n";

const wchar_t* kVerbs[] = {
    L"GET",
    L"POST"
};

const wchar_t* MethodToVerb(HttpRequest::Method method)
{
    if (method >= _countof(kVerbs)) {
        NOTREACHED();
        return nullptr;
    }

    return kVerbs[method];
}

bool ReadResponse(HINTERNET request, std::string* response_body)
{
    const DWORD kBufSize = 4 * 1024;
    std::vector<char> buf(kBufSize);

    DWORD bytes_read = 0;
    BOOL rv = FALSE;

    while (true) {
        rv = InternetReadFile(request, buf.data(), kBufSize, &bytes_read);
        if (!rv || bytes_read == 0) {
            PLOG_IF(WARNING, !rv) << "InternetReadFile failed!";
            break;
        }

        response_body->append(buf.data(), bytes_read);

        buf.clear();
    }

    return !!rv;
}

}   // namespace

namespace bililive {

bool HttpRequest::Send(const std::string& url,
                       Method method,
                       const std::string& headers,
                       const std::string& params,
                       std::chrono::seconds timeout,
                       std::string* response_body,
                       int* response_code)
{
    if (response_code) {
        *response_code = 0;
    }

    URL_COMPONENTS components;
    memset(&components, 0, sizeof(components));

    // Keep these buffers alive until we are done with components.
    wchar_t scheme[16], host[256], path[256];

    components.dwStructSize = sizeof(components);
    components.lpszScheme = scheme;
    components.dwSchemeLength = _countof(scheme);
    components.lpszHostName = host;
    components.dwHostNameLength = _countof(host);
    components.lpszUrlPath = path;
    components.dwUrlPathLength = _countof(path);

    // Break up the url into components.

    auto request_url = base::ASCIIToWide(url);
    if (method == Method::Get && !params.empty()) {
        request_url.append(L"?").append(base::ASCIIToWide(params));
    }

    if (!InternetCrackUrlW(request_url.c_str(), static_cast<DWORD>(request_url.size()), 0, &components)) {
        PLOG(WARNING) << "InternetCrackUrl failed!";
        return false;
    }

    // Initialize environment for WinINet.
    auto internet = InternetOpenW(kUserAgent, INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    ON_SCOPE_EXIT { if (internet) InternetCloseHandle(internet); };
    if (!internet) {
        PLOG(WARNING) << "InternetOpen failed!";
        return false;
    }

    // Open a HTTP session.
    auto connection = InternetConnectW(internet, components.lpszHostName, components.nPort, nullptr, nullptr,
                                       INTERNET_SERVICE_HTTP, 0, 0);
    ON_SCOPE_EXIT { if (connection) InternetCloseHandle(connection); };
    if (!connection) {
        PLOG(WARNING) << "InternetConnect failed!";
        return false;
    }

    bool secure = wcscmp(components.lpszScheme, L"https") == 0 ? true : false;
    DWORD http_open_flags = secure ? INTERNET_FLAG_SECURE : 0;
    // Do not automatically add cookie headers to requests, which may inadvertently clobber
    // our buvid cookie header.
    http_open_flags |= INTERNET_FLAG_NO_COOKIES;
    auto request = HttpOpenRequestW(connection, MethodToVerb(method), components.lpszUrlPath, nullptr,
                                    nullptr, nullptr, http_open_flags, 0);
    ON_SCOPE_EXIT { if (request) InternetCloseHandle(request); };
    if (!request) {
        PLOG(WARNING) << "HttpOpenRequest failed!";
        return false;
    }

    // Set up request headers.
    if (!headers.empty()) {
        auto extra_headers = base::ASCIIToWide(headers);
        if (!HttpAddRequestHeadersW(request, extra_headers.c_str(), static_cast<DWORD>(extra_headers.length()),
                                    HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE)) {
            PLOG(WARNING) << "HttpAddRequestHeaders failed!";
            return false;
        }
    }

    // Only url-encoded is currently supported.
    std::string request_body;
    if (method == Method::Post && !params.empty()) {
        HttpAddRequestHeadersW(request, kContentTypeURLEncoded, _countof(kContentTypeURLEncoded),
                               HTTP_ADDREQ_FLAG_ADD);
        request_body = params;
    }

    // Set send & receive timeout if specified.
    if (timeout.count() != 0) {
        int in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
        if (!InternetSetOptionW(request, INTERNET_OPTION_SEND_TIMEOUT, &in_ms, sizeof(in_ms)) ||
            !InternetSetOptionW(request, INTERNET_OPTION_RECEIVE_TIMEOUT, &in_ms, sizeof(in_ms))) {
            PLOG(WARNING) << "InternetSetOption failed!";
            return false;
        }
    }

    void* body_data = nullptr ;
    DWORD body_size = 0;

    if (method == Method::Post) {
        body_data = const_cast<char*>(request_body.data());
        body_size = static_cast<DWORD>(request_body.size());
    }

    if (!HttpSendRequestW(request, nullptr, 0, body_data, body_size)) {
        PLOG(WARNING) << "HttpSendRequest faield!";
        return false;
    }

    int status_code = 0;
    DWORD code_size = sizeof(status_code);
    if (!HttpQueryInfoW(request, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status_code, &code_size,
                        nullptr)) {
        PLOG(WARNING) << "HttpQueryInfo failed!";
        return false;
    }

    if (response_code) {
        *response_code = status_code;
    }

    bool succeeded = status_code == 200;
    if (!succeeded) {
        LOG(WARNING) << "Request failed with status code: " << status_code;
        return false;
    }

    if (response_body) {
        succeeded = ReadResponse(request, response_body);
    }

    return succeeded;
}

}   // namespace bililive
