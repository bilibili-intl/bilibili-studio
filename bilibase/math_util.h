/*
 @ 0xCCCCCCCC
*/

#ifndef BILIBASE_MATH_UTIL_H_
#define BILIBASE_MATH_UTIL_H_

namespace bilibase {

// We force using cast to avoid silent arithmetic overflow.
template<typename R, typename T>
R Percentize(T numerator, T denominator)
{
    static_assert(std::is_arithmetic<T>::value &&
                  std::is_arithmetic<R>::value, "Must be integers or floating numbers");
    return static_cast<R>(static_cast<double>(numerator) / static_cast<double>(denominator) * 100);
}

inline int CeilDiv(int num, int den)
{
    return (num + den - 1) / den;
}

}   // namespace bilibase

#endif  // BILIBASE_MATH_UTIL_H_
