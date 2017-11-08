#include "gtest/gtest.h"
using namespace std;
int realmain(int argc, char **argv);

TEST(HelloTest, CanParseGerald) {
    const char *args[] = {"ruby-typer", "--no-payload", "rbi/stdlib.rbi", "test/testdata/first/gerald.rb"};
    ASSERT_EQ(realmain(4, (char **)args), 0);
}
