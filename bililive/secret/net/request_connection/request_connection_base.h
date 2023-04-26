/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_NET_REQUEST_CONNECTION_REQUEST_CONNECTION_BASE_H_
#define BILILIVE_SECRET_NET_REQUEST_CONNECTION_REQUEST_CONNECTION_BASE_H_

#include "base/memory/ref_counted.h"
#include "base/single_thread_task_runner.h"

namespace secret {

enum class ReplyScheduler {
    CallingThread,
    TaskThread
};

class RequestConnectionBase : public base::RefCountedThreadSafe<RequestConnectionBase> {
public:
    class RequestCompleteCallback {
    public:
        virtual ~RequestCompleteCallback() {}

        virtual void OnRequestEnd(RequestConnectionBase* instance) = 0;
    };

    void SetReplyScheduler(ReplyScheduler scheduler)
    {
        reply_scheduler_ = scheduler;
    }

    ReplyScheduler reply_scheduler() const
    {
        return reply_scheduler_;
    }

    void SetTimeoutDelta(int delta_in_ms)
    {
        timeout_delta_ = delta_in_ms;
    }

    int timeout_delta() const
    {
        return timeout_delta_;
    }

    // The module maintains another series copies of global/static members, even though they have
    // already been initialized in other modules; therefore, we need this interface to incorporate
    // in the task runner of calling thread.
    virtual void SetCallingThreadRunner(const scoped_refptr<base::SingleThreadTaskRunner>& runner) = 0;

    virtual void Start() = 0;

protected:
    virtual ~RequestConnectionBase() {}

    friend class base::RefCountedThreadSafe<RequestConnectionBase>;

private:
    ReplyScheduler reply_scheduler_ = ReplyScheduler::CallingThread;
    int timeout_delta_ = 60000; // 1min
};

}   // namespace secret

#endif  // BILILIVE_SECRET_NET_REQUEST_CONNECTION_REQUEST_CONNECTION_BASE_H_