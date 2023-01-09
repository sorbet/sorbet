#include "doctest/doctest.h"
// has to go first as it violates our requirements

#include "main/lsp/LSPLoop.h"

using namespace std;

namespace sorbet::realmain::lsp::test {

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
                           "\n"
                           "    # This is a method with documentation above its\n"
                           "    # one-line sig block.\n"
                           "    sig {returns(Integer)}\n"
                           "    def one_line_sig_block\n"
                           "      1\n"
                           "    end\n"
                           "\n"
                           "    # This is a method with documentation above its\n"
                           "    # multi-line sig block.\n"
                           "    sig do\n"
                           "      params(x: Integer)\n"
                           "      .returns(Integer)\n"
                           "    end\n"
                           "    def multi_line_sig_block\n"
                           "      1\n"
                           "    end\n"
                           "\n"
                           "    # This is a method with documentation above its\n"
                           "    # multi-line sig(:final) block.\n"
                           "    sig(:final) do\n"
                           "      params(x: Integer)\n"
                           "      .returns(Integer)\n"
                           "    end\n"
                           "    def multi_line_sig_final_block\n"
                           "      1\n"
                           "    end\n"
                           "\n"
                           "    # This is a method with documentation above its\n"
                           "    # broken multi-line sig block. You shouldn't see this. \n"
                           "    not_a_sig do \n"
                           "      params(x: Integer)\n"
                           "      .returns(Integer)\n"
                           "    end\n"
                           "    def broken_multi_line_sig_block\n"
                           "      1\n"
                           "    end\n"
                           "\n"
                           "    # Docs for qux. You shouldn't see this.\n"
                           "    sig {}\n"
                           "    def qux\n"
                           "    end\n"
                           "    def consecutive_method_1\n"
                           "    end\n"
                           "\n"
                           "    # Docs for quxx. You shouldn't see this.\n"
                           "    sig do\n"
                           "      void\n"
                           "    end\n"
                           "    def quxx\n"
                           "    end\n"
                           "    def consecutive_method_2\n"
                           "    end\n"
                           "\n"
                           "    # Docs for bam. You shouldn't see this.\n"
                           "    sig do; params(x: Integer).void; end\n"
                           "    def bam(x)\n"
                           "    end\n"
                           "    def consecutive_method_3\n"
                           "    end\n"
                           "\n"
                           "    # First line\n"
                           "    #Second line\n"
                           "    sig do; params(x: Integer).void; end\n"
                           "    def ensure_one_whitespace(x)\n"
                           "    end\n"
                           "\n"
                           "    # First line\n"
                           "    # Second line\n"
                           "    # @param x this is x\n"
                           "    # @param y this is y\n"
                           "    sig do; params(x: Integer, y: String).void; end\n"
                           "    def extra_newlines_for_yarddoc(x, y)\n"
                           "    end\n"
                           "end\n";
string_view file = string_view(file_string);

TEST_CASE("OneLineDocumentation") {
    int position = file.find("abcde");
    optional<string> b = findDocumentation(file, position);
    REQUIRE_EQ(*b, "This is an instance method with a standard line documentation");
}

TEST_CASE("MultiLineDocumentation") {
    int position = file.find("multidoc_instance");
    optional<string> b = findDocumentation(file, position);
    REQUIRE_EQ(*b, "This is a multiline documented instance method.\nAll of the lines should be displayed in the "
                   "docs.\nIncluding this one.");
}
TEST_CASE("SeparatedDocumentation") {
    int position = file.find("nodocs");
    optional<string> b = findDocumentation(file, position);
    REQUIRE(!b);
}
TEST_CASE("TopOfFile") {
    int position = file.find("B");
    optional<string> b = findDocumentation(file, position);
    REQUIRE_EQ(*b, "this is a class.");
}
TEST_CASE("DifferentIndentation") {
    int position = file.find("weirdindent");
    optional<string> b = findDocumentation(file, position);
    REQUIRE_EQ(*b, "weird indentation\nis in this documentation.");
}
TEST_CASE("Constant") {
    int position = file.find("ZZZZZZ");
    optional<string> b = findDocumentation(file, position);
    REQUIRE_EQ(*b, "This is the documentation for a constant.\nThis is the second line for a constant.");
}
TEST_CASE("OneLineSig") {
    int position = file.find("one_line_sig_block");
    optional<string> b = findDocumentation(file, position);
    REQUIRE_EQ(*b, "This is a method with documentation above its\none-line sig block.");
}
TEST_CASE("MultiLineSig") {
    int position = file.find("multi_line_sig_block");
    optional<string> b = findDocumentation(file, position);
    REQUIRE_EQ(*b, "This is a method with documentation above its\nmulti-line sig block.");
}
TEST_CASE("MultiLineSigFinal") {
    int position = file.find("multi_line_sig_final_block");
    optional<string> b = findDocumentation(file, position);
    REQUIRE_EQ(*b, "This is a method with documentation above its\nmulti-line sig(:final) block.");
}
TEST_CASE("BrokenMultiLineSig") {
    int position = file.find("broken_multi_line_sig_block");
    optional<string> b = findDocumentation(file, position);
    REQUIRE(!b);
}
TEST_CASE("ConsecutiveMethodsOneLineSig") {
    int position = file.find("consecutive_method_1");
    optional<string> b = findDocumentation(file, position);
    REQUIRE(!b);
}
TEST_CASE("ConsecutiveMethodsMultiLineSig") {
    int position = file.find("consecutive_method_2");
    optional<string> b = findDocumentation(file, position);
    REQUIRE(!b);
}
TEST_CASE("ConsecutiveMethodsBlockShorthandSig") {
    int position = file.find("consecutive_method_3");
    optional<string> b = findDocumentation(file, position);
    REQUIRE(!b);
}
TEST_CASE("EnsureOneWhitespace") {
    int position = file.find("ensure_one_whitespace");
    optional<string> b = findDocumentation(file, position);
    REQUIRE_EQ(*b, "First line\nSecond line");
}
TEST_CASE("ExtraNewLinesForYarddoc") {
    int position = file.find("extra_newlines_for_yarddoc");
    optional<string> b = findDocumentation(file, position);
    REQUIRE_EQ(*b, "First line\nSecond line\n\n@param x this is x\n\n@param y this is y");
}

} // namespace sorbet::realmain::lsp::test
