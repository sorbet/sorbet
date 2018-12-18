#include "gtest/gtest.h"
#include <cxxopts.hpp>
// has to go first as it violates are requirements

#include "absl/strings/match.h"
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
#include "test/expectations.h"
#include "test/lsp_test_helpers.h"
#include "test/position_assertions.h"
#include <algorithm>
#include <cstdio>
#include <dirent.h>
#include <fstream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <vector>

namespace spd = spdlog;
using namespace std;

string singleTest;

vector<Expectations> getInputs(string singleTest);

string prettyPrintTest(testing::TestParamInfo<Expectations> arg) {
    string res = arg.param.folder + arg.param.basename;
    auto ext = sorbet::FileOps::getExtension(res);
    if (ext == "rb") {
        res.erase(res.end() - ext.size() - 1, res.end());
    }
    absl::c_replace(res, '/', '_');
    return res;
}

class ExpectationTest : public testing::TestWithParam<Expectations> {
public:
    ~ExpectationTest() override = default;
    void SetUp() override {
        exp_ = GetParam();
    }
    void TearDown() override {}

protected:
    Expectations exp_;
};

// taken from https://stackoverflow.com/questions/16491675/how-to-send-custom-message-in-google-c-testing-framework
namespace testing::internal {
enum GTestColor { COLOR_DEFAULT, COLOR_RED, COLOR_GREEN, COLOR_YELLOW };

extern void ColoredPrintf(GTestColor color, const char *fmt, ...);
} // namespace testing::internal

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

class CFG_Collector_and_Typer {
    bool raw = false;
    bool typedSource = false;

public:
    CFG_Collector_and_Typer(bool raw = false, bool typedSource = false) : raw(raw), typedSource(typedSource){};
    vector<string> cfgs;
    unique_ptr<sorbet::ast::MethodDef> preTransformMethodDef(sorbet::core::Context ctx,
                                                             unique_ptr<sorbet::ast::MethodDef> m) {
        if (m->symbol.data(ctx)->isOverloaded()) {
            return m;
        }
        auto cfg = sorbet::cfg::CFGBuilder::buildFor(ctx.withOwner(m->symbol), *m);
        if (raw || typedSource) {
            cfg = sorbet::cfg::CFGBuilder::addDebugEnvironment(ctx.withOwner(m->symbol), move(cfg));
        }
        cfg = sorbet::infer::Inference::run(ctx.withOwner(m->symbol), move(cfg));
        if (typedSource) {
            cfg->recordAnnotations(ctx);
        }

        cfgs.emplace_back(cfg->toString(ctx));
        return m;
    }
};

class ErrorAndPos {
public:
    string error;
    int beginPos = -1;
    int endPos = -1;
};

sorbet::UnorderedSet<string> knownPasses = {
    "parse-tree",   "parse-tree-json", "ast",           "ast-raw",      "dsl-tree",         "dsl-tree-raw",
    "symbol-table", "name-tree",       "name-tree-raw", "resolve-tree", "resolve-tree-raw", "cfg",
    "cfg-raw",      "typed-source",    "autogen"};

unique_ptr<sorbet::ast::Expression> testSerialize(sorbet::core::GlobalState &gs,
                                                  unique_ptr<sorbet::ast::Expression> expr) {
    auto saved = sorbet::core::serialize::Serializer::storeExpression(gs, expr);
    auto restored = sorbet::core::serialize::Serializer::loadExpression(gs, saved.data());
    return restored;
}

TEST_P(ExpectationTest, PerPhaseTest) { // NOLINT
    vector<unique_ptr<sorbet::core::Error>> errors;
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
    auto errorQueue = make_shared<sorbet::core::ErrorQueue>(*logger, *logger);
    sorbet::core::GlobalState gs(errorQueue);
    sorbet::core::serialize::Serializer::loadGlobalState(gs, getNameTablePayload);
    sorbet::core::MutableContext ctx(gs, sorbet::core::Symbols::root());

    // Parser
    vector<sorbet::core::FileRef> files;
    {
        sorbet::core::UnfreezeFileTable fileTableAccess(gs);

        for (auto &srcPath : test.sourceFiles) {
            auto path = test.folder + srcPath;
            auto src = sorbet::FileOps::read(path.c_str());
            files.emplace_back(gs.enterFile(path, src));
        }
    }
    vector<unique_ptr<sorbet::ast::Expression>> trees;
    map<string, string> got;

    vector<sorbet::core::ErrorRegion> errs;
    for (auto file : files) {
        errs.emplace_back(gs, file);
        if (file.data(ctx).source().find("# typed:") == string::npos) {
            ADD_FAILURE_AT(file.data(gs).path().data(), 1) << "Add a `# typed: strict` line to the top of this file";
        }

        unique_ptr<sorbet::parser::Node> nodes;
        {
            sorbet::core::UnfreezeNameTable nameTableAccess(gs); // enters original strings

            nodes = sorbet::parser::Parser::run(gs, file);
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
        unique_ptr<sorbet::ast::Expression> desugared;
        {
            sorbet::core::UnfreezeNameTable nameTableAccess(gs); // enters original strings

            desugared = testSerialize(gs, sorbet::ast::desugar::node2Tree(ctx, move(nodes)));
        }

        expectation = test.expectations.find("ast");
        if (expectation != test.expectations.end()) {
            got["ast"].append(desugared->toString(gs)).append("\n");
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }

        expectation = test.expectations.find("ast-raw");
        if (expectation != test.expectations.end()) {
            got["ast-raw"].append(desugared->showRaw(gs)).append("\n");
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }
        unique_ptr<sorbet::ast::Expression> dslUnwound;

        if (test.expectations.find("autogen") == test.expectations.end()) {
            // DSL
            {
                sorbet::core::UnfreezeNameTable nameTableAccess(gs); // enters original strings

                dslUnwound = testSerialize(gs, sorbet::dsl::DSL::run(ctx, move(desugared)));
            }

            expectation = test.expectations.find("dsl-tree");
            if (expectation != test.expectations.end()) {
                got["dsl-tree"].append(dslUnwound->toString(gs)).append("\n");
                auto newErrors = errorQueue->drainAllErrors();
                errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
            }

            expectation = test.expectations.find("dsl-tree-raw");
            if (expectation != test.expectations.end()) {
                got["dsl-tree-raw"].append(dslUnwound->showRaw(gs)).append("\n");
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
        unique_ptr<sorbet::ast::Expression> namedTree;
        {
            sorbet::core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
            sorbet::core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
            namedTree = testSerialize(gs, sorbet::namer::Namer::run(ctx, move(dslUnwound)));
        }

        expectation = test.expectations.find("name-tree");
        if (expectation != test.expectations.end()) {
            got["name-tree"].append(namedTree->toString(gs)).append("\n");
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }

        expectation = test.expectations.find("name-tree-raw");
        if (expectation != test.expectations.end()) {
            got["name-tree-raw"].append(namedTree->showRaw(gs));
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }

        trees.emplace_back(move(namedTree));
    }

    auto expectation = test.expectations.find("autogen");
    if (expectation != test.expectations.end()) {
        {
            sorbet::core::UnfreezeNameTable nameTableAccess(gs);
            sorbet::core::UnfreezeSymbolTable symbolAccess(gs);

            trees = sorbet::resolver::Resolver::runConstantResolution(ctx, move(trees));
        }

        for (auto &tree : trees) {
            auto pf = sorbet::autogen::Autogen::generate(ctx, move(tree));
            tree = move(pf.tree);
            got["autogen"].append(pf.toString(ctx));
        }

        auto newErrors = errorQueue->drainAllErrors();
        errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
    } else {
        sorbet::core::UnfreezeNameTable nameTableAccess(gs);     // Resolver::defineAttr
        sorbet::core::UnfreezeSymbolTable symbolTableAccess(gs); // enters stubs
        trees = sorbet::resolver::Resolver::run(ctx, move(trees));
        auto newErrors = errorQueue->drainAllErrors();
        errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
    }

    expectation = test.expectations.find("symbol-table");
    if (expectation != test.expectations.end()) {
        got["symbol-table"] = gs.toString() + '\n';
        auto newErrors = errorQueue->drainAllErrors();
        errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
    }

    for (auto &resolvedTree : trees) {
        expectation = test.expectations.find("resolve-tree");
        if (expectation != test.expectations.end()) {
            got["resolve-tree"].append(resolvedTree->toString(gs)).append("\n");
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }

        expectation = test.expectations.find("resolve-tree-raw");
        if (expectation != test.expectations.end()) {
            got["resolve-tree-raw"].append(resolvedTree->showRaw(gs)).append("\n");
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }
    }

    for (auto &resolvedTree : trees) {
        auto file = resolvedTree->loc.file();
        auto checkTree = [&]() {
            if (resolvedTree == nullptr) {
                auto path = file.data(ctx).path();
                ADD_FAILURE_AT(path.begin(), 1) << "Already used tree. You can only have 1 CFG-ish .exp file";
            }
        };
        auto checkPragma = [&](string ext) {
            if (file.data(ctx).sigil == sorbet::core::StrictLevel::Stripe) {
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
            CFG_Collector_and_Typer collector;
            auto cfg = sorbet::ast::TreeMap::apply(ctx, collector, move(resolvedTree));
            resolvedTree.reset();

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

        expectation = test.expectations.find("cfg-raw");
        if (expectation != test.expectations.end()) {
            checkTree();
            checkPragma("cfg-raw");
            CFG_Collector_and_Typer collector(true);
            auto cfg = sorbet::ast::TreeMap::apply(ctx, collector, move(resolvedTree));
            resolvedTree.reset();

            stringstream dot;
            dot << "digraph \"" << rbName << "\" {" << '\n';
            for (auto &cfg : collector.cfgs) {
                dot << cfg << '\n' << '\n';
            }
            dot << "}" << '\n' << '\n';
            got["cfg-raw"].append(dot.str());

            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }

        expectation = test.expectations.find("typed-source");
        if (expectation != test.expectations.end()) {
            checkTree();
            checkPragma("typed-source");
            CFG_Collector_and_Typer collector(false, true);
            sorbet::ast::TreeMap::apply(ctx, collector, move(resolvedTree));
            resolvedTree.reset();

            got["typed-source"].append(gs.showAnnotatedSource(file));

            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }

        // If there is a tree left with a typed: pragma, run the inferencer
        if (resolvedTree != nullptr && file.data(ctx).sigil != sorbet::core::StrictLevel::Stripe) {
            checkTree();
            CFG_Collector_and_Typer collector;
            sorbet::ast::TreeMap::apply(ctx, collector, move(resolvedTree));
            resolvedTree.reset();
            auto newErrors = errorQueue->drainAllErrors();
            errors.insert(errors.end(), make_move_iterator(newErrors.begin()), make_move_iterator(newErrors.end()));
        }
    }

    for (auto &gotPhase : got) {
        auto expectation = test.expectations.find(gotPhase.first);
        ASSERT_TRUE(expectation != test.expectations.end()) << "missing expectation for " << gotPhase.first;
        auto checker = test.folder + expectation->second;
        auto expect = sorbet::FileOps::read(checker.c_str());
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

    // Check warnings and errors

    map<pair<sorbet::core::FileRef, int>, ErrorAndPos> expectedErrors;
    regex errorRegex("# error: ?(.*)");           // something like 'foo   # error: Badness'
    regex errorPosRegex("([ ]*#[ ]+)(\\^+)[ ]*"); // someting like '   #    ^^^^^  '
    regex commendOut("^[ ]*#");

    for (auto file : files) {
        string src(file.data(gs).source().begin(), file.data(gs).source().end());
        stringstream ss(src);
        string line;
        int linenum = 1;

        while (getline(ss, line, '\n')) {
            smatch matches;
            if (regex_search(line, matches, errorRegex)) {
                if (regex_search(line, commendOut)) {
                    continue;
                }
                string match = matches[1].str();
                int len = match.size();
                if (len < 10 && match.find("MULTI") == string::npos) {
                    ADD_FAILURE_AT(file.data(gs).path().data(), linenum)
                        << "Too short of a error message at " << len
                        << " characters. Use MULTI or write something longer than: " << match;
                }
                expectedErrors[make_pair(file, linenum)].error = match;
            } else if (regex_match(line, matches, errorPosRegex)) {
                auto expectedError = expectedErrors.find(make_pair(file, linenum - 1));
                if (expectedError == expectedErrors.end()) {
                    ADD_FAILURE_AT(file.data(gs).path().data(), linenum)
                        << "Position comment must come right after a line with a `error:` comment. Found position "
                           "comment on line "
                        << linenum << " matching " << matches[0].str();
                }
                expectedError->second.beginPos = matches[1].str().size() + 1; // We start our columns at 1
                expectedError->second.endPos = expectedError->second.beginPos + matches[2].str().size();
            }
            linenum += 1;
        }
    }

    map<pair<sorbet::core::FileRef, int>, int> seenErrorLines;
    int unknownLocErrorLine = 1;
    for (auto &error : errors) {
        if (error->isSilenced) {
            continue;
        }
        auto filePath = error->loc.file().data(gs).path();
        if (!error->loc.exists()) {
            // The convention is to put `error: Unknown Location Error` at
            // the top of the file for each of these so that they are eaten
            // first when reporting mismatched errors.
            int line = unknownLocErrorLine++;
            auto expectedError = expectedErrors.find(make_pair(files.front(), line));
            if (expectedError == expectedErrors.end()) {
                ADD_FAILURE_AT(filePath.data(), line) << "Unknown location error thrown but not annotated." << '\n'
                                                      << "Reported error: " << error->header;
            } else if (error->header.find(expectedError->second.error) == string::npos) {
                ADD_FAILURE_AT(filePath.data(), line) << "Error string mismatch." << '\n'
                                                      << " Expectation: " << expectedError->second.error << '\n'
                                                      << " Reported error: " << error->header;
            } else {
                seenErrorLines[make_pair(files.front(), line)]++;
            }
            continue;
        }

        auto pos = error->loc.position(gs);
        bool found = false;
        for (int i = pos.first.line; i <= pos.second.line; i++) {
            auto expectedErrorIt = expectedErrors.find(make_pair(error->loc.file(), i));
            if (expectedErrorIt != expectedErrors.end()) {
                string expectedError = expectedErrorIt->second.error;
                found = true;
                seenErrorLines[make_pair(error->loc.file(), i)]++;
                if (expectedError.empty()) {
                    ADD_FAILURE_AT(filePath.data(), i) << "Error occurred, but no expected text found. Please put (a "
                                                          "substring of) the expected error after `# error:` "
                                                       << '\n'
                                                       << "The message was: '" << error->header << "'";
                } else if (expectedError.find("MULTI") != string::npos) { // multiple errors. Ignore message and pos
                    continue;
                } else if (error->header.find(expectedError) == string::npos) {
                    ADD_FAILURE_AT(filePath.data(), i) << "Error string mismatch." << '\n'
                                                       << " Expectation: " << expectedError << '\n'
                                                       << " Reported error: " << error->header;
                } else if (expectedErrorIt->second.beginPos != -1 &&
                           pos.first.column != expectedErrorIt->second.beginPos) {
                    ADD_FAILURE_AT(filePath.data(), i)
                        << "Wrong starting error position. Expected error starting at position "
                        << expectedErrorIt->second.beginPos << " but found one starting at position "
                        << pos.first.column;
                } else if (expectedErrorIt->second.endPos != -1 &&
                           pos.second.column != expectedErrorIt->second.endPos) {
                    ADD_FAILURE_AT(filePath.data(), i)
                        << "Wrong ending error position. Expected error ending at position "
                        << expectedErrorIt->second.endPos << " but found one ending at position " << pos.second.column;
                }
            }
        }
        if (!found) {
            ADD_FAILURE_AT(filePath.data(), pos.first.line) << "Unexpected error:\n " << error->toString(gs);
        }
    }

    for (auto &error : expectedErrors) {
        auto filePath = error.first.first.data(gs).path();
        auto found = seenErrorLines.find(error.first);
        if (found == seenErrorLines.end()) {
            ADD_FAILURE_AT(filePath.data(), error.first.second) << "Expected error did not happen.";
        } else {
            int count = found->second;
            if (error.second.error.find("MULTI") != string::npos) {
                if (count == 1) {
                    ADD_FAILURE_AT(filePath.data(), error.first.second)
                        << "Expected multiple errors, but only saw one.";
                }
            }
            // TODO: Fail on duplicate errors. We currently tolerate them.
            /* else if (count > 1) {
                ADD_FAILURE_AT(filePath.data(), error.first.second)
                    << "Expected error once, but found error multiple times.";
            }*/
        }
    }

    // Allow later phases to have errors that we didn't test for
    errorQueue->drainAllErrors();

    TEST_COUT << "errors OK" << '\n';
}

/** Returns true if a and b are different Diagnostic objects but have the same range and message. */
bool isDuplicateDiagnostic(const Diagnostic *a, const Diagnostic *b) {
    return a != b && rangeComparison(a->range, b->range) == 0 && a->message == b->message;
}

/** Given a root URI and a URI, returns the file path relative to the repository root. */
string filenameFromUri(const std::string &rootUri, const std::string &uri) {
    if (uri.substr(0, rootUri.length()) != rootUri) {
        ADD_FAILURE() << fmt::format(
            "Unrecognized URI: `{}` is not contained in root URI `{}`, and thus does not correspond to a test file.",
            uri, rootUri);
        return "";
    }

    // Remove rootUri + '/'
    return uri.substr(rootUri.length() + 1);
}

TEST_P(LSPTest, PositionTests) {
    string rootPath = "/Users/jvilk/stripe/pay-server";
    string rootUri = fmt::format("file://{}", rootPath);

    // filename => URI
    sorbet::UnorderedMap<string, string> testFileUris;
    for (auto &filename : filenames) {
        testFileUris[filename] = fmt::format("{}/{}", rootUri, filename);
    }

    // Fake root document; used to allocate JSONAny values.
    auto fakeRoot = make_unique<int>(0);
    // Used to allocate JSONValues.
    auto rapidjsonDoc = make_unique<rapidjson::Document>();
    auto doc = make_unique<JSONDocument<int>>(rapidjsonDoc, fakeRoot);
    int nextId = 0;

    // Send 'initialize' message.
    {
        unique_ptr<JSONBaseType> initializeParams = makeInitializeParams(rootPath, rootUri);
        auto responses = sendRequest(makeRequestMessage(doc, "initialize", nextId++, initializeParams));

        // Should just have an 'initialize' response.
        ASSERT_EQ(1, responses.size());
        auto lspResponse = move(responses.at(0));

        auto maybeDoc = assertResponseMessage(0, lspResponse);
        if (maybeDoc.has_value()) {
            auto &doc = *maybeDoc;
            auto &respMsg = doc->root;
            ASSERT_FALSE(respMsg->error.has_value());
            ASSERT_TRUE(respMsg->result.has_value());

            auto &result = *respMsg->result;
            // TODO(jvilk): Need a better way to unwrap these.
            auto initializeResult = InitializeResult::fromJSONValue(doc->memoryOwner->GetAllocator(), *result.get(),
                                                                    "ResponseMessage.result");
            checkServerCapabilities(initializeResult->capabilities);
        }
    }

    // Complete initialization handshake with an 'initialized' message.
    {
        rapidjson::Value emptyObject(rapidjson::kObjectType);
        auto initialized = make_unique<NotificationMessage>();
        initialized->jsonrpc = "2.0";
        initialized->method = "initialized";
        initialized->params = make_unique<rapidjson::Value>(rapidjson::kObjectType);
        auto initializedResponses = sendNotification(initialized);
        EXPECT_EQ(0, initializedResponses.size()) << "Should not receive any response to 'initialized' message.";
    }

    // Tell LSP that we opened a bunch of brand new, empty files (the test files).
    {
        for (auto &filename : filenames) {
            auto didOpenTextDocParams = make_unique<DidOpenTextDocumentParams>();

            auto textDocument = make_unique<TextDocumentItem>();
            textDocument->languageId = "ruby";
            textDocument->version = 1;
            textDocument->uri = testFileUris[filename];
            textDocument->text = "";
            didOpenTextDocParams->textDocument = move(textDocument);

            unique_ptr<JSONBaseType> cast = move(didOpenTextDocParams);
            auto responses = sendRequest(makeRequestMessage(doc, "textDocument/didOpen", nextId++, cast));
            EXPECT_EQ(0, responses.size()) << "Should not receive any response to opening an empty file.";
        }
    }

    // Tell LSP that the new files now have the contents from the test files on disk.
    {
        vector<unique_ptr<JSONDocument<JSONBaseType>>> allResponses;
        for (auto &filename : filenames) {
            auto didChangeParams = make_unique<DidChangeTextDocumentParams>();

            auto textDocId = make_unique<VersionedTextDocumentIdentifier>();
            textDocId->uri = testFileUris[filename];
            textDocId->version = 2;
            didChangeParams->textDocument = std::move(textDocId);

            auto textDocChange = make_unique<TextDocumentContentChangeEvent>();
            textDocChange->text = fileContents[filename];
            didChangeParams->contentChanges.push_back(std::move(textDocChange));

            auto didChangeNotif = make_unique<NotificationMessage>();
            didChangeNotif->jsonrpc = "2.0";
            didChangeNotif->method = "textDocument/didChange";
            didChangeNotif->params = didChangeParams->toJSONValue(doc);

            auto responses = sendNotification(didChangeNotif);
            allResponses.insert(allResponses.end(), make_move_iterator(responses.begin()),
                                make_move_iterator(responses.end()));
        }

        // "Newly pushed diagnostics always replace previously pushed diagnostics. There is no merging that happens on
        // the client side."
        // Prune irrelevant diagnostics, and only keep the newest diagnostics for a file.
        vector<unique_ptr<PublishDiagnosticsParams>> allDiagnosticsParams;
        {
            // filename => latest diagnostics for that file
            sorbet::UnorderedMap<string, unique_ptr<PublishDiagnosticsParams>> latestDiagnosticParams;
            for (auto &response : allResponses) {
                auto maybeDoc = assertNotificationMessage("textDocument/publishDiagnostics", response);
                if (!maybeDoc.has_value()) {
                    continue;
                }
                auto &doc = *maybeDoc;
                auto maybeDiagnosticParams = getPublishDiagnosticParams(doc);
                ASSERT_TRUE(maybeDiagnosticParams.has_value());
                auto &diagnosticParams = *maybeDiagnosticParams;
                auto filename = filenameFromUri(rootUri, diagnosticParams->uri);
                EXPECT_NE(testFileUris.end(), testFileUris.find(filename))
                    << fmt::format("Diagnostic URI is not a test file URI: {}", diagnosticParams->uri);

                // Will explicitly overwrite older diagnostics that are irrelevant.
                latestDiagnosticParams[filename] = move(diagnosticParams);
            }

            // Push publishDiagnostics information into allDiagnosticsParams, and sort each's diagnostics vector.
            for (auto &latestParam : latestDiagnosticParams) {
                auto &filename = latestParam.first;
                // Sort diagnostics in range, message order
                fast_sort(latestParam.second->diagnostics,
                          [&filename](const unique_ptr<Diagnostic> &a, const unique_ptr<Diagnostic> &b) -> bool {
                              return errorComparison(filename, a->range, a->message, filename, b->range, b->message) ==
                                     -1;
                          });
                allDiagnosticsParams.push_back(move(latestParam.second));
            }
        }

        // Sort diagnostic messages in filename order. Now, iterating through these and their
        // diagnostics should match the assertion sort order.
        fast_sort(allDiagnosticsParams,
                  [](const unique_ptr<PublishDiagnosticsParams> &a,
                     const unique_ptr<PublishDiagnosticsParams> &b) -> bool { return a->uri.compare(b->uri) == -1; });

        auto errorAssertions = getErrorAssertions();
        auto assertionsIt = errorAssertions.begin();

        for (auto &diagnosticParams : allDiagnosticsParams) {
            auto &diagnostics = diagnosticParams->diagnostics;
            auto diagnosticsIt = diagnostics.begin();
            auto *lastDiagnostic = diagnosticsIt == diagnostics.end() ? nullptr : (*diagnosticsIt).get();

            // For asserting that MULTI assertions happen multiple times.
            int multiCount = 0;

            while (diagnosticsIt != diagnostics.end() && assertionsIt != errorAssertions.end()) {
                // See if the ranges match.
                auto &diagnostic = *diagnosticsIt;
                auto &assertion = *assertionsIt;

                // TODO: LSP tests currently remove duplicate diagnostics for parity
                // with regular runner. Remove this check when ruby types team fixes
                // duplicate diagnostics.
                if (isDuplicateDiagnostic(lastDiagnostic, diagnostic.get())) {
                    diagnosticsIt++;
                    continue;
                }
                lastDiagnostic = diagnostic.get();

                switch (assertion->compare(filenameFromUri(rootUri, diagnosticParams->uri), diagnostic->range)) {
                    case 1: {
                        // Diagnostic comes *before* this assertion, so we don't
                        // have an assertion that matches the diagnostic.
                        string filename = filenameFromUri(rootUri, diagnosticParams->uri);
                        reportUnexpectedLSPError(filename, diagnostic,
                                                 getSourceLine(filename, diagnostic->range->start->line));
                        // We've 'consumed' the diagnostic -- nothing matches it.
                        diagnosticsIt++;
                        break;
                    }
                    case -1: {
                        // Diagnostic comes *after* this assertion

                        if (assertion->message == "MULTI") {
                            // Assertion is a MULTI assertion; since it comes before the diagnostic,
                            // we're done with this MULTI assertion.
                            assertionsIt++;
                            if (multiCount < 2) {
                                ADD_FAILURE_AT(assertion->filename.c_str(), assertion->range->start->line)
                                    << "MULTI assertion did not happen multiple times.";
                            }
                            multiCount = 0;
                            // Re-run loop on diagnostic with next assertion.
                            break;
                        }

                        // We don't have a diagnostic that matches the assertion.
                        reportMissingError(assertion->filename, assertion,
                                           getSourceLine(assertion->filename, assertion->range->start->line));
                        // We've 'consumed' this error assertion -- nothing matches it.
                        assertionsIt++;
                        break;
                    }
                    default: {
                        // Ranges match, so check the assertion.
                        assertion->check(diagnostic, getSourceLine(assertion->filename, assertion->range->start->line));
                        // We've 'consumed' the diagnostic.
                        diagnosticsIt++;
                        // Keep MULTI assertions around for another loop, but non-MULTI are done.
                        // TODO(jvilk): Remove MULTI assertions in favor of multiple different assertions.
                        if (assertion->message != "MULTI") {
                            assertionsIt++;
                        } else {
                            multiCount++;
                        }
                        break;
                    }
                }
            }

            while (diagnosticsIt != diagnostics.end()) {
                // We had more diagnostics than error assertions.
                // Drain dupes.
                // TODO: Remove when ruby types team fixes duplicate diagnostics; see note above.
                if (!isDuplicateDiagnostic(lastDiagnostic, (*diagnosticsIt).get())) {
                    auto &diagnostic = *diagnosticsIt;
                    string filename = filenameFromUri(rootUri, diagnosticParams->uri);
                    reportUnexpectedLSPError(filename, diagnostic,
                                             getSourceLine(filename, diagnostic->range->start->line));
                }
                lastDiagnostic = (*diagnosticsIt).get();
                diagnosticsIt++;
            }

            // We've finished processing diagnostics for a file. If assertionsIt still points to a
            // MULTI assertion, process it.
            // TODO: Remove when MULTI assertions are deprecated.
            if (assertionsIt != errorAssertions.end() && (*assertionsIt)->message == "MULTI") {
                if (multiCount < 2) {
                    ADD_FAILURE_AT((*assertionsIt)->filename.c_str(), (*assertionsIt)->range->start->line)
                        << "MULTI assertion did not happen multiple times.";
                }
                assertionsIt++;
            }
        }

        while (assertionsIt != errorAssertions.end()) {
            // Had more error assertions than diagnostics
            reportMissingError((*assertionsIt)->filename, *assertionsIt,
                               getSourceLine((*assertionsIt)->filename, (*assertionsIt)->range->start->line));
            assertionsIt++;
        }
    }

    // TODO(jvilk): Request/response assertions (like Find Def/Usages/Autocomplete)
}

INSTANTIATE_TEST_CASE_P(PosTests, ExpectationTest, testing::ValuesIn(getInputs(singleTest)), prettyPrintTest);
INSTANTIATE_TEST_CASE_P(PositionTests, LSPTest, testing::ValuesIn(getInputs(singleTest)), prettyPrintTest);

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
    auto lext = sorbet::FileOps::getExtension(left);
    auto rext = sorbet::FileOps::getExtension(right);
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
// substrantially modified from https://stackoverflow.com/a/8438663
vector<Expectations> listDir(const char *name) {
    vector<Expectations> result;
    DIR *dir;
    struct dirent *entry;
    vector<string> names;

    if ((dir = opendir(name)) == nullptr) {
        return result;
    }

    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            continue;
        } else {
            names.emplace_back(entry->d_name);
        }
    }
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
        } else {
        }
    }
    if (!current.basename.empty()) {
        result.emplace_back(current);
        current = Expectations();
    }

    closedir(dir);
    return result;
}

vector<Expectations> getInputs(string singleTest) {
    vector<Expectations> result;
    if (singleTest.empty()) {
        sorbet::Exception::raise("No test specified. Pass one with --single_test=<test_path>");
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
            result.emplace_back(f);
        }
    }

    if (result.empty()) {
        sorbet::Exception::raise("None tests found!");
    }
    return result;
}

int main(int argc, char *argv[]) {
    cxxopts::Options options("test_corpus", "Test corpus for Ruby Typer");
    options.add_options()("single_test", "run over single test.", cxxopts::value<string>()->default_value(""),
                          "testpath");
    auto res = options.parse(argc, argv);
    singleTest = res["single_test"].as<string>();

    ::testing::InitGoogleTest(&argc, (char **)argv);
    return RUN_ALL_TESTS();
}
