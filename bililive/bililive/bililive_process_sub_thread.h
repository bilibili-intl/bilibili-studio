#ifndef BILILIVE_BILILIVE_BILILIVE_PROCESS_SUB_THREAD_H
#define BILILIVE_BILILIVE_BILILIVE_PROCESS_SUB_THREAD_H


#include "base/basictypes.h"
#include "bililive/bililive/bililive_thread_impl.h"
#include "base/notification/notification_service.h"

namespace base
{
    namespace win
    {
        class ScopedCOMInitializer;
    }
}

class NotificationService;


class BililiveProcessSubThread
    : public BililiveThreadImpl
{
public:
    explicit BililiveProcessSubThread(BililiveThread::ID identifier);
    virtual ~BililiveProcessSubThread();

protected:
    virtual void Init() OVERRIDE;
    virtual void CleanUp() OVERRIDE;

private:
    void IOThreadPreCleanUp();

    scoped_ptr<base::win::ScopedCOMInitializer> com_initializer_;

    scoped_ptr<base::NotificationService> notification_service_;

    DISALLOW_COPY_AND_ASSIGN(BililiveProcessSubThread);
};

#endif