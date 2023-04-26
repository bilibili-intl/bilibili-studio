/*
 @ 0xCCCCCCCC
*/

#ifndef BILIBASE_RATIONAL_H_
#define BILIBASE_RATIONAL_H_

#include <cmath>

namespace bilibase {

// A simple implmenetation for rational numbers.
// We don't handle possible cases where overflow happens.

namespace internal {

inline int GCD(int a, int b)
{
    int c = 0;
    while (a != 0) {
        c = a;
        a = b % a;
        b = c;
    }

    return b;
}

}   // namespace internal

class Rational {
public:
    // Implicit conversion is allowed.
    Rational(int num)
        : num_(num), den_(1)
    {}

    Rational(int numerator, int denominator)
        : num_(numerator), den_(denominator)
    {}

    int numerator() const
    {
        return num_;
    }

    int denominator() const
    {
        return den_;
    }

    double ToDouble() const
    {
        return static_cast<double>(num_) / den_;
    }

    int Floor() const
    {
        return num_ / den_;
    }

    int Ceil() const
    {
        // Ignore potential overflow here.
        return (num_ + den_ - 1) / den_;
    }

    int Round() const
    {
        return std::lround(ToDouble());
    }

    Rational Inverse() const
    {
        int new_num = den_;
        int new_den = num_;

        if (new_den == 0) {
            return Rational(0);
        }

        return Rational(new_num, new_den);
    }

private:
    int num_;
    int den_;
};

inline bool operator==(const Rational& lhs, const Rational& rhs)
{
    return lhs.numerator() == rhs.numerator() && lhs.denominator() == rhs.denominator();
}

inline bool operator!=(const Rational& lhs, const Rational& rhs)
{
    return !(lhs == rhs);
}

inline Rational operator*(const Rational& lhs, const Rational& rhs)
{
    int new_num = lhs.numerator() * rhs.numerator();
    int new_den = lhs.denominator() * rhs.denominator();
    auto gcd = internal::GCD(new_num, new_den);
    return {new_num / gcd, new_den / gcd};
}

}   // namespace bilibase

#endif  // BILIBASE_RATIONAL_H_
