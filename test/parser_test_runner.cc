#include "doctest/doctest.h"
#include <cxxopts.hpp>
// has to go first as it violates our requirements

// has to go first, as it violates poisons
#include "core/proto/proto.h"

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/treemap/treemap.h"
#include "cfg/CFG.h"
#include "cfg/builder/builder.h"
#include "class_flatten/class_flatten.h"
#include "common/FileOps.h"
#include "common/common.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include "common/web_tracer_framework/tracing.h"
#include "core/Error.h"
#include "core/ErrorCollector.h"
#include "core/ErrorQueue.h"
#include "core/Unfreeze.h"
#include "core/serialize/serialize.h"
#include "definition_validator/validator.h"
#include "infer/infer.h"
#include "main/autogen/autogen.h"
#include "namer/namer.h"
#include "parser/parser.h"
#include "payload/binary/binary.h"
#include "resolver/resolver.h"
#include "rewriter/rewriter.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "test/helpers/expectations.h"
#include "test/helpers/position_assertions.h"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <vector>

namespace sorbet::test {
using namespace std;

string singleTest;

UnorderedSet<string> knownExpectations = {
    "parse-tree",       "parse-tree-json", "parse-tree-whitequark", "desugar-tree", "desugar-tree-raw", "rewrite-tree",
    "rewrite-tree-raw", "index-tree",      "index-tree-raw",        "symbol-table", "symbol-table-raw", "name-tree",
    "name-tree-raw",    "resolve-tree",    "resolve-tree-raw",      "flatten-tree", "flatten-tree-raw", "cfg",
    "cfg-raw",          "autogen",         "document-symbols"};

/** Converts a Sorbet Error object into an equivalent LSP Diagnostic object. */
unique_ptr<Diagnostic> errorToDiagnostic(const core::GlobalState &gs, const core::Error &error) {
    if (!error.loc.exists()) {
        return nullptr;
    }
    return make_unique<Diagnostic>(Range::fromLoc(gs, error.loc), error.header);
}

TEST_CASE("WhitequarkParserTest") {
    Expectations test = Expectations::getExpectations(singleTest);

    vector<unique_ptr<core::Error>> errors;
    auto inputPath = test.folder + test.basename;
    auto rbName = test.basename + ".rb";

    for (auto &exp : test.expectations) {
        auto it = knownExpectations.find(exp.first);
        if (it == knownExpectations.end()) {
            FAIL_CHECK("Unknown pass: " << exp.first);
        }
    }

    auto logger = spdlog::stderr_color_mt("fixtures: " + inputPath);
    auto errorCollector = make_shared<core::ErrorCollector>();
    auto errorQueue = make_shared<core::ErrorQueue>(*logger, *logger, errorCollector);
    core::GlobalState gs(errorQueue);

    for (auto provider : sorbet::pipeline::semantic_extension::SemanticExtensionProvider::getProviders()) {
        gs.semanticExtensions.emplace_back(provider->defaultInstance());
    }

    gs.censorForSnapshotTests = true;
    auto workers = WorkerPool::create(0, gs.tracer());

    auto assertions = RangeAssertion::parseAssertions(test.sourceFileContents);
    if (BooleanPropertyAssertion::getValue("no-stdlib", assertions).value_or(false)) {
        gs.initEmpty();
    } else {
        core::serialize::Serializer::loadGlobalState(gs, getNameTablePayload);
    }
    // Parser
    vector<core::FileRef> files;
    constexpr string_view whitelistedTypedNoneTest = "missing_typed_sigil.rb"sv;
    {
        core::UnfreezeFileTable fileTableAccess(gs);

        for (auto &sourceFile : test.sourceFiles) {
            auto fref = gs.enterFile(test.sourceFileContents[test.folder + sourceFile]);
            if (FileOps::getFileName(sourceFile) == whitelistedTypedNoneTest) {
                fref.data(gs).strictLevel = core::StrictLevel::False;
            }
            files.emplace_back(fref);
        }
    }
    vector<ast::ParsedFile> trees;
    map<string, string> got;

    for (auto file : files) {
        if (FileOps::getFileName(file.data(gs).path()) != whitelistedTypedNoneTest &&
            file.data(gs).source().find("# typed:") == string::npos) {
            ADD_FAIL_CHECK_AT(file.data(gs).path().data(), 1, "Add a `# typed: strict` line to the top of this file");
        }
        unique_ptr<parser::Node> nodes;
        {
            core::UnfreezeNameTable nameTableAccess(gs); // enters original strings

            // whitequark/parser declares these 3 meta variables to
            // simplify testing cases around local variables
            vector<string> initialLocals = {"foo", "bar", "baz"};
            auto settings = parser::Parser::Settings{};
            nodes = parser::Parser::run(gs, file, settings, initialLocals);
        }
        {
            errorQueue->flushAllErrors(gs);
            auto newErrors = errorCollector->drainErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }

        auto expectation = test.expectations.find("parse-tree-whitequark");
        if (expectation != test.expectations.end()) {
            got["parse-tree-whitequark"].append(nodes->toWhitequark(gs)).append("\n");
            errorQueue->flushAllErrors(gs);
            auto newErrors = errorCollector->drainErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }
    }

    for (auto &gotPhase : got) {
        auto expectation = test.expectations.find(gotPhase.first);
        REQUIRE_MESSAGE(expectation != test.expectations.end(), "missing expectation for " << gotPhase.first);
        REQUIRE_MESSAGE(expectation->second.size() == 1,
                        "found unexpected multiple expectations of type " << gotPhase.first);

        auto checker = test.folder + expectation->second.begin()->second;
        auto expect = FileOps::read(checker.c_str());
        {
            INFO("Mismatch on: " << checker);
            CHECK_EQ(expect, gotPhase.second);
        }
        if (expect == gotPhase.second) {
            MESSAGE(gotPhase.first << " OK");
        }
    }

    // Check warnings and errors
    {
        map<string, vector<unique_ptr<Diagnostic>>> diagnostics;
        for (auto &error : errors) {
            if (error->isSilenced) {
                continue;
            }
            auto diag = errorToDiagnostic(gs, *error);
            if (diag == nullptr) {
                continue;
            }
            auto path = error->loc.file().data(gs).path();
            diagnostics[string(path.begin(), path.end())].push_back(std::move(diag));
        }
        ErrorAssertion::checkAll(test.sourceFileContents, RangeAssertion::getErrorAssertions(assertions), diagnostics);
    }

    MESSAGE("errors OK");
} // namespace sorbet::test

} // namespace sorbet::test

int main(int argc, char *argv[]) {
    cxxopts::Options options("test_corpus", "Test corpus for Sorbet typechecker");
    options.allow_unrecognised_options().add_options()("single_test", "run over single test.",
                                                       cxxopts::value<std::string>()->default_value(""), "testpath");
    auto res = options.parse(argc, argv);

    if (res.count("single_test") != 1) {
        printf("--single_test=<filename> argument expected\n");
        return 1;
    }

    sorbet::test::singleTest = res["single_test"].as<std::string>();

    doctest::Context context(argc, argv);
    return context.run();
}
