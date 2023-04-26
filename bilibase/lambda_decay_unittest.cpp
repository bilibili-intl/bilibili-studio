/*
 @ 0xCCCCCCCC
*/

#include "gtest/gtest.h"

#include "lambda_decay.h"

namespace bilibase {

TEST(LambdaDecay, General)
{
    // Ignore static analysis error.
    auto pfn = lambda_decay([](int lhs, int rhs) {
                   return lhs + rhs;
               });
    EXPECT_EQ(15, pfn(5, 10));
}

}   // namespace bilibase
