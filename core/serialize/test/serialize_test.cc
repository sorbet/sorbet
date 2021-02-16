#include "doctest.h"
// has to go first as it violates our requirements
#include "core/serialize/pickler.h"
#include "core/serialize/serialize.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using namespace std;

namespace spd = spdlog;
namespace sorbet::core::serialize {

auto logger = spd::stderr_color_mt("serialize_test");

TEST_CASE("U4") { // NOLINT
    Pickler p;
    p.putU4(0);
    p.putU4(1);
    p.putU4(4294967295);
    UnPickler u(p.result(Serializer::GLOBAL_STATE_COMPRESSION_DEGREE).data(), *logger);
    CHECK_EQ(u.getU4(), 0);
    CHECK_EQ(u.getU4(), 1);
    CHECK_EQ(u.getU4(), 4294967295);
}

TEST_CASE("U4GROUP") {
    Pickler p;
    p.putU4Group(257, 1, 76349, 17983207);
    p.putU1(2);
    p.putU4Group(257, 17983207, 1, 76349);
    UnPickler u(p.result(Serializer::GLOBAL_STATE_COMPRESSION_DEGREE).data(), *logger);
    u4 a, b, c, d;
    u.getU4Group(&a, &b, &c, &d);
    CHECK_EQ(a, 257);
    CHECK_EQ(b, 1);
    CHECK_EQ(c, 76349);
    CHECK_EQ(d, 17983207);
    CHECK_EQ(u.getU1(), 2);
    u.getU4Group(&a, &b, &c, &d);
    CHECK_EQ(a, 257);
    CHECK_EQ(b, 17983207);
    CHECK_EQ(c, 1);
}
TEST_CASE("U4U1") { // NOLINT
    Pickler p;
    p.putU4(0);
    p.putU4(0);
    p.putStr("aaaaa");
    p.putU4(0);
    p.putU4(0);
    p.putU1(1);
    p.putU4(1);
    p.putU1(0);
    p.putU4(4294967295);
    UnPickler u(p.result(Serializer::GLOBAL_STATE_COMPRESSION_DEGREE).data(), *logger);
    CHECK_EQ(u.getU4(), 0);
    CHECK_EQ(u.getU4(), 0);
    CHECK_EQ(u.getStr(), "aaaaa");
    CHECK_EQ(u.getU4(), 0);
    CHECK_EQ(u.getU4(), 0);
    CHECK_EQ(u.getU1(), 1);
    CHECK_EQ(u.getU4(), 1);
    CHECK_EQ(u.getU1(), 0);
    CHECK_EQ(u.getU4(), 4294967295);
}

TEST_CASE("U8") { // NOLINT
    Pickler p;
    p.putS8(0);
    p.putS8(1);
    p.putS8(-1);
    p.putS8(9223372036854775807);
    UnPickler u(p.result(Serializer::GLOBAL_STATE_COMPRESSION_DEGREE).data(), *logger);
    CHECK_EQ(u.getS8(), 0);
    CHECK_EQ(u.getS8(), 1);
    CHECK_EQ(u.getS8(), -1);
    CHECK_EQ(u.getS8(), 9223372036854775807);
}

TEST_CASE("Strings") { // NOLINT
    Pickler p;
    p.putStr("");
    p.putStr("a");
    p.putStr("aaaaa");
    p.putStr("1");
    p.putStr("Z");
    p.putStr("НЯ");
    p.putStr("\0\0\0\t\n\f\rНЯЯЯЯЯ");
    UnPickler u(p.result(Serializer::GLOBAL_STATE_COMPRESSION_DEGREE).data(), *logger);
    CHECK_EQ(u.getStr(), "");
    CHECK_EQ(u.getStr(), "a");
    CHECK_EQ(u.getStr(), "aaaaa");
    CHECK_EQ(u.getStr(), "1");
    CHECK_EQ(u.getStr(), "Z");
    CHECK_EQ(u.getStr(), "НЯ");
    CHECK_EQ(u.getStr(), "\0\0\0\t\n\f\rНЯЯЯЯЯ");
}
} // namespace sorbet::core::serialize
