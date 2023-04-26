#include "bililive_process_sub_thread.h"
#include "base/debug/leak_tracker.h"
#include "base/threading/thread_restrictions.h"
#include "build/build_config.h"
#include "base/win/scoped_com_initializer.h"
#include "base/notification/notification_service_impl.h"


BililiveProcessSubThread::BililiveProcessSubThread(BililiveThread::ID identifier)
    : BililiveThreadImpl(identifier)
{
}

BililiveProcessSubThread::~BililiveProcessSubThread()
{
    Stop();
}

void BililiveProcessSubThread::Init()
{
    com_initializer_.reset(new base::win::ScopedCOMInitializer());

    notification_service_.reset(new base::NotificationServiceImpl());

    BililiveThreadImpl::Init();

    if (BililiveThread::CurrentlyOn(BililiveThread::IO))
    {
        // Though this thread is called the "IO" thread, it actually just routes
        // messages around; it shouldn't be allowed to perform any blocking disk
        // I/O.
        base::ThreadRestrictions::SetIOAllowed(false);
        base::ThreadRestrictions::DisallowWaiting();
    }
}

void BililiveProcessSubThread::CleanUp()
{
    if (BililiveThread::CurrentlyOn(BililiveThread::IO))
    {
        IOThreadPreCleanUp();
    }

    BililiveThreadImpl::CleanUp();

    notification_service_.reset();

    com_initializer_.reset();
}

void BililiveProcessSubThread::IOThreadPreCleanUp()
{

}