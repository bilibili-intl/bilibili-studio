/*
 @ 0xCCCCCCCC
*/

#ifndef BASE_EXT_COUNT_DOWN_LATCH_H_
#define BASE_EXT_COUNT_DOWN_LATCH_H_

#include "base/synchronization/condition_variable.h"
#include "base/synchronization/lock.h"
#include "base/time/time.h"

namespace base {

class CountdownLatch {
public:
    explicit CountdownLatch(int count);

    ~CountdownLatch() = default;

    CountdownLatch(const CountdownLatch&) = delete;

    CountdownLatch& operator=(const CountdownLatch&) = delete;

    void Countdown();

    void Wait();

    void WaitFor(const TimeDelta& wait_time);

private:
    Lock count_mutex_;
    ConditionVariable stall_latch_;
    int count_;
};

}   // namespace base

#endif  // BASE_EXT_COUNT_DOWN_LATCH_H_
