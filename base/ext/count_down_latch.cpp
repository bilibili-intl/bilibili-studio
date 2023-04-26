/*
 @ 0xCCCCCCCC
*/

#include "base/ext/count_down_latch.h"

#include "base/logging.h"

namespace base {

CountdownLatch::CountdownLatch(int count)
    : stall_latch_(&count_mutex_), count_(count)
{}

void CountdownLatch::Countdown()
{
    AutoLock lock(count_mutex_);
    DCHECK(count_ > 0) << "Count quota mismatch!";
    if (--count_ <= 0) {
        stall_latch_.Broadcast();
    }
}

void CountdownLatch::Wait()
{
    AutoLock lock(count_mutex_);
    while (count_ > 0) {
        stall_latch_.Wait();
    }
}

void CountdownLatch::WaitFor(const TimeDelta& wait_time)
{
    AutoLock lock(count_mutex_);
    while (count_ > 0) {
        stall_latch_.TimedWait(wait_time);
    }
}

}   // namespace base
