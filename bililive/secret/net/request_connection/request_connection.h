/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_NET_REQUEST_CONNECTION_REQUEST_CONNECTION_H_
#define BILILIVE_SECRET_NET_REQUEST_CONNECTION_REQUEST_CONNECTION_H_

#include <map>
#include <memory>
#include <utility>

#include "base/basictypes.h"
#include "base/synchronization/waitable_event.h"
#include "base/sys_info.h"
#include "base/threading/thread_checker.h"
#include "net/url_request/url_fetcher.h"
#include "net/url_request/url_fetcher_delegate.h"
#include "url/gurl.h"

#include "bilibase/apply_tuple_to_function.h"
#include "bilibase/string_format.h"

#include "bililive/common/bililive_context.h"
#include "bililive/secret/net/request_connection_common_types.h"
#include "bililive/secret/net/request_connection/request_connection_base.h"
#include "bililive/secret/net/request_connection/request_connection_context_getter.h"
#include "bililive/secret/services/service_utils.h"

namespace secret {

template<typename ResponseParser, typename Handler>
class RequestConnection
    : public RequestConnectionBase,
      public net::URLFetcherDelegate {
public:
    ~RequestConnection()
    {
        if (request_) {
            auto relay_thread_runner = request_context_getter_->GetRelayThreadRunner();
            if (relay_thread_runner->BelongsToCurrentThread()) {
                DismissRequestOnRelayThread();
            } else {
                // Once we get into destructor, the ref-count of the instance has already been
                // down to 0. Don't sabotage it.
                relay_thread_runner->PostTask(FROM_HERE,
                                              base::Bind(&RequestConnection::DismissRequestOnRelayThread,
                                                         base::Unretained(this)));

            }

            request_dismissed_event_.Wait();
        }
    }

    static scoped_refptr<RequestConnection> Create(
        const GURL& url,
        net::URLFetcher::RequestType method,
        const RequestHeaders& extra_request_headers,
        const RequestData& request_data,
        const ResponseParser& parser,
        const Handler& handler,
        const scoped_refptr<RequestConnectionContextGetter>& context_getter,
        RequestCompleteCallback* callback)
    {
        DCHECK(url.is_valid());
        DCHECK(context_getter);
        DCHECK(callback);

        return scoped_refptr<RequestConnection>(
            new RequestConnection(url,
                                  method,
                                  extra_request_headers,
                                  request_data,
                                  parser,
                                  handler,
                                  context_getter,
                                  callback));
    }

    // Overrides RequestConnectionBase.

    void Start() override
    {
        CHECK(calling_thread_runner_);

        auto relay_thread_runner = request_context_getter_->GetRelayThreadRunner();
        relay_thread_runner->PostTask(FROM_HERE,
                                      base::Bind(&RequestConnection::CreateAndStartRequestOnRelayThread,
                                                 this));
    }

    void SetCallingThreadRunner(const scoped_refptr<base::SingleThreadTaskRunner>& runner) override
    {
        calling_thread_runner_ = runner;
    }

private:
    using parser_result_package_type =
        decltype(std::declval<ResponseParser>()({0, 0, nullptr, nullptr, nullptr}, {}));

    RequestConnection(const GURL& url,
                      net::URLFetcher::RequestType method,
                      const RequestHeaders& extra_request_headers,
                      const RequestData& request_data,
                      const ResponseParser& parser,
                      const Handler& handler,
                      const scoped_refptr<RequestConnectionContextGetter>& context_getter,
                      RequestCompleteCallback* callback)
        : url_(url),
          method_(method),
          extra_request_headers_(extra_request_headers),
          request_data_(request_data.Clone()),
          response_parser_(parser),
          handler_(handler),
          complete_callback_(callback),
          request_context_getter_(context_getter),
          request_dismissed_event_(false, false),
          weakptr_for_timeout_task_(this)
    {
        if (method == net::URLFetcher::GET && !request_data.empty()) {
            auto query_string = request_data.ToRequestContent().second;
            url_ = AppendQueryStringToGURL(url, query_string);
        }
    }

    void CreateAndStartRequestOnRelayThread()
    {
        DCHECK(request_context_getter_->GetRelayThreadRunner()->BelongsToCurrentThread());

        request_.reset(net::URLFetcher::Create(url_, method_, this));

        if (!request_context_getter_->use_system_proxy()) {
            request_->SetLoadFlags(net::LOAD_BYPASS_PROXY);
        }

        request_->SetRequestContext(request_context_getter_.get());

        static std::string ua;
        if (ua.empty())
        {
            //ios:
            //bili-universal/63500100 CFNetwork/1240.0.4 Darwin/20.5.0 os/ios model/iPhone 11 mobi_app/iphone build/63500100 osVer/14.6 network/2 channel/AppStore
            //
            //安卓:
            //Mozilla/5.0 BiliDroid/6.35.0 (bbcallen@gmail.com) os/android model/DUK-AL20 mobi_app/android build/6350300 channel/huawei innerVer/6350310 osVer/9 network/2

            // 旧UA
            // Bililive/3.40.0.2287 (LiveHime; Windows)
            //
            // 新UA
            // LiveHime/3.40.0.2287 os/Windows pc_app/livehime build/2287 osVer/10.0_x86_64
            auto os_version = base::SysInfo::OperatingSystemVersion();
            auto os_arch = base::SysInfo::OperatingSystemArchitecture();
            ua = bilibase::StringFormat("LiveHime/{0} os/Windows pc_app/livehime build/{1} osVer/{2}_{3}",
                BililiveContext::Current()->GetExecutableVersionAsASCII(),
                BililiveContext::Current()->GetExecutableBuildNumber(),
                os_version,
                os_arch
            );
        }

        // Set User-Agent identity.
        request_->AddExtraRequestHeader(
            bilibase::StringFormat("{0}: {1}",
                net::HttpRequestHeaders::kUserAgent,
                ua));
        /*request_->AddExtraRequestHeader(
            bilibase::StringFormat("{0}: Bililive/{1} (LiveHime; Windows)",
                                   net::HttpRequestHeaders::kUserAgent,
                                   BililiveContext::Current()->GetExecutableVersionAsASCII()));*/

        if (!extra_request_headers_.empty()) {
            for (const auto& header : extra_request_headers_) {
                request_->AddExtraRequestHeader(
                    bilibase::StringFormat("{0}: {1}", header.first, header.second));
            }

            // According to RFC 6265, the use of multiple cookie headers in request is
            // prohibited. If we want to set our own cookie, we must first prevent URLFetcher
            // sending its tracked cookie, which would overwrite the cookie set by us.
            if (extra_request_headers_.find(net::HttpRequestHeaders::kCookie) !=
                extra_request_headers_.cend()) {
                //auto new_flags = request_->GetLoadFlags() | net::LOAD_DO_NOT_SEND_COOKIES;
                int new_flags = request_->GetLoadFlags();
                //new_flags &= ~net::LOAD_DO_NOT_SAVE_COOKIES;
                new_flags &= ~net::LOAD_DO_NOT_SEND_COOKIES;
                request_->SetLoadFlags(new_flags);
            }
        }

        // Fill the request body.
        // HTTP PUT passes parameters via request body as POST does.
        if (method_ != net::URLFetcher::GET && !request_data_->empty()) {
            auto request_content = request_data_->ToRequestContent();
            request_->SetUploadData(request_content.first, request_content.second);
        }

        request_->Start();

        // 启动计时
        base::ThreadTaskRunnerHandle::Get()->PostDelayedTask(FROM_HERE,
            base::Bind(&RequestConnection::OnRequestTimeout, weakptr_for_timeout_task_.GetWeakPtr()), 
            base::TimeDelta::FromMilliseconds(timeout_delta()));
    }

    void DismissRequestOnRelayThread()
    {
        DCHECK(request_context_getter_->GetRelayThreadRunner()->BelongsToCurrentThread());

        request_ = nullptr;
        request_dismissed_event_.Signal();
    }

    void OnURLFetchComplete(const net::URLFetcher* source) override
    {
        weakptr_for_timeout_task_.InvalidateWeakPtrs();

        // As the matter of fact, we have no idea how to handle unsuccessful cases while with
        // such little knowledge about the request context.
        // Thus, let the response-parser make the right call.
        ResponseInfo response_info(source->GetStatus().error(), source->GetResponseCode(),
                                   source->GetResponseHeaders(), &url_, request_data_.get());

        std::string response_data;
        if (!source->GetResponseAsString(&response_data)) {
            LOG(WARNING) << "Failed to read response as string!";
        }

        DoResponse(response_info, response_data);
    }

    void OnRequestTimeout()
    {
        LOG(WARNING) << "request \"" << url_.path() << "\" timeout with " << timeout_delta() << " ms!";

        DCHECK(request_context_getter_->GetRelayThreadRunner()->BelongsToCurrentThread());

        weakptr_for_timeout_task_.InvalidateWeakPtrs();

        ResponseInfo response_info(net::ERR_TIMED_OUT, -1,
            nullptr, &url_, request_data_.get()); 

        DoResponse(response_info, "");
    }

    void DoResponse(ResponseInfo response_info, const std::string& response_data)
    {
        DCHECK(request_);
        if (!request_)
        {
            return;
        }

        auto result_package = response_parser_(response_info, response_data);
        auto* package = new parser_result_package_type(std::move(result_package));

        // `request_` must be released on the same thread as where it was created,
        // i.e. relay thread. So we just do it here.
        request_ = nullptr;

        if (reply_scheduler() == ReplyScheduler::TaskThread)
        {
            auto task_thread_runner = request_context_getter_->GetNetworkTaskRunner();
            task_thread_runner->PostTaskAndReply(
                FROM_HERE,
                base::Bind(&RequestConnection::InvokeHandlerOnTaskThread,
                    this,
                    base::Owned(package)),
                base::Bind(&RequestConnection::TransitionToNotifyRequestEnd,
                    this));
        }
        else
        {
            calling_thread_runner_->PostTask(
                FROM_HERE,
                base::Bind(&RequestConnection::CompleteRequestOnCallingThread,
                    this,
                    base::Owned(package)));
        }
    }

    void CompleteRequestOnCallingThread(parser_result_package_type* package)
    {
        DCHECK(calling_thread_checker_.CalledOnValidThread());

        bilibase::apply(handler_, *package);
        NotifyRequestEnd();
    }

    void InvokeHandlerOnTaskThread(parser_result_package_type* package)
    {
        DCHECK(request_context_getter_->GetNetworkTaskRunner()->BelongsToCurrentThread());

        bilibase::apply(handler_, *package);
    }

    void TransitionToNotifyRequestEnd()
    {
        calling_thread_runner_->PostTask(FROM_HERE,
                                         base::Bind(&RequestConnection::NotifyRequestEnd,
                                                    this));
    }

    void NotifyRequestEnd()
    {
        DCHECK(calling_thread_checker_.CalledOnValidThread());

        if (!request_context_getter_->IsShuttingDown() && complete_callback_) {
            complete_callback_->OnRequestEnd(this);
        }
    }
        
    DISALLOW_COPY_AND_ASSIGN(RequestConnection);

private:
    GURL url_;
    net::URLFetcher::RequestType method_;
    RequestHeaders extra_request_headers_;
    std::unique_ptr<RequestData> request_data_;
    ResponseParser response_parser_;
    Handler handler_;
    RequestCompleteCallback* complete_callback_;
    base::ThreadChecker calling_thread_checker_;
    scoped_refptr<RequestConnectionContextGetter> request_context_getter_;
    scoped_refptr<base::SingleThreadTaskRunner> calling_thread_runner_;
    base::WaitableEvent request_dismissed_event_;
    std::unique_ptr<net::URLFetcher> request_;
    base::WeakPtrFactory<RequestConnection> weakptr_for_timeout_task_;
};

}   // namespace secret

#endif  // BILILIVE_SECRET_NET_REQUEST_CONNECTION_REQUEST_CONNECTION_H_
