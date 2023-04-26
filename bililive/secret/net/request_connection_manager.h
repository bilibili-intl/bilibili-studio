/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_NET_REQUEST_CONNECTION_MANAGER_H_
#define BILILIVE_SECRET_NET_REQUEST_CONNECTION_MANAGER_H_

#include <set>

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread.h"
#include "url/gurl.h"

#include "bililive/secret/net/request_connection_common_types.h"
#include "bililive/secret/net/request_connection/request_connection.h"
#include "bililive/secret/public/request_connection_proxy.h"
#include "cef/cef_proxy_dll/public/livehime_cef_proxy_calldata.h"

namespace secret {

class RequestConnectionManager : public RequestConnectionBase::RequestCompleteCallback {
public:
    RequestConnectionManager();

    ~RequestConnectionManager();

    template<typename Parser, typename Handler>
    RequestProxy NewRequestConnection(const GURL& url, RequestType method, const RequestData& req_data,
                                      const Parser& parser, const Handler& handler)
    {
        RequestHeaders modify_extra_headers;
        GURL modify_url;
        FastForwardChangeEnv(url, {}, modify_url, modify_extra_headers);
        auto req_conn = RequestConnection<Parser, Handler>::Create(
            modify_url,
            method,
            modify_extra_headers,
            req_data,
            parser,
            handler,
            request_context_getter_,
            this);

        auto ptr = AddRequestConnection(req_conn);

        return RequestProxy(ptr);
    }

    template<typename Parser, typename Handler>
    RequestProxy NewRequestConnection(const GURL& url,
                                      RequestType method,
                                      const RequestHeaders& extra_headers,
                                      const RequestData& req_data,
                                      const Parser& parser,
                                      const Handler& handler)
    {
        RequestHeaders modify_extra_headers;
        GURL modify_url;
        FastForwardChangeEnv(url, extra_headers, modify_url, modify_extra_headers);
        auto req_conn = RequestConnection<Parser, Handler>::Create(
            modify_url,
            method,
            modify_extra_headers,
            req_data,
            parser,
            handler,
            request_context_getter_,
            this);

        auto ptr = AddRequestConnection(req_conn);

        return RequestProxy(ptr);
    }

    void SetAuthCookie(const cef_proxy::cookies& cookie);

	string GetCookieString() { return cookies_; }
    string GetCookieString(const std::string& doMain);
private:
    RequestConnectionBase* AddRequestConnection(const scoped_refptr<RequestConnectionBase>& req_conn);

    void RemoveRequestConnection(RequestConnectionBase* req_conn);

    // Override RequestConnectionBase::RequestCompleteCallback.

    void OnRequestEnd(RequestConnectionBase* req_conn) override;

    void OnCookieSet(bool success);
    // ff ÇÐ»»»·¾³
    void FastForwardChangeEnv(const GURL& url,const RequestHeaders& extra_headers,
         GURL& modify_url, RequestHeaders& modify_headers);

    DISALLOW_COPY_AND_ASSIGN(RequestConnectionManager);

private:
    base::Lock conn_access_mutex_;
    base::Thread network_io_thread_;
    base::Thread relay_thread_;

    bool ff_switch_ = false;
    std::string proxy_id_ = "";
    std::string proxy_env_ = "";

    scoped_refptr<RequestConnectionContextGetter> request_context_getter_;
    std::set<scoped_refptr<RequestConnectionBase>> request_connections_;

    base::WeakPtrFactory<RequestConnectionManager> weak_factory_;
    string cookies_;
    std::vector<std::string>cookies_list_;
};

}   // namespace secret

#endif  // BILILIVE_SECRET_NET_REQUEST_CONNECTION_MANAGER_H_