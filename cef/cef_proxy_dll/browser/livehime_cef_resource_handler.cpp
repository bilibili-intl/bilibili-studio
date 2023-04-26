#include "stdafx.h"
#include "livehime_cef_resource_handler.h"

#include <mutex>
#include <WinInet.h>

#include "bililive_browser/public/bililive_browser_ipc_messages.h"
#include "bililive_browser/public/bililive_browser_switches.h"
#include "bililive_browser/public/bililive_cef_headers.h"

#include "cef_core/include/cef_app.h"
#include "cef_core/include/cef_browser.h"
#include "cef_core/include/wrapper/cef_closure_task.h"

#include "cef_proxy_dll/event_dispatcher/livehime_cef_proxy_events_dispatcher.h"
#include "cef_proxy_dll/public/livehime_cef_proxy_constants.h"


struct ExtensionToMimeType {
    const char* extension;
    const char* mime_type;
};

// The mapping from web resources file name extension to MIME type
constexpr ExtensionToMimeType kMimeTypeMappings[] = {
    {".html", "text/html"},
    {".js", "application/javascript"},
    {".css", "text/css"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpeg", "image/jpeg"},
    {".jpe", "image/jpeg"},
    {".jpg", "image/jpeg"},
    {".mp4", "video/mp4"},
    {".webp", "image/webp"},
    {".svg", "image/svg+xml"},
    {".ttf", "application/x-font-ttf"},
};

LivehimeCefResourceHandler::LivehimeCefResourceHandler(std::string url, 
    std::wstring path, std::string mime_type)
    : url_(url), 
      path_(path), 
      mime_type_(mime_type), 
      size_(0),
      offset_(0),
      buffer_(nullptr) {
    DCHECK(!path_.empty());
    DCHECK(!mime_type_.empty());
    DCHECK(!buffer_);
}

LivehimeCefResourceHandler::~LivehimeCefResourceHandler() {
    DCHECK(!buffer_);
    if (buffer_) {
        delete[] buffer_;
        buffer_ = nullptr;
    }
}

bool LivehimeCefResourceHandler::Open(CefRefPtr<CefRequest> request,
    bool& handle_request,
    CefRefPtr<CefCallback> callback) {
    request_url_ = request->GetURL();

    std::ifstream file;
    file.open(path_, std::ios_base::in | std::ios_base::binary);
    if (file.is_open()) {
        std::filebuf* pbuf;
        pbuf = file.rdbuf();
        size_ = pbuf->pubseekoff(0, std::ios_base::end, std::ios_base::in);
        pbuf->pubseekpos(0, std::ios_base::in);

        DCHECK(!buffer_);
        buffer_ = new unsigned char[size_];
        pbuf->sgetn(reinterpret_cast<char*>(buffer_), size_);
        file.close();
        offset_ = 0;
    } else {
        DCHECK(false);
        LOG(WARNING) << "[OFFLINE] webview offline resource open failed, url=" << request_url_ << ", path=" << path_;
        handle_request = false;
        return false;
    }

    handle_request = true;
    return true;
}

bool LivehimeCefResourceHandler::ProcessRequest(CefRefPtr<CefRequest> request,
    CefRefPtr<CefCallback> callback) {
    request_url_ = request->GetURL();
    callback->Continue();
    return true;
}

void LivehimeCefResourceHandler::GetResponseHeaders(CefRefPtr<CefResponse> response,
    int64& response_length,
    CefString& redirectUrl) {
    CefResponse::HeaderMap headerMap;
    headerMap.insert(std::pair<CefString, CefString>("accept-ranges", "bytes"));
    headerMap.insert(std::pair<CefString, CefString>("access-control-allow-credentials", "true"));
    headerMap.insert(std::pair<CefString, CefString>("access-control-allow-headers", "Origin,No-Cache,X-Requested-With,If-Modified-Since,Pragma,Last-Modified,Cache-Control,Expires,Content-Type,Access-Control-Allow-Credentials,DNT,X-CustomHeader,Keep-Alive,User-Agent,X-Cache-Webcdn"));
    headerMap.insert(std::pair<CefString, CefString>("access-control-allow-methods", "GET, POST, OPTIONS"));
    headerMap.insert(std::pair<CefString, CefString>("access-control-allow-origin", "*"));
    headerMap.insert(std::pair<CefString, CefString>("access-control-expose-headers", "Content-Length,X-Cache-Webcdn"));
    headerMap.insert(std::pair<CefString, CefString>("cache-control", "no-cache"));
    headerMap.insert(std::pair<CefString, CefString>("server", "openresty"));
    headerMap.insert(std::pair<CefString, CefString>("etag", "62bbf82d-e42"));
    headerMap.insert(std::pair<CefString, CefString>("x-contents-from", "offline"));

    DCHECK(!mime_type_.empty());
    if (mime_type_.empty()) {
        LOG(WARNING) << "[OFFLINE] unresolved MimeType, url=" << request_url_;
        // 如果没有配置MimeType，从当前资源url中再尝试查找识别一次
        for (const auto& entry : kMimeTypeMappings) {
            if ((int)(request_url_.find(entry.extension)) >= 0) {
                LOG(WARNING) << "[OFFLINE] resolved MimeType=" << entry.mime_type;
                mime_type_ = entry.mime_type;
                break;
            }
        }
    }

    response->SetMimeType(mime_type_);
    headerMap.insert(std::pair<CefString, CefString>("Content-Type", mime_type_));

    response_length = size_;
    std::string length = std::to_string(response_length);
    headerMap.insert(std::pair<CefString, CefString>("content-length", length));

    response->SetHeaderMap(headerMap);
    response->SetStatus(200);
    response->SetURL(CefString(request_url_));
}

bool LivehimeCefResourceHandler::Skip(int64 bytes_to_skip,
    int64& bytes_skipped,
    CefRefPtr<CefResourceSkipCallback> callback) {
    bytes_skipped = -2;
    return false;
}

bool LivehimeCefResourceHandler::Read(void* data_out,
    int bytes_to_read,
    int& bytes_read,
    CefRefPtr<CefResourceReadCallback> callback) {
    bool has_data = false;
    bytes_read = 0;
    size_t size = size_;
    if (offset_ < size) {
        int transfer_size = (std::min)(bytes_to_read, static_cast<int>(size - offset_));
        memcpy(data_out, buffer_ + offset_, transfer_size);
        offset_ += transfer_size;
        bytes_read = transfer_size;
        has_data = true;
    } else {
        delete[] buffer_;
        buffer_ = nullptr;
    }

    return has_data;
}

bool LivehimeCefResourceHandler::ReadResponse(void* data_out,
    int bytes_to_read,
    int& bytes_read,
    CefRefPtr<CefCallback> callback) {
    bytes_read = -2;
    return false;
}

void LivehimeCefResourceHandler::Cancel() {
}