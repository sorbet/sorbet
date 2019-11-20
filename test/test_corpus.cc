#include "gtest/gtest.h"
#include <cxxopts.hpp>
// has to go first as it violates are requirements

// has to go first, as it violates poisons
#include "core/proto/proto.h"

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/treemap/treemap.h"
#include "cfg/CFG.h"
#include "cfg/builder/builder.h"
#include "cfg/proto/proto.h"
#include "common/FileOps.h"
#include "common/common.h"
#include "common/formatting.h"
#include "common/sort.h"
#include "core/Error.h"
#include "core/Unfreeze.h"
#include "core/serialize/serialize.h"
#include "definition_validator/validator.h"
#include "flattener/flatten.h"
#include "infer/infer.h"
#include "local_vars/local_vars.h"
#include "main/autogen/autogen.h"
#include "namer/namer.h"
#include "parser/parser.h"
#include "payload/binary/binary.h"
#include "resolver/resolver.h"
#include "rewriter/rewriter.h"
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

class WhitequarkParserTest : public testing::TestWithParam<Expectations> {
public:
    ~WhitequarkParserTest() override = default;
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
    vector<unique_ptr<cfg::CFG>> cfgs;
    unique_ptr<ast::MethodDef> preTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> m) {
        if (m->symbol.data(ctx)->isOverloaded()) {
            return m;
        }
        auto cfg = cfg::CFGBuilder::buildFor(ctx.withOwner(m->symbol), *m);
        auto symbol = cfg->symbol;
        cfg = infer::Inference::run(ctx.withOwner(symbol), move(cfg));
        cfgs.push_back(move(cfg));
        return m;
    }
};

UnorderedSet<string> knownExpectations = {
    "parse-tree",       "parse-tree-json", "parse-tree-whitequark", "desugar-tree", "desugar-tree-raw", "rewrite-tree",
    "rewrite-tree-raw", "symbol-table",    "symbol-table-raw",      "name-tree",    "name-tree-raw",    "resolve-tree",
    "resolve-tree-raw", "flatten-tree",    "flatten-tree-raw",      "cfg",          "cfg-raw",          "cfg-json",
    "autogen",          "document-symbols"};

ast::ParsedFile testSerialize(core::GlobalState &gs, ast::ParsedFile expr) {
    auto saved = core::serialize::Serializer::storeExpression(gs, expr.tree);
    auto restored = core::serialize::Serializer::loadExpression(gs, saved.data());
    return {move(restored), expr.file};
}

/** Converts a Sorbet Error object into an equivalent LSP Diagnostic object. */
unique_ptr<Diagnostic> errorToDiagnostic(const core::GlobalState &gs, const core::Error &error) {
    if (!error.loc.exists()) {
        return nullptr;
    }
    return make_unique<Diagnostic>(Range::fromLoc(gs, error.loc), error.header);
}

class ExpectationHandler {
    Expectations &test;
    shared_ptr<core::ErrorQueue> &errorQueue;

public:
    vector<unique_ptr<core::Error>> errors;
    UnorderedMap<string_view, string> got;

    ExpectationHandler(Expectations &test, shared_ptr<core::ErrorQueue> &errorQueue)
        : test(test), errorQueue(errorQueue){};

    void addObserved(string_view expectationType, std::function<string()> mkExp, bool addNewline = true) {
        if (test.expectations.contains(expectationType)) {
            got[expectationType].append(mkExp());
            if (addNewline) {
                got[expectationType].append("\n");
            }
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }
    }

    void checkExpectations(string prefix = "") {
        for (auto &gotPhase : got) {
            auto expectation = test.expectations.find(gotPhase.first);
            ASSERT_TRUE(expectation != test.expectations.end())
                << prefix << "missing expectation for " << gotPhase.first;
            ASSERT_TRUE(expectation->second.size() == 1)
                << prefix << "found unexpected multiple expectations of type " << gotPhase.first;

            auto checker = test.folder + expectation->second.begin()->second;
            auto expect = FileOps::read(checker);
            EXPECT_EQ(expect, gotPhase.second) << prefix << "Mismatch on: " << checker;
            if (expect == gotPhase.second) {
                TEST_COUT << gotPhase.first << " OK" << '\n';
            }
        }
    }

    void drainErrors() {
        auto newErrors = errorQueue->drainAllErrors();
        errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
    }

    void clear() {
        got.clear();
        errorQueue->drainAllErrors();
    }
};

TEST_P(ExpectationTest, PerPhaseTest) { // NOLINT
    Expectations test = GetParam();
    auto inputPath = test.folder + test.basename;
    auto rbName = test.basename + ".rb";
    SCOPED_TRACE(inputPath);

    for (auto &exp : test.expectations) {
        if (!knownExpectations.contains(exp.first)) {
            ADD_FAILURE() << "Unknown pass: " << exp.first;
        }
    }

    auto logger = spd::stderr_color_mt("fixtures: " + inputPath);
    auto errorQueue = make_shared<core::ErrorQueue>(*logger, *logger);
    auto gs = make_unique<core::GlobalState>(errorQueue);
    gs->censorForSnapshotTests = true;
    auto workers = WorkerPool::create(0, gs->tracer());

    auto assertions = RangeAssertion::parseAssertions(test.sourceFileContents);
    if (BooleanPropertyAssertion::getValue("no-stdlib", assertions).value_or(false)) {
        gs->initEmpty();
    } else {
        core::serialize::Serializer::loadGlobalState(*gs, getNameTablePayload);
    }
    core::MutableContext ctx(*gs, core::Symbols::root());
    // Parser
    vector<core::FileRef> files;
    constexpr string_view whitelistedTypedNoneTest = "missing_typed_sigil.rb"sv;
    {
        core::UnfreezeFileTable fileTableAccess(*gs);

        for (auto &sourceFile : test.sourceFiles) {
            auto fref = gs->enterFile(test.sourceFileContents[test.folder + sourceFile]);
            if (FileOps::getFileName(sourceFile) == whitelistedTypedNoneTest) {
                fref.data(*gs).strictLevel = core::StrictLevel::False;
            }
            files.emplace_back(fref);
        }
    }
    vector<ast::ParsedFile> trees;
    ExpectationHandler handler(test, errorQueue);

    vector<core::ErrorRegion> errs;
    for (auto file : files) {
        errs.emplace_back(*gs, file);

        if (FileOps::getFileName(file.data(ctx).path()) != whitelistedTypedNoneTest &&
            file.data(ctx).source().find("# typed:") == string::npos) {
            ADD_FAILURE_AT(file.data(*gs).path().data(), 1) << "Add a `# typed: strict` line to the top of this file";
        }
        unique_ptr<parser::Node> nodes;
        {
            core::UnfreezeNameTable nameTableAccess(*gs); // enters original strings

            nodes = parser::Parser::run(*gs, file);
        }

        handler.drainErrors();
        handler.addObserved("parse-tree", [&]() { return nodes->toString(*gs); });
        handler.addObserved("parse-tree-whitequark", [&]() { return nodes->toWhitequark(*gs); });
        handler.addObserved("parse-tree-json", [&]() { return nodes->toJSON(*gs); });

        // Desugarer
        ast::ParsedFile desugared;
        {
            core::UnfreezeNameTable nameTableAccess(*gs); // enters original strings
            auto file = nodes->loc.file();
            desugared = testSerialize(*gs, ast::ParsedFile{ast::desugar::node2Tree(ctx, move(nodes)), file});
        }

        handler.addObserved("desugar-tree", [&]() { return desugared.tree->toString(*gs); });
        handler.addObserved("desugar-tree-raw", [&]() { return desugared.tree->showRaw(*gs); });

        ast::ParsedFile rewriten;
        ast::ParsedFile localNamed;

        if (!test.expectations.contains("autogen")) {
            // Rewriter
            {
                core::UnfreezeNameTable nameTableAccess(*gs); // enters original strings

                rewriten = testSerialize(
                    *gs, ast::ParsedFile{rewriter::Rewriter::run(ctx, move(desugared.tree)), desugared.file});
            }

            handler.addObserved("rewrite-tree", [&]() { return rewriten.tree->toString(*gs); });
            handler.addObserved("rewrite-tree-raw", [&]() { return rewriten.tree->showRaw(*gs); });

            localNamed = testSerialize(*gs, local_vars::LocalVars::run(ctx, move(rewriten)));
        } else {
            localNamed = testSerialize(*gs, local_vars::LocalVars::run(ctx, move(desugared)));
            if (test.expectations.contains("rewrite-tree-raw") || test.expectations.contains("rewrite-tree")) {
                ADD_FAILURE() << "Running Rewriter passes with autogen isn't supported";
            }
        }

        // Namer
        ast::ParsedFile namedTree;
        {
            core::UnfreezeNameTable nameTableAccess(*gs);     // creates singletons and class names
            core::UnfreezeSymbolTable symbolTableAccess(*gs); // enters symbols
            vector<ast::ParsedFile> vTmp;
            vTmp.emplace_back(move(localNamed));
            vTmp = namer::Namer::run(ctx, move(vTmp));
            namedTree = testSerialize(*gs, move(vTmp[0]));
        }

        handler.addObserved("name-tree", [&]() { return namedTree.tree->toString(*gs); });
        handler.addObserved("name-tree-raw", [&]() { return namedTree.tree->showRaw(*gs); });

        trees.emplace_back(move(namedTree));
    }

    if (test.expectations.contains("autogen")) {
        {
            core::UnfreezeNameTable nameTableAccess(*gs);
            core::UnfreezeSymbolTable symbolAccess(*gs);

            trees = resolver::Resolver::runConstantResolution(ctx, move(trees), *workers);
        }
        handler.addObserved(
            "autogen",
            [&]() {
                stringstream payload;
                for (auto &tree : trees) {
                    auto pf = autogen::Autogen::generate(ctx, move(tree));
                    tree = move(pf.tree);
                    payload << pf.toString(ctx);
                }
                return payload.str();
            },
            false);
    } else {
        core::UnfreezeNameTable nameTableAccess(*gs);     // Resolver::defineAttr
        core::UnfreezeSymbolTable symbolTableAccess(*gs); // enters stubs
        trees = move(resolver::Resolver::run(ctx, move(trees), *workers).result());
        handler.drainErrors();
    }

    handler.addObserved("symbol-table", [&]() { return gs->toString(); });
    handler.addObserved("symbol-table-raw", [&]() { return gs->showRaw(); });

    for (auto &resolvedTree : trees) {
        handler.addObserved("resolve-tree", [&]() { return resolvedTree.tree->toString(*gs); });
        handler.addObserved("resolve-tree-raw", [&]() { return resolvedTree.tree->showRaw(*gs); });
    }

    // Simulate what pipeline.cc does: We want to start typeckecking big files first because it helps with better work
    // distribution
    fast_sort(trees, [&](const auto &lhs, const auto &rhs) -> bool {
        return lhs.file.data(ctx).source().size() > rhs.file.data(ctx).source().size();
    });

    for (auto &resolvedTree : trees) {
        auto file = resolvedTree.file;

        resolvedTree = definition_validator::runOne(ctx, move(resolvedTree));
        handler.drainErrors();

        resolvedTree = flatten::runOne(ctx, move(resolvedTree));

        handler.addObserved("flatten-tree", [&]() { return resolvedTree.tree->toString(*gs); });
        handler.addObserved("flatten-tree-raw", [&]() { return resolvedTree.tree->showRaw(*gs); });

        auto checkTree = [&]() {
            if (resolvedTree.tree == nullptr) {
                auto path = file.data(ctx).path();
                ADD_FAILURE_AT(path.begin(), 1) << "Already used tree. You can only have 1 CFG-ish .exp file";
            }
        };
        auto checkPragma = [&](string ext) {
            if (file.data(ctx).strictLevel < core::StrictLevel::True) {
                auto path = file.data(ctx).path();
                ADD_FAILURE_AT(path.begin(), 1)
                    << "Missing `# typed:` pragma. Sources with ." << ext << ".exp files must specify # typed:";
            }
        };

        // CFG
        if (test.expectations.contains("cfg") || test.expectations.contains("cfg-raw") ||
            test.expectations.contains("cfg-json")) {
            checkTree();
            checkPragma("cfg");
            CFGCollectorAndTyper collector;
            auto cfg = ast::TreeMap::apply(ctx, collector, move(resolvedTree.tree));
            resolvedTree.tree.reset();

            handler.addObserved("cfg", [&]() {
                stringstream dot;
                dot << "digraph \"" << rbName << "\" {" << '\n';
                for (auto &cfg : collector.cfgs) {
                    dot << cfg->toString(ctx) << '\n' << '\n';
                }
                dot << "}" << '\n';
                return dot.str();
            });

            handler.addObserved("cfg-raw", [&]() {
                stringstream dot;
                dot << "digraph \"" << rbName << "\" {" << '\n';
                dot << "  graph [fontname = \"Courier\"];\n";
                dot << "  node [fontname = \"Courier\"];\n";
                dot << "  edge [fontname = \"Courier\"];\n";
                for (auto &cfg : collector.cfgs) {
                    dot << cfg->showRaw(ctx) << '\n' << '\n';
                }
                dot << "}" << '\n';
                return dot.str();
            });

            handler.addObserved(
                "cfg-json",
                [&]() {
                    stringstream payload;
                    for (auto &cfg : collector.cfgs) {
                        if (cfg->shouldExport(ctx)) {
                            auto proto = cfg::Proto::toProto(ctx.state, *cfg);
                            payload << core::Proto::toJSON(proto);
                        }
                    }
                    return payload.str();
                },
                false);
        }

        // If there is a tree left with a typed: pragma, run the inferencer
        if (resolvedTree.tree != nullptr && file.data(ctx).originalSigil >= core::StrictLevel::True) {
            checkTree();
            CFGCollectorAndTyper collector;
            ast::TreeMap::apply(ctx, collector, move(resolvedTree.tree));
            resolvedTree.tree.reset();
            handler.drainErrors();
        }
    }

    handler.checkExpectations();

    if (test.expectations.contains("symbol-table")) {
        string table = gs->toString() + '\n';
        EXPECT_EQ(handler.got["symbol-table"], table) << " symbol-table should not be mutated by CFG+inference";
    }

    if (test.expectations.contains("symbol-table-raw")) {
        string table = gs->showRaw() + '\n';
        EXPECT_EQ(handler.got["symbol-table-raw"], table) << " symbol-table-raw should not be mutated by CFG+inference";
    }

    // Check warnings and errors
    {
        map<string, vector<unique_ptr<Diagnostic>>> diagnostics;
        for (auto &error : handler.errors) {
            if (error->isSilenced) {
                continue;
            }
            auto diag = errorToDiagnostic(*gs, *error);
            if (diag == nullptr) {
                continue;
            }
            auto path = error->loc.file().data(*gs).path();
            diagnostics[string(path.begin(), path.end())].push_back(std::move(diag));
        }
        ErrorAssertion::checkAll(test.sourceFileContents, RangeAssertion::getErrorAssertions(assertions), diagnostics);
    }

    // Allow later phases to have errors that we didn't test for
    errorQueue->drainAllErrors();

    // now we test the incremental resolver

    auto disableStressIncremental =
        BooleanPropertyAssertion::getValue("disable-stress-incremental", assertions).value_or(false);
    auto isAutogenTest = test.expectations.contains("autogen");
    if (disableStressIncremental || isAutogenTest) {
        TEST_COUT << "errors OK" << '\n';
        return;
    }

    handler.clear();
    auto symbolsBefore = gs->symbolsUsed();

    vector<ast::ParsedFile> newTrees;
    for (auto &f : trees) {
        const int prohibitedLines = f.file.data(*gs).source().size();
        auto newSource = absl::StrCat(string(prohibitedLines + 1, '\n'), f.file.data(*gs).source());
        auto newFile =
            make_shared<core::File>((string)f.file.data(*gs).path(), move(newSource), f.file.data(*gs).sourceType);
        gs = core::GlobalState::replaceFile(move(gs), f.file, move(newFile));

        // this replicates the logic of pipeline::indexOne
        auto nodes = parser::Parser::run(*gs, f.file);
        handler.addObserved("parse-tree", [&]() { return nodes->toString(*gs); });
        handler.addObserved("parse-tree-json", [&]() { return nodes->toJSON(*gs); });

        core::MutableContext ctx(*gs, core::Symbols::root());
        ast::ParsedFile file = testSerialize(*gs, ast::ParsedFile{ast::desugar::node2Tree(ctx, move(nodes)), f.file});
        handler.addObserved("desguar-tree", [&]() { return file.tree->toString(*gs); });
        handler.addObserved("desugar-tree-raw", [&]() { return file.tree->showRaw(*gs); });

        // Rewriter pass
        file = testSerialize(*gs, ast::ParsedFile{rewriter::Rewriter::run(ctx, move(file.tree)), file.file});
        handler.addObserved("rewrite-tree", [&]() { return file.tree->toString(*gs); });
        handler.addObserved("rewrite-tree-raw", [&]() { return file.tree->showRaw(*gs); });

        // local vars
        file = testSerialize(*gs, local_vars::LocalVars::run(ctx, move(file)));

        // namer
        {
            core::UnfreezeSymbolTable symbolTableAccess(*gs);
            vector<ast::ParsedFile> vTmp;
            vTmp.emplace_back(move(file));
            vTmp = namer::Namer::run(ctx, move(vTmp));
            file = testSerialize(*gs, move(vTmp[0]));
        }

        handler.addObserved("name-tree", [&]() { return file.tree->toString(*gs); });
        handler.addObserved("name-tree-raw", [&]() { return file.tree->showRaw(*gs); });
        newTrees.emplace_back(move(file));
    }

    // resolver
    trees = resolver::Resolver::runTreePasses(ctx, move(newTrees));

    for (auto &resolvedTree : trees) {
        handler.addObserved("resolve-tree", [&]() { return resolvedTree.tree->toString(*gs); });
        handler.addObserved("resolve-tree-raw", [&]() { return resolvedTree.tree->showRaw(*gs); });
    }

    handler.checkExpectations("[stress-incremental] ");

    // and drain all the remaining errors
    errorQueue->drainAllErrors();

    EXPECT_EQ(symbolsBefore, gs->symbolsUsed()) << "the incremental resolver should not add new symbols";

    TEST_COUT << "errors OK" << '\n';
} // namespace sorbet::test

TEST_P(WhitequarkParserTest, PerPhaseTest) { // NOLINT
    vector<unique_ptr<core::Error>> errors;
    Expectations test = GetParam();
    auto inputPath = test.folder + test.basename;
    auto rbName = test.basename + ".rb";
    SCOPED_TRACE(inputPath);

    for (auto &exp : test.expectations) {
        auto it = knownExpectations.find(exp.first);
        if (it == knownExpectations.end()) {
            ADD_FAILURE() << "Unknown pass: " << exp.first;
        }
    }

    auto logger = spd::stderr_color_mt("fixtures: " + inputPath);
    auto errorQueue = make_shared<core::ErrorQueue>(*logger, *logger);
    core::GlobalState gs(errorQueue);
    gs.censorForSnapshotTests = true;
    auto workers = WorkerPool::create(0, gs.tracer());

    auto assertions = RangeAssertion::parseAssertions(test.sourceFileContents);
    if (BooleanPropertyAssertion::getValue("no-stdlib", assertions).value_or(false)) {
        gs.initEmpty();
    } else {
        core::serialize::Serializer::loadGlobalState(gs, getNameTablePayload);
    }
    core::MutableContext ctx(gs, core::Symbols::root());
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

    vector<core::ErrorRegion> errs;
    for (auto file : files) {
        errs.emplace_back(gs, file);

        if (FileOps::getFileName(file.data(ctx).path()) != whitelistedTypedNoneTest &&
            file.data(ctx).source().find("# typed:") == string::npos) {
            ADD_FAILURE_AT(file.data(gs).path().data(), 1) << "Add a `# typed: strict` line to the top of this file";
        }
        unique_ptr<parser::Node> nodes;
        {
            core::UnfreezeNameTable nameTableAccess(gs); // enters original strings

            // whitequark/parser declares these 3 meta variables to
            // simplify testing cases around local variables
            vector<string> initialLocals = {"foo", "bar", "baz"};
            nodes = parser::Parser::run(gs, file, initialLocals);
        }
        {
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }

        auto expectation = test.expectations.find("parse-tree-whitequark");
        if (expectation != test.expectations.end()) {
            got["parse-tree-whitequark"].append(nodes->toWhitequark(gs)).append("\n");
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }
    }

    for (auto &gotPhase : got) {
        auto expectation = test.expectations.find(gotPhase.first);
        ASSERT_TRUE(expectation != test.expectations.end()) << "missing expectation for " << gotPhase.first;
        ASSERT_TRUE(expectation->second.size() == 1)
            << "found unexpected multiple expectations of type " << gotPhase.first;

        auto checker = test.folder + expectation->second.begin()->second;
        auto expect = FileOps::read(checker.c_str());
        EXPECT_EQ(expect, gotPhase.second) << "Mismatch on: " << checker;
        if (expect == gotPhase.second) {
            TEST_COUT << gotPhase.first << " OK" << '\n';
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

    // Allow later phases to have errors that we didn't test for
    errorQueue->drainAllErrors();

    TEST_COUT << "errors OK" << '\n';
} // namespace sorbet::test

bool isTestMessage(const LSPMessage &msg) {
    return msg.isNotification() && msg.method() == LSPMethod::SorbetTypecheckRunInfo;
}

// "Newly pushed diagnostics always replace previously pushed diagnostics. There is no merging that happens
// on the client side." Only keep the newest diagnostics for a file.
void updateDiagnostics(const LSPConfiguration &config, UnorderedMap<string, string> &testFileUris,
                       vector<unique_ptr<LSPMessage>> &responses,
                       map<string, vector<unique_ptr<Diagnostic>>> &diagnostics) {
    for (auto &response : responses) {
        if (isTestMessage(*response)) {
            continue;
        }
        ASSERT_NO_FATAL_FAILURE(assertNotificationMessage(LSPMethod::TextDocumentPublishDiagnostics, *response));
        auto maybeDiagnosticParams = getPublishDiagnosticParams(response->asNotification());
        ASSERT_TRUE(maybeDiagnosticParams.has_value());
        auto &diagnosticParams = *maybeDiagnosticParams;
        auto filename = uriToFilePath(config, diagnosticParams->uri);
        EXPECT_NE(testFileUris.end(), testFileUris.find(filename))
            << fmt::format("Diagnostic URI is not a test file URI: {}", diagnosticParams->uri);

        // Will explicitly overwrite older diagnostics that are irrelevant.
        diagnostics[filename] = move(diagnosticParams->diagnostics);
    }
}

int countNonTestMessages(const vector<unique_ptr<LSPMessage>> &msgs) {
    int count = 0;
    for (auto &m : msgs) {
        if (!isTestMessage(*m)) {
            count++;
        }
    }
    return count;
}

string documentSymbolsToString(const variant<JSONNullObject, vector<unique_ptr<DocumentSymbol>>> &symbolResult) {
    if (get_if<JSONNullObject>(&symbolResult)) {
        return "null";
    } else {
        auto &symbols = get<vector<unique_ptr<DocumentSymbol>>>(symbolResult);
        return fmt::format("{}", fmt::map_join(symbols.begin(), symbols.end(), ", ",
                                               [](const auto &sym) -> string { return sym->toJSON(true); }));
    }
}

void testQuickFixCodeActions(LSPWrapper &lspWrapper, Expectations &test, UnorderedSet<string> &filenames,
                             vector<shared_ptr<RangeAssertion>> &assertions, UnorderedMap<string, string> &testFileUris,
                             int &nextId) {
    UnorderedMap<string, vector<shared_ptr<ApplyCodeActionAssertion>>> applyCodeActionAssertionsByFilename;
    for (auto &assertion : assertions) {
        if (auto applyCodeActionAssertion = dynamic_pointer_cast<ApplyCodeActionAssertion>(assertion)) {
            applyCodeActionAssertionsByFilename[applyCodeActionAssertion->filename].push_back(applyCodeActionAssertion);
        }
    }

    bool exhaustiveApplyCodeAction =
        BooleanPropertyAssertion::getValue("exhaustive-apply-code-action", assertions).value_or(false);

    if (applyCodeActionAssertionsByFilename.empty() && !exhaustiveApplyCodeAction) {
        return;
    }

    auto errors = RangeAssertion::getErrorAssertions(assertions);
    UnorderedMap<string, std::vector<std::shared_ptr<RangeAssertion>>> errorsByFilename;
    for (auto &error : errors) {
        errorsByFilename[error->filename].emplace_back(error);
    }

    for (auto &filename : filenames) {
        auto applyCodeActionAssertions = applyCodeActionAssertionsByFilename[filename];

        // Request code actions for each of this file's error.
        for (auto &error : errorsByFilename[filename]) {
            vector<unique_ptr<Diagnostic>> diagnostics;
            auto fileUri = testFileUris[filename];
            // Unfortunately there's no simpler way to copy the range (yet).
            auto params =
                make_unique<CodeActionParams>(make_unique<TextDocumentIdentifier>(fileUri), error->range->copy(),
                                              make_unique<CodeActionContext>(move(diagnostics)));
            auto req = make_unique<RequestMessage>("2.0", nextId++, LSPMethod::TextDocumentCodeAction, move(params));
            auto responses = lspWrapper.getLSPResponsesFor(make_unique<LSPMessage>(move(req)));
            EXPECT_EQ(responses.size(), 1) << "Did not receive exactly one response for a codeAction request.";
            if (responses.size() != 1) {
                continue;
            }

            auto &msg = responses.at(0);
            EXPECT_TRUE(msg->isResponse());
            if (!msg->isResponse()) {
                continue;
            }

            auto &response = msg->asResponse();
            ASSERT_TRUE(response.result) << "Code action request returned error: " << msg->toJSON();
            auto &receivedCodeActionResponse =
                get<variant<JSONNullObject, vector<unique_ptr<CodeAction>>>>(*response.result);
            EXPECT_FALSE(get_if<JSONNullObject>(&receivedCodeActionResponse));
            if (get_if<JSONNullObject>(&receivedCodeActionResponse)) {
                continue;
            }

            UnorderedMap<string, unique_ptr<CodeAction>> receivedCodeActionsByTitle;
            auto &receivedCodeActions = get<vector<unique_ptr<CodeAction>>>(receivedCodeActionResponse);
            for (auto &codeAction : receivedCodeActions) {
                bool codeActionTitleUnique =
                    receivedCodeActionsByTitle.find(codeAction->title) == receivedCodeActionsByTitle.end();
                EXPECT_TRUE(codeActionTitleUnique) << "Found code action with duplicate title: " << codeAction->title;

                if (codeActionTitleUnique) {
                    receivedCodeActionsByTitle[codeAction->title] = move(codeAction);
                }
            }

            u4 receivedCodeActionsCount = receivedCodeActionsByTitle.size();
            vector<shared_ptr<ApplyCodeActionAssertion>> matchedCodeActionAssertions;

            // Test code action assertions matching the range of this error.
            auto it = applyCodeActionAssertions.begin();
            while (it != applyCodeActionAssertions.end()) {
                auto codeActionAssertion = it->get();
                if (!(error->range->start->cmp(*codeActionAssertion->range->start) <= 0 &&
                      error->range->end->cmp(*codeActionAssertion->range->end) >= 0)) {
                    ++it;
                    continue;
                }

                // Ensure we received a code action matching the assertion.
                auto it2 = receivedCodeActionsByTitle.find(codeActionAssertion->title);
                EXPECT_NE(it2, receivedCodeActionsByTitle.end())
                    << fmt::format("Did not receive code action matching assertion `{}` for error `{}`...",
                                   codeActionAssertion->toString(), error->toString());

                // Ensure that the received code action applies correctly.
                if (it2 != receivedCodeActionsByTitle.end()) {
                    auto codeAction = move(it2->second);
                    codeActionAssertion->check(test.sourceFileContents, lspWrapper, *codeAction.get());

                    // Some bookkeeping to make surfacing errors re. extra/insufficient
                    // apply-code-action annotations easier.
                    receivedCodeActionsByTitle.erase(it2);
                    matchedCodeActionAssertions.emplace_back(*it);
                    it = applyCodeActionAssertions.erase(it);
                } else {
                    ++it;
                }
            }

            if (exhaustiveApplyCodeAction) {
                if (matchedCodeActionAssertions.size() > receivedCodeActionsCount) {
                    ADD_FAILURE() << fmt::format(
                        "Found apply-code-action assertions without "
                        "corresponding code actions from the server:\n{}",
                        fmt::map_join(applyCodeActionAssertions.begin(), applyCodeActionAssertions.end(), ", ",
                                      [](const auto &assertion) -> string { return assertion->toString(); }));
                } else if (matchedCodeActionAssertions.size() < receivedCodeActionsCount) {
                    ADD_FAILURE() << fmt::format(
                        "Received code actions without corresponding apply-code-action assertions:\n{}",
                        fmt::map_join(receivedCodeActionsByTitle.begin(), receivedCodeActionsByTitle.end(), "\n",
                                      [](const auto &action) -> string { return action.second->toJSON(); }));
                }
            }
        }

        // We've already removed any code action assertions that matches a received code action assertion.
        // Any remaining are therefore extraneous.
        EXPECT_EQ(applyCodeActionAssertions.size(), 0)
            << fmt::format("Found extraneous apply-code-action assertions:\n{}",
                           fmt::map_join(applyCodeActionAssertions.begin(), applyCodeActionAssertions.end(), "\n",
                                         [](const auto &assertion) -> string { return assertion->toString(); }));
    }
}

void testDocumentSymbols(LSPWrapper &lspWrapper, Expectations &test, int &nextId, string_view uri,
                         string_view testFile) {
    auto expectationFileName = test.expectations["document-symbols"][testFile];
    if (expectationFileName.empty()) {
        // No .exp file found; nothing to do.
        return;
    }

    auto params = make_unique<DocumentSymbolParams>(make_unique<TextDocumentIdentifier>(string(uri)));
    auto req = make_unique<RequestMessage>("2.0", nextId++, LSPMethod::TextDocumentDocumentSymbol, move(params));
    auto responses = lspWrapper.getLSPResponsesFor(make_unique<LSPMessage>(move(req)));
    ASSERT_EQ(responses.size(), 1) << "Did not receive exactly one response for a documentSymbols request.";
    auto &msg = responses.at(0);
    ASSERT_TRUE(msg->isResponse());
    auto &response = msg->asResponse();
    ASSERT_TRUE(response.result) << "Document symbols request returned error: " << msg->toJSON();
    auto &receivedSymbolResponse = get<variant<JSONNullObject, vector<unique_ptr<DocumentSymbol>>>>(*response.result);

    auto expectedSymbolsPath = test.folder + expectationFileName;
    auto expected = LSPMessage::fromClient(FileOps::read(expectedSymbolsPath.c_str()));
    auto &expectedResp = expected->asResponse();
    auto &expectedSymbolResponse =
        get<variant<JSONNullObject, vector<unique_ptr<DocumentSymbol>>>>(*expectedResp.result);

    // Simple string comparison, just like other *.exp files.
    EXPECT_EQ(documentSymbolsToString(expectedSymbolResponse), documentSymbolsToString(receivedSymbolResponse))
        << "Mismatch on: " << expectedSymbolsPath;
}

TEST_P(LSPTest, All) {
    const auto &config = lspWrapper->config();

    // Perform initialize / initialized handshake.
    {
        string rootPath = fmt::format("/Users/{}/stripe/sorbet", std::getenv("USER"));
        string rootUri = fmt::format("file://{}", rootPath);
        auto sorbetInitOptions = make_unique<SorbetInitializationOptions>();
        sorbetInitOptions->enableTypecheckInfo = true;
        auto initializedResponses =
            initializeLSP(rootPath, rootUri, *lspWrapper, nextId, true, move(sorbetInitOptions));
        EXPECT_EQ(0, countNonTestMessages(initializedResponses))
            << "Should not receive any response to 'initialized' message.";
    }

    // filename => URI; do post-initialization so LSPConfiguration has rootUri set.
    UnorderedMap<string, string> testFileUris;
    for (auto &filename : filenames) {
        testFileUris[filename] = filePathToUri(config, filename);
    }

    // Tell LSP that we opened a bunch of brand new, empty files (the test files).
    {
        for (auto &filename : filenames) {
            auto params = make_unique<DidOpenTextDocumentParams>(
                make_unique<TextDocumentItem>(testFileUris[filename], "ruby", 1, ""));
            auto responses = lspWrapper->getLSPResponsesFor(make_unique<LSPMessage>(
                make_unique<NotificationMessage>("2.0", LSPMethod::TextDocumentDidOpen, move(params))));
            EXPECT_EQ(0, countNonTestMessages(responses))
                << "Should not receive any response to opening an empty file.";
        }
    }

    // filename => diagnostics for file (persist for fast path tests)
    map<string, vector<unique_ptr<Diagnostic>>> diagnostics;

    // Tell LSP that the new files now have the contents from the test files on disk.
    {
        bool slowPathPassed = true;
        bool skipFastPath = BooleanPropertyAssertion::getValue("disable-fast-path", assertions).value_or(false);
        vector<string> errorPrefixes = {"", "[After running fast path] "};
        // Run changes through LSP twice: The first time is a slow path, the second time is a fast path.
        // Surfaces errors that occur due to differences in how slow and fast paths run.
        // Skip the second iteration if slow path fails to avoid printing out duplicate errors.
        for (int i = 0; i < (skipFastPath ? 1 : 2) && slowPathPassed; i++) {
            vector<unique_ptr<LSPMessage>> updates;
            for (auto &filename : filenames) {
                auto textDocContents = test.sourceFileContents[filename]->source();
                updates.push_back(
                    makeChange(testFileUris[filename], string(textDocContents.begin(), textDocContents.end()), 2 + i));
            }
            auto responses = lspWrapper->getLSPResponsesFor(move(updates));
            updateDiagnostics(config, testFileUris, responses, diagnostics);
            slowPathPassed = ErrorAssertion::checkAll(
                test.sourceFileContents, RangeAssertion::getErrorAssertions(assertions), diagnostics, errorPrefixes[i]);
            if (i == 2) {
                ADD_FAILURE() << "Note: To disable fast path tests, add `# disable-fast-path: true` to the file.";
            }
        }
    }

    for (auto &filename : filenames) {
        testDocumentSymbols(*lspWrapper, test, nextId, testFileUris[filename], filename);
    }
    testQuickFixCodeActions(*lspWrapper, test, filenames, assertions, testFileUris, nextId);

    // Usage and def assertions
    {
        // Sort by symbol.
        // symbol => [ (version => DefAssertion), (DefAssertion | UsageAssertion)[] ]
        // Note: Using a vector in pair since order matters; assertions are ordered by location, which
        // is used when comparing against LSP responses.
        UnorderedMap<string, pair<UnorderedMap<int, shared_ptr<DefAssertion>>, vector<shared_ptr<RangeAssertion>>>>
            defUsageMap;

        // symbol => [ TypeDefAssertion[], TypeAssertion[] ]
        //
        // (The first element of the pair is only ever TypeDefAssertions but we only ever care that they're
        // RangeAssertions, so rather than fiddle with up casting, we'll just make the whole vector RangeAssertions)
        UnorderedMap<string, pair<vector<shared_ptr<RangeAssertion>>, vector<shared_ptr<TypeAssertion>>>> typeDefMap;
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
            } else if (auto typeDefAssertion = dynamic_pointer_cast<TypeDefAssertion>(assertion)) {
                auto &[typeDefs, _typeAssertions] = typeDefMap[typeDefAssertion->symbol];
                typeDefs.push_back(typeDefAssertion);
            } else if (auto typeAssertion = dynamic_pointer_cast<TypeAssertion>(assertion)) {
                auto &[_typeDefs, typeAssertions] = typeDefMap[typeAssertion->symbol];
                typeAssertions.push_back(typeAssertion);
            }
        }

        // Check each def/usage assertion.
        for (auto &entry : defUsageMap) {
            auto &entryAssertions = entry.second.second;
            // Sort assertions in (filename, range) order
            fast_sort(entryAssertions,
                      [](const shared_ptr<RangeAssertion> &a, const shared_ptr<RangeAssertion> &b) -> bool {
                          return a->cmp(*b) < 0;
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
                    auto queryLoc = assertion->getLocation(config);
                    // Check that a definition request at this location returns def.
                    def->check(test.sourceFileContents, *lspWrapper, nextId, *queryLoc);
                    // Check that a reference request at this location returns entryAssertions.
                    UsageAssertion::check(test.sourceFileContents, *lspWrapper, nextId, symbol, *queryLoc,
                                          entryAssertions);
                    // Check that a highlight request at this location returns all of the entryAssertions for the same
                    // file as the request.
                    vector<shared_ptr<RangeAssertion>> filteredEntryAssertions;
                    for (auto &e : entryAssertions) {
                        if (absl::StartsWith(e->getLocation(config)->uri, queryLoc->uri)) {
                            filteredEntryAssertions.push_back(e);
                        }
                    }
                    UsageAssertion::checkHighlights(test.sourceFileContents, *lspWrapper, nextId, symbol, *queryLoc,
                                                    filteredEntryAssertions);
                } else {
                    ADD_FAILURE() << fmt::format(
                        "Found usage comment for label {0} version {1} without matching def comment. Please add a `# "
                        "^^ def: {0} {1}` assertion that points to the definition of the pointed-to thing being used.",
                        symbol, version);
                }
            }
        }

        // Check each type-def/type assertion.
        for (auto &[symbol, typeDefAndAssertions] : typeDefMap) {
            auto &[typeDefs, typeAssertions] = typeDefAndAssertions;
            for (auto &typeAssertion : typeAssertions) {
                auto queryLoc = typeAssertion->getLocation(config);
                // Check that a type definition request at this location returns type-def.
                TypeDefAssertion::check(test.sourceFileContents, *lspWrapper, nextId, symbol, *queryLoc, typeDefs);
            }
        }
    }

    // Hover assertions
    HoverAssertion::checkAll(assertions, test.sourceFileContents, *lspWrapper, nextId);

    // Completion assertions
    CompletionAssertion::checkAll(assertions, test.sourceFileContents, *lspWrapper, nextId);
    ApplyCompletionAssertion::checkAll(assertions, test.sourceFileContents, *lspWrapper, nextId);

    // Workspace Symbol assertions
    SymbolSearchAssertion::checkAll(assertions, test.sourceFileContents, *lspWrapper, nextId);

    // Fast path tests: Asserts that certain changes take the fast/slow path, and produce any expected diagnostics.
    {
        // sourceLSPFileUpdates is unordered (and we can't use an ordered map unless we make its contents `const`)
        // Sort by version.
        vector<int> sortedUpdates;
        const int baseVersion = 4;
        for (auto &update : test.sourceLSPFileUpdates) {
            sortedUpdates.push_back(update.first);
        }
        fast_sort(sortedUpdates);

        // Apply updates in order.
        for (auto version : sortedUpdates) {
            auto errorPrefix = fmt::format("[*.{}.rbupdate] ", version);
            auto &updates = test.sourceLSPFileUpdates[version];
            vector<unique_ptr<LSPMessage>> lspUpdates;
            UnorderedMap<std::string, std::shared_ptr<core::File>> updatesAndContents;

            for (auto &update : updates) {
                auto originalFile = test.folder + update.first;
                auto updateFile = test.folder + update.second;
                auto fileContents = FileOps::read(updateFile);
                lspUpdates.push_back(makeChange(testFileUris[originalFile], fileContents, baseVersion + version));
                updatesAndContents[originalFile] =
                    make_shared<core::File>(string(originalFile), move(fileContents), core::File::Type::Normal);
            }
            auto assertions = RangeAssertion::parseAssertions(updatesAndContents);
            auto assertFastPath = FastPathAssertion::get(assertions);
            auto assertSlowPath = BooleanPropertyAssertion::getValue("assert-slow-path", assertions);
            auto responses = lspWrapper->getLSPResponsesFor(move(lspUpdates));
            bool foundTypecheckRunInfo = false;

            for (auto &r : responses) {
                if (r->isNotification()) {
                    if (r->method() == LSPMethod::SorbetTypecheckRunInfo) {
                        foundTypecheckRunInfo = true;
                        auto &params = get<unique_ptr<SorbetTypecheckRunInfo>>(r->asNotification().params);
                        if (assertSlowPath.value_or(false)) {
                            EXPECT_EQ(params->tookFastPath, false)
                                << errorPrefix << "Expected Sorbet to take slow path, but it took the fast path.";
                        }
                        if (assertFastPath.has_value()) {
                            (*assertFastPath)->check(*params, test.folder, version, errorPrefix);
                        }
                    } else if (r->method() != LSPMethod::TextDocumentPublishDiagnostics) {
                        ADD_FAILURE() << errorPrefix
                                      << fmt::format("Unexpected message response to file update of type {}:\n{}",
                                                     convertLSPMethodToString(r->method()), r->toJSON());
                    }
                } else {
                    ADD_FAILURE() << errorPrefix
                                  << fmt::format("Unexpected message response to file update:\n{}", r->toJSON());
                }
            }

            if (!foundTypecheckRunInfo) {
                ADD_FAILURE() << errorPrefix << "Sorbet did not send expected typechecking metadata.";
            }

            updateDiagnostics(config, testFileUris, responses, diagnostics);

            for (auto &update : updates) {
                auto originalFile = test.folder + update.first;
                auto updateFile = test.folder + update.second;
                testDocumentSymbols(*lspWrapper, test, nextId, testFileUris[originalFile], updateFile);
            }

            const bool passed = ErrorAssertion::checkAll(
                updatesAndContents, RangeAssertion::getErrorAssertions(assertions), diagnostics, errorPrefix);

            if (!passed) {
                // Abort if an update fails its assertions, as subsequent updates will likely fail as well.
                break;
            }

            // Check any new HoverAssertions in the updates.
            HoverAssertion::checkAll(assertions, updatesAndContents, *lspWrapper, nextId);
        }
    }
}
// namespace sorbet::test

INSTANTIATE_TEST_SUITE_P(PosTests, ExpectationTest, ::testing::ValuesIn(getInputs(singleTest)), prettyPrintTest);
INSTANTIATE_TEST_SUITE_P(LSPTests, LSPTest, ::testing::ValuesIn(getInputs(singleTest)), prettyPrintTest);
INSTANTIATE_TEST_SUITE_P(WhitequarkParserTests, WhitequarkParserTest, ::testing::ValuesIn(getInputs(singleTest)),
                         prettyPrintTest);

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

    // If the base names match, make files with the ".rb" extension come before all others.
    // The remaining files will be sorted by reverse order on extension.
    auto lext = FileOps::getExtension(left);
    auto rext = FileOps::getExtension(right);
    if (lext != rext) {
        if (lext == "rb") {
            return true;
        } else if (rext == "rb") {
            return false;
        } else {
            return rext < lext;
        }
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

    vector<string> names = sorbet::FileOps::listFilesInDir(name, {".rb", ".rbi", ".rbupdate", ".exp"}, false, {}, {});
    const int prefixLen = strnlen(name, 1024) + 1;
    // Trim off the input directory from the name.
    transform(names.begin(), names.end(), names.begin(),
              [&prefixLen](auto &name) -> string { return name.substr(prefixLen); });
    fast_sort(names, compareNames);

    Expectations current;
    for (auto &s : names) {
        if (absl::EndsWith(s, ".rb") || absl::EndsWith(s, ".rbi")) {
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
                string source_file_path = string(name) + "/" + s.substr(0, kind_start);
                current.expectations[kind][source_file_path] = s;
            }
        } else if (absl::EndsWith(s, ".rbupdate")) {
            if (absl::StartsWith(s, current.basename)) {
                // Should be `.[number].rbupdate`
                auto pos = s.rfind('.', s.length() - 10);
                if (pos != string::npos) {
                    int version = stoi(s.substr(pos + 1, s.length() - 9));
                    current.sourceLSPFileUpdates[version].emplace_back(absl::StrCat(s.substr(0, pos), ".rb"), s);
                } else {
                    cout << "Ignoring " << s << ": No version number provided (expected .[number].rbupdate).\n";
                }
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
