#ifndef BILIBASE_SCOPE_GUARD_H_
#define BILIBASE_SCOPE_GUARD_H_

#include <functional>

#include "bilibase/basic_macros.h"

#define ON_SCOPE_EXIT   \
    auto ANONYMOUS_VAR(_exit_) = ::bilibase::internal::ScopeGuardDriver() + [&]()

#define MAKE_SCOPE_GUARD    \
    ::bilibase::internal::ScopeGuardDriver() + [&]()

namespace bilibase {

class ScopeGuard {
private:
    using ExitCallback = std::function<void()>;

public:
    template<typename F>
    explicit ScopeGuard(F&& fn)
        : exit_callback_(std::forward<F>(fn)), dismissed_(false)
    {}

    ScopeGuard(ScopeGuard&& other)
        : exit_callback_(std::move(other.exit_callback_)), dismissed_(other.dismissed_)
    {
        other.dismissed_ = true;
    }

    ~ScopeGuard()
    {
        if (!dismissed_)
        {
            exit_callback_();
        }
    }

    ScopeGuard& operator=(ScopeGuard&&) = delete;

    DISABLE_COPY(ScopeGuard);

    void Dismiss()
    {
        dismissed_ = true;
    }

private:
    ExitCallback exit_callback_;
    bool dismissed_;
};

namespace internal {

struct ScopeGuardDriver {};

template<typename F>
ScopeGuard operator+(ScopeGuardDriver, F&& fn)
{
    return ScopeGuard(std::forward<F>(fn));
}

}   // namespace internal

}   // namespace bilibase

#endif  // BILIBASE_SCOPE_GUARD_H_