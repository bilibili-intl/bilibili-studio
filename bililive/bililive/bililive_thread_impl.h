#ifndef BILILIVE_BILILIVE_BILILIVE_THREAD_IMPL_H
#define BILILIVE_BILILIVE_BILILIVE_THREAD_IMPL_H


#include "base/threading/thread.h"
#include "bililive/public/bililive/bililive_thread.h"


class BililiveThreadImpl
    : public BililiveThread
    , public base::Thread
{
public:
    explicit BililiveThreadImpl(BililiveThread::ID identifier);

    BililiveThreadImpl(BililiveThread::ID identifier,
                       base::MessageLoop *message_loop);
    virtual ~BililiveThreadImpl();

    static void ShutdownThreadPool();

protected:
    virtual void Init() OVERRIDE;
    virtual void Run(base::MessageLoop *message_loop) OVERRIDE;
    virtual void CleanUp() OVERRIDE;

private:
    friend class BililiveThread;

    void UIThreadRun(base::MessageLoop *message_loop);
    void DBThreadRun(base::MessageLoop *message_loop);
    void FileThreadRun(base::MessageLoop *message_loop);
    void IOThreadRun(base::MessageLoop *message_loop);

    static bool PostTaskHelper(
        BililiveThread::ID identifier,
        const tracked_objects::Location &from_here,
        const base::Closure &task,
        base::TimeDelta delay,
        bool nestable);

    void Initialize();

    friend class ContentTestSuiteBaseListener;
    static void FlushThreadPoolHelper();

    ID identifier_;
};

#endif