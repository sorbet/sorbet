#include "doctest/doctest.h"
// has to go first as it violates our requirements
#include "ast/desugar/Desugar.h"
#include "core/ErrorQueue.h"
#include "core/Unfreeze.h"
#include "core/core.h"
#include "main/autogen/constant_hash.h"
#include "parser/parser.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using namespace std;

namespace sorbet {
auto logger = spdlog::stderr_color_mt("parser_test");
auto errorQueue = make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

struct Helper {
    core::GlobalState gs;
    int unique;

    Helper() : gs(core::GlobalState(errorQueue)), unique(0) {
        gs.initEmpty();
    }

    unsigned int hashExample(string_view example) {
        string filename = fmt::format("{}.rb", unique);
        unique += 1;
        core::FileRef file;
        {
            core::UnfreezeFileTable fileTableAccess(gs);
            file = gs.enterFile(filename, example);
        }
        core::UnfreezeNameTable nameTableAccess(gs);
        auto settings = parser::Parser::Settings{false, false, false};
        auto node = parser::Parser::run(gs, file, settings);

        core::MutableContext ctx{gs, core::Symbols::root(), file};
        auto ast = ast::desugar::node2Tree(ctx, move(node));
        ast::ParsedFile pf{move(ast), file};
        return autogen::constantHashTree(gs, std::move(pf)).constantHash;
    }
};

TEST_CASE("Stability") { // NOLINT
    Helper helper;

    // the reason this hashes to something other than 0 is because the
    // desugaring pass adds a "fake" class with no name to the root
    CHECK_EQ(98084193, helper.hashExample("\n"));

    // a few hard-coded examples to check for stability
    CHECK_EQ(3145201656, helper.hashExample("class Foo; end"));
    CHECK_EQ(2505704364, helper.hashExample("class Bar; end"));
    CHECK_EQ(2162981476, helper.hashExample("class Foo::Bar; end"));
    CHECK_EQ(3908589664, helper.hashExample("module Foo; end"));
    CHECK_EQ(3269092372, helper.hashExample("module Bar; end"));
    CHECK_EQ(1735599820, helper.hashExample("module Foo::Bar; end"));
    CHECK_EQ(1304948417, helper.hashExample("X = T.let(5, Integer)"));
    CHECK_EQ(638052961, helper.hashExample("X = Y"));
    CHECK_EQ(3058241009, helper.hashExample("require 'this'"));
}

TEST_CASE("SimpleClass") { // NOLINT
    Helper helper;

    // we'll use this simple file as our baseline
    auto basicHash = helper.hashExample("class Foo\n"
                                        "end\n");

    // the following are all transformations to the file that do not
    // affect the constant hash:

    // comments do not affect the hashed value
    CHECK_EQ(basicHash, helper.hashExample("# comment\n"
                                           "class Foo\n"
                                           "end\n"));
    // the presence of method calls does not affect the hashed value
    CHECK_EQ(basicHash, helper.hashExample("a_method!\n"
                                           "class Foo\n"
                                           "end\n"));
    // the presence of method definitions does not affect the hashed value
    CHECK_EQ(basicHash, helper.hashExample("class Foo\n"
                                           "  def hello!; end\n"
                                           "  def goodbye!; end\n"
                                           "end\n"));
    // the presence of locals (eugh) does not affect the hashed value
    CHECK_EQ(basicHash, helper.hashExample("class Foo\n"
                                           "  foo = whatever\n"
                                           "end\n"));
    // a send, even containing a constant, should not affect the hashed value
    CHECK_EQ(basicHash, helper.hashExample("class Foo\n"
                                           "  arglbargl(This)\n"
                                           "end\n"));

    // the following are all transformations which SHOULD affect the hashed value

    // changing the name of the class SHOULD affect the hashed value
    CHECK_NE(basicHash, helper.hashExample("class Fwoo\n"
                                           "end\n"));

    // changing the scoping of the class SHOULD affect the hashed value
    CHECK_NE(basicHash, helper.hashExample("class Something::Foo\n"
                                           "end\n"));

    // changing `class` to `module` SHOULD affect the hashed value
    CHECK_NE(basicHash, helper.hashExample("module Foo\n"
                                           "end\n"));

    // changing the superclass SHOULD affect the hashed value
    CHECK_NE(basicHash, helper.hashExample("class Foo < Bar\n"
                                           "end\n"));

    // adding an `include` SHOULD affect the hashed value
    CHECK_NE(basicHash, helper.hashExample("class Foo < Bar\n"
                                           "  include This\n"
                                           "end\n"));

    // adding an `extend` SHOULD affect the hashed value
    CHECK_NE(basicHash, helper.hashExample("class Foo < Bar\n"
                                           "  extend This\n"
                                           "end\n"));

    // because changing a `require` could cause the autoloader to
    // regenerate, adding or removing a `require` SHOULD affect the
    // hashed value
    CHECK_NE(basicHash, helper.hashExample("require 'something'\n"
                                           "class Foo < Bar\n"
                                           "end\n"));
}

TEST_CASE("Constant Assignments") { // NOLINT
    Helper helper;

    // for constant aliases, changing the LHS or the RHS should affect
    // the hash
    auto alias = helper.hashExample("module Example\n"
                                    "  X = Y\n"
                                    "end\n");

    // changing the LHS should affect the hash
    CHECK_NE(alias, helper.hashExample("module Example\n"
                                       "  Z = Y\n"
                                       "end\n"));

    // changing the RHS should affect the hash
    CHECK_NE(alias, helper.hashExample("module Example\n"
                                       "  X = Z\n"
                                       "end\n"));

    // on the other hand, for non-aliases, we ONLY care about the LHS
    auto cnst = helper.hashExample("module Example\n"
                                   "  X = T.let(Y, nil)\n"
                                   "end");

    // changing the LHS should affect the hash
    CHECK_NE(cnst, helper.hashExample("module Example\n"
                                      "  Z = T.let(Y, nil)\n"
                                      "end\n"));

    // changing the RHS SHOULD NOT affect the hash here
    CHECK_EQ(cnst, helper.hashExample("module Example\n"
                                      "  X = T.let(Z, nil)\n"
                                      "end\n"));
}

TEST_CASE("Require") { // NOLINT
    Helper helper;

    // changing requires should affect the hash
    auto req = helper.hashExample("require 'foo'\n");

    // changing the name of a require should affect the hash
    CHECK_NE(req, helper.hashExample("require 'bar'\n"));

    // adding a new require should affect the hash
    CHECK_NE(req, helper.hashExample("require 'foo'\n"
                                     "require 'bar'\n"));

    // obviously deleting the require should affect the hash
    CHECK_NE(req, helper.hashExample("\n"));

    // other sends should not affect the hash
    CHECK_EQ(req, helper.hashExample("require 'foo'\n"
                                     "do_the_thing!"));
}

TEST_CASE("Extend/Include") {
    Helper helper;

    // changing requires should affect the hash
    auto incl = helper.hashExample("class Example\n"
                                   "  include Foo\n"
                                   "end\n");

    // changing the name of the included module should affect the hash
    CHECK_NE(incl, helper.hashExample("class Example\n"
                                      "  include Bar\n"
                                      "end\n"));

    // changing the module to be extended should affect the hash
    CHECK_NE(incl, helper.hashExample("class Example\n"
                                      "  extend Foo\n"
                                      "end\n"));
}

}; // namespace sorbet
