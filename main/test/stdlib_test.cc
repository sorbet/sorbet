#include "gtest/gtest.h"
using namespace std;
int realmain(int argc, char **argv);

TEST(HelloTest, CanParseStdlib) {
    const char *args[] = {"ruby-typer", "rbi/stdlib.rbi"};
    ASSERT_EQ(realmain(2, (char **)args), 0);
}