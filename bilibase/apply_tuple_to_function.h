#ifndef BILIBASE_APPLY_TUPLE_TO_FUNCTION_H_
#define BILIBASE_APPLY_TUPLE_TO_FUNCTION_H_

#include <tuple>
#include <type_traits>

// Home-made implementation for std::apply(introduced since C++ 17) with only C++ 11 features
// available.
// Function apply(fn, tuple) invokes `fn` and forwards elements in `tuple` as arguments to it.

namespace bilibase {

template<size_t... Ints>
struct index_sequence {
    using type = index_sequence;
    using value_type = size_t;

    static std::size_t size()
    {
        return sizeof...(Ints);
    }
};

template<class Sequence1, class Sequence2>
struct merge_and_renumber;

template<size_t... I1, size_t... I2>
struct merge_and_renumber<index_sequence<I1...>, index_sequence<I2...>>
    : index_sequence<I1..., (sizeof...(I1) + I2)...>
{};

template<size_t N>
struct make_index_sequence
    : merge_and_renumber<typename make_index_sequence<N / 2>::type,
                         typename make_index_sequence<N - N / 2>::type>
{};

template<>
struct make_index_sequence<0> : index_sequence<>
{};

template<>
struct make_index_sequence<1> : index_sequence<0>
{};

// Why the hell can't we just use C++ 14 features? decltype(auto) is more succinct.
// Oh, we don't even have to roll our own index_sequence if C++ 14 features are available.

template<typename Func, typename Tuple, std::size_t... index>
auto apply_helper(Func&& func, Tuple&& tuple, index_sequence<index...>) ->
    decltype(func(std::get<index>(std::forward<Tuple>(tuple))...))
{
    return func(std::get<index>(std::forward<Tuple>(tuple))...);
}

template<typename Func, typename Tuple>
auto apply(Func&& func, Tuple&& tuple) ->
    decltype(apply_helper(std::forward<Func>(func),
                          std::forward<Tuple>(tuple),
                          make_index_sequence<std::tuple_size<typename std::decay<Tuple>::type>::value>{}))
{
    return apply_helper(std::forward<Func>(func),
                        std::forward<Tuple>(tuple),
                        make_index_sequence<std::tuple_size<typename std::decay<Tuple>::type>::value>{});
}

}   // namespace bilibase

#endif  // BILIBASE_APPLY_TUPLE_TO_FUNCTION_H_