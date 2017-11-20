#include "../serialize.h"
#include "gtest/gtest.h"

namespace spd = spdlog;
using namespace std;

namespace ruby_typer {
namespace core {
namespace serialize {

TEST(SerializeTest, U4) {
    GlobalStateSerializer::Picker p;
    p.putU4(0);
    p.putU4(1);
    p.putU4(4294967295);
    GlobalStateSerializer::UnPicker u(p.data.data());
    EXPECT_EQ(u.getU4(), 0);
    EXPECT_EQ(u.getU4(), 1);
    EXPECT_EQ(u.getU4(), 4294967295);
}

TEST(SerializeTest, U8) {
    GlobalStateSerializer::Picker p;
    p.putS8(0);
    p.putS8(1);
    p.putS8(-1);
    p.putS8(9223372036854775807);
    GlobalStateSerializer::UnPicker u(p.data.data());
    EXPECT_EQ(u.getS8(), 0);
    EXPECT_EQ(u.getS8(), 1);
    EXPECT_EQ(u.getS8(), -1);
    EXPECT_EQ(u.getS8(), 9223372036854775807);
}

TEST(SerializeTest, Strings) {
    GlobalStateSerializer::Picker p;
    p.putStr("");
    p.putStr("a");
    p.putStr("aaaaa");
    p.putStr("1");
    p.putStr("Z");
    p.putStr("НЯ");
    p.putStr("\0\0\0\t\n\f\rНЯЯЯЯЯ");
    GlobalStateSerializer::UnPicker u(p.data.data());
    EXPECT_EQ(u.getStr(), "");
    EXPECT_EQ(u.getStr(), "a");
    EXPECT_EQ(u.getStr(), "aaaaa");
    EXPECT_EQ(u.getStr(), "1");
    EXPECT_EQ(u.getStr(), "Z");
    EXPECT_EQ(u.getStr(), "НЯ");
    EXPECT_EQ(u.getStr(), "\0\0\0\t\n\f\rНЯЯЯЯЯ");
}

} // namespace serialize

} // namespace core
} // namespace ruby_typer
