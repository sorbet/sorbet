#include "gtest/gtest.h"
using namespace std;
namespace ruby_typer {
int realmain(int argc, char **argv);
}

TEST(HelloTest, CanParseGerald) { // NOLINT
    const char *args[] = {"ruby-typer", "--no-stdlib", "rbi/stdlib.rbi", "test/testdata/first/gerald.rb"};
    ASSERT_EQ(ruby_typer::realmain(4, (char **)args), 0);
}
