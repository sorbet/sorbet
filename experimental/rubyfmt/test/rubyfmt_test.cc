#include "doctest.h"
// has to go first as it violates our requirements
#include "experimental/rubyfmt/rubyfmt.h"

using namespace std;
using namespace experimental::rubyfmt;

#ifdef RUBYFMT_ENABLED
TEST_CASE("Formats files") {
    auto expected = "class Foo\nend\n";
    auto formatted = format(" class Foo; end");
    CHECK_EQ(formatted.status, RubyfmtFormatError::RUBYFMT_FORMAT_ERROR_OK);
    {
        INFO("Expected:\n" << expected << "\n\nActual:\n" << formatted.formatted);
        CHECK_EQ(formatted.formatted, expected);
    }

    // Can be run again
    auto formatted2 = format("      class Foo; end");
    CHECK_EQ(formatted2.status, RubyfmtFormatError::RUBYFMT_FORMAT_ERROR_OK);
    {
        INFO("Expected:\n" << expected << "\n\nActual:\n" << formatted2.formatted);
        CHECK_EQ(formatted2.formatted, expected);
    }
}
#else
// No Rubyfmt
TEST_CASE("Rubyfmt is disabled and should be a noop") {
    auto expected = " class Foo; end";
    auto formatted = format(" class Foo; end");
    CHECK_EQ(formatted.formatted, expected);
}
#endif
