/*
 @ 0xCCCCCCCC
*/

#ifndef BASE_EXT_CALLABLE_CALLBACK_H_
#define BASE_EXT_CALLABLE_CALLBACK_H_

#include "base/bind.h"

namespace base {

// Wraps a base::Callback object into a suited function object to be used with std::function.

template<typename>
class CallableCallback;

template<typename R, typename... Args>
class CallableCallback<R(Args...)> {
public:
    using callback_t = base::Callback<R(Args...)>;

    explicit CallableCallback(callback_t callback)
        : callback_(callback)
    {}

    R operator()(Args... args) const
    {
        return callback_.Run(args...);
    }

private:
    callback_t callback_;
};

template<typename Callback>
auto MakeCallable(Callback callback)->CallableCallback<typename Callback::RunType>
{
    using Sig = typename Callback::RunType;
    return CallableCallback<Sig>(callback);
}

}   // namespace base

#endif  // BASE_EXT_CALLABLE_CALLBACK_H_
