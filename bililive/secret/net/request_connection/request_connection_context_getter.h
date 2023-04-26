/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_NET_REQUEST_CONNECTION_REQUEST_CONNECTION_CONTEXT_GETTER_H_
#define BILILIVE_SECRET_NET_REQUEST_CONNECTION_REQUEST_CONNECTION_CONTEXT_GETTER_H_

#include <atomic>
#include <memory>

#include "base/memory/ref_counted.h"
#include "base/single_thread_task_runner.h"
#include "base/synchronization/waitable_event.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"

namespace secret {

class RequestConnectionContextGetter : public net::URLRequestContextGetter {
public:
    RequestConnectionContextGetter(const scoped_refptr<base::SingleThreadTaskRunner>& task_runner,
                                   const scoped_refptr<base::SingleThreadTaskRunner>& relay_runner);

    ~RequestConnectionContextGetter();

    scoped_refptr<base::SingleThreadTaskRunner> GetRelayThreadRunner() const;

    // Override net::URLRequestContextGetter.

    net::URLRequestContext* GetURLRequestContext() override;

    scoped_refptr<base::SingleThreadTaskRunner> GetNetworkTaskRunner() const override;

    void MarkAsShuttingDown();

    bool IsShuttingDown() const;

    bool use_system_proxy() const;

private:
    void Cleanup();

    DISALLOW_COPY_AND_ASSIGN(RequestConnectionContextGetter);

private:
    scoped_refptr<base::SingleThreadTaskRunner> task_thread_runner_;
    scoped_refptr<base::SingleThreadTaskRunner> relay_thread_runner_;
    std::unique_ptr<net::URLRequestContext> context_;
    bool use_system_proxy_;
    std::atomic<bool> shutting_down_;
    base::WaitableEvent shutdown_event_;
};

}   // namespace secret

#endif  // BILILIVE_SECRET_NET_REQUEST_CONNECTION_REQUEST_CONNECTION_CONTEXT_GETTER_H_
