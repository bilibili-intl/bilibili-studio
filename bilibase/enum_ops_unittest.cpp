/*
 @ 0xCCCCCCCC
*/

#include "gtest/gtest.h"

#include "basic_types.h"

using bilibase::enum_cast;

enum class ChangeLevel : unsigned int {
    None = 0,
    Local = 1 << 0,
    Server = 1 << 1,
    All = Local | Server
};

TEST(EnumOps, General)
{
    using namespace bilibase::enum_ops;

    ChangeLevel level = ChangeLevel::None;
    level |= ChangeLevel::Local;
    level |= ChangeLevel::Server;

    EXPECT_TRUE(level == ChangeLevel::All);
    EXPECT_TRUE(enum_cast(level & ChangeLevel::Local) != 0);
    EXPECT_TRUE(enum_cast(level & ChangeLevel::Server) != 0);
}

TEST(EnumOps, NonEnumType)
{
    using namespace bilibase::enum_ops;

    unsigned int l = 1;
    unsigned int r = 1 << 1;
    auto rv = l | r;
    EXPECT_EQ(3, rv);

    //std::string s1 = "hello";
    //std::string s2 = "world";
    //s1 | s2;
}