/*
 @ 0xCCCCCCCC
*/

#include "gtest/gtest.h"

#include "data_view.h"

namespace bilibase {

TEST(DataView, Property)
{
    data_view<char> dv;
    EXPECT_TRUE(dv.empty());
    EXPECT_EQ(0, dv.size());
    EXPECT_TRUE(dv.begin() == dv.end());

    const char s[] = "hello world";
    dv = data_view<char>(s, _countof(s));
    EXPECT_TRUE(!dv.empty());
    EXPECT_EQ(_countof(s), dv.size());
    EXPECT_EQ(_countof(s), std::distance(dv.begin(), dv.end()));
}

TEST(DataView, MakeDataView)
{
    std::string s = "hello world";
    auto dv = make_data_view(s.data(), s.size());
    EXPECT_EQ(s.size(), dv.size());
}

}   // namespace bilibase
