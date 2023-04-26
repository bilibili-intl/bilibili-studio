#ifndef BILILIVE_BILILIVE_BILILIVE_THREAD_H
#define BILILIVE_BILILIVE_BILILIVE_THREAD_H


#include <string>
#include "base/basictypes.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/task_runner_util.h"
#include "base/time/time.h"


namespace base
{
    class MessageLoop;
    class SequencedWorkerPool;
    class Thread;
}


class BililiveThreadDelegate;
class BililiveThreadImpl;


class BililiveThread
{
public:
    enum ID
    {
        UI,

        DB,

        FILE,

        IO,

        ID_COUNT
    };

    static bool PostTask(ID identifier,
                         const tracked_objects::Location &from_here,
                         const base::Closure &task);
    static bool PostDelayedTask(ID identifier,
                                const tracked_objects::Location &from_here,
                                const base::Closure &task,
                                base::TimeDelta delay);
    static bool PostNonNestableTask(ID identifier,
                                    const tracked_objects::Location &from_here,
                                    const base::Closure &task);
    static bool PostNonNestableDelayedTask(
        ID identifier,
        const tracked_objects::Location &from_here,
        const base::Closure &task,
        base::TimeDelta delay);

    static bool PostTaskAndReply(
        ID identifier,
        const tracked_objects::Location &from_here,
        const base::Closure &task,
        const base::Closure &reply);

    template <typename ReturnType, typename ReplyArgType>
    static bool PostTaskAndReplyWithResult(
        ID identifier,
        const tracked_objects::Location &from_here,
        const base::Callback<ReturnType(void)> &task,
        const base::Callback<void(ReplyArgType)> &reply)
    {
        scoped_refptr<base::MessageLoopProxy> message_loop_proxy =
            GetMessageLoopProxyForThread(identifier);
        return base::PostTaskAndReplyWithResult(
                   message_loop_proxy.get(), from_here, task, reply);
    }

    template <class T>
    static bool DeleteSoon(ID identifier,
                           const tracked_objects::Location &from_here,
                           const T *object)
    {
        return GetMessageLoopProxyForThread(identifier)->DeleteSoon(
                   from_here, object);
    }

    template <class T>
    static bool ReleaseSoon(ID identifier,
                            const tracked_objects::Location &from_here,
                            const T *object)
    {
        return GetMessageLoopProxyForThread(identifier)->ReleaseSoon(
                   from_here, object);
    }

    static bool PostBlockingPoolTask(const tracked_objects::Location &from_here,
                                     const base::Closure &task);
    static bool PostBlockingPoolTaskAndReply(
        const tracked_objects::Location &from_here,
        const base::Closure &task,
        const base::Closure &reply);
    static bool PostBlockingPoolSequencedTask(
        const std::string &sequence_token_name,
        const tracked_objects::Location &from_here,
        const base::Closure &task);

    static base::SequencedWorkerPool *GetBlockingPool();

    static bool IsThreadInitialized(ID identifier);

    static bool CurrentlyOn(ID identifier);

    static bool IsMessageLoopValid(ID identifier);

    static bool GetCurrentThreadIdentifier(ID *identifier);

    static scoped_refptr<base::MessageLoopProxy> GetMessageLoopProxyForThread(
        ID identifier);

    static base::MessageLoop *UnsafeGetMessageLoopForThread(ID identifier);

    static void SetDelegate(ID identifier, BililiveThreadDelegate *delegate);

    template<ID thread>
    struct DeleteOnThread
    {
        template<typename T>
        static void Destruct(const T *x)
        {
            if (CurrentlyOn(thread))
            {
                delete x;
            }
            else
            {
                if (!DeleteSoon(thread, FROM_HERE, x))
                {
#if defined(UNIT_TEST)
                    LOG(ERROR) << "DeleteSoon failed on thread " << thread;
#endif
                }
            }
        }
    };

    struct DeleteOnUIThread : public DeleteOnThread<UI> { };
    struct DeleteOnIOThread : public DeleteOnThread<IO> { };
    struct DeleteOnFileThread : public DeleteOnThread<FILE> { };
    struct DeleteOnDBThread : public DeleteOnThread<DB> { };

private:
    friend class BililiveThreadImpl;

    BililiveThread() {}
    DISALLOW_COPY_AND_ASSIGN(BililiveThread);
};


#endif