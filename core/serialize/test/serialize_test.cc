#include "doctest/doctest.h"
// has to go first as it violates our requirements
#include "core/serialize/pickler.h"
#include "core/serialize/serialize.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using namespace std;

namespace sorbet::core::serialize {

auto logger = spdlog::stderr_color_mt("serialize_test");

TEST_CASE("U4") { // NOLINT
    Pickler p;
    p.putU4(0);
    p.putU4(1);
    p.putU4(4294967295);
    UnPickler u(p.result().data(), *logger);
    CHECK_EQ(u.getU4(), 0);
    CHECK_EQ(u.getU4(), 1);
    CHECK_EQ(u.getU4(), 4294967295);
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
    UnPickler u(p.result().data(), *logger);
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
    UnPickler u(p.result().data(), *logger);
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
    UnPickler u(p.result().data(), *logger);
    CHECK_EQ(u.getStr(), "");
    CHECK_EQ(u.getStr(), "a");
    CHECK_EQ(u.getStr(), "aaaaa");
    CHECK_EQ(u.getStr(), "1");
    CHECK_EQ(u.getStr(), "Z");
    CHECK_EQ(u.getStr(), "НЯ");
    CHECK_EQ(u.getStr(), "\0\0\0\t\n\f\rНЯЯЯЯЯ");
}

TEST_CASE("Symbol flags") {
    Field::Flags fieldFlags;
    Method::Flags mflags;
    TypeParameter::Flags tpflags;
    ClassOrModule::Flags cmflags;
    // All unused bits should be zero, and all flags should default to false.
    CHECK_EQ(fieldFlags.serialize(), 0);
    CHECK_EQ(mflags.serialize(), 0);
    CHECK_EQ(tpflags.serialize(), 0);
    CHECK_EQ(cmflags.serialize(), 0);

    mflags.isAbstract = true;
    CHECK_NE(mflags.serialize(), 0);

    fieldFlags.isField = true;
    CHECK_NE(fieldFlags.serialize(), 0);

    tpflags.isContravariant = true;
    CHECK_NE(tpflags.serialize(), 0);

    cmflags.isAbstract = true;
    CHECK_NE(cmflags.serialize(), 0);

    // I wish I could check that the mask is correct.
    // https://stackoverflow.com/a/45837449 indicates it is possible to count the number of fields in a class, but it
    // requires a lot of shenanigans.
}
} // namespace sorbet::core::serialize
