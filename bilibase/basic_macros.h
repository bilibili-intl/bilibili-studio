/*
 @ 0xCCCCCCCC
*/

#ifndef BILIBASE_BASIC_MACROS_H_
#define BILIBASE_BASIC_MACROS_H_

#define DISABLE_COPY(classname)                         \
    classname(const classname&) = delete;               \
    classname& operator=(const classname&) = delete

#define DISABLE_MOVE(classname)                         \
    classname(classname&&) = delete;                    \
    classname& operator=(classname&&) = delete

// Default function is implicitly constexpr and noexcept, whenever it can be.

#define DEFAULT_COPY(classname)                         \
    classname(const classname&) = default;              \
    classname& operator=(const classname&) = default

#define DEFAULT_MOVE(classname)                         \
    classname(classname&&) = default;                   \
    classname& operator=(classname&&) = default

#define UNUSED_VAR(x)                                   \
    ::bilibase::internal::SilenceUnusedVariableWarning(x)

#define CONCATENATE_IMPL(part1, part2) part1##part2
#define CONCATENATE(part1, part2) CONCATENATE_IMPL(part1, part2)
#define ANONYMOUS_VAR(tag) CONCATENATE(tag, __LINE__)

#define FORCE_AS_MEMBER_FUNCTION()                      \
    UNUSED_VAR(this)

#define FORCE_AS_NON_CONST_MEMBER_FUNCTION()            \
    UNUSED_VAR(this)

#define DECLARE_DLL_FUNCTION(fn, type, dll)                     \
    auto ANONYMOUS_VAR(_module_) = GetModuleHandleW(L##dll);    \
    auto fn = ANONYMOUS_VAR(_module_) ?                         \
                reinterpret_cast<type>(GetProcAddress(ANONYMOUS_VAR(_module_), #fn)) : nullptr

// Put complicated implementation below.

namespace bilibase {

namespace internal {

template<typename T>
void SilenceUnusedVariableWarning(T&&)
{}

}   // namespace internal

}   // namespace bilibase

#endif  // BILIBASE_BASIC_MACROS_H_