#include "bililive/bililive/bililive_thread_impl.h"

#include <string>
#include "base/atomicops.h"
#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/lazy_instance.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/threading/thread_restrictions.h"
#include "bililive/public/bililive/bililive_thread_delegate.h"

namespace
{
    static const char *g_bililive_thread_names[BililiveThread::ID_COUNT] =
    {
        "",  // UI (name assembled in bililive_main.cc).
        "DBThread",  // DB
        "FileThread",  // FILE
        "IOThread",  // IO
    };

    struct BililiveThreadGlobals
    {
        BililiveThreadGlobals()
            : blocking_pool(new base::SequencedWorkerPool(3, "Blocking"))
        {
            memset(threads, 0, BililiveThread::ID_COUNT * sizeof(threads[0]));
            memset(thread_delegates, 0,
                   BililiveThread::ID_COUNT * sizeof(thread_delegates[0]));
        }

        base::Lock lock;

        BililiveThreadImpl *threads[BililiveThread::ID_COUNT];

        BililiveThreadDelegate *thread_delegates[BililiveThread::ID_COUNT];

        const scoped_refptr<base::SequencedWorkerPool> blocking_pool;
    };

    base::LazyInstance<BililiveThreadGlobals>::Leaky
    g_globals = LAZY_INSTANCE_INITIALIZER;

}

BililiveThreadImpl::BililiveThreadImpl(ID identifier)
    : Thread(g_bililive_thread_names[identifier])
    , identifier_(identifier)
{
    Initialize();
}

BililiveThreadImpl::BililiveThreadImpl(ID identifier,
                                       base::MessageLoop *message_loop)
    : Thread(message_loop->thread_name().c_str()), identifier_(identifier)
{
    set_message_loop(message_loop);
    Initialize();
}

// static
void BililiveThreadImpl::ShutdownThreadPool()
{
    const int kMaxNewShutdownBlockingTasks = 1000;
    BililiveThreadGlobals &globals = g_globals.Get();
    globals.blocking_pool->Shutdown(kMaxNewShutdownBlockingTasks);
}

// static
void BililiveThreadImpl::FlushThreadPoolHelper()
{
    if (g_globals == NULL)
    {
        return;
    }
    g_globals.Get().blocking_pool->FlushForTesting();
}

void BililiveThreadImpl::Init()
{
    BililiveThreadGlobals &globals = g_globals.Get();

    using base::subtle::AtomicWord;
    AtomicWord *storage =
        reinterpret_cast<AtomicWord *>(&globals.thread_delegates[identifier_]);
    AtomicWord stored_pointer = base::subtle::NoBarrier_Load(storage);
    BililiveThreadDelegate *delegate =
        reinterpret_cast<BililiveThreadDelegate *>(stored_pointer);
    if (delegate)
    {
        delegate->Init();
        message_loop()->PostTask(FROM_HERE,
                                 base::Bind(&BililiveThreadDelegate::InitAsync,
                                            // Delegate is expected to exist for the
                                            // duration of the thread's lifetime
                                            base::Unretained(delegate)));
    }
}

MSVC_DISABLE_OPTIMIZE()
MSVC_PUSH_DISABLE_WARNING(4748)

NOINLINE void BililiveThreadImpl::UIThreadRun(base::MessageLoop *message_loop)
{
    volatile int line_number = __LINE__;
    Thread::Run(message_loop);
    CHECK_GT(line_number, 0);
}

NOINLINE void BililiveThreadImpl::DBThreadRun(base::MessageLoop *message_loop)
{
    volatile int line_number = __LINE__;
    Thread::Run(message_loop);
    CHECK_GT(line_number, 0);
}

NOINLINE void BililiveThreadImpl::FileThreadRun(
    base::MessageLoop *message_loop)
{
    volatile int line_number = __LINE__;
    Thread::Run(message_loop);
    CHECK_GT(line_number, 0);
}

NOINLINE void BililiveThreadImpl::IOThreadRun(base::MessageLoop *message_loop)
{
    volatile int line_number = __LINE__;
    Thread::Run(message_loop);
    CHECK_GT(line_number, 0);
}

MSVC_POP_WARNING()
MSVC_ENABLE_OPTIMIZE();

void BililiveThreadImpl::Run(base::MessageLoop *message_loop)
{
    BililiveThread::ID thread_id;
    if (!GetCurrentThreadIdentifier(&thread_id))
    {
        return Thread::Run(message_loop);
    }

    switch (thread_id)
    {
    case BililiveThread::UI:
        return UIThreadRun(message_loop);
    case BililiveThread::DB:
        return DBThreadRun(message_loop);
    case BililiveThread::FILE:
        return FileThreadRun(message_loop);
    case BililiveThread::IO:
        return IOThreadRun(message_loop);
    case BililiveThread::ID_COUNT:
        CHECK(false);  // This shouldn't actually be reached!
        break;
    }
    Thread::Run(message_loop);
}

void BililiveThreadImpl::CleanUp()
{
    BililiveThreadGlobals &globals = g_globals.Get();

    using base::subtle::AtomicWord;
    AtomicWord *storage =
        reinterpret_cast<AtomicWord *>(&globals.thread_delegates[identifier_]);
    AtomicWord stored_pointer = base::subtle::NoBarrier_Load(storage);
    BililiveThreadDelegate *delegate =
        reinterpret_cast<BililiveThreadDelegate *>(stored_pointer);

    if (delegate)
    {
        delegate->CleanUp();
    }
}

void BililiveThreadImpl::Initialize()
{
    BililiveThreadGlobals &globals = g_globals.Get();

    base::AutoLock lock(globals.lock);
    DCHECK(identifier_ >= 0 && identifier_ < ID_COUNT);
    DCHECK(globals.threads[identifier_] == NULL);
    globals.threads[identifier_] = this;
}

BililiveThreadImpl::~BililiveThreadImpl()
{
    Stop();

    BililiveThreadGlobals &globals = g_globals.Get();
    base::AutoLock lock(globals.lock);
    globals.threads[identifier_] = NULL;
#ifndef NDEBUG
    // Double check that the threads are ordered correctly in the enumeration.
    for (int i = identifier_ + 1; i < ID_COUNT; ++i)
    {
        DCHECK(!globals.threads[i]) <<
                                    "Threads must be listed in the reverse order that they die";
    }
#endif
}

// static
bool BililiveThreadImpl::PostTaskHelper(
    BililiveThread::ID identifier,
    const tracked_objects::Location &from_here,
    const base::Closure &task,
    base::TimeDelta delay,
    bool nestable)
{
    DCHECK(identifier >= 0 && identifier < ID_COUNT);

    BililiveThread::ID current_thread;
    bool target_thread_outlives_current =
        GetCurrentThreadIdentifier(&current_thread) &&
        current_thread >= identifier;

    BililiveThreadGlobals &globals = g_globals.Get();
    if (!target_thread_outlives_current)
    {
        globals.lock.Acquire();
    }

    base::MessageLoop *message_loop =
        globals.threads[identifier] ? globals.threads[identifier]->message_loop()
        : NULL;
    if (message_loop)
    {
        if (nestable)
        {
            message_loop->PostDelayedTask(from_here, task, delay);
        }
        else
        {
            message_loop->PostNonNestableDelayedTask(from_here, task, delay);
        }
    }

    if (!target_thread_outlives_current)
    {
        globals.lock.Release();
    }

    return !!message_loop;
}


class BililiveThreadMessageLoopProxy
    : public base::MessageLoopProxy
{
public:
    explicit BililiveThreadMessageLoopProxy(BililiveThread::ID identifier)
        : id_(identifier)
    {
    }

    // MessageLoopProxy implementation.
    virtual bool PostDelayedTask(
        const tracked_objects::Location &from_here,
        const base::Closure &task, base::TimeDelta delay) OVERRIDE
    {
        return BililiveThread::PostDelayedTask(id_, from_here, task, delay);
    }

    virtual bool PostNonNestableDelayedTask(
        const tracked_objects::Location &from_here,
        const base::Closure &task,
        base::TimeDelta delay) OVERRIDE
    {
        return BililiveThread::PostNonNestableDelayedTask(id_, from_here, task,
        delay);
    }

    virtual bool RunsTasksOnCurrentThread() const OVERRIDE
    {
        return BililiveThread::CurrentlyOn(id_);
    }

protected:
    virtual ~BililiveThreadMessageLoopProxy() {}

private:
    BililiveThread::ID id_;
    DISALLOW_COPY_AND_ASSIGN(BililiveThreadMessageLoopProxy);
};

// static
bool BililiveThread::PostBlockingPoolTask(
    const tracked_objects::Location &from_here,
    const base::Closure &task)
{
    return g_globals.Get().blocking_pool->PostWorkerTask(from_here, task);
}

bool BililiveThread::PostBlockingPoolTaskAndReply(
    const tracked_objects::Location &from_here,
    const base::Closure &task,
    const base::Closure &reply)
{
    return g_globals.Get().blocking_pool->PostTaskAndReply(
               from_here, task, reply);
}

// static
bool BililiveThread::PostBlockingPoolSequencedTask(
    const std::string &sequence_token_name,
    const tracked_objects::Location &from_here,
    const base::Closure &task)
{
    return g_globals.Get().blocking_pool->PostNamedSequencedWorkerTask(
               sequence_token_name, from_here, task);
}

// static
base::SequencedWorkerPool *BililiveThread::GetBlockingPool()
{
    return g_globals.Get().blocking_pool.get();
}

// static
bool BililiveThread::IsThreadInitialized(ID identifier)
{
    if (g_globals == NULL)
    {
        return false;
    }

    BililiveThreadGlobals &globals = g_globals.Get();
    base::AutoLock lock(globals.lock);
    DCHECK(identifier >= 0 && identifier < ID_COUNT);
    return globals.threads[identifier] != NULL;
}

// static
bool BililiveThread::CurrentlyOn(ID identifier)
{
    // We shouldn't use MessageLoop::current() since it uses LazyInstance which
    // may be deleted by ~AtExitManager when a WorkerPool thread calls this
    // function.
    // http://crbug.com/63678
    base::ThreadRestrictions::ScopedAllowSingleton allow_singleton;
    BililiveThreadGlobals &globals = g_globals.Get();
    base::AutoLock lock(globals.lock);
    DCHECK(identifier >= 0 && identifier < ID_COUNT);
    return globals.threads[identifier] &&
           globals.threads[identifier]->message_loop() ==
           base::MessageLoop::current();
}

// static
bool BililiveThread::IsMessageLoopValid(ID identifier)
{
    if (g_globals == NULL)
    {
        return false;
    }

    BililiveThreadGlobals &globals = g_globals.Get();
    base::AutoLock lock(globals.lock);
    DCHECK(identifier >= 0 && identifier < ID_COUNT);
    return globals.threads[identifier] &&
           globals.threads[identifier]->message_loop();
}

// static
bool BililiveThread::PostTask(ID identifier,
                              const tracked_objects::Location &from_here,
                              const base::Closure &task)
{
    return BililiveThreadImpl::PostTaskHelper(
               identifier, from_here, task, base::TimeDelta(), true);
}

// static
bool BililiveThread::PostDelayedTask(ID identifier,
                                     const tracked_objects::Location &from_here,
                                     const base::Closure &task,
                                     base::TimeDelta delay)
{
    return BililiveThreadImpl::PostTaskHelper(
               identifier, from_here, task, delay, true);
}

// static
bool BililiveThread::PostNonNestableTask(
    ID identifier,
    const tracked_objects::Location &from_here,
    const base::Closure &task)
{
    return BililiveThreadImpl::PostTaskHelper(
               identifier, from_here, task, base::TimeDelta(), false);
}

// static
bool BililiveThread::PostNonNestableDelayedTask(
    ID identifier,
    const tracked_objects::Location &from_here,
    const base::Closure &task,
    base::TimeDelta delay)
{
    return BililiveThreadImpl::PostTaskHelper(
               identifier, from_here, task, delay, false);
}

// static
bool BililiveThread::PostTaskAndReply(
    ID identifier,
    const tracked_objects::Location &from_here,
    const base::Closure &task,
    const base::Closure &reply)
{
    return GetMessageLoopProxyForThread(identifier)->PostTaskAndReply(from_here,
            task,
            reply);
}

// static
bool BililiveThread::GetCurrentThreadIdentifier(ID *identifier)
{
    if (g_globals == NULL)
    {
        return false;
    }

    // We shouldn't use MessageLoop::current() since it uses LazyInstance which
    // may be deleted by ~AtExitManager when a WorkerPool thread calls this
    // function.
    // http://crbug.com/63678
    base::ThreadRestrictions::ScopedAllowSingleton allow_singleton;
    base::MessageLoop *cur_message_loop = base::MessageLoop::current();
    BililiveThreadGlobals &globals = g_globals.Get();
    for (int i = 0; i < ID_COUNT; ++i)
    {
        if (globals.threads[i] &&
            globals.threads[i]->message_loop() == cur_message_loop)
        {
            *identifier = globals.threads[i]->identifier_;
            return true;
        }
    }

    return false;
}

// static
scoped_refptr<base::MessageLoopProxy>
BililiveThread::GetMessageLoopProxyForThread(ID identifier)
{
    return make_scoped_refptr(new BililiveThreadMessageLoopProxy(identifier));
}

// static
base::MessageLoop *BililiveThread::UnsafeGetMessageLoopForThread(ID identifier)
{
    if (g_globals == NULL)
    {
        return NULL;
    }

    BililiveThreadGlobals &globals = g_globals.Get();
    base::AutoLock lock(globals.lock);
    base::Thread *thread = globals.threads[identifier];
    DCHECK(thread);
    base::MessageLoop *loop = thread->message_loop();
    return loop;
}

// static
void BililiveThread::SetDelegate(ID identifier,
                                 BililiveThreadDelegate *delegate)
{
    using base::subtle::AtomicWord;
    BililiveThreadGlobals &globals = g_globals.Get();
    AtomicWord *storage = reinterpret_cast<AtomicWord *>(
                              &globals.thread_delegates[identifier]);
    AtomicWord old_pointer = base::subtle::NoBarrier_AtomicExchange(
                                 storage, reinterpret_cast<AtomicWord>(delegate));

    // This catches registration when previously registered.
    DCHECK(!delegate || !old_pointer);
}