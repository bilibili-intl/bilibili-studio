/*
 @ 0xCCCCCCCC
*/

#include "gtest/gtest.h"

#include "math_util.h"

namespace bilibase {

TEST(MathUtil, Percentize)
{
    EXPECT_EQ(38, Percentize<int>(123, 321));

    double v = Percentize<double>(123, 321);
    auto s = std::to_string(v).substr(0, 5);
    EXPECT_EQ("38.31", s);
}

TEST(MathUtil, CeilDiv)
{
    EXPECT_EQ(2, CeilDiv(10, 5));
    EXPECT_EQ(1, CeilDiv(5, 10));
}

}   // namespace bilibase
