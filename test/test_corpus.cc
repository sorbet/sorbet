#include "gtest/gtest.h"
#include <cxxopts.hpp>
// has to go first as it violates are requirements

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/treemap/treemap.h"
#include "cfg/CFG.h"
#include "cfg/builder/builder.h"
#include "common/common.h"
#include "core/Error.h"
#include "core/Unfreeze.h"
#include "core/serialize/serialize.h"
#include "dsl/dsl.h"
#include "infer/infer.h"
#include "main/autogen/autogen.h"
#include "namer/namer.h"
#include "parser/parser.h"
#include "payload/binary/binary.h"
#include "resolver/resolver.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "test/LSPTest.h"
#include "test/helpers/expectations.h"
#include "test/helpers/lsp.h"
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

// taken from https://stackoverflow.com/questions/16491675/how-to-send-custom-message-in-google-c-testing-framework
namespace testing::internal {

extern void ColoredPrintf(GTestColor color, const char *fmt, ...);
} // namespace testing::internal

namespace sorbet::test {
namespace spd = spdlog;
using namespace std;

string singleTest;

vector<Expectations> getInputs(string singleTest);

string prettyPrintTest(testing::TestParamInfo<Expectations> arg) {
    string res = arg.param.folder + arg.param.basename;
    auto ext = FileOps::getExtension(res);
    if (ext == "rb") {
        res.erase(res.end() - ext.size() - 1, res.end());
    }
    absl::c_replace(res, '/', '_');
    return res;
}

class ExpectationTest : public testing::TestWithParam<Expectations> {
public:
    ~ExpectationTest() override = default;
    void SetUp() override {}
    void TearDown() override {}
};

#define PRINTF(...)                                                                        \
    do {                                                                                   \
        testing::internal::ColoredPrintf(testing::internal::COLOR_GREEN, "[          ] "); \
        testing::internal::ColoredPrintf(testing::internal::COLOR_GREEN, __VA_ARGS__);     \
    } while (0)

// C++ stream interface
class TestCout : public stringstream {
public:
    ~TestCout() override {
        PRINTF("%s", str().c_str());
    }
};

#define TEST_COUT TestCout()

class CFGCollectorAndTyper {
public:
    vector<string> cfgs;
    unique_ptr<ast::MethodDef> preTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> m) {
        if (m->symbol.data(ctx)->isOverloaded()) {
            return m;
        }
        auto cfg = cfg::CFGBuilder::buildFor(ctx.withOwner(m->symbol), *m);
        cfg = infer::Inference::run(ctx.withOwner(cfg->symbol), move(cfg));
        cfgs.emplace_back(cfg->toString(ctx));
        return m;
    }
};

UnorderedSet<string> knownPasses = {"parse-tree", "parse-tree-json", "ast",          "ast-raw",
                                    "dsl-tree",   "dsl-tree-raw",    "symbol-table", "symbol-table-raw",
                                    "name-tree",  "name-tree-raw",   "resolve-tree", "resolve-tree-raw",
                                    "cfg",        "autogen"};

ast::ParsedFile testSerialize(core::GlobalState &gs, ast::ParsedFile expr) {
    auto saved = core::serialize::Serializer::storeExpression(gs, expr.tree);
    auto restored = core::serialize::Serializer::loadExpression(gs, saved.data());
    return {move(restored), expr.file};
}

/** Converts a Sorbet Detail object into an equivalent LSP Position object. */
unique_ptr<Position> detailToPosition(const core::Loc::Detail &detail) {
    // 1-indexed => 0-indexed
    return make_unique<Position>(detail.line - 1, detail.column - 1);
}

/** Converts a Sorbet Error object into an equivalent LSP Diagnostic object. */
unique_ptr<Diagnostic> errorToDiagnostic(core::GlobalState &gs, const core::Error &error) {
    auto position = error.loc.position(gs);
    auto range = make_unique<Range>(detailToPosition(position.first), detailToPosition(position.second));
    return make_unique<Diagnostic>(move(range), error.header);
}

TEST_P(ExpectationTest, PerPhaseTest) { // NOLINT
    vector<unique_ptr<core::Error>> errors;
    Expectations test = GetParam();
    auto inputPath = test.folder + test.basename;
    auto rbName = test.basename + ".rb";
    SCOPED_TRACE(inputPath);

    for (auto &exp : test.expectations) {
        auto it = knownPasses.find(exp.first);
        if (it == knownPasses.end()) {
            ADD_FAILURE() << "Unknown pass: " << exp.first;
        }
    }

    auto logger = spd::stderr_color_mt("fixtures: " + inputPath);
    auto errorQueue = make_shared<core::ErrorQueue>(*logger, *logger);
    core::GlobalState gs(errorQueue);
    core::serialize::Serializer::loadGlobalState(gs, getNameTablePayload);
    core::MutableContext ctx(gs, core::Symbols::root());

    // Parser
    vector<core::FileRef> files;
    {
        core::UnfreezeFileTable fileTableAccess(gs);

        for (auto &sourceFile : test.sourceFiles) {
            files.emplace_back(gs.enterFile(test.sourceFileContents[test.folder + sourceFile]));
        }
    }
    vector<ast::ParsedFile> trees;
    map<string, string> got;

    vector<core::ErrorRegion> errs;
    for (auto file : files) {
        errs.emplace_back(gs, file);
        if (file.data(ctx).source().find("# typed:") == string::npos) {
            ADD_FAILURE_AT(file.data(gs).path().data(), 1) << "Add a `# typed: strict` line to the top of this file";
        }

        unique_ptr<parser::Node> nodes;
        {
            core::UnfreezeNameTable nameTableAccess(gs); // enters original strings

            nodes = parser::Parser::run(gs, file);
        }
        {
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }

        auto expectation = test.expectations.find("parse-tree");
        if (expectation != test.expectations.end()) {
            got["parse-tree"].append(nodes->toString(gs)).append("\n");
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }

        expectation = test.expectations.find("parse-tree-json");
        if (expectation != test.expectations.end()) {
            got["parse-tree-json"].append(nodes->toJSON(gs)).append("\n");
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }

        // Desugarer
        ast::ParsedFile desugared;
        {
            core::UnfreezeNameTable nameTableAccess(gs); // enters original strings
            auto file = nodes->loc.file();
            desugared = testSerialize(gs, ast::ParsedFile{ast::desugar::node2Tree(ctx, move(nodes)), file});
        }

        expectation = test.expectations.find("ast");
        if (expectation != test.expectations.end()) {
            got["ast"].append(desugared.tree->toString(gs)).append("\n");
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }

        expectation = test.expectations.find("ast-raw");
        if (expectation != test.expectations.end()) {
            got["ast-raw"].append(desugared.tree->showRaw(gs)).append("\n");
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }
        ast::ParsedFile dslUnwound;

        if (test.expectations.find("autogen") == test.expectations.end()) {
            // DSL
            {
                core::UnfreezeNameTable nameTableAccess(gs); // enters original strings

                dslUnwound =
                    testSerialize(gs, ast::ParsedFile{dsl::DSL::run(ctx, move(desugared.tree)), desugared.file});
            }

            expectation = test.expectations.find("dsl-tree");
            if (expectation != test.expectations.end()) {
                got["dsl-tree"].append(dslUnwound.tree->toString(gs)).append("\n");
                auto newErrors = errorQueue->drainAllErrors();
                errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
            }

            expectation = test.expectations.find("dsl-tree-raw");
            if (expectation != test.expectations.end()) {
                got["dsl-tree-raw"].append(dslUnwound.tree->showRaw(gs)).append("\n");
                auto newErrors = errorQueue->drainAllErrors();
                errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
            }
        } else {
            dslUnwound = move(desugared);
            if (test.expectations.find("dsl-tree-raw") != test.expectations.end() ||

                test.expectations.find("dsl-tree") != test.expectations.end()) {
                ADD_FAILURE() << "Running DSL passes with autogen isn't supported";
            }
        }

        // Namer
        ast::ParsedFile namedTree;
        {
            core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
            core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
            namedTree = testSerialize(gs, namer::Namer::run(ctx, move(dslUnwound)));
        }

        expectation = test.expectations.find("name-tree");
        if (expectation != test.expectations.end()) {
            got["name-tree"].append(namedTree.tree->toString(gs)).append("\n");
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }

        expectation = test.expectations.find("name-tree-raw");
        if (expectation != test.expectations.end()) {
            got["name-tree-raw"].append(namedTree.tree->showRaw(gs));
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }

        trees.emplace_back(move(namedTree));
    }

    auto expectation = test.expectations.find("autogen");
    if (expectation != test.expectations.end()) {
        {
            core::UnfreezeNameTable nameTableAccess(gs);
            core::UnfreezeSymbolTable symbolAccess(gs);

            trees = resolver::Resolver::runConstantResolution(ctx, move(trees));
        }

        for (auto &tree : trees) {
            auto pf = autogen::Autogen::generate(ctx, move(tree));
            tree = move(pf.tree);
            got["autogen"].append(pf.toString(ctx));
        }

        auto newErrors = errorQueue->drainAllErrors();
        errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
    } else {
        core::UnfreezeNameTable nameTableAccess(gs);     // Resolver::defineAttr
        core::UnfreezeSymbolTable symbolTableAccess(gs); // enters stubs
        trees = resolver::Resolver::run(ctx, move(trees));
        auto newErrors = errorQueue->drainAllErrors();
        errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
    }

    expectation = test.expectations.find("symbol-table");
    if (expectation != test.expectations.end()) {
        got["symbol-table"] = gs.toString() + '\n';
        auto newErrors = errorQueue->drainAllErrors();
        errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
    }

    expectation = test.expectations.find("symbol-table-raw");
    if (expectation != test.expectations.end()) {
        got["symbol-table-raw"] = gs.toStringWithOptions(false, true) + '\n';
        auto newErrors = errorQueue->drainAllErrors();
        errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
    }

    for (auto &resolvedTree : trees) {
        expectation = test.expectations.find("resolve-tree");
        if (expectation != test.expectations.end()) {
            got["resolve-tree"].append(resolvedTree.tree->toString(gs)).append("\n");
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }

        expectation = test.expectations.find("resolve-tree-raw");
        if (expectation != test.expectations.end()) {
            got["resolve-tree-raw"].append(resolvedTree.tree->showRaw(gs)).append("\n");
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }
    }

    for (auto &resolvedTree : trees) {
        auto file = resolvedTree.file;
        auto checkTree = [&]() {
            if (resolvedTree.tree == nullptr) {
                auto path = file.data(ctx).path();
                ADD_FAILURE_AT(path.begin(), 1) << "Already used tree. You can only have 1 CFG-ish .exp file";
            }
        };
        auto checkPragma = [&](string ext) {
            if (file.data(ctx).strictLevel < core::StrictLevel::Typed) {
                auto path = file.data(ctx).path();
                ADD_FAILURE_AT(path.begin(), 1)
                    << "Missing `# typed:` pragma. Sources with ." << ext << ".exp files must specify # typed:";
            }
        };
        // CFG

        expectation = test.expectations.find("cfg");
        if (expectation != test.expectations.end()) {
            checkTree();
            checkPragma("cfg");
            CFGCollectorAndTyper collector;
            auto cfg = ast::TreeMap::apply(ctx, collector, move(resolvedTree.tree));
            resolvedTree.tree.reset();

            stringstream dot;
            dot << "digraph \"" << rbName << "\" {" << '\n';
            for (auto &cfg : collector.cfgs) {
                dot << cfg << '\n' << '\n';
            }
            dot << "}" << '\n' << '\n';
            got["cfg"].append(dot.str());

            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }

        // If there is a tree left with a typed: pragma, run the inferencer
        if (resolvedTree.tree != nullptr && file.data(ctx).originalSigil >= core::StrictLevel::Typed) {
            checkTree();
            CFGCollectorAndTyper collector;
            ast::TreeMap::apply(ctx, collector, move(resolvedTree.tree));
            resolvedTree.tree.reset();
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }
    }

    for (auto &gotPhase : got) {
        auto expectation = test.expectations.find(gotPhase.first);
        ASSERT_TRUE(expectation != test.expectations.end()) << "missing expectation for " << gotPhase.first;
        auto checker = test.folder + expectation->second;
        auto expect = FileOps::read(checker.c_str());
        EXPECT_EQ(expect, gotPhase.second) << "Mismatch on: " << checker;
        if (expect == gotPhase.second) {
            TEST_COUT << gotPhase.first << " OK" << '\n';
        }
    }

    expectation = test.expectations.find("symbol-table");
    if (expectation != test.expectations.end()) {
        string table = gs.toString() + '\n';
        EXPECT_EQ(got["symbol-table"], table) << " symbol-table should not be mutated by CFG+inference";
    }

    expectation = test.expectations.find("symbol-table-raw");
    if (expectation != test.expectations.end()) {
        string table = gs.toStringWithOptions(false, true) + '\n';
        EXPECT_EQ(got["symbol-table-raw"], table) << " symbol-table-raw should not be mutated by CFG+inference";
    }

    // Check warnings and errors
    {
        auto assertions = RangeAssertion::parseAssertions(test.sourceFileContents);

        map<string, vector<unique_ptr<Diagnostic>>> diagnostics;
        for (auto &error : errors) {
            if (error->isSilenced) {
                continue;
            }
            auto path = error->loc.file().data(gs).path();
            diagnostics[string(path.begin(), path.end())].push_back(errorToDiagnostic(gs, *error));
        }
        ErrorAssertion::checkAll(test.sourceFileContents, RangeAssertion::getErrorAssertions(assertions), diagnostics);
    }

    // Allow later phases to have errors that we didn't test for
    errorQueue->drainAllErrors();

    TEST_COUT << "errors OK" << '\n';
}

TEST_P(LSPTest, All) {
    string rootPath = "/Users/jvilk/stripe/pay-server";
    string rootUri = fmt::format("file://{}", rootPath);

    // filename => URI
    UnorderedMap<string, string> testFileUris;
    for (auto &filename : filenames) {
        testFileUris[filename] = filePathToUri(rootUri, filename);
    }

    // Perform initialize / initialized handshake.
    {
        auto initializedResponses = initializeLSP(rootPath, rootUri, *lspWrapper, nextId);
        EXPECT_EQ(0, initializedResponses.size()) << "Should not receive any response to 'initialized' message.";
    }

    // Tell LSP that we opened a bunch of brand new, empty files (the test files).
    {
        for (auto &filename : filenames) {
            auto params = make_unique<DidOpenTextDocumentParams>(
                make_unique<TextDocumentItem>(testFileUris[filename], "ruby", 1, ""));
            LSPMessage message(make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidOpen, move(params)));
            auto responses = lspWrapper->getLSPResponsesFor(message);
            EXPECT_EQ(0, responses.size()) << "Should not receive any response to opening an empty file.";
        }
    }

    // Tell LSP that the new files now have the contents from the test files on disk.
    {
        vector<unique_ptr<LSPMessage>> allResponses;
        bool slowPathPassed = true;
        bool skipFastPath = DisableFastPath::getValue(assertions);
        // Run changes through LSP twice: The first time is a slow path, the second time is a fast path.
        // Surfaces errors that occur due to differences in how slow and fast paths run.
        // Skip the second iteration if slow path fails to avoid printing out duplicate errors.
        for (int i = 0; i < 2 && slowPathPassed; i++) {
            std::string errorPrefix = "";
            if (i == 1) {
                if (skipFastPath) {
                    break;
                }
                errorPrefix = "[After running fast path] ";
            }

            vector<unique_ptr<LSPMessage>> updates;
            for (auto &filename : filenames) {
                auto textDoc = make_unique<VersionedTextDocumentIdentifier>(testFileUris[filename], 2);
                auto textDocContents = test.sourceFileContents[filename]->source();
                auto text = string(textDocContents.begin(), textDocContents.end());

                auto textDocChange = make_unique<TextDocumentContentChangeEvent>(text);
                vector<unique_ptr<TextDocumentContentChangeEvent>> textChanges;
                textChanges.push_back(move(textDocChange));

                auto didChangeParams = make_unique<DidChangeTextDocumentParams>(move(textDoc), move(textChanges));
                auto didChangeNotif =
                    make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidChange, move(didChangeParams));
                updates.push_back(make_unique<LSPMessage>(move(didChangeNotif)));
            }
            auto responses = lspWrapper->getLSPResponsesFor(updates);
            allResponses.insert(allResponses.end(), make_move_iterator(responses.begin()),
                                make_move_iterator(responses.end()));

            // "Newly pushed diagnostics always replace previously pushed diagnostics. There is no merging that happens
            // on the client side." Prune irrelevant diagnostics, and only keep the newest diagnostics for a file.
            // filename => diagnostics for file
            map<string, vector<unique_ptr<Diagnostic>>> diagnostics;
            {
                for (auto &response : allResponses) {
                    if (assertNotificationMessage(LSPMethod::TextDocumentPublishDiagnostics, *response)) {
                        auto maybeDiagnosticParams = getPublishDiagnosticParams(response->asNotification());
                        ASSERT_TRUE(maybeDiagnosticParams.has_value());
                        auto &diagnosticParams = *maybeDiagnosticParams;
                        auto filename = uriToFilePath(rootUri, diagnosticParams->uri);
                        EXPECT_NE(testFileUris.end(), testFileUris.find(filename))
                            << fmt::format("Diagnostic URI is not a test file URI: {}", diagnosticParams->uri);

                        // Will explicitly overwrite older diagnostics that are irrelevant.
                        diagnostics[filename] = move(diagnosticParams->diagnostics);
                    }
                }
            }
            slowPathPassed = ErrorAssertion::checkAll(
                test.sourceFileContents, RangeAssertion::getErrorAssertions(assertions), diagnostics, errorPrefix);
            if (!slowPathPassed && i == 1) {
                ADD_FAILURE() << "Note: To disable fast path tests, add `# disable-fast-path: true` to the file.";
            }
        }
    }

    // Usage and def assertions
    {
        // Sort by symbol.
        // symbol => [version => DefAssertion, [DefAssertion+UsageAssertion][]]
        // Note: Using a vector in pair since order matters; assertions are ordered by location, which
        // is used when comparing against LSP responses.
        UnorderedMap<string, pair<UnorderedMap<int, shared_ptr<DefAssertion>>, vector<shared_ptr<RangeAssertion>>>>
            defUsageMap;
        for (auto &assertion : assertions) {
            if (auto defAssertion = dynamic_pointer_cast<DefAssertion>(assertion)) {
                auto &entry = defUsageMap[defAssertion->symbol];
                auto &defMap = entry.first;
                EXPECT_FALSE(defMap.contains(defAssertion->version)) << fmt::format(
                    "Found multiple def comments for label `{}` version `{}`.\nPlease use unique labels and versions "
                    "for definition assertions. Note that these labels do not need to match the pointed-to "
                    "identifiers.\nFor example, the following is completely valid:\n foo = 3\n#^^^ def: bar 100",
                    defAssertion->symbol, defAssertion->version);
                defMap[defAssertion->version] = defAssertion;
                entry.second.push_back(defAssertion);
            } else if (auto usageAssertion = dynamic_pointer_cast<UsageAssertion>(assertion)) {
                auto &entry = defUsageMap[usageAssertion->symbol];
                entry.second.push_back(usageAssertion);
            }
        }

        // Check each assertion.
        for (auto &entry : defUsageMap) {
            auto &entryAssertions = entry.second.second;
            // Sort assertions in (filename, range) order
            fast_sort(entryAssertions,
                      [](const shared_ptr<RangeAssertion> &a, const shared_ptr<RangeAssertion> &b) -> bool {
                          return errorComparison(a->filename, *a->range, "", b->filename, *b->range, "") == -1;
                      });

            auto &defAssertions = entry.second.first;
            // Shouldn't be possible to have an entry with 0 assertions, but explicitly check anyway.
            EXPECT_GE(entryAssertions.size(), 1);

            for (auto &assertion : entryAssertions) {
                string_view symbol;
                int version = -1;
                if (auto defAssertion = dynamic_pointer_cast<DefAssertion>(assertion)) {
                    version = defAssertion->version;
                    symbol = defAssertion->symbol;
                } else if (auto usageAssertion = dynamic_pointer_cast<UsageAssertion>(assertion)) {
                    version = usageAssertion->version;
                    symbol = usageAssertion->symbol;
                }
                auto entry = defAssertions.find(version);
                if (entry != defAssertions.end()) {
                    auto &def = entry->second;
                    auto queryLoc = assertion->getLocation(rootUri);
                    // Check that a definition request at this location returns def.
                    def->check(test.sourceFileContents, *lspWrapper, nextId, rootUri, *queryLoc);
                    // Check that a reference request at this location returns entryAssertions.
                    UsageAssertion::check(test.sourceFileContents, *lspWrapper, nextId, rootUri, symbol, *queryLoc,
                                          entryAssertions);
                } else {
                    ADD_FAILURE() << fmt::format(
                        "Found usage comment for label {0} version {1} without matching def comment. Please add a `# "
                        "^^ def: {0} {1}` assertion that points to the definition of the pointed-to thing being used.",
                        symbol, version);
                }
            }
        }
    }
}

INSTANTIATE_TEST_CASE_P(PosTests, ExpectationTest, ::testing::ValuesIn(getInputs(singleTest)), prettyPrintTest);
INSTANTIATE_TEST_CASE_P(LSPTests, LSPTest, ::testing::ValuesIn(getInputs(singleTest)), prettyPrintTest);

bool compareNames(string_view left, string_view right) {
    auto lsplit = left.find("__");
    if (lsplit == string::npos) {
        lsplit = left.find(".");
    }
    auto rsplit = right.find("__");
    if (rsplit == string::npos) {
        rsplit = right.find(".");
    }
    string_view lbase(left.data(), lsplit == string::npos ? left.size() : lsplit);
    string_view rbase(right.data(), rsplit == string::npos ? right.size() : rsplit);
    if (lbase != rbase) {
        return left < right;
    }

    // If the base names match, compare by reverse order on extension, so that
    // .exp comes after .rb.
    auto lext = FileOps::getExtension(left);
    auto rext = FileOps::getExtension(right);
    if (lext != rext) {
        return rext < lext;
    }

    // Sort multi-part tests
    return left < right;
}

string rbFile2BaseTestName(string rbFileName) {
    auto basename = rbFileName;
    auto lastDirSeparator = basename.find_last_of("/");
    if (lastDirSeparator != string::npos) {
        basename = basename.substr(lastDirSeparator + 1);
    }
    auto split = basename.rfind(".");
    if (split != string::npos) {
        basename = basename.substr(0, split);
    }
    split = basename.find("__");
    if (split != string::npos) {
        basename = basename.substr(0, split);
    }
    string testName = basename;
    if (lastDirSeparator != string::npos) {
        testName = rbFileName.substr(0, lastDirSeparator + 1 + testName.length());
    }
    return testName;
}

vector<Expectations> listDir(const char *name) {
    vector<Expectations> result;

    vector<string> names = sorbet::FileOps::listFilesInDir(name, {".rb", ".exp"}, false, {}, {});
    const int prefixLen = strnlen(name, 1024) + 1;
    // Trim off the input directory from the name.
    transform(names.begin(), names.end(), names.begin(),
              [&prefixLen](auto &name) -> string { return name.substr(prefixLen); });
    fast_sort(names, compareNames);

    Expectations current;
    for (auto &s : names) {
        if (absl::EndsWith(s, ".rb")) {
            auto basename = rbFile2BaseTestName(s);
            if (basename != s) {
                if (basename == current.basename) {
                    current.sourceFiles.emplace_back(s);
                    continue;
                }
            }

            if (!current.basename.empty()) {
                result.emplace_back(current);
                current = Expectations();
            }
            current.basename = basename;
            current.sourceFiles.emplace_back(s);
            current.folder = name;
            current.folder += "/";
            current.testName = current.folder + current.basename;
        } else if (absl::EndsWith(s, ".exp")) {
            if (absl::StartsWith(s, current.basename)) {
                auto kind_start = s.rfind(".", s.size() - strlen(".exp") - 1);
                string kind = s.substr(kind_start + 1, s.size() - kind_start - strlen(".exp") - 1);
                current.expectations[kind] = s;
            }
        }
    }
    if (!current.basename.empty()) {
        result.emplace_back(current);
        current = Expectations();
    }

    return result;
}

vector<Expectations> getInputs(string singleTest) {
    vector<Expectations> result;
    if (singleTest.empty()) {
        Exception::raise("No test specified. Pass one with --single_test=<test_path>");
    }

    string parentDir;
    {
        auto lastDirSeparator = singleTest.find_last_of("/");
        if (lastDirSeparator == string::npos) {
            parentDir = ".";
        } else {
            parentDir = singleTest.substr(0, lastDirSeparator);
        }
    }
    auto scan = listDir(parentDir.c_str());
    auto lookingFor = rbFile2BaseTestName(singleTest);
    cout << lookingFor;
    for (Expectations &f : scan) {
        if (f.testName == lookingFor) {
            for (auto &file : f.sourceFiles) {
                string filename = f.folder + file;
                string fileContents = FileOps::read(filename);
                f.sourceFileContents[filename] =
                    make_shared<core::File>(move(filename), move(fileContents), core::File::Type::Normal);
            }
            result.emplace_back(f);
        }
    }

    if (result.empty()) {
        Exception::raise("None tests found!");
    }
    return result;
}
} // namespace sorbet::test

int main(int argc, char *argv[]) {
    cxxopts::Options options("test_corpus", "Test corpus for Sorbet typechecker");
    options.allow_unrecognised_options().add_options()("single_test", "run over single test.",
                                                       cxxopts::value<std::string>()->default_value(""), "testpath");
    options.add_options()("lsp-disable-fastpath", "disable fastpath in lsp tests");
    auto res = options.parse(argc, argv);

    if (res.count("single_test") != 1) {
        printf("--single_test=<filename> argument expected\n");
        return 1;
    }

    if (res["lsp-disable-fastpath"].as<bool>()) {
        printf("disabling lsp fastpath\n");
        sorbet::test::LSPTest::fastpathDisabled = true;
    }

    sorbet::test::singleTest = res["single_test"].as<std::string>();

    ::testing::InitGoogleTest(&argc, (char **)argv);
    return RUN_ALL_TESTS();
}
