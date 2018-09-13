#include "absl/algorithm/container.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/treemap/treemap.h"
#include "cfg/CFG.h"
#include "cfg/builder/builder.h"
#include "common/common.h"
#include "core/Errors.h"
#include "core/Unfreeze.h"
#include "core/serialize/serialize.h"
#include "dsl/dsl.h"
#include "infer/infer.h"
#include "main/autogen/autogen.h"
#include "main/errorqueue/ConcurrentErrorQueue.h"
#include "namer/namer.h"
#include "parser/parser.h"
#include "payload/binary/binary.h"
#include "resolver/resolver.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <cstdio>
#include <cxxopts.hpp>
#include <dirent.h>
#include <fstream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unordered_set>
#include <vector>
namespace spd = spdlog;
using namespace std;

struct Expectations {
    string folder;
    string basename;
    string testName;
    vector<string> sourceFiles;
    sorbet::UnorderedMap<string, string> expectations;
};

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
namespace testing {
namespace internal {
enum GTestColor { COLOR_DEFAULT, COLOR_RED, COLOR_GREEN, COLOR_YELLOW };

extern void ColoredPrintf(GTestColor color, const char *fmt, ...);
} // namespace internal
} // namespace testing

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
        if (m->symbol.data(ctx).isOverloaded()) {
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

        cfgs.push_back(cfg->toString(ctx));
        return m;
    }
};

class ErrorAndPos {
public:
    string error;
    int beginPos = -1;
    int endPos = -1;
};

unordered_set<string> knownPasses = {
    "parse-tree", "parse-tree-json", "ast",           "ast-raw",      "dsl-tree",         "dsl-tree-raw",
    "name-table", "name-tree",       "name-tree-raw", "resolve-tree", "resolve-tree-raw", "cfg",
    "cfg-raw",    "typed-source",    "autogen"};

unique_ptr<sorbet::ast::Expression> testSerialize(sorbet::core::GlobalState &gs,
                                                  unique_ptr<sorbet::ast::Expression> expr) {
    auto saved = sorbet::core::serialize::Serializer::storeExpression(gs, expr);
    auto restored = sorbet::core::serialize::Serializer::loadExpression(gs, saved.data());
    return restored;
}

TEST_P(ExpectationTest, PerPhaseTest) { // NOLINT
    vector<unique_ptr<sorbet::core::BasicError>> errors;
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
    auto errorQueue = make_shared<sorbet::realmain::ConcurrentErrorQueue>(*logger, *logger);
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
            files.push_back(gs.enterFile(path, src));
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

        // DSL
        unique_ptr<sorbet::ast::Expression> dslUnwound;
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
    }

    expectation = test.expectations.find("name-table");
    if (expectation != test.expectations.end()) {
        got["name-table"] = gs.toString() + "\n";
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
            got["resolve-tree-raw"].append(resolvedTree->showRaw(gs));
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

    expectation = test.expectations.find("name-table");
    if (expectation != test.expectations.end()) {
        string table = gs.toString() + "\n";
        EXPECT_EQ(got["name-table"], table) << " name-table should not be mutated by CFG+inference";
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
                                                      << "Reported error: " << error->formatted;
            } else if (error->formatted.find(expectedError->second.error) == string::npos) {
                ADD_FAILURE_AT(filePath.data(), line) << "Error string mismatch." << '\n'
                                                      << " Expectation: " << expectedError->second.error << '\n'
                                                      << " Reported error: " << error->formatted;
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
                                                       << "The message was: '" << error->formatted << "'";
                } else if (expectedError.find("MULTI") != string::npos) { // multiple errors. Ignore message and pos
                    continue;
                } else if (error->formatted.find(expectedError) == string::npos) {
                    ADD_FAILURE_AT(filePath.data(), i) << "Error string mismatch." << '\n'
                                                       << " Expectation: " << expectedError << '\n'
                                                       << " Reported error: " << error->formatted;
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
        if (seenErrorLines.find(error.first) == seenErrorLines.end()) {
            ADD_FAILURE_AT(filePath.data(), error.first.second) << "Expected error did not happen.";
        }
        if (error.second.error.find("MULTI") != string::npos && seenErrorLines[error.first] == 1) {
            ADD_FAILURE_AT(filePath.data(), error.first.second) << "Expected multiple errors, but only saw one.";
        }
    }

    // Allow later phases to have errors that we didn't test for
    errorQueue->drainAllErrors();

    TEST_COUT << "errors OK" << '\n';
}

INSTANTIATE_TEST_CASE_P(PosTests, ExpectationTest, testing::ValuesIn(getInputs(singleTest)), prettyPrintTest);

bool endsWith(const string &a, const string &b) {
    if (b.size() > a.size()) {
        return false;
    }
    return equal(a.begin() + a.size() - b.size(), a.end(), b.begin());
}

static bool startsWith(const string &str, const string &prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix.c_str(), prefix.size());
}

bool compareNames(const string &left, const string &right) {
    auto lsplit = left.find("__");
    if (lsplit == string::npos) {
        lsplit = left.find(".");
    }
    auto rsplit = right.find("__");
    if (rsplit == string::npos) {
        rsplit = right.find(".");
    }
    absl::string_view lbase(left.data(), lsplit == string::npos ? left.size() : lsplit);
    absl::string_view rbase(right.data(), rsplit == string::npos ? right.size() : rsplit);
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
    absl::c_sort(names, compareNames);

    Expectations current;
    for (auto &s : names) {
        if (endsWith(s, ".rb")) {
            auto basename = rbFile2BaseTestName(s);
            if (basename != s) {
                if (basename == current.basename) {
                    current.sourceFiles.push_back(s);
                    continue;
                }
            }

            if (!current.basename.empty()) {
                result.push_back(current);
                current = Expectations();
            }
            current.basename = basename;
            current.sourceFiles.push_back(s);
            current.folder = name;
            current.folder += "/";
            current.testName = current.folder + current.basename;
        } else if (endsWith(s, ".exp")) {
            if (startsWith(s, current.basename)) {
                auto kind_start = s.rfind(".", s.size() - strlen(".exp") - 1);
                string kind = s.substr(kind_start + 1, s.size() - kind_start - strlen(".exp") - 1);
                current.expectations[kind] = s;
            }
        } else {
        }
    }
    if (!current.basename.empty()) {
        result.push_back(current);
        current = Expectations();
    }

    closedir(dir);
    return result;
}

vector<Expectations> getInputs(string singleTest) {
    vector<Expectations> result;
    if (singleTest.empty()) {
        sorbet::Error::raise("No test specified. Pass one with --single_test=<test_path>");
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
            result.push_back(f);
        }
    }

    if (result.empty()) {
        sorbet::Error::raise("None tests found!");
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
