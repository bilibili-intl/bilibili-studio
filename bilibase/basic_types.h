/*
 @ 0xCCCCCCCC
*/

#ifndef BILIBASE_BASIC_TYPES_H_
#define BILIBASE_BASIC_TYPES_H_

#include <type_traits>

#define input_to_string(enum_value) #enum_value
#define input_to_wstring(enum_value) L#enum_value

namespace bilibase {

// Casts an enum value into an equivalent integer.
template<typename E>
std::underlying_type_t<E> enum_cast(E e)
{
    return static_cast<std::underlying_type_t<E>>(e);
}

inline std::string safe_c_str_cast(const char* str)
{
    const char* valid_str = str ? str : "";
    return std::string(valid_str);
}

// Try not to expose these overloaded functions in any header files.

namespace enum_ops {

template<typename E>
std::enable_if_t<std::is_enum<E>::value, E> operator|(E lhs, E rhs)
{
    return E(enum_cast(lhs) | enum_cast(rhs));
}

template<typename E>
std::enable_if_t<std::is_enum<E>::value, E&> operator|=(E& lhs, E rhs)
{
    lhs = E(enum_cast(lhs) | enum_cast(rhs));
    return lhs;
}

template<typename E>
std::enable_if_t<std::is_enum<E>::value, E> operator&(E lhs, E rhs)
{
    return E(enum_cast(lhs) & enum_cast(rhs));
}

template<typename E>
std::enable_if_t<std::is_enum<E>::value, E&> operator&=(E& lhs, E rhs)
{
    lhs = E(enum_cast(lhs) & enum_cast(rhs));
    return lhs;
}

template<typename E>
std::enable_if_t<std::is_enum<E>::value, E> operator^(E lhs, E rhs)
{
    return E(enum_cast(lhs) ^ enum_cast(rhs));
}

template<typename E>
std::enable_if_t<std::is_enum<E>::value, E&> operator^=(E& lhs, E rhs)
{
    lhs = E(enum_cast(lhs) ^ enum_cast(rhs));
    return lhs;
}

template<typename E>
std::enable_if_t<std::is_enum<E>::value, E> operator~(E op)
{
    return E(~enum_cast(op));
}

}   // namespace enum_ops

}   // namespace bilibase

#endif  // BILIBASE_BASIC_TYPES_H_