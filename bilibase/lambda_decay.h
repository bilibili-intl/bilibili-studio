#ifndef BILIBASE_LAMBDA_DECAY_H_
#define BILIBASE_LAMBDA_DECAY_H_

namespace bilibase {

namespace internal {

template<typename T>
struct dememberize;

// No need to support mutable lambdas because we don't decay a capturing lambda.
template<typename C, typename R, typename... Args>
struct dememberize<R(C::*)(Args...) const> {
    using type = R(*)(Args...);
};

template<typename T>
struct lambda_pointerize_impl {
    using type = typename dememberize<decltype(&T::operator())>::type;
};

}   // namespace internal

template<typename T>
using lambda_pointerize = typename internal::lambda_pointerize_impl<T>::type;

// Decay a non-capturing lambda to a corresponding pure function pointer.
// We need this utility because base::Bind() from chromium doesn't accept lambda expression, and
// writing a dedicated function (with less than five statements) could sometimes be tedious, not
// even mentioned to writing a lambda but cast manually.

template<typename F>
lambda_pointerize<F> lambda_decay(F lambda)
{
    return lambda_pointerize<F>(lambda);
}

}   // namespace bilibase

#endif  // BILIBASE_LAMBDA_DECAY_H_