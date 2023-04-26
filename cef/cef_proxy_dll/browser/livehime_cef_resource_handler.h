#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "cef_core/include/cef_resource_handler.h"
#include "cef_core/include/cef_command_line.h"

#include "cef_proxy_dll/public/livehime_cef_proxy_exports.h"


class LivehimeCefResourceHandler : public CefResourceHandler {
public:
    explicit LivehimeCefResourceHandler(std::string url, std::wstring path, std::string mime_type);
    virtual ~LivehimeCefResourceHandler();

    // CefResourceHandler
    bool Open(CefRefPtr<CefRequest> request,
        bool& handle_request,
        CefRefPtr<CefCallback> callback) override;

    bool ProcessRequest(CefRefPtr<CefRequest> request,
        CefRefPtr<CefCallback> callback) override;

    void GetResponseHeaders(CefRefPtr<CefResponse> response,
        int64& response_length,
        CefString& redirectUrl) override;

    bool Skip(int64 bytes_to_skip,
        int64& bytes_skipped,
        CefRefPtr<CefResourceSkipCallback> callback) override;

    bool Read(void* data_out,
        int bytes_to_read,
        int& bytes_read,
        CefRefPtr<CefResourceReadCallback> callback) override;

    bool ReadResponse(void* data_out,
        int bytes_to_read,
        int& bytes_read,
        CefRefPtr<CefCallback> callback) override;

    void Cancel() override;

private:
    size_t offset_ = 0;
    std::string url_;
    std::wstring path_;
    std::string mime_type_;
    std::string request_url_;
    unsigned char* buffer_ = nullptr;
    long size_ = 0;

    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(LivehimeCefResourceHandler);
    DISALLOW_COPY_AND_ASSIGN(LivehimeCefResourceHandler);
};