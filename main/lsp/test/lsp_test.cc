#include "main/lsp/lsp.h"
#include "gtest/gtest.h"

namespace spd = spdlog;
using namespace std;

namespace sorbet {
namespace realmain {
namespace lsp {
namespace test {

const string file_string = "# typed: true\n"
                           "# this is a class.\n"
                           "class B\n"
                           "    # This is the documentation for a constant.\n"
                           "    # This is the second line for a constant.\n"
                           "    ZZZZZZ = 11\n"
                           "\n"
                           "    # This is an instance method with a standard line documentation\n"
                           "    def abcde\n"
                           "      1\n"
                           "    end\n"
                           "\n"
                           "    # This is a multiline documented instance method.\n"
                           "    # All of the lines should be displayed in the docs.\n"
                           "    # Including this one.\n"
                           "    def multidoc_instance\n"
                           "      1\n"
                           "    end\n"
                           "\n"
                           "    # If there is a line between the documentation and the file,\n"
                           "    # there will be no documentation displayed.\n"
                           "\n"
                           "    def nodocs\n"
                           "      1\n"
                           "    end\n"
                           "\n"
                           " # weird indentation\n"
                           "                                       # is in this documentation.\n"
                           "    def weirdindent\n"
                           "      1\n"
                           "    end\n"
                           "end\n";
const string_view file = string_view(file_string);

TEST(FindDocumentationTest, OneLineDocumentation) { // NOLINT
    int position = file.find("abcde");
    unique_ptr<string> b = findDocumentation(file, position);
    ASSERT_EQ(*b, " This is an instance method with a standard line documentation\n");
}

TEST(FindDocumentationTest, MultiLineDocumentation) { // NOLINT
    int position = file.find("multidoc_instance");
    unique_ptr<string> b = findDocumentation(file, position);
    ASSERT_EQ(*b, " This is a multiline documented instance method.\n All of the lines should be displayed in the "
                  "docs.\n Including this one.\n");
}
TEST(FindDocumentationTest, SeparatedDocumentation) { // NOLINT
    int position = file.find("nodocs");
    unique_ptr<string> b = findDocumentation(file, position);
    ASSERT_EQ(b, nullptr);
}
TEST(FindDocumentationTest, TopOfFile) { // NOLINT
    int position = file.find("B");
    unique_ptr<string> b = findDocumentation(file, position);
    ASSERT_EQ(*b, " this is a class.\n");
}
TEST(FindDocumentationTest, DifferentIndentation) { // NOLINT
    int position = file.find("weirdindent");
    unique_ptr<string> b = findDocumentation(file, position);
    ASSERT_EQ(*b, " weird indentation\n is in this documentation.\n");
}
TEST(FindDocumentationTest, Constant) { // NOLINT
    int position = file.find("ZZZZZZ");
    unique_ptr<string> b = findDocumentation(file, position);
    ASSERT_EQ(*b, " This is the documentation for a constant.\n This is the second line for a constant.\n");
}

} // namespace test
} // namespace lsp
} // namespace realmain
} // namespace sorbet