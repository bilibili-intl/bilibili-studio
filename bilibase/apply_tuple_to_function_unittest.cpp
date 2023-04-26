/*
 @ 0xCCCCCCCC
*/

#include "gtest/gtest.h"

#include "apply_tuple_to_function.h"

namespace {

using Tuple = std::tuple<bool, int, std::string>;

Tuple Composite(bool b, int n, const std::string& s)
{
    return Tuple(b, n, s);
}

}   // namespace

namespace bilibase {

TEST(ApplyTupleToFunction, General)
{
    std::tuple<bool, int, std::string> ts(true, 123, "test");
    auto rts = apply(Composite, ts);
    EXPECT_EQ(ts, rts);
}

TEST(ApplyTupleToFunction, Mutation)
{
    std::tuple<bool, int, std::string> ts(true, 123, "test");
    apply([](bool& b, int, std::string& s) {
        b = false;
        s.clear();
    }, ts);

    EXPECT_EQ(false, std::get<0>(ts));
    EXPECT_TRUE(std::get<2>(ts).empty());
}

}   // namespace bilibase
