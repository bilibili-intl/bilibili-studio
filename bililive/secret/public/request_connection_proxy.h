/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_PUBLIC_REQUEST_CONNECTION_PROXY_H_
#define BILILIVE_SECRET_PUBLIC_REQUEST_CONNECTION_PROXY_H_

#include "bilibase/basic_macros.h"

#include "bililive/secret/net/request_connection/request_connection_base.h"
#include "base/thread_task_runner_handle.h"
#include "base/logging.h"

namespace secret {

// Shields manipulation details of request connection related classes away from client users.
// Also inlines all functions on purpose, to avoid becoming a part of module on binary level.
// This class should avoid any operations involving heap allocation, otherwise, we might find
// ourselves stuck in deallocate-across-boundary issues.
class RequestProxy {
public:
    explicit RequestProxy(RequestConnectionBase* request)
        : request_(request)
    {
        //DCHECK(request_);
    }

    ~RequestProxy() = default;

    DEFAULT_COPY(RequestProxy);

    const RequestProxy& ReplyOn(ReplyScheduler scheduler) const
    {
        if (request_) {
            request_->SetReplyScheduler(scheduler);
        }

        return *this;
    }

    void Call(int timeout_delta_in_ms = 60000) const
    {
        // 为实现国内和海外埋点兼容，当request_为nullptr的话，则认为是空请求，上层调用call后这里不做处理
        if (request_) {
            request_->SetCallingThreadRunner(base::ThreadTaskRunnerHandle::Get());
            request_->SetTimeoutDelta(timeout_delta_in_ms);
            request_->Start();
        }
    }

private:
    RequestConnectionBase* request_;
};

}   // namespace secret

#endif  // BILILIVE_SECRET_PUBLIC_REQUEST_CONNECTION_PROXY_H_